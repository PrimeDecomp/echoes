#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# dependencies = []
# ///
"""Generate Echoes SLdr types, readers, and LoadX factory adapters.

Usage:
```sh
uv run --script scripts/generate_script_loaders.py --header-output build/loader-headers --cpp-output build/loader-sources
uv run --script scripts/generate_script_loaders.py --ref <commit-sha> --object Counter --object STAU --header-output build/loader-headers
```

Or use a local checkout, with no network access:

```sh
uv run --script scripts/generate_script_loaders.py --templates ../retro-script-object-templates --header-output include/MetroidPrime/ScriptLoader
uv run --script scripts/generate_script_loaders.py --templates ../retro-script-object-templates/MP2 --object Counter --header-output build/loader-headers
```
"""

from __future__ import annotations

import argparse
import copy
import io
import json
import math
import re
import sys
import urllib.error
import urllib.parse
import urllib.request
import xml.etree.ElementTree as ET
import zipfile
from collections.abc import Sequence
from dataclasses import dataclass, field, replace
from pathlib import Path, PurePosixPath
from textwrap import dedent
from typing import Protocol

REPOSITORY = "PrimeDecomp/retro-script-object-templates"
KEYWORDS: set[str] = {
    "alignas",
    "alignof",
    "and",
    "asm",
    "auto",
    "bitand",
    "bitor",
    "bool",
    "break",
    "case",
    "catch",
    "char",
    "class",
    "compl",
    "const",
    "constexpr",
    "const_cast",
    "continue",
    "decltype",
    "default",
    "delete",
    "do",
    "double",
    "dynamic_cast",
    "else",
    "enum",
    "explicit",
    "export",
    "extern",
    "false",
    "float",
    "for",
    "friend",
    "goto",
    "if",
    "inline",
    "int",
    "long",
    "mutable",
    "namespace",
    "new",
    "noexcept",
    "not",
    "nullptr",
    "operator",
    "or",
    "private",
    "protected",
    "public",
    "register",
    "reinterpret_cast",
    "return",
    "short",
    "signed",
    "sizeof",
    "static",
    "static_assert",
    "static_cast",
    "struct",
    "switch",
    "template",
    "this",
    "thread_local",
    "throw",
    "true",
    "try",
    "typedef",
    "typeid",
    "typename",
    "union",
    "unsigned",
    "using",
    "virtual",
    "void",
    "volatile",
    "wchar_t",
    "while",
    "xor",
}


@dataclass(frozen=True)
class Primitive:
    cpp_type: str
    read_expression: str
    header: str | None = None


PRIMITIVES: dict[str, Primitive] = {
    "Bool": Primitive("bool", "input.ReadBool()"),
    "Short": Primitive("short", "input.ReadInt16()"),
    "Int": Primitive("int", "input.ReadInt32()"),
    "Choice": Primitive("int", "input.ReadInt32()"),
    "Flags": Primitive("uint", "uint(input.ReadInt32())"),
    "Sound": Primitive("int", "input.ReadInt32()"),
    "Float": Primitive("float", "input.ReadFloat()"),
    "Asset": Primitive(
        "CAssetId", "CAssetId(input.ReadInt32())", "Kyoto/SObjectTag.hpp"
    ),
    "String": Primitive("rstl::string", "rstl::string(input)", "rstl/string.hpp"),
    "Vector": Primitive("CVector3f", "CVector3f(input)", "Kyoto/Math/CVector3f.hpp"),
    "Color": Primitive("CColor", "CColor(input)", "Kyoto/Graphics/CColor.hpp"),
    "Spline": Primitive(
        "SLdrSpline", "SLdrSpline(input, propertySize)", "Kyoto/Math/CMayaSpline.hpp"
    ),
}


class TemplateError(ValueError):
    pass


def identifier(text: str, fallback: str = "unknown") -> str:
    name = re.sub(r"[^A-Za-z0-9_]", "", text)
    if not name:
        name = fallback
    if name[0].isdigit():
        name = "field_" + name
    if name in KEYWORDS or name.startswith("_"):
        name = "field_" + name
    return name


def integer(text: str) -> int:
    text = text.strip()
    return int(text, 16 if text.lower().startswith(("0x", "-0x")) else 10)


def property_id(node: ET.Element) -> int:
    value = integer(node.attrib["ID"])
    if not 0 <= value <= 0xFFFFFFFF:
        raise TemplateError("Property ID outside uint32: " + node.attrib["ID"])
    return value


def float_literal(text: str) -> str:
    value = float(text)
    if not math.isfinite(value):
        raise TemplateError("Non-finite default: " + text)
    result = text.strip().lower()
    if "." not in result and "e" not in result:
        result += ".0"
    return result + "f"


class TemplateSource(Protocol):
    description: str

    def read(self, path: str) -> bytes: ...

    def xml(self, path: str) -> ET.Element: ...


class Source:
    """One local tree or one remote archive snapshot; never extract archive paths."""

    def __init__(self, local: str | Path | None = None, ref: str = "main") -> None:
        self.root: Path | None = None
        self.files: dict[str, bytes] = {}
        if local is not None:
            root = Path(local).resolve()
            if root.name == "Game.xml":
                root = root.parent
            if root.name == "MP2":
                root = root.parent
            self.root = root
            self.description = str(root)
        else:
            self.files = self._download_snapshot(ref)
            self.description = "https://github.com/" + REPOSITORY + "/tree/" + ref

    @staticmethod
    def _download_snapshot(ref: str) -> dict[str, bytes]:
        url = f"https://codeload.github.com/{REPOSITORY}/zip/{urllib.parse.quote(ref, safe='')}"
        request = urllib.request.Request(
            url, headers={"User-Agent": "echoes-loader-generator"}
        )
        with urllib.request.urlopen(request, timeout=60) as response:
            archive = response.read()

        files: dict[str, bytes] = {}
        with zipfile.ZipFile(io.BytesIO(archive)) as bundle:
            for entry in bundle.infolist():
                path = PurePosixPath(entry.filename)
                relative = PurePosixPath(*path.parts[1:]).as_posix()
                if relative in {"PropertyMap.xml", "LICENSE"} or (
                    relative.startswith("MP2/") and relative.endswith(".xml")
                ):
                    files[relative] = bundle.read(entry)
        return files

    def read(self, path: str) -> bytes:
        relative = PurePosixPath(path)
        if (
            relative.is_absolute()
            or ".." in relative.parts
            or "\\" in path
            or ":" in path
        ):
            raise TemplateError("Unsafe template path: " + path)
        if self.root is not None:
            target = (self.root / path).resolve()
            if self.root not in target.parents:
                raise TemplateError("Template escapes source tree: " + path)
            return target.read_bytes()
        if path not in self.files:
            raise TemplateError("Missing template: " + path)
        return self.files[path]

    def xml(self, path: str) -> ET.Element:
        return ET.fromstring(self.read(path))


def merge(base: ET.Element, override: ET.Element) -> ET.Element:
    """Archetype overrides patch child IDs without changing the inherited order."""
    result = copy.deepcopy(base)
    result.tag = override.tag
    result.attrib.update(override.attrib)
    for child in override:
        old = result.find(child.tag)
        if child.tag == "SubProperties" and old is not None:
            by_id = {property_id(item): item for item in old}
            for item in child:
                pid = property_id(item)
                if pid not in by_id:
                    raise TemplateError(
                        f"Override introduces unknown property 0x{pid:08x}"
                    )
                position = list(old).index(by_id[pid])
                patched = merge(by_id[pid], item)
                old.remove(by_id[pid])
                old.insert(position, patched)
                by_id[pid] = patched
        else:
            if old is not None:
                result.remove(old)
            result.append(copy.deepcopy(child))
    return result


@dataclass(frozen=True)
class Field:
    name: str
    node: ET.Element
    cpp: str
    dependency: str | None = None
    item: Field | None = None


@dataclass
class Struct:
    name: str
    node: ET.Element
    path: str
    is_object: bool
    fields: list[Field] = field(default_factory=list)
    atomic: bool = False
    scalar: bool = False


@dataclass(frozen=True)
class Loader:
    name: str
    cpp_type: str
    fourccs: tuple[str, ...]


class Generator:
    def __init__(self, source: TemplateSource) -> None:
        self.source: TemplateSource = source
        game = source.xml("MP2/Game.xml")
        if game.get("Game") != "Echoes":
            raise TemplateError("MP2/Game.xml is not an Echoes template")
        self.archetypes: dict[str, str] = self.index(game, "PropertyArchetypes")
        self.objects: dict[str, str] = self.index(game, "ScriptObjects")
        self.names: dict[tuple[int, str], str] = {}
        # PropertyMap is keyed by both ID and type (the same ID may have several names).
        for entry in source.xml("PropertyMap.xml").findall("PropertyMap/Element"):
            key, value = entry.find("Key"), entry.find("Value")
            if key is None or value is None:
                raise TemplateError("PropertyMap entry lacks Key or Value")
            self.names[(property_id(key), key.attrib["Type"].lower())] = value.attrib[
                "Name"
            ]
        self.raw: dict[str, ET.Element] = {}
        self.resolved: dict[str, ET.Element] = {}
        self.structs: dict[str, Struct] = {}
        self.loading: set[str] = set()
        self.loaders: list[Loader] = []
        self.uses_animation_parameters = False
        self.header_owners: dict[str, str] = {}

    @staticmethod
    def index(game: ET.Element, section: str) -> dict[str, str]:
        result: dict[str, str] = {}
        for entry in game.findall(section + "/Element"):
            key = entry.findtext("Key")
            value = entry.find("Value")
            if key is None or value is None:
                raise TemplateError("Malformed " + section + " entry")
            if key in result:
                raise TemplateError("Duplicate " + section + " key: " + key)
            result[key] = "MP2/" + value.attrib["Path"]
        return result

    def archetype(self, name: str, trail: tuple[str, ...] = ()) -> ET.Element:
        if name in trail:
            raise TemplateError("Archetype cycle: " + " -> ".join(trail + (name,)))
        if name not in self.archetypes:
            raise TemplateError("Unknown archetype: " + name)
        if name not in self.raw:
            raw = self.source.xml(self.archetypes[name]).find("PropertyArchetype")
            if raw is None:
                raise TemplateError("Missing PropertyArchetype: " + name)
            self.raw[name] = raw
        if name not in self.resolved:
            self.resolved[name] = self.resolve(self.raw[name], trail + (name,))
        return copy.deepcopy(self.resolved[name])

    def resolve(
        self, node: ET.Element | None, trail: tuple[str, ...] = ()
    ) -> ET.Element:
        if node is None:
            raise TemplateError("Missing Properties/PropertyArchetype")
        name = node.get("Archetype")
        if name:
            base = self.archetype(name, trail)
            if node.get("Type") != base.get("Type"):
                raise TemplateError("Archetype type mismatch: " + name)
            # An archetype's type name is not the instance's property name.
            inherited_name = base.find("Name")
            if node.find("Name") is None and inherited_name is not None:
                base.remove(inherited_name)
            node = merge(base, node)
        else:
            node = copy.deepcopy(node)
        children = node.find("SubProperties")
        if children is not None:
            for index, child in enumerate(list(children)):
                children.remove(child)
                children.insert(index, self.resolve(child, trail))
        return node

    def member_name(self, node: ET.Element) -> str:
        pid = property_id(node) if "ID" in node.attrib else 0
        name = node.findtext("Name")
        if not name:
            kind = node.get("Archetype", node.attrib["Type"]).lower()
            name = self.names.get((pid, kind), "")
        if not name or name.lower().startswith("unknown"):
            return f"unknown_0x{pid:08x}"
        name = identifier(name)
        return identifier(name[0].lower() + name[1:])

    def make_field(self, node: ET.Element, name: str, owner: str) -> Field:
        kind = node.attrib["Type"]
        if kind == "Array":
            item_node = node.find("ItemArchetype")
            if item_node is None:
                raise TemplateError("Array has no ItemArchetype: " + owner)
            item = self.make_field(self.resolve(item_node), "item", owner + "_Item")
            return Field(name, node, "rstl::vector< " + item.cpp + " >", item=item)
        archetype = node.get("Archetype")
        if kind == "Struct" or archetype:
            if archetype:
                cpp = "SLdr" + identifier(archetype)
                self.add_struct(
                    cpp, self.archetype(archetype), self.archetypes[archetype]
                )
            else:
                cpp = "SLdr" + identifier(owner + "_" + name)
                self.add_struct(cpp, node, "inline " + owner)
            return Field(name, node, cpp, cpp)
        if kind == "AnimationSet":
            self.uses_animation_parameters = True
            return Field(
                name, node, "SLdrAnimationParameters", "SLdrAnimationParameters"
            )
        if kind not in PRIMITIVES:
            raise TemplateError("Unsupported property type " + kind + " in " + owner)
        return Field(name, node, PRIMITIVES[kind].cpp_type)

    def add_struct(
        self, name: str, node: ET.Element, path: str, is_object: bool = False
    ) -> None:
        if name in self.loading:
            raise TemplateError("Recursive by-value type: " + name)
        if name in self.structs:
            if self.structs[name].path != path:
                raise TemplateError("C++ type name collision: " + name)
            return
        self.loading.add(name)
        kind = node.attrib["Type"]
        struct = Struct(
            name,
            node,
            path,
            is_object=is_object,
            atomic=node.findtext("Atomic") == "true",
            scalar=kind != "Struct",
        )
        if struct.scalar:
            if kind not in PRIMITIVES:
                raise TemplateError("Unsupported scalar archetype: " + kind)
            struct.fields.append(Field("value", node, PRIMITIVES[kind].cpp_type))
        else:
            seen_ids: set[int] = set()
            seen_names: set[str] = set()
            for child in node.findall("SubProperties/Element"):
                pid = property_id(child)
                if pid in seen_ids:
                    raise TemplateError("Duplicate property ID in " + name)
                seen_ids.add(pid)
                member = self.member_name(child)
                if member in seen_names:
                    member += f"_0x{pid:08x}"
                if member in seen_names:
                    raise TemplateError("C++ member collision in " + name)
                seen_names.add(member)
                struct.fields.append(self.make_field(child, member, name[4:]))
        self.loading.remove(name)
        self.structs[name] = struct

    def collect(self, selected: Sequence[str]) -> None:
        paths = sorted(set(self.objects.values()))
        if selected:
            requested = set(selected)
            known = set(self.objects) | {
                PurePosixPath(path).stem for path in self.objects.values()
            }
            if unknown := requested - known:
                raise TemplateError("Unknown objects: " + ", ".join(sorted(unknown)))
            paths = sorted(
                {
                    path
                    for key, path in self.objects.items()
                    if key in requested or PurePosixPath(path).stem in requested
                }
            )
        used: set[str] = set()
        for path in paths:
            node = self.resolve(self.source.xml(path).find("Properties"))
            name = identifier(PurePosixPath(path).stem)
            if name in used:
                raise TemplateError("Object name collision: " + name)
            used.add(name)
            cpp = "SLdr" + name
            self.add_struct(cpp, node, path, is_object=True)
            keys = sorted(key for key, value in self.objects.items() if value == path)
            self.loaders.append(Loader(name, cpp, tuple(keys)))
        self._assign_header_owners()

    @staticmethod
    def _field_dependencies(prop: Field) -> set[str]:
        if prop.item is not None:
            return Generator._field_dependencies(prop.item)
        return {prop.dependency} if prop.dependency else set()

    def _assign_header_owners(self) -> None:
        parents: dict[str, set[str]] = {name: set() for name in self.structs}
        for struct in self.structs.values():
            for prop in struct.fields:
                for dependency in self._field_dependencies(prop):
                    if dependency in parents:
                        parents[dependency].add(struct.name)

        def owner(name: str) -> str:
            if name in self.header_owners:
                return self.header_owners[name]
            struct = self.structs[name]
            parent_names = parents[name]
            if struct.is_object or not parent_names:
                result = name
            else:
                # Several direct users may still share one owning header.
                parent_owners = {owner(parent) for parent in parent_names}
                result = next(iter(parent_owners)) if len(parent_owners) == 1 else name
            self.header_owners[name] = result
            return result

        for name in self.structs:
            owner(name)

    def defaults(self, prop: Field, target: str) -> list[str]:
        kind = prop.node.attrib["Type"]
        if prop.dependency and kind != "AnimationSet":
            struct = self.structs[prop.dependency]
            if struct.scalar:
                inherited = struct.node.find("DefaultValue")
                actual_default = prop.node.find("DefaultValue")
                if actual_default is None or (
                    inherited is not None
                    and ET.tostring(actual_default) == ET.tostring(inherited)
                ):
                    return []
                scalar = Field("value", prop.node, struct.fields[0].cpp)
                return self.defaults(scalar, target + ".value")
            children = {
                property_id(child): child
                for child in prop.node.findall("SubProperties/Element")
            }
            result: list[str] = []
            for member in struct.fields:
                actual = replace(member, node=children[property_id(member.node)])
                if ET.tostring(actual.node) != ET.tostring(member.node):
                    result.extend(self.defaults(actual, target + "." + member.name))
            return result
        default = prop.node.find("DefaultValue")
        if default is None:
            return []  # Value-initialize absent primitive defaults; keep class defaults.
        if kind in ("Array", "Spline", "AnimationSet"):
            raise TemplateError("Unsupported explicit " + kind + " default")
        value = (default.text or "").strip()
        if kind in ("Vector", "Color"):
            components = "XYZ" if kind == "Vector" else "RGBA"
            args = [
                float_literal(
                    default.findtext(component, "1.0" if component == "A" else "0.0")
                )
                for component in components
            ]
            expression = prop.cpp + "(" + ", ".join(args) + ")"
        elif kind == "String":
            expression = (
                "rstl::string("
                + json.dumps(default.text or "", ensure_ascii=True)
                + ")"
            )
        elif kind == "Bool":
            if value not in ("true", "false", "0", "1"):
                raise TemplateError("Invalid bool default: " + value)
            expression = "true" if value in ("true", "1") else "false"
        elif kind == "Float":
            expression = float_literal(value)
        else:
            number = integer(value)
            expression = str(number)
            if kind in ("Asset", "Flags"):
                expression = "0x%08xu" % (number & 0xFFFFFFFF)
        return [target + " = " + expression + ";"]

    def read_field(
        self, prop: Field, target: str, size: str | None = None, depth: int = 0
    ) -> list[str]:
        kind = prop.node.attrib["Type"]
        if prop.dependency:
            return [f"LoadTypedef{prop.cpp}({target}, input);"]
        if kind == "Array":
            return self.read_array(prop, target, depth)
        if kind == "Spline" and size is None:
            raise TemplateError("Spline without a property-size boundary")
        expression = PRIMITIVES[kind].read_expression
        if size is not None:
            expression = expression.replace("propertySize", size)
        return [f"{target} = {expression};"]

    def read_array(self, prop: Field, target: str, depth: int) -> list[str]:
        item_field = prop.item
        if item_field is None:
            raise TemplateError("Array has no item field: " + target)
        count, index, item = (f"{label}{depth}" for label in ("count", "i", "item"))
        lines = [
            f"const int {count} = input.ReadInt32();",
            f"{target}.clear();",
            f"{target}.reserve({count});",
            f"for (int {index} = 0; {index} < {count}; ++{index}) {{",
            f"  {item_field.cpp} {item} = {item_field.cpp}();",
        ]
        lines.extend("  " + line for line in self.defaults(item_field, item))
        lines.extend(
            "  " + line for line in self.read_field(item_field, item, depth=depth + 1)
        )
        lines.extend([f"  {target}.push_back({item});", "}"])
        return ["{", *("  " + line for line in lines), "}"]

    def _include_path_for(self, struct_name: str) -> str:
        if struct_name == "SLdrAnimationParameters":
            return "MetroidPrime/ScriptLoader/Structs/SLdrAnimationParameters.hpp"
        owner = self.header_owners[struct_name]
        prefix = "" if self.structs[owner].is_object else "Structs/"
        return f"MetroidPrime/ScriptLoader/{prefix}{owner}.hpp"

    def includes(self, prop: Field) -> set[str]:
        if prop.item:
            return {"rstl/vector.hpp"} | self.includes(prop.item)

        if prop.dependency:
            return {self._include_path_for(prop.dependency)}

        header = PRIMITIVES[prop.node.attrib["Type"]].header
        return {header} if header else set()

    def render_struct_header(self, struct: Struct) -> str:
        name = struct.name
        members = {
            member.name: member
            for member in self.structs.values()
            if self.header_owners[member.name] == name
        }
        ordered: list[Struct] = []
        visited: set[str] = set()

        def visit(member: Struct) -> None:
            if member.name in visited:
                return
            visited.add(member.name)
            for prop in member.fields:
                for dependency in sorted(self._field_dependencies(prop)):
                    if dependency in members:
                        visit(members[dependency])
            ordered.append(member)

        visit(struct)
        headers = {"Kyoto/Streams/CInputStream.hpp"}
        own_header = self._include_path_for(name)
        for member in ordered:
            for prop in member.fields:
                headers |= self.includes(prop) - {own_header}
        guard = "_" + name.upper() + "_HPP"
        header = ["#ifndef " + guard, "#define " + guard, ""]
        header += ['#include "' + h + '"' for h in sorted(headers)]
        for member in ordered:
            member_name = member.name
            header += [
                "",
                "struct " + member_name + " {",
                "  " + member_name + "();",
                "  ~" + member_name + "();",
                "",
            ]
            for prop in member.fields:
                comment = (
                    f" // 0x{property_id(prop.node):08x}" if not member.scalar else ""
                )
                header.append("  " + prop.cpp + " " + prop.name + ";" + comment)
            header += [
                "};",
                "",
                "void LoadTypedef"
                + member_name
                + "("
                + member_name
                + "& data, CInputStream& input);",
            ]
        header += ["", "#endif", ""]
        return "\n".join(header)

    def render_struct_source(self, struct: Struct) -> str:
        name = struct.name
        initializers = ", ".join(p.name + "()" for p in struct.fields)
        source = [
            f'#include "{self._include_path_for(name)}"',
            "",
            f"{name}::{name}()" + (" : " + initializers if initializers else "") + " {",
        ]
        for prop in struct.fields:
            source += ["  " + line for line in self.defaults(prop, prop.name)]
        source += [
            "}",
            "",
            name + "::~" + name + "() {}",
            "",
            "void LoadTypedef"
            + name
            + "("
            + name
            + "& sldrThis, CInputStream& input) {",
        ]
        source += (
            self.render_sequential_reader(struct)
            if struct.atomic or struct.scalar
            else self.render_tagged_reader(struct)
        )
        source += ["}", ""]
        return "\n".join(source)

    def render_sequential_reader(self, struct: Struct) -> list[str]:
        return [
            "  " + line
            for prop in struct.fields
            for line in self.read_field(prop, "sldrThis." + prop.name)
        ]

    def render_tagged_reader(self, struct: Struct) -> list[str]:
        lines = [
            "  const int propertyCount = input.ReadUint16();",
            "  for (int i = 0; i < propertyCount; ++i) {",
            "    const uint propertyId = input.ReadInt32();",
            "    const u16 propertySize = input.ReadUint16();",
            "    switch (propertyId) {",
        ]
        for prop in struct.fields:
            lines.append(f"    case 0x{property_id(prop.node):08x}: {{")
            lines.extend(
                "      " + line
                for line in self.read_field(
                    prop, "sldrThis." + prop.name, "propertySize"
                )
            )
            lines.extend(["      break;", "    }"])
        lines.extend(
            [
                "    default:",
                "      input.ReadBytes(nullptr, propertySize);",
                "      break;",
                "    }",
                "  }",
            ]
        )
        return lines

    def render_struct(self, struct: Struct) -> tuple[str, str]:
        return self.render_struct_header(struct), self.render_struct_source(struct)

    @staticmethod
    def render_animation_parameters() -> dict[str, str]:
        return {
            "Structs/SLdrAnimationParameters.hpp": dedent("""\
                #ifndef GENERATED_SLDRANIMATIONPARAMETERS_HPP
                #define GENERATED_SLDRANIMATIONPARAMETERS_HPP

                #include "Kyoto/SObjectTag.hpp"
                #include "Kyoto/Streams/CInputStream.hpp"

                struct SLdrAnimationParameters {
                  SLdrAnimationParameters() : ancs(kInvalidAssetId), character_index(0), initial_anim(0) {}
                  CAssetId ancs;
                  int character_index;
                  int initial_anim;
                };

                void LoadTypedefSLdrAnimationParameters(SLdrAnimationParameters&, CInputStream&);

                #endif
                """),
            "Structs/SLdrAnimationParameters.cpp": dedent("""\
                #include "MetroidPrime/ScriptLoader/Structs/SLdrAnimationParameters.hpp"

                void LoadTypedefSLdrAnimationParameters(SLdrAnimationParameters& data, CInputStream& input) {
                  data.ancs = input.ReadInt32();
                  data.character_index = input.ReadInt32();
                  data.initial_anim = input.ReadInt32();
                }
                """),
        }

    def render(self) -> dict[str, str]:
        files: dict[str, str] = {}

        for name, struct in sorted(self.structs.items()):
            prefix = "" if struct.is_object else "Structs/"
            if self.header_owners[name] == name:
                files[f"{prefix}{name}.hpp"] = self.render_struct_header(struct)
            files[f"{prefix}{name}.cpp"] = self.render_struct_source(struct)

        if self.uses_animation_parameters:
            if "SLdrAnimationParameters" in self.structs:
                raise TemplateError(
                    "AnimationParameters collides with built-in wire type"
                )
            files.update(self.render_animation_parameters())

        for name, content in files.items():
            files[name] = (
                "// Generated by scripts/generate_script_loaders.py. Review before integration.\n"
                + content
            )
        files["manifest.json"] = (
            json.dumps(
                {
                    "source": self.source.description,
                    "objects": [
                        {
                            "name": loader.name,
                            "type": loader.cpp_type,
                            "fourccs": loader.fourccs,
                        }
                        for loader in self.loaders
                    ],
                    "files": sorted(files),
                },
                indent=2,
            )
            + "\n"
        )
        files["TEMPLATE-LICENSE"] = (
            self.source.read("LICENSE").decode("utf-8").replace("\r\n", "\n")
        )
        return files


def write_output(
    files: dict[str, str], output: str | Path, force: bool = False
) -> None:
    output = Path(output)

    def normalized(data: bytes) -> bytes:
        return data.replace(b"\r\n", b"\n")

    conflicts = [
        name
        for name in files
        if (output / name).exists()
        and normalized((output / name).read_bytes()) != files[name].encode("utf-8")
    ]
    if conflicts and not force:
        raise TemplateError(
            "Output differs; use a fresh directory or --force: "
            + ", ".join(conflicts[:5])
        )
    output.mkdir(parents=True, exist_ok=True)
    for name, content in sorted(files.items()):
        target = output / name
        data = content.encode("utf-8")
        if target.exists():
            existing = target.read_bytes()
            if normalized(existing) == data:
                continue
            if b"\r\n" in existing:
                data = data.replace(b"\n", b"\r\n")
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--templates",
        type=Path,
        help="local repository root, MP2 directory, or MP2/Game.xml (offline)",
    )
    parser.add_argument(
        "--ref", default="main", help="remote branch, tag or commit (default: main)"
    )
    parser.add_argument(
        "--header-output",
        type=Path,
        help="directory to write generated C++ header files",
    )
    parser.add_argument(
        "--cpp-output", type=Path, help="directory to write generated C++ files"
    )
    parser.add_argument(
        "--object",
        action="append",
        default=[],
        help="object filename stem or FourCC; repeat to select several",
    )
    parser.add_argument(
        "--force",
        action="store_true",
        help="replace differing generated files in the output directory",
    )
    args = parser.parse_args(argv)
    if not args.header_output and not args.cpp_output:
        parser.error("provide --header-output and/or --cpp-output")
    try:
        generator = Generator(Source(args.templates, args.ref))
        generator.collect(args.object)
        files = generator.render()
        if args.header_output:
            write_output(
                {
                    name: content
                    for name, content in files.items()
                    if name.endswith(".hpp")
                },
                args.header_output,
                args.force,
            )
        if args.cpp_output:
            write_output(
                {
                    name: content
                    for name, content in files.items()
                    if name.endswith(".cpp")
                },
                args.cpp_output,
                args.force,
            )
    except (
        ET.ParseError,
        zipfile.BadZipFile,
        urllib.error.URLError,
    ) as error:
        parser.exit(1, "error: " + str(error) + "\n")
    destinations = [
        str(path) for path in (args.header_output, args.cpp_output) if path is not None
    ]
    print(f"Generated {len(generator.structs)} types in {', '.join(destinations)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
