"""Test xtf-runner.parse_test_instance_string() using tests/python/conftest"""
import pytest


def test_parse_test_instance_string(xtf_runner):
    """
    Test env, name, variation = xtf-runner.parse_test_instance_string(string):

    Valid argument strings conform to: [[test-]$ENV-]$NAME[~$VARIATION]
    Returns a tuple with the environment, name, and variation (if present).

    This test uses the pytest fixture xtf_runner from tests/python/conftest.py
    to import and initialize the xtf-runner Python script as module under test.
    """
    # Test the argument "pv64-example":
    for arg in ("pv64-example", "test-pv64-example"):
        env, name, variation = xtf_runner.parse_test_instance_string(arg)
        assert env == "pv64"
        assert name == "example"
        assert variation is None

    # Test the argument "xsa-444":
    env, name, variation = xtf_runner.parse_test_instance_string("xsa-444")
    assert env is None
    assert name == "xsa-444"
    assert variation is None

    # Test that passing a nonexisting variation argument raises RunnerError:
    with pytest.raises(xtf_runner.RunnerError):
        xtf_runner.parse_test_instance_string("xsa-444~nonexisting_variation")

    # Test that passing a nonexisting test argument raises RunnerError:
    with pytest.raises(xtf_runner.RunnerError):
        xtf_runner.parse_test_instance_string("test-nonexisting_test-raises")
