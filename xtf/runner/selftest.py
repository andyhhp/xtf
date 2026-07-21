"""Selftests for the importable xtf-runner implementation."""

import contextlib
import io
import json
import os
import sys
import tempfile
import unittest
from os import path
from types import SimpleNamespace
from typing import Generator, List, Optional, Tuple
from unittest import mock

from xtf.runner import cli as runner


def _write_test_info(
    root: str, name: str, envs: Optional[List[str]] = None, category: str = "functional"
) -> None:
    """Create a minimal XTF test metadata file under a temporary root."""
    if envs is None:
        envs = ["hvm64"]

    test_dir = path.join(root, "tests", name)
    os.makedirs(test_dir)

    with open(path.join(test_dir, "info.json"), "w", encoding="utf-8") as info:
        json.dump(
            {
                "name": name,
                "category": category,
                "environments": envs,
                "variations": [],
            },
            info,
        )


@contextlib.contextmanager
def _temporary_runner_root() -> Generator[str, None, None]:
    """Use a temporary directory as the runner's repository root."""
    cwd = os.getcwd()
    argv = sys.argv[:]

    with tempfile.TemporaryDirectory() as tmpdir:
        os.makedirs(path.join(tmpdir, "tests"))

        runner.reset_all_test_info()
        runner.reset_virt_caps()

        try:
            yield tmpdir
        finally:
            os.chdir(cwd)
            sys.argv = argv
            runner.reset_all_test_info()
            runner.reset_virt_caps()


class RunnerCliTests(unittest.TestCase):
    """Tests for legacy CLI behaviour through xtf.runner.cli."""

    def test_list_prints_selected_test_instances(self) -> None:
        """`xtf-runner --list NAME` prints matching test instances."""
        with _temporary_runner_root() as tmpdir:
            _write_test_info(tmpdir, "alpha", ["hvm64", "hvm32"])
            sys.argv = [path.join(tmpdir, "xtf-runner"), "--list", "alpha"]

            stdout = io.StringIO()
            with contextlib.redirect_stdout(stdout):
                rc = runner.main(root=tmpdir, line_buffer_stdout=False)

        self.assertIsNone(rc)
        self.assertEqual(
            stdout.getvalue().splitlines(),
            [
                "test-hvm32-alpha",
                "test-hvm64-alpha",
            ],
        )

    def test_run_prints_combined_results_and_exit_code(self) -> None:
        """Running a selected test preserves summary and exit-code mapping."""
        with _temporary_runner_root() as tmpdir:
            _write_test_info(tmpdir, "alpha")
            sys.argv = [path.join(tmpdir, "xtf-runner"), "hvm64-alpha", "-q"]

            stdout = io.StringIO()
            with mock.patch.object(
                runner, "get_virt_caps", return_value={"hvm"}
            ), mock.patch.object(
                runner, "run_test_console", return_value="SUCCESS"
            ) as run_console, contextlib.redirect_stdout(
                stdout
            ):
                rc = runner.main(root=tmpdir, line_buffer_stdout=False)

        self.assertEqual(rc, runner.exit_code("SUCCESS"))
        run_console.assert_called_once()
        self.assertEqual(str(run_console.call_args[0][1]), "test-hvm64-alpha")
        self.assertIn("Combined test results:", stdout.getvalue())
        self.assertIn("test-hvm64-alpha", stdout.getvalue())
        self.assertIn("SUCCESS", stdout.getvalue())

    def test_run_test_console_uses_xl_lifecycle(self) -> None:
        """Console-mode execution still uses create-paused/console/unpause."""
        opts = SimpleNamespace(quiet=1)
        test = SimpleNamespace(
            cfg_path=lambda: "tests/alpha/test-hvm64-alpha.cfg",
            vm_name=lambda: "test-hvm64-alpha",
        )

        popen_calls = []

        class FakeProcess:
            """Small fake for subprocess.Popen return objects."""

            def __init__(
                self,
                cmd: List[str],
                stdout: Optional[object] = None,
                stderr: Optional[object] = None,
            ) -> None:
                del stdout, stderr
                self.cmd = cmd
                self.returncode = 0
                popen_calls.append(cmd)

            def communicate(self) -> Tuple[str, Optional[str]]:
                """Simulate a successful console session with a test result."""
                if self.cmd[:2] == ["xl", "console"]:
                    return "boot\nTest result: SUCCESS\n", None
                return "", ""

        with mock.patch.object(
            runner, "Popen", side_effect=FakeProcess
        ), mock.patch.object(runner, "subproc_call", return_value=0) as call:
            result = runner.run_test_console(opts, test)  # type: ignore[arg-type]

        self.assertEqual(result, "SUCCESS")
        self.assertEqual(
            popen_calls,
            [
                ["xl", "create", "-p", "tests/alpha/test-hvm64-alpha.cfg"],
                ["xl", "console", "test-hvm64-alpha"],
            ],
        )
        call.assert_called_once_with(["xl", "unpause", "test-hvm64-alpha"])

    def test_marked_crash_is_xfail(self) -> None:
        """Marked crashes are reported as expected failures."""
        self.assertEqual(
            runner.interpret_console_lines(["boot", "XFAIL: expected crash"]),
            "XFAIL",
        )
        self.assertEqual(runner.interpret_console_lines(["boot"]), "CRASH")


if __name__ == "__main__":
    unittest.main()
