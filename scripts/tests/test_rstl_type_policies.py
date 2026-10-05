"""Compile rstl policy edge cases with the active G2ME01 game compiler.

Run after configuring G2ME01: python -m unittest discover -s scripts/tests -p test_rstl_type_policies.py
No normal build objects or dependency files are overwritten.
"""
from pathlib import Path
import os
import shlex
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[2]


class RstlTypePoliciesTests(unittest.TestCase):
    def test_policy_dispatch(self):
        commands = subprocess.check_output(
            ["ninja", "-t", "commands", "build/G2ME01/src/MetroidPrime/TGameTypes.o"],
            cwd=ROOT, text=True,
        )
        command = commands.strip().splitlines()[-1]
        args = shlex.split(command, posix=os.name != "nt")
        if os.name == "nt":
            args = [arg[1:-1] if arg.startswith('"') and arg.endswith('"') else arg for arg in args]
        self.assertIn("-DBUILD_VERSION=0", args)
        self.assertIn("-c", args)
        self.assertIn("-o", args)
        args = [arg for arg in args if arg != "-MMD"]
        args[args.index("-c") + 1] = str(Path(__file__).with_name("rstl_type_policies.cpp"))
        with tempfile.TemporaryDirectory(prefix="rstl-policies-") as directory:
            output = Path(directory) / "policies.o"
            args[args.index("-o") + 1] = str(output)
            result = subprocess.run(args, cwd=ROOT, capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertTrue(output.is_file())


if __name__ == "__main__":
    unittest.main()
