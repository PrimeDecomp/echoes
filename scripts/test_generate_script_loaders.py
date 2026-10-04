"""Offline regression checks for native instance XML defaults."""

import unittest
import xml.etree.ElementTree as ET

from generate_script_loaders import Generator, TemplateError


class MemorySource:
    description = "in-memory fixtures"

    def __init__(self, value="3", name="Flags"):
        self.files = {
            "LICENSE": "Fixture license\n",
            "MP2/Game.xml": """
                <Game Game="Echoes">
                  <PropertyArchetypes><Element><Key>EditorProperties</Key>
                    <Value Path="Editor.xml"/></Element></PropertyArchetypes>
                  <ScriptObjects>
                    <Element><Key>SOUN</Key><Value Path="Sound.xml"/></Element>
                    <Element><Key>OTHR</Key><Value Path="Other.xml"/></Element>
                  </ScriptObjects>
                </Game>""",
            "PropertyMap.xml": "<Root><PropertyMap/></Root>",
            "MP2/Editor.xml": f"""
                <PropertyTemplate><PropertyArchetype Type="Struct">
                  <SubProperties><Element Type="Flags" ID="0x5D298A43">
                    <Name>{name}</Name><DefaultValue>{value}</DefaultValue>
                  </Element></SubProperties>
                </PropertyArchetype></PropertyTemplate>""",
        }
        for object_name in ("Sound", "Other"):
            self.files[f"MP2/{object_name}.xml"] = """
                <ScriptObject><Properties Type="Struct"><SubProperties>
                  <Element Type="Struct" ID="0x255A4580" Archetype="EditorProperties">
                    <Name>Editor</Name>
                  </Element>
                </SubProperties></Properties></ScriptObject>"""

    def read(self, path):
        return self.files[path].encode("utf-8")

    def xml(self, path):
        return ET.fromstring(self.read(path))


class NativeInstanceDefaultsTests(unittest.TestCase):
    def test_uses_xml_value_and_member_names(self):
        generator = Generator(MemorySource(value="9", name="RenamedFlags"))
        generator.collect([])
        files = generator.render()
        self.assertIn("editor.renamedFlags = 0x00000009u;", files["SLdrSound.hpp"])
        self.assertNotIn("editor.renamedFlags =", files["SLdrOther.hpp"])
        self.assertIn("renamedFlags = 0x00000009u;", files["Structs/SLdrEditorProperties.cpp"])

    def test_annotations_are_isolated_to_instance_xml(self):
        source = MemorySource()
        original = dict(source.files)
        generator = Generator(source)
        generator.collect([])
        default_path = "SubProperties/Element/SubProperties/Element/DefaultValue"
        self.assertEqual(
            generator.structs["SLdrSound"].node.find(default_path).get("NativeInstance"),
            "true",
        )
        self.assertIsNone(
            generator.structs["SLdrOther"].node.find(default_path).get("NativeInstance")
        )
        self.assertIsNone(
            generator.archetype("EditorProperties")
            .find("SubProperties/Element/DefaultValue")
            .get("NativeInstance")
        )
        self.assertEqual(source.files, original)
        second = Generator(source)
        second.collect([])
        self.assertEqual(generator.render(), second.render())

    def test_missing_property_is_rejected(self):
        node = ET.fromstring('<Properties Type="Struct"><SubProperties/></Properties>')
        with self.assertRaisesRegex(TemplateError, "Missing native instance property"):
            Generator.apply_native_instance_defaults("SLdrSound", node)

    def test_missing_default_is_rejected(self):
        source = MemorySource()
        source.files["MP2/Editor.xml"] = source.files["MP2/Editor.xml"].replace(
            "<DefaultValue>3</DefaultValue>", ""
        )
        with self.assertRaisesRegex(TemplateError, "Missing native instance default"):
            Generator(source).collect([])


if __name__ == "__main__":
    unittest.main()
