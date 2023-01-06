"""pytest fixtures for unit-testing functions in the xtf-runner script"""
import os
import sys

import pytest


def import_script_as_module(relative_script_path):
    "Import a Python script without the .py extension as a python module"

    script_path = os.path.join(os.path.dirname(__file__), relative_script_path)
    module_name = os.path.basename(script_path)

    if sys.version_info.major == 2:
        # Use deprecated imp module because it needs also to run with Python27:
        # pylint: disable-next=import-outside-toplevel
        import imp  # pyright: ignore[reportMissingImports]

        return imp.load_source(module_name, script_path)
    else:
        # For Python 3.11+: Import Python script without the .py extension:
        # https://gist.github.com/bernhardkaindl/1aaa04ea925fdc36c40d031491957fd3:

        # pylint: disable-next=import-outside-toplevel
        from importlib import (  # pylint: disable=no-name-in-module
            machinery,
            util,
        )

        loader = machinery.SourceFileLoader(module_name, script_path)
        spec = util.spec_from_loader(module_name, loader)
        assert spec
        assert spec.loader
        module = util.module_from_spec(spec)
        # It is probably a good idea to add the imported module to sys.modules:
        sys.modules[module_name] = module
        spec.loader.exec_module(module)
        return module


@pytest.fixture(scope="session")
def imported_xtf_runner():
    """Fixture to import a script as a module for unit testing its functions"""
    return import_script_as_module("../../xtf-runner")


@pytest.fixture(scope="function")
def xtf_runner(imported_xtf_runner):  # pylint: disable=redefined-outer-name
    """Test fixture for unit tests: initializes module for each test function"""
    # Init the imported xtf-runner, so each unit test function gets it pristine:
    # May be used to unit-test xtf-runner with other, different test dirs:
    imported_xtf_runner._all_test_info = {}  # pylint: disable=protected-access
    # The GitHub pre-commit action for does not start the checks in the src dir:
    # os.chdir(os.path.join(os.path.dirname(__file__), "../.."))
    return imported_xtf_runner
