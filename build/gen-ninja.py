#!/usr/bin/env python3
"""Generate build/xtf.ninja from the metadata manifest emitted by Make.

The top-level Makefile first reduces the current build configuration to a small
tab-separated manifest.  This script turns that manifest into the concrete
Ninja file used to build the selected tests and install their outputs.

At a high level, the script follows the same structure as the recursive make
path: read the build configuration, resolve the per-environment and per-test
data, then emit the concrete commands needed for objects, linker scripts,
binaries, cfg files, metadata files, and install targets.
"""

import os
import sys
from dataclasses import dataclass


@dataclass
class EnvInfo:
    """Describe one concrete build environment loaded from the manifest.

    Each instance contains the already-expanded flag strings and file paths
    needed to emit build statements for one environment such as pv64 or hvm32pae.
    """

    guest: str
    arch: str
    aflags_arch: str
    cflags_arch: str
    aflags_env: str
    cflags_env: str
    link: str
    ldflags: str
    defcfg: str


@dataclass
class TestInfo:
    """Hold all manifest data needed to emit one test directory.

    The generator uses this record to create the per-test outputs that would
    otherwise be expanded by the recursive make path: binaries, cfg files,
    variation cfg files, metadata, and install copies.
    """

    key: str
    directory: str
    name: str
    category: str
    envs: list[str]
    extra_cfg: str
    vary_cfg: list[str]
    vcpus: str
    local_objs: list[str]


def split_words(value: str) -> list[str]:
    """Split one manifest field into whitespace-separated words.

    The manifest stores make list values in plain text fields, so this helper
    recreates the corresponding Python list representation.
    """

    return [word for word in value.split() if word]


def parse_manifest(path: str) -> tuple[
    dict[str, str],
    dict[str, EnvInfo],
    dict[str, list[str]],
    list[TestInfo],
    list[str],
    list[str],
]:
    """Parse the Make-generated manifest into typed Python structures.

    The manifest is line-oriented and tab-separated.  Each record starts with a
    kind tag such as global, env, or test followed by the fields for that
    record type.

    The parsing work is deliberately kept simple and explicit because the input
    format is produced by the Makefile rather than by a schema-aware tool.  The
    result mirrors the manifest's own structure:

    * globals_map holds scalar tool and path settings.
    * envs holds one EnvInfo per named runtime environment.
    * env_objects holds per-environment object lists.
    * tests holds one TestInfo per selected test directory.
    * objects_perbits and objects_perenv keep the shared object lists.

    Keeping those groups separate makes the later emission code easier to read.
    The generator can iterate over the same conceptual pieces that exist in the
    build system instead of repeatedly unpacking raw tab-separated strings.
    """

    globals_map: dict[str, str] = {}
    envs: dict[str, EnvInfo] = {}
    env_objects: dict[str, list[str]] = {}
    tests: list[TestInfo] = []
    objects_perbits: list[str] = []
    objects_perenv: list[str] = []

    with open(path, encoding="utf-8") as handle:
        for raw_line in handle:
            line = raw_line.rstrip("\n")
            if not line:
                continue
            fields = line.split("\t")
            kind = fields[0]

            if kind == "global":
                _, key, value = fields
                globals_map[key] = value
            elif kind == "objects":
                _, scope, value = fields
                if scope == "perbits":
                    objects_perbits = split_words(value)
                elif scope == "perenv":
                    objects_perenv = split_words(value)
            elif kind == "env":
                (
                    _,
                    name,
                    guest,
                    arch,
                    aflags_arch,
                    cflags_arch,
                    aflags_env,
                    cflags_env,
                    link,
                    ldflags,
                    defcfg,
                ) = fields
                envs[name] = EnvInfo(
                    guest=guest,
                    arch=arch,
                    aflags_arch=aflags_arch,
                    cflags_arch=cflags_arch,
                    aflags_env=aflags_env,
                    cflags_env=cflags_env,
                    link=link,
                    ldflags=ldflags,
                    defcfg=defcfg,
                )
            elif kind == "env_objects":
                _, name, value = fields
                env_objects[name] = split_words(value)
            elif kind == "test":
                (
                    _,
                    key,
                    directory,
                    name,
                    category,
                    env_list,
                    extra_cfg,
                    vary_cfg,
                    vcpus,
                    local_objs,
                ) = fields
                tests.append(
                    TestInfo(
                        key=key,
                        directory=directory,
                        name=name,
                        category=category,
                        envs=split_words(env_list),
                        extra_cfg=extra_cfg,
                        vary_cfg=split_words(vary_cfg),
                        vcpus=vcpus,
                        local_objs=split_words(local_objs),
                    )
                )
            else:
                raise ValueError(f"Unexpected manifest line: {line}")

    return globals_map, envs, env_objects, tests, objects_perbits, objects_perenv


def to_rel(root: str, path: str) -> str:
    """Return a path in the form expected by the generated Ninja file.

    The manifest can contain either absolute or already-relative paths.  Ninja
    files are generated relative to the repository root, so absolute paths are
    normalised back to that form here.
    """

    if not path:
        return ""
    if os.path.isabs(path):
        return os.path.relpath(path, root)
    return path


def source_for_obj(root: str, obj: str) -> tuple[str, str]:
    """Resolve one object name to its source path and compile mode.

    XTF object lists name the target object, not the original source.  This
    helper probes for .c and .S siblings and returns both the chosen
    source file and the Ninja rule name that should compile it.
    """

    rel_obj = to_rel(root, obj)
    c_src = rel_obj[:-2] + ".c"
    s_src = rel_obj[:-2] + ".S"

    if os.path.exists(os.path.join(root, c_src)):
        return c_src, "cc"
    if os.path.exists(os.path.join(root, s_src)):
        return s_src, "as"

    raise FileNotFoundError(f"No source found for object {obj}")


def depfile_for(output: str) -> str:
    """Map a generated output file to its dependency file name."""

    if output.endswith(".lds"):
        return output[:-4] + ".d"
    if output.endswith(".o"):
        return output[:-2] + ".d"
    raise ValueError(f"No depfile mapping for {output}")


def emit_rule(
    lines: list[str],
    name: str,
    command: str,
    *,
    depfile: str | None = None,
    deps: str | None = None,
) -> None:
    """Append a reusable Ninja command definition.

    In Ninja terminology, a rule names the command template, while the real
    per-file work is described later by build lines that reference that rule.
    """

    lines.append(f"rule {name}")
    lines.append(f"  command = {command}")
    if depfile is not None:
        lines.append(f"  depfile = {depfile}")
    if deps is not None:
        lines.append(f"  deps = {deps}")
    lines.append("")


def emit_phony(lines: list[str], output: str, inputs: list[str]) -> None:
    """Append a phony target.

    This is the Ninja equivalent of a grouping target with no recipe of its own
    whose purpose is to collect other concrete outputs behind a stable name.
    """

    emit_build(lines, output, "phony", inputs)


def emit_build(
    lines: list[str],
    output: str,
    rule: str,
    inputs: list[str],
    variables: dict[str, str] | None = None,
    implicit_inputs: list[str] | None = None,
) -> None:
    """Append one concrete Ninja build statement.

    For maintainers used to GNU make, this is the closest equivalent to writing
    out one fully expanded target rule after all variables have been resolved.
    implicit_inputs are emitted after | so Ninja tracks them as
    dependencies without adding them to the command line itself.
    """

    line = f"build {output}: {rule}"
    if inputs:
        line += " " + " ".join(inputs)
    if implicit_inputs:
        line += " | " + " ".join(implicit_inputs)
    lines.append(line)
    if variables:
        for key, value in variables.items():
            lines.append(f"  {key} = {value}")
    lines.append("")


def python_modules(root: str, package: str) -> list[str]:
    """Return Python source files under package, excluding cache directories."""

    modules: list[str] = []
    package_root = os.path.join(root, package)

    for dirpath, dirnames, filenames in os.walk(package_root):
        dirnames[:] = [name for name in dirnames if name != "__pycache__"]

        for filename in filenames:
            if not filename.endswith(".py"):
                continue
            modules.append(os.path.relpath(os.path.join(dirpath, filename), root))

    return sorted(modules)


def build_ninja(
    root: str,
    globals_map: dict[str, str],
    envs: dict[str, EnvInfo],
    env_objects: dict[str, list[str]],
    tests: list[TestInfo],
    objects_perbits: list[str],
    objects_perenv: list[str],
) -> list[str]:
    """Construct the full Ninja file as a list of text lines.

    The implementation works in two stages.  First it emits the small set of
    reusable command definitions shared by the whole file.  Then it walks every
    selected test and every enabled environment for that test, emitting the
    concrete statements needed to build the required outputs.

    Ninja often calls those concrete statements edges in the dependency
    graph.  In GNU make terms, you can read them as explicit instantiated rules
    connecting a specific output file to the exact inputs and command variables
    needed to rebuild it.

    The returned list is written directly to build/xtf.ninja by main().
    """

    cc = globals_map["CC"]
    cpp = globals_map["CPP"]
    ld = globals_map["LD"]
    objcopy = globals_map["OBJCOPY"]
    python = globals_map["PYTHON"]
    destdir = globals_map["DESTDIR"]
    hvm64_format = globals_map["HVM64_FORMAT"]
    install_data = globals_map["INSTALL_DATA"]
    install_program = globals_map["INSTALL_PROGRAM"]
    xtfdir = globals_map["xtfdir"]
    xtftestdir = globals_map["xtftestdir"]

    install_xtfdir = f"{destdir}{xtfdir}" if destdir else xtfdir
    install_xtftestdir = f"{destdir}{xtftestdir}" if destdir else xtftestdir

    lines = [
        "# Autogenerated by build/gen-ninja.py. Do not edit.",
        "ninja_required_version = 1.3",
        "",
    ]

    # Define the reusable command blocks referenced later by concrete build
    # statements.  Ninja separates the command definition from each individual
    # output.  If you think in make terms, this is similar to writing down the
    # shared recipe form once and then reusing it with different file-specific
    # variables for each concrete target.
    emit_rule(
        lines,
        "cc",
        f"{cc} $cflags -MT $out -MF $depfile -c $in -o $out",
        depfile="$depfile",
        deps="gcc",
    )
    emit_rule(
        lines,
        "as",
        f"{cc} $aflags -MT $out -MF $depfile -c $in -o $out",
        depfile="$depfile",
        deps="gcc",
    )
    emit_rule(
        lines,
        "cpp_lds",
        f"{cpp} $aflags -MT $out -MF $depfile -P $in -o $out",
        depfile="$depfile",
        deps="gcc",
    )
    emit_rule(lines, "link", f"{ld} $ldflags $in -o $out")
    emit_rule(
        lines,
        "link_hvm64",
        f"{ld} $ldflags $in -o $tmpout && {objcopy} $tmpout -O {hvm64_format} $out"
        " && rm -f $tmpout",
    )
    emit_rule(
        lines,
        "mkcfg",
        f'{python} build/mkcfg.py $out "$defcfg" "$vcpus" "$extracfg" "$varycfg"',
    )
    emit_rule(
        lines,
        "mkinfo",
        f'{python} build/mkinfo.py $out "$name" "$category" "$envs" "$variations"',
    )
    emit_rule(lines, "install_data", f"mkdir -p $outdir && {install_data} $in $out")
    emit_rule(
        lines, "install_program", f"mkdir -p $outdir && {install_program} $in $out"
    )

    seen_outputs: set[str] = set()
    build_targets: list[str] = []
    install_targets: list[str] = []

    def emit_object(original_obj: str, output_obj: str, flags: str) -> None:
        """Emit one object-file build statement if it is not already present.

        Multiple tests can depend on the same shared object, so the generator
        must deduplicate these outputs while still discovering whether the input
        source is C or assembly.
        """

        if output_obj in seen_outputs:
            return
        source, rule = source_for_obj(root, original_obj)
        emit_build(
            lines,
            output_obj,
            rule,
            [source],
            {
                "cflags" if rule == "cc" else "aflags": flags,
                "depfile": depfile_for(output_obj),
            },
        )
        seen_outputs.add(output_obj)

    def emit_link_script(env_name: str) -> str:
        """Emit one generated linker script when an environment first needs it.

        The recursive make path treats these linker scripts as generated files,
        so the Ninja path mirrors that behaviour and emits them lazily.
        """

        output = to_rel(root, envs[env_name].link)
        if output in seen_outputs:
            return output
        emit_build(
            lines,
            output,
            "cpp_lds",
            ["common/link.lds.S"],
            {
                "aflags": envs[env_name].aflags_env,
                "depfile": depfile_for(output),
            },
        )
        seen_outputs.add(output)
        return output

    for test in tests:
        info_output = os.path.join(test.directory, "info.json")

        # Start with the descriptive metadata for the test directory itself.
        # This is not part of the compiled binary, but it travels with the test
        # outputs and is installed alongside them for consumers that need to
        # know the test name, category, supported environments, and variations.
        # Emit the metadata file describing what this test is, which
        # environments it supports, and which config variations exist.
        emit_build(
            lines,
            info_output,
            "mkinfo",
            [os.path.join(test.directory, "Makefile"), "build/mkinfo.py"],
            {
                "name": test.name,
                "category": test.category,
                "envs": " ".join(test.envs),
                "variations": " ".join(test.vary_cfg),
            },
        )
        build_targets.append(info_output)

        install_info = os.path.join(
            to_rel(root, install_xtftestdir), test.name, "info.json"
        )
        emit_build(
            lines,
            install_info,
            "install_data",
            [info_output],
            {"outdir": os.path.dirname(install_info)},
        )
        install_targets.append(install_info)

        for env_name in test.envs:
            env = envs[env_name]

            dep_outputs: list[str] = []

            # First emit the objects that are shared by all tests of the same
            # bitness.  These correspond to the common object lists in the make
            # build and are reused across many later link steps.
            for obj in objects_perbits:
                rel_obj = to_rel(root, obj)
                output_obj = rel_obj[:-2] + f"-{env.arch}.o"
                emit_object(
                    obj,
                    output_obj,
                    (
                        env.cflags_arch
                        if source_for_obj(root, obj)[1] == "cc"
                        else env.aflags_arch
                    ),
                )
                dep_outputs.append(output_obj)

            # Collect the object files that are specific to this exact
            # environment.  This combines shared per-environment sources with
            # the test directory's own object list.  Together with the per-bits
            # objects above, this produces the full set of link inputs for one
            # test binary in one environment.
            for obj in env_objects.get(env_name, []) + objects_perenv + test.local_objs:
                rel_obj = to_rel(root, obj)
                output_obj = rel_obj[:-2] + f"-{env_name}.o"
                emit_object(
                    obj,
                    output_obj,
                    (
                        env.cflags_env
                        if source_for_obj(root, obj)[1] == "cc"
                        else env.aflags_env
                    ),
                )
                dep_outputs.append(output_obj)

            # Emit the final link step for one test binary.  The linker script
            # is tracked as an implicit dependency: Ninja rebuilds when it
            # changes, but it is not appended to the link command as a normal
            # input file.  This keeps the command line aligned with what the
            # linker actually consumes while still preserving correct rebuilds.
            link_script = emit_link_script(env_name)
            bin_output = os.path.join(test.directory, f"test-{env_name}-{test.name}")
            link_inputs = dep_outputs
            link_vars = {"ldflags": env.ldflags}
            link_rule = "link"
            if env_name == "hvm64":
                link_rule = "link_hvm64"
                link_vars["tmpout"] = bin_output + ".tmp"
            emit_build(
                lines,
                bin_output,
                link_rule,
                link_inputs,
                link_vars,
                implicit_inputs=[link_script],
            )
            build_targets.append(bin_output)

            install_bin = os.path.join(
                to_rel(root, install_xtftestdir),
                test.name,
                os.path.basename(bin_output),
            )
            emit_build(
                lines,
                install_bin,
                "install_program",
                [bin_output],
                {"outdir": os.path.dirname(install_bin)},
            )
            install_targets.append(install_bin)

            # Emit the default xl cfg file for this test/environment pair.
            # In practice this is the generated runtime configuration derived
            # from the environment default plus any test-local extras.
            cfg_output = os.path.join(
                test.directory, f"test-{env_name}-{test.name}.cfg"
            )
            cfg_inputs = [
                "build/mkcfg.py",
                to_rel(root, env.defcfg),
                os.path.join(test.directory, "Makefile"),
            ]
            if test.extra_cfg:
                cfg_inputs.append(to_rel(root, test.extra_cfg))
            emit_build(
                lines,
                cfg_output,
                "mkcfg",
                cfg_inputs,
                {
                    "defcfg": to_rel(root, env.defcfg),
                    "vcpus": test.vcpus,
                    "extracfg": to_rel(root, test.extra_cfg),
                    "varycfg": "",
                },
            )
            build_targets.append(cfg_output)

            install_cfg = os.path.join(
                to_rel(root, install_xtftestdir),
                test.name,
                os.path.basename(cfg_output),
            )
            emit_build(
                lines,
                install_cfg,
                "install_data",
                [cfg_output],
                {"outdir": os.path.dirname(install_cfg)},
            )
            install_targets.append(install_cfg)

            for variation in test.vary_cfg:
                local_vary = os.path.join(test.directory, f"{variation}.cfg.in")

                # Variation fragments can either be private to the test or come
                # from the shared config/ directory, so resolve the input path
                # before emitting the cfg-generation statement.  Resolving that
                # choice here keeps the build rule emission below straightforward
                # because it only has to work with the already-selected input.
                vary_input = (
                    local_vary
                    if os.path.exists(os.path.join(root, local_vary))
                    else os.path.join("config", f"{variation}.cfg.in")
                )
                vary_output = os.path.join(
                    test.directory, f"test-{env_name}-{test.name}~{variation}.cfg"
                )
                vary_inputs = [
                    "build/mkcfg.py",
                    to_rel(root, env.defcfg),
                    os.path.join(test.directory, "Makefile"),
                    vary_input,
                ]
                if test.extra_cfg:
                    vary_inputs.append(to_rel(root, test.extra_cfg))
                emit_build(
                    lines,
                    vary_output,
                    "mkcfg",
                    vary_inputs,
                    {
                        "defcfg": to_rel(root, env.defcfg),
                        "vcpus": test.vcpus,
                        "extracfg": to_rel(root, test.extra_cfg),
                        "varycfg": vary_input,
                    },
                )
                build_targets.append(vary_output)

                install_vary = os.path.join(
                    to_rel(root, install_xtftestdir),
                    test.name,
                    os.path.basename(vary_output),
                )
                emit_build(
                    lines,
                    install_vary,
                    "install_data",
                    [vary_output],
                    {"outdir": os.path.dirname(install_vary)},
                )
                install_targets.append(install_vary)

    # The install side of the graph also needs the top-level xtf-runner helper,
    # which is not associated with any one test directory.  It is emitted last
    # because unlike the per-test install targets above, it depends only on the
    # top-level repository layout and not on any manifest test record.
    runner_install = os.path.join(to_rel(root, install_xtfdir), "xtf-runner")
    emit_build(
        lines,
        runner_install,
        "install_program",
        ["xtf-runner"],
        {"outdir": os.path.dirname(runner_install)},
    )
    install_targets.append(runner_install)

    for module in python_modules(root, "xtf"):
        module_install = os.path.join(to_rel(root, install_xtfdir), module)
        emit_build(
            lines,
            module_install,
            "install_data",
            [module],
            {"outdir": os.path.dirname(module_install)},
        )
        install_targets.append(module_install)

    emit_phony(lines, "build", build_targets)
    emit_phony(lines, "install", install_targets)
    lines.append("default build")
    lines.append("")
    return lines


def main() -> int:
    """Load the manifest, generate Ninja lines, and write the output file."""

    if len(sys.argv) != 3:
        print("Usage: gen-ninja.py MANIFEST OUT", file=sys.stderr)
        return 2

    manifest_path, output_path = sys.argv[1:3]

    # Decode the Make-generated manifest first so the rest of the file can work
    # in terms of environments, tests, and object lists rather than raw text
    # records.  This keeps the parsing concerns local to one place and lets the
    # emitter logic below read more like the structure of the build itself.
    globals_map, envs, env_objects, tests, objects_perbits, objects_perenv = (
        parse_manifest(manifest_path)
    )
    root = globals_map["ROOT"]

    # Build the full Ninja file in memory first, then let the Makefile's
    # temporary-output convention decide whether the generated file changed.
    # That separation keeps this script focused purely on content generation.
    lines = build_ninja(
        root, globals_map, envs, env_objects, tests, objects_perbits, objects_perenv
    )

    with open(output_path, "w", encoding="utf-8") as handle:
        handle.write("\n".join(lines))

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
