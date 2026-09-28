"""Offline tests for module-scoped, reproducible loader generation."""

from __future__ import annotations

import contextlib
import io
import json
import sys
import tempfile
import unittest
import xml.etree.ElementTree as ET
from pathlib import Path
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import generate_script_loaders as loaders

from tools.loader_profile import LoaderProfile

REF = "a" * 40


class MemorySource:
    description = "in-memory test templates"

    def __init__(self) -> None:
        self.files = {
            "MP2/Game.xml": """<Game Game="Echoes">
              <PropertyArchetypes><Element><Key>Shared</Key>
                <Value Path="Structs/Shared.xml"/></Element></PropertyArchetypes>
              <ScriptObjects>
                <Element><Key>AAAA</Key><Value Path="Objects/A.xml"/></Element>
                <Element><Key>BBBB</Key><Value Path="Objects/B.xml"/></Element>
              </ScriptObjects></Game>""",
            "MP2/Structs/Shared.xml": """<Root><PropertyArchetype Type="Struct">
              <SubProperties><Element ID="1" Type="Int"><Name>Count</Name>
              <DefaultValue>7</DefaultValue></Element></SubProperties>
              </PropertyArchetype></Root>""",
            "PropertyMap.xml": "<Root><PropertyMap/></Root>",
            "LICENSE": "test license",
        }
        for name in ("A", "B"):
            self.files[f"MP2/Objects/{name}.xml"] = """<Root><Properties Type="Struct">
              <SubProperties><Element ID="2" Type="Struct" Archetype="Shared">
              <Name>Settings</Name></Element></SubProperties></Properties></Root>"""

    def read(self, path: str) -> bytes:
        return self.files[path].encode()

    def xml(self, path: str) -> ET.Element:
        return ET.fromstring(self.read(path))


class LoaderProfileTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.profile_path = self.root / "profile.json"
        self.data = {
            "version": 1,
            "name": "Test",
            "template_ref": REF,
            "sources": ["SLdrA.cpp"],
        }
        self.save_profile()

    def save_profile(self) -> None:
        self.profile_path.write_text(json.dumps(self.data), encoding="utf-8")

    def test_profile_preserves_reviewed_order(self) -> None:
        self.data["sources"] = ["SLdrB.cpp", "SLdrA.cpp", "Structs/SLdrShared.cpp"]
        self.save_profile()
        profile = LoaderProfile.read(self.profile_path)
        self.assertEqual(profile.sources, tuple(self.data["sources"]))
        self.assertEqual(profile.template_ref, REF)

    def test_profile_rejects_unsafe_or_ambiguous_inputs(self) -> None:
        cases = [
            ("version", 2),
            ("version", True),
            ("name", ""),
            ("template_ref", "main"),
            ("template_ref", 42),
            ("sources", []),
            ("sources", "SLdrA.cpp"),
            ("sources", ["SLdrA.cpp", "SLdrA.cpp"]),
            ("sources", ["../SLdrA.cpp"]),
            ("sources", ["/SLdrA.cpp"]),
            ("sources", ["C:/SLdrA.cpp"]),
            ("sources", ["Structs\\SLdrA.cpp"]),
            ("sources", ["SLdrA.hpp"]),
            ("sources", [1]),
        ]
        for key, value in cases:
            with self.subTest(key=key, value=value):
                original = self.data[key]
                self.data[key] = value
                self.save_profile()
                with self.assertRaises(ValueError):
                    LoaderProfile.read(self.profile_path)
                self.data[key] = original

    def test_unknown_profile_fields_are_rejected(self) -> None:
        self.data["soruces"] = []
        self.save_profile()
        with self.assertRaises(ValueError):
            LoaderProfile.read(self.profile_path)

    def test_profile_keeps_global_header_ownership(self) -> None:
        generator = loaders.Generator(MemorySource())
        generator.collect([])
        selected = loaders.profile_files(
            generator.render(), LoaderProfile.read(self.profile_path)
        )
        self.assertEqual(
            set(selected), {"SLdrA.cpp", "SLdrA.hpp", "Structs/SLdrShared.hpp"}
        )
        self.assertIn(
            '"MetroidPrime/ScriptLoader/Structs/SLdrShared.hpp"', selected["SLdrA.hpp"]
        )
        self.assertNotIn("struct SLdrShared", selected["SLdrA.hpp"])

    def test_unknown_generated_source_is_rejected(self) -> None:
        with self.assertRaisesRegex(loaders.TemplateError, "not generated"):
            loaders.profile_files({}, LoaderProfile.read(self.profile_path))

    def test_read_only_check_handles_missing_stale_and_crlf(self) -> None:
        files = {"SLdrA.cpp": "one\ntwo\n"}
        destination = self.root / "absent"
        self.assertEqual(loaders.output_differences(files, destination), ["SLdrA.cpp"])
        self.assertFalse(destination.exists())
        destination.mkdir()
        target = destination / "SLdrA.cpp"
        target.write_bytes(b"one\r\ntwo\r\n")
        self.assertEqual(loaders.output_differences(files, destination), [])
        target.write_bytes(b"hand edit")
        self.assertEqual(loaders.output_differences(files, destination), ["SLdrA.cpp"])
        self.assertEqual(target.read_bytes(), b"hand edit")

    def test_cli_profile_pins_ref_and_check_does_not_write(self) -> None:
        destination = self.root / "generated"
        args = ["--profile", str(self.profile_path), "--cpp-output", str(destination)]
        with patch.object(loaders, "Source", return_value=MemorySource()) as source:
            with contextlib.redirect_stdout(io.StringIO()):
                self.assertEqual(loaders.main(args), 0)
            source.assert_called_once_with(None, REF)
        self.assertEqual([p.name for p in destination.iterdir()], ["SLdrA.cpp"])
        target = destination / "SLdrA.cpp"
        before = target.read_bytes(), target.stat().st_mtime_ns
        with (
            patch.object(loaders, "Source", return_value=MemorySource()),
            contextlib.redirect_stdout(io.StringIO()),
        ):
            self.assertEqual(loaders.main([*args, "--check"]), 0)
        self.assertEqual((target.read_bytes(), target.stat().st_mtime_ns), before)
        target.write_text("hand edit", encoding="utf-8")
        with (
            patch.object(loaders, "Source", return_value=MemorySource()),
            contextlib.redirect_stderr(io.StringIO()),
        ):
            self.assertEqual(loaders.main([*args, "--check"]), 1)
        self.assertEqual(target.read_text(), "hand edit")

    def test_cli_rejects_conflicting_selection_without_fetching(self) -> None:
        for extra in (["--object", "AAAA"], ["--ref", "main"], ["--check", "--force"]):
            with self.subTest(extra=extra), patch.object(loaders, "Source") as source:
                with (
                    contextlib.redirect_stderr(io.StringIO()),
                    self.assertRaises(SystemExit),
                ):
                    loaders.main(
                        [
                            "--profile",
                            str(self.profile_path),
                            "--cpp-output",
                            str(self.root),
                            *extra,
                        ]
                    )
                source.assert_not_called()

    def test_cli_reports_bad_profile_without_traceback(self) -> None:
        self.profile_path.write_text("{invalid", encoding="utf-8")
        with (
            contextlib.redirect_stderr(io.StringIO()) as stderr,
            self.assertRaises(SystemExit) as error,
        ):
            loaders.main(
                [
                    "--profile",
                    str(self.profile_path),
                    "--cpp-output",
                    str(self.root),
                ]
            )
        self.assertEqual(error.exception.code, 1)
        self.assertNotIn("Traceback", stderr.getvalue())


if __name__ == "__main__":
    unittest.main()
