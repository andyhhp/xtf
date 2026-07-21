#!/usr/bin/env python3
"""
xtf-runner - A utility for enumerating and running XTF tests.

Currently assumes the presence and availability of the `xl` toolstack.
"""

import json
import os
import subprocess
import sys
from argparse import ArgumentParser, Namespace, RawDescriptionHelpFormatter
from functools import partial
from os import path
from subprocess import PIPE
from typing import Any, Dict, List, Optional, Set, Tuple

# Wrap Subprocess functions to use universal_newlines by default
Popen = partial(subprocess.Popen, universal_newlines=True)
subproc_call = partial(subprocess.call, universal_newlines=True)
check_output = partial(subprocess.check_output, universal_newlines=True)


# All results of a test, keep in sync with C code report.h where applicable.
# Notes:
#  - WARNING is not a result on its own.
#  - CRASH isn't known to the C code, but covers all cases where a valid
#    result was not found.
#  - XFAIL isn't known to the C code, but covers marked expected crashes.
all_results = ("SUCCESS", "XFAIL", "SKIP", "ERROR", "FAILURE", "CRASH")


# Return the exit code for different states.  Avoid using 1 and 2 because
# python interpreter uses them -- see document for sys.exit.
def exit_code(state: str) -> int:
    """Convert a test result to an xtf-runner exit code."""
    return {
        "SUCCESS": 0,
        "XFAIL": 0,
        "SKIP": 3,
        "ERROR": 4,
        "FAILURE": 5,
        "CRASH": 6,
    }[state]


# All test categories
default_categories = {"functional", "xsa", "nested-svm"}
non_default_categories = {"special", "utility", "in-development"}
all_categories = default_categories | non_default_categories

# All test environments
pv_environments = {"pv64", "pv32pae"}
hvm_environments = {"hvm64", "hvm32pae", "hvm32pse", "hvm32"}
all_environments = pv_environments | hvm_environments


class RunnerError(Exception):
    """Errors relating to xtf-runner itself"""


def env_to_virt_caps(env: str) -> Set[str]:
    """Identify which virt cap(s) are needed for an environment"""
    if env in hvm_environments:
        return {"hvm"}
    caps = {"pv"}
    if env == "pv32pae":
        caps |= {"pv32"}
    return caps


class TestInstance:
    """Object representing a single test."""

    def __init__(self, arg: str) -> None:
        """Parse and verify 'arg' as a test instance."""
        env_result, name_result, variation_result = parse_test_instance_string(arg)
        self.env: Optional[str] = env_result
        self.name: str = name_result
        self.variation: Optional[str] = variation_result

        if self.env is None:
            raise RunnerError(f"No environment for '{arg}'")

        if self.variation is None and get_all_test_info()[self.name].variations:
            raise RunnerError(f"Test '{self.name}' has variations, but none specified")

        self.req_caps: Set[str] = env_to_virt_caps(self.env)
        self.req_caps |= {"hap", "shadow"} & {self.variation}

    def vm_name(self) -> str:
        """Return the VM name as `xl` expects it."""
        return repr(self)

    def cfg_path(self) -> str:
        """Return the path to the `xl` config file for this test."""
        return path.join("tests", self.name, repr(self) + ".cfg")

    def __repr__(self) -> str:
        if not self.variation:
            return f"test-{self.env}-{self.name}"
        return f"test-{self.env}-{self.name}~{self.variation}"

    def __hash__(self) -> int:
        return hash(repr(self))

    def __eq__(self, other: Any) -> bool:
        return repr(self) == repr(other)


class TestInfo:
    """Object representing a tests info.json, in a more convenient form."""

    def __init__(self, test_json: Any) -> None:
        """Parse and verify 'test_json'.

        May raise KeyError, TypeError or ValueError.
        """

        name: Any = test_json["name"]
        if not isinstance(name, str):
            raise TypeError(f"Expected string for 'name', got '{type(name)}'")
        self.name: str = name

        cat: Any = test_json["category"]
        if not isinstance(cat, str):
            raise TypeError(f"Expected string for 'category', got '{type(cat)}'")
        if cat not in all_categories:
            raise ValueError(f"Unknown category '{cat}'")
        self.cat: str = cat

        envs: Any = test_json["environments"]
        if not isinstance(envs, list):
            raise TypeError(f"Expected list for 'environments', got '{type(envs)}'")
        if not envs:
            raise ValueError("Expected at least one environment")
        for env in envs:
            if env not in all_environments:
                raise ValueError(f"Unknown environments '{env}'")
        self.envs: List[str] = envs

        variations: Any = test_json["variations"]
        if not isinstance(variations, list):
            raise TypeError(f"Expected list for 'variations', got '{type(variations)}'")
        self.variations: List[str] = variations

    def all_instances(
        self,
        env_filter: Optional[Set[str]] = None,
        vary_filter: Optional[List[str]] = None,
    ) -> List[TestInstance]:
        """Return a list of TestInstances, for each supported environment.
        Optionally filtered by env_filter.  May return an empty list if
        the filter doesn't match any supported environment.
        """

        if env_filter:
            envs_list = list(set(env_filter).intersection(self.envs))
        else:
            envs_list = self.envs

        if vary_filter:
            variations_list = list(set(vary_filter).intersection(self.variations))
        else:
            variations_list = self.variations

        res = []
        if variations_list:
            for env in envs_list:
                for vary in variations_list:
                    res.append(TestInstance(f"test-{env}-{self.name}~{vary}"))
        else:
            res = [TestInstance(f"test-{env}-{self.name}") for env in envs_list]
        return res

    def __repr__(self) -> str:
        return f"TestInfo({self.name})"


def parse_test_instance_string(arg: str) -> Tuple[Optional[str], str, Optional[str]]:
    """Parse a test instance string.

    Has the form: '[[test-]$ENV-]$NAME[~$VARIATION]'

    Optional 'test-' prefix
    Optional $ENV environment part
    Mandatory $NAME
    Optional ~$VARIATION suffix

    Verifies:
      - $NAME is valid
      - if $ENV, it is valid for $NAME
      - if $VARIATION, it is valid for $NAME

    Returns: tuple($ENV or None, $NAME, $VARIATION or None)
    """

    all_tests = get_all_test_info()

    variation = None
    if "~" in arg:
        arg, variation = arg.split("~", 1)

    parts = arg.split("-", 2)
    parts_len = len(parts)

    env = None
    name = ""

    # If arg =~ test-$ENV-$NAME
    if parts_len == 3 and parts[0] == "test" and parts[1] in all_environments:
        _, env, name = parts

    # If arg =~ $ENV-$NAME
    elif parts_len > 0 and parts[0] in all_environments:
        env, name = parts[0], "-".join(parts[1:])

    # If arg =~ $NAME
    elif arg in all_tests:
        env, name = None, arg

    # Otherwise, give up
    else:
        raise RunnerError(f"Unrecognised test '{arg}'")

    # At this point, 'env' has always been checked for plausibility.  'name'
    # might not be

    if name not in all_tests:
        raise RunnerError(f"Unrecognised test name '{name}' for '{arg}'")

    info = all_tests[name]

    if env and env not in info.envs:
        raise RunnerError(f"Test '{name}' has no environment '{env}'")

    # If a variation has been given, check it is valid
    if variation is not None:
        if not info.variations:
            raise RunnerError(f"Test '{name}' has no variations")
        if variation not in info.variations:
            raise RunnerError(f"No variation '{variation}' for test '{name}'")

    return env, name, variation


# Cached data from tests/*/info.json
_all_test_info: Dict[str, TestInfo] = {}


def get_all_test_info() -> Dict[str, TestInfo]:
    """Open and collate each info.json"""
    if not _all_test_info:  # Cache on first request

        for test in os.listdir("tests"):
            try:
                with open(path.join("tests", test, "info.json")) as file:
                    info = TestInfo(json.load(file))

                    if info.name != test:
                        raise ValueError  # JSON also looks bad

                    _all_test_info[test] = info

            except (
                IOError,  # Ignore directories without a info.json
                ValueError,
                KeyError,
                TypeError,
            ):  # Ignore bad JSON
                continue

    return _all_test_info


# Cached virt caps
_virt_caps: Set[str] = set()


def get_virt_caps() -> Set[str]:
    """Query Xen for the virt capabilities of the host"""
    global _virt_caps

    if not _virt_caps:  # Cache on first request

        # Filter down to caps we're happy for tests to use
        caps = {"pv", "hvm", "hap", "shadow"}
        caps &= set(check_output(["xl", "info", "virt_caps"]).split())

        # Synthesize a pv32 virt cap by looking at xen_caps
        if "pv" in caps and "xen-3.0-x86_32p" in check_output(
            ["xl", "info", "xen_caps"]
        ):
            caps |= {"pv32"}

        _virt_caps = caps

    return _virt_caps


def reset_all_test_info() -> None:
    """Reset the cached test info (mainly for testing)."""
    global _all_test_info
    _all_test_info = {}


def reset_virt_caps() -> None:
    """Reset the cached virt caps (mainly for testing)."""
    global _virt_caps
    _virt_caps = set()


def tests_from_selection(
    cats: Optional[Set[str]],
    envs: Optional[Set[str]],
    tests: Set[TestInstance],
    caps: Optional[Set[str]],
) -> List[TestInstance]:
    """Given a selection of possible categories, environment and tests, return
    all tests within the provided parameters.

    Multiple entries for each individual parameter are combined or-wise.
    e.g. cats=['special', 'functional'] chooses all tests which are either
    special or functional.  envs=['hvm64', 'pv64'] chooses all tests which are
    either pv64 or hvm64.

    Multiple parameter are combined and-wise, taking the intersection rather
    than the union.  e.g. cats=['functional'], envs=['pv64'] gets the tests
    which are both part of the functional category and the pv64 environment.

    By default, not all categories are available.  Selecting envs=['pv64']
    alone does not include the non-default categories, as this is most likely
    not what the author intended.  Any reference to non-default categories in
    cats[] or tests[] turns them all back on, so non-default categories are
    available when explicitly referenced.
    """

    all_tests = get_all_test_info()
    all_test_info = all_tests.values()
    res = []

    if cats:
        # If a selection of categories have been requested, start with all test
        # instances in any of the requested categories.
        for info in all_test_info:
            if info.cat in cats:
                res.extend(info.all_instances())

    if envs:
        # If a selection of environments have been requested, reduce the
        # category selection to requested environments, or pick all suitable
        # tests matching the environments request.
        if res:
            res = [x for x in res if x.env in envs]
        else:
            # Work out whether to include non-default categories or not.
            categories = default_categories
            if cats and non_default_categories & set(cats):
                categories = all_categories

            elif tests:
                sel_test_names = {x.name for x in tests}
                sel_test_cats = {all_tests[x].cat for x in sel_test_names}

                if non_default_categories & sel_test_cats:
                    categories = all_categories

            for info in all_test_info:
                if info.cat in categories:
                    res.extend(info.all_instances(env_filter=envs))

    if tests:
        # If a selection of tests has been requested, reduce the results so
        # far to the requested tests (this is meaningful in the case that
        # tests[] has been specified without a specific environment), or just
        # take the tests verbatim.
        if res:
            res = [x for x in res if x in tests]
        else:
            res = list(tests)

    if caps:
        res = [x for x in res if x.req_caps.issubset(caps)]

    # Sort the results.  Variation third, Env second and Name fist.
    res = sorted(res, key=lambda test: test.variation if test.variation else "")
    res = sorted(res, key=lambda test: test.env if test.env else "")
    res = sorted(res, key=lambda test: test.name)
    return res


def interpret_selection(opts: Namespace) -> List[TestInstance]:
    """Interpret the argument list as a collection of categories, environments,
    pseduo-environments and partial and complete test names.

    Returns a list of all test instances within the selection.
    """

    args = set(opts.args)

    # First, filter into large buckets
    cats = all_categories & args
    envs = all_environments & args
    others = args - cats - envs

    # Add all categories if --all or --non-default is passed
    if opts.all:
        cats |= default_categories
    if opts.non_default:
        cats |= non_default_categories

    # Allow "pv" and "hvm" as a combination of environments
    if "pv" in others:
        envs |= pv_environments
        others -= {"pv"}

    if "hvm" in others:
        envs |= hvm_environments
        others -= {"hvm"}

    # No input? No selection.
    if not cats and not envs and not others:
        return []

    all_tests = get_all_test_info()
    tests = []

    # Second, sanity check others as full or partial test names
    for arg in others:
        env, name, vary = parse_test_instance_string(arg)

        instances = all_tests[name].all_instances(
            env_filter={env} if env else None,
            vary_filter=[vary] if vary else None,
        )

        if not instances:
            raise RunnerError(f"No appropriate instances for '{arg}' (env {env})")

        tests.extend(instances)

    # Third, if --host is passed, also filter by capabilities
    caps = None
    if opts.host:
        caps = get_virt_caps()

    return tests_from_selection(cats, envs, set(tests), caps)


def list_tests(opts: Namespace) -> None:
    """List tests"""

    if opts.environments:
        # The caller only wants the environment list
        for env in sorted(all_environments):
            print(env)
        return

    if not opts.selection:
        raise RunnerError("No tests selected")

    for sel in opts.selection:
        print(sel)


def interpret_result(logline: str) -> str:
    """Interpret the final log line of a guest for a result"""

    if "Test result:" not in logline:
        return "CRASH"

    for res in all_results:
        if res in logline:
            return res

    return "CRASH"


def interpret_console_lines(lines: List[str]) -> str:
    """Interpret a guest console log for a result."""

    res = interpret_result(lines[-1])
    if res == "CRASH" and any("XFAIL:" in line for line in lines):
        return "XFAIL"

    return res


def run_test_console(opts: Namespace, test: TestInstance) -> str:
    """Run a specific, obtaining results via xenconsole"""

    cmd = ["xl", "create", "-p", test.cfg_path()]
    if not opts.quiet:
        print(f"Executing '{' '.join(cmd)}'")

    create = Popen(cmd, stdout=PIPE, stderr=PIPE)
    _, stderr = create.communicate()

    if create.returncode:
        if opts.quiet:
            print(f"Executing '{' '.join(cmd)}'")
        print(stderr)
        raise RunnerError("Failed to create VM")

    cmd = ["xl", "console", test.vm_name()]
    if not opts.quiet:
        print(f"Executing '{' '.join(cmd)}'")

    console = Popen(cmd, stdout=PIPE)

    cmd = ["xl", "unpause", test.vm_name()]
    if not opts.quiet:
        print(f"Executing '{' '.join(cmd)}'")

    rc = subproc_call(cmd)
    if rc:
        if opts.quiet:
            print(f"Executing '{' '.join(cmd)}'")
        raise RunnerError("Failed to unpause VM")

    stdout, _ = console.communicate()

    if console.returncode:
        raise RunnerError("Failed to obtain VM console")

    lines = stdout.splitlines()

    if lines:
        if not opts.quiet:
            print("\n".join(lines))
            print("")

    else:
        return "CRASH"

    return interpret_console_lines(lines)


def run_test_logfile(opts: Namespace, test: TestInstance) -> str:
    """Run a specific test, obtaining results from a logfile"""

    logpath = path.join(opts.logfile_dir, opts.logfile_pattern.replace("%s", str(test)))

    if not opts.quiet:
        print(f"Using logfile '{logpath}'")

    fd = os.open(logpath, os.O_CREAT | os.O_RDONLY, 0o644)
    logfile = os.fdopen(fd)
    logfile.seek(0, os.SEEK_END)

    cmd = ["xl", "create", "-F", test.cfg_path()]
    if not opts.quiet:
        print(f"Executing '{' '.join(cmd)}'")

    guest = Popen(cmd, stdout=PIPE, stderr=PIPE)

    _, stderr = guest.communicate()

    if guest.returncode:
        if opts.quiet:
            print(f"Executing '{' '.join(cmd)}'")
        print(stderr)
        raise RunnerError("Failed to run test")

    line = ""
    saw_xfail = False
    for line in logfile.readlines():

        line = line.rstrip()
        if not opts.quiet:
            print(line)

        saw_xfail |= "XFAIL:" in line

        if "Test result:" in line:
            print("")
            break

    logfile.close()

    res = interpret_result(line)
    if res == "CRASH" and saw_xfail:
        return "XFAIL"

    return res


def run_test(opts: Namespace, test: TestInstance) -> str:
    """Run a single test instance"""

    # If caps say the test can't run, short circuit to SKIP
    if not test.req_caps.issubset(get_virt_caps()):
        return "SKIP"

    fn: Any = {
        "console": run_test_console,
        "logfile": run_test_logfile,
    }[opts.results_mode]

    return fn(opts, test)  # type: ignore[no-any-return]


def run_tests(opts: Namespace) -> int:
    """Run tests"""

    tests = opts.selection
    if not tests:
        raise RunnerError("No tests to run")

    rc = all_results.index("SUCCESS")
    results = []

    for test in tests:

        res = run_test(opts, test)
        res_idx = all_results.index(res)
        rc = max(rc, res_idx)

        results.append(res)

    print("Combined test results:")

    for test, res in zip(tests, results):

        if res == "SUCCESS" and opts.quiet >= 2:
            continue

        print(f"{str(test):<40} {res}")

    return exit_code(all_results[rc])


def main(root: Optional[str] = None, line_buffer_stdout: bool = True) -> int:
    """Main entrypoint"""

    # Change stdout to be line-buffered.
    if line_buffer_stdout:
        sys.stdout = os.fdopen(sys.stdout.fileno(), "w", 1)

    # Normalise $CWD to the directory this script is in
    if root is None:
        root = path.dirname(path.abspath(sys.argv[0]))
    os.chdir(root)

    parser = ArgumentParser(
        usage="%(prog)s [--list] <SELECTION> [options]",
        description="Xen Test Framework enumeration and running tool",
        formatter_class=RawDescriptionHelpFormatter,
        epilog=(
            "\n"
            "Overview:\n"
            "  Running with --list will print the entire selection\n"
            "  to the console.  Running without --list will execute\n"
            "  all tests in the selection, printing a summary of their\n"
            "  results at the end.\n"
            "\n"
            "  To determine how runner should get output from Xen, use\n"
            '  --results-mode option. The default value is "console", \n'
            "  which means using xenconsole program to extract output.\n"
            '  The other supported value is "logfile", which\n'
            "  means to get output from log file.\n"
            "\n"
            '  The "logfile" mode requires users to configure\n'
            "  xenconsoled to log guest console output. This mode\n"
            "  is useful for Xen version < 4.8. Also see --logfile-dir\n"
            "  and --logfile-pattern options.\n"
            "\n"
            "Selections:\n"
            "  A selection is zero or more of any of the following\n"
            "  parameters: Categories, Environments and Tests.\n"
            "  Multiple instances of the same type of parameter are\n"
            "  unioned while the end result in intersected across\n"
            "  types.  e.g.\n"
            "\n"
            "    'functional xsa'\n"
            "       All tests in the functional and xsa categories\n"
            "\n"
            "    'functional xsa hvm32'\n"
            "       All tests in the functional and xsa categories\n"
            "       which are implemented for the hvm32 environment\n"
            "\n"
            "    'invlpg example'\n"
            "       The invlpg and example tests in all implemented\n"
            "       environments\n"
            "\n"
            "    'invlpg example pv'\n"
            "       The pv environments of the invlpg and example tests\n"
            "\n"
            "    'pv32pae-pv-iopl'\n"
            "       The pv32pae environment of the pv-iopl test only\n"
            "\n"
            "  Additionally, --host may be passed to restrict the\n"
            "  selection to tests applicable to the current host.\n"
            "  --all may be passed to choose all default categories\n"
            "  without needing to explicitly name them.  --non-default\n"
            "  is available to obtain the non-default categories.\n"
            "\n"
            "  The special parameter --environments may be passed to\n"
            "  get the full list of environments.  This option does not\n"
            "  make sense combined with a selection.\n"
            "\n"
            "Examples:\n"
            "  Listing all tests implemented for hvm32 environment:\n"
            "    ./xtf-runner --list hvm32\n"
            "\n"
            "  Listing all functional tests appropriate for this host:\n"
            "    ./xtf-runner --list functional --host\n"
            "\n"
            "  Running all the pv-iopl tests:\n"
            "    ./xtf-runner pv-iopl\n"
            "      <console output>\n"
            "    Combined test results:\n"
            "    test-pv64-pv-iopl                        SUCCESS\n"
            "    test-pv32pae-pv-iopl                     SUCCESS\n"
            "\n"
            "  Exit code for this script:\n"
            "    0:    everything is ok\n"
            "    1,2:  reserved for python interpreter\n"
            "    3:    test(s) are skipped\n"
            "    4:    test(s) report error\n"
            "    5:    test(s) report failure\n"
            "    6:    test(s) crashed\n"
            "\n"
        ),
    )

    parser.add_argument(
        "-l",
        "--list",
        action="store_true",
        dest="list_tests",
        help="List tests in the selection",
    )
    parser.add_argument(
        "-a",
        "--all",
        action="store_true",
        dest="all",
        help="Select all default categories",
    )
    parser.add_argument(
        "--non-default",
        action="store_true",
        dest="non_default",
        help="Select all non default categories",
    )
    parser.add_argument(
        "--environments",
        action="store_true",
        dest="environments",
        help="List all the known environments",
    )
    parser.add_argument(
        "--host",
        action="store_true",
        dest="host",
        help="Restrict selection to applicable" " tests for the current host",
    )
    parser.add_argument(
        "-m",
        "--results-mode",
        action="store",
        dest="results_mode",
        default="console",
        choices=["console", "logfile"],
        help="Control how xtf-runner gets its test results",
    )
    parser.add_argument(
        "--logfile-dir",
        action="store",
        dest="logfile_dir",
        default="/var/log/xen/console/",
        help=(
            "Specify the directory to look for console logs, "
            'defaults to "/var/log/xen/console/"'
        ),
    )
    parser.add_argument(
        "--logfile-pattern",
        action="store",
        dest="logfile_pattern",
        default="guest-%s.log",
        help=("Specify the log file name pattern, " 'defaults to "guest-%%s.log"'),
    )
    parser.add_argument(
        "-q",
        "--quiet",
        action="count",
        dest="quiet",
        default=0,
        help=(
            "Progressively make the output less verbose.  "
            "1) No console logs, only test results.  "
            "2) Not even SUCCESS results."
        ),
    )

    opts, args = parser.parse_known_args()
    opts.args = args
    opts.selection = interpret_selection(opts)

    if opts.list_tests:
        list_tests(opts)
        return 0
    return run_tests(opts)
