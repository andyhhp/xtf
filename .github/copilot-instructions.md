# Repository Instructions for GitHub Copilot

## Building and Testing

Rules:
- Run "make" in the repository root to build all code, documentation,
  and tests.
- Never attempt to compile only a subset of the repository.
  Always build the entire repository.
- If the build does not rebuild the files you changed, you can assume that
  another user already ran a build that produced the same output. You can skip the build and run tests directly.
- Test your changes by actually running the test using:

  ./xtf-runner <testcase>

  Example: When testing the `tests/nested-svm-clgi-stgi` test, run:

  ./xtf-runner nested-svm-clgi-stgi

- Only after building and running tests should you run the final
  validation command for Copilot-authored changes.

## CI validation

This repository uses `pre-commit` in CI.

It checks for code formatting, linting, and other issues in code,
build system, documentation, and tests. It is configured to run on
all files in the repository.

Among other checks, it runs:
- Adds missing trailing newlines to all files (add a trailing newline yourself)
- Removes trailing whitespace from all files.
- `isort`, `black`, and `flake8` for Python code.

The full list of checks is in the `.pre-commit-config.yaml` file in the
repository.

After making code, build-system, documentation, or test changes in this
repository, run:

```sh
SKIP=git-diff pre-commit run -av
```

Treat this as the final validation command for Copilot-authored changes.
If the command is unavailable or cannot complete in the current environment,
report that clearly along with any narrower checks that were run instead
and tell a human reviewer to install pre-commit using `pip install pre-commit`
and run the full command before merging.
