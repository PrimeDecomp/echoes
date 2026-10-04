#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# dependencies = []
# ///
"""Generate Echoes SLdr headers and readers from XML templates.

Generate into a staging directory and review the diff before replacing tracked files.
Make corrections in the generator or templates so regeneration preserves them.
The Tweaks profile pins the templates and selects the module's sources.
Register new translation units in configure.py.

Records listed in a profile, and the shared Structs/ typedefs, have out-of-line
constructors, destructors and LoadTypedef readers in their own source. Every other
script object is header-only: its constructor is inline and its tagged property loop
is an includable SLdrX.inc fragment for the hand-written LoadX in the object's source:

    SLdrWaypoint sldrThis;
    #include "MetroidPrime/ScriptLoader/SLdrWaypoint.inc"
    return rs_new CScriptWaypoint(...);

Type and reader names follow the Echoes Wii SEL exports (LoadTypedefEditorProperties
taking SLdrEditorProperties&, SLdrAnimationSet).
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
from binascii import crc32
from collections.abc import Sequence
from dataclasses import dataclass, field, replace
from itertools import groupby
from pathlib import Path, PurePosixPath
from textwrap import dedent
from typing import Protocol

REPOSITORY = "PrimeDecomp/retro-script-object-templates"
PROFILE_DIRECTORY = Path(__file__).resolve().parent.parent / "config" / "loader_profiles"

# The XML labels these as PAL additions. G2ME01's native record has nine icons;
# retain the existing layout for other builds until their binaries are checked.
G2ME01_ABSENT_PROPERTIES: dict[str, frozenset[int]] = {
    "SLdrTweakPlayerRes_AutoMapperIcons": frozenset(
        {0x5096BFA5, 0xF4E6E0EB, 0x65700CCC, 0xA0D73242, 0x5291EB5F}
    ),
}

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


def is_matching(name: str, hash_value: int) -> bool:
    return (crc32(name.encode()) ^ 0xFFFFFFFF) == hash_value



def is_invalid_sound(prop: "Field") -> bool:
    """A Sound defaulting to -1 is constructed in the initializer list."""
    if prop.node.attrib["Type"] != "Sound":
        return False
    default = prop.node.find("DefaultValue")
    return default is not None and (default.text or "").strip() == "-1"


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
# Enumeration archetypes that G2ME01 wraps in a record with its own constructor,
# destructor and reader (SLdrPlayerItem: 0x8023E0C4, 0x8023E0DC, 0x8023E118).
ENUM_RECORDS = frozenset({"PlayerItem"})

# G2ME01 CTweakGame's indexed getters at 80216CD0..80216D20 access these
# homogeneous records as five-element arrays; retain each XML property ID.
NATIVE_INDEXED_RECORDS = {
    "SLdrTweakGame_FragLimitChoices": ("fragLimit", "int"),
    "SLdrTweakGame_TimeLimitChoices": ("timeLimit", "float"),
    "SLdrTweakGame_CoinLimitChoices": ("coinLimit", "int"),
}

# Nested members a native constructor re-stores with their archetype default, so
# the templates have nothing to override (SLdrPickup::SLdrPickup, G2ME01 0x800B40B4).
# Record -> property-ID paths to re-store. Values and C++ names come from XML.
NATIVE_INSTANCE_DEFAULTS: dict[str, tuple[tuple[int, ...], ...]] = {
    # SLdrAmbientAI, G2ME01 0x801795D8: two actor defaults, no editor re-store.
    "SLdrAmbientAI": (
        (0x7E397FED, 0xB028DB0E, 0xA33E5B0E),
        (0x7E397FED, 0x05AD250E, 0xCA19E8C6),
    ),
    "SLdrWorldTeleporter": ((0x255A4580, 0x5D298A43),),
    "SLdrControllerAction": ((0x4C6EEFAE, 0x94BA5737),),
    "SLdrTriggerEllipsoid": ((0x255A4580, 0x5D298A43),),
    "SLdrPlayerHint": ((0x255A4580, 0x5D298A43),),
    "SLdrSound": ((0x255A4580, 0x5D298A43),),
    # SLdrPlatform, G2ME01 0x8009FE84: editor, ambient color and visor re-stores.
    "SLdrPlatform": (
        (0x255A4580, 0x5D298A43),
        (0x7E397FED, 0xB028DB0E, 0xA33E5B0E),
        (0x7E397FED, 0x05AD250E, 0xCA19E8C6),
    ),
    "SLdrPickup": (
        (0x255A4580, 0x5D298A43),
        (0x7E397FED, 0xB028DB0E, 0xA33E5B0E),
        (0x7E397FED, 0x05AD250E, 0xCA19E8C6),
    ),
}

# An enumeration without a template default is still assigned zero in the body.
ZERO_DEFAULT_KINDS = frozenset({"Choice"})
pwe_type_lookup = {
    "Bool": "bool",
    "Short": "short",
    "Int": "int",
    "Choice": "choice",
    "Sound": "sound",
    "Float": "float",
    "Asset": "asset",
    "String": "string",
    "Spline": "spline",
    "Array": "array",
    "Guid": "guid",
    "Enum": "enum",
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

    def __init__(
        self,
        local: str | Path | None = None,
        ref: str = "main",
    ) -> None:
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
    matching_name: bool | None
    dependency: str | None = None
    item: Field | None = None
    condition: str | None = None


def conditional_lines(chunks: Sequence[tuple[str | None, list[str]]]) -> list[str]:
    """Group adjacent fields under their shared build condition."""
    result: list[str] = []
    nonempty = (chunk for chunk in chunks if chunk[1])
    for condition, group in groupby(nonempty, key=lambda chunk: chunk[0]):
        if condition:
            result.append("#if " + condition)
        for _, lines in group:
            result.extend(lines)
        if condition:
            result.append("#endif")
    return result


@dataclass
class Struct:
    name: str
    node: ET.Element
    path: str
    is_object: bool
    fields: list[Field] = field(default_factory=list)
    atomic: bool = False


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
        # Records with their own source; every other object record is header-only.
        self.sourced: set[str] = set()

    def is_inline(self, name: str) -> bool:
        if name in self.sourced:
            return False
        return self.structs[self.header_owners[name]].is_object

    @staticmethod
    def loader_name(cpp: str) -> str:
        """Native readers are named after the template, not the SLdr record."""
        return "LoadTypedef" + cpp.removeprefix("SLdr")

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

    def raw_name(self, node: ET.Element) -> str | None:
        pid = property_id(node) if "ID" in node.attrib else 0
        name = node.findtext("Name")
        if not name:
            archetype = node.get("Archetype", "").lower()
            name = self.names.get((pid, archetype))
            if name:
                return name

            kind = node.attrib["Type"].lower()
            name = self.names.get((pid, kind))
        return name

    def member_name(self, node: ET.Element) -> str:
        pid = property_id(node) if "ID" in node.attrib else 0
        name = self.raw_name(node) or ""
        if not name or name.lower().startswith("unknown"):
            return f"unknown_0x{pid:08x}"
        name = identifier(name)
        return identifier(name[0].lower() + name[1:])

    def _get_other_type(self, node: ET.Element) -> str | None:
        pid = property_id(node)
        all_types = [ptype for prop_id, ptype in self.names if prop_id == pid]
        if len(all_types) > 1:
            return all_types[-1]
        return None

    def make_field(self, node: ET.Element, name: str, owner: str) -> Field:
        kind: str = node.attrib["Type"]
        archetype = node.get("Archetype")

        matching_name = False

        raw_name = self.raw_name(node)

        if raw_name is not None and not raw_name.lower().startswith("unknown"):
            matching_type_name = self._get_other_type(node) or kind
            if matching_type_name in pwe_type_lookup:
                matching_type_name = pwe_type_lookup[matching_type_name]
            elif archetype:
                matching_type_name = archetype

            if matching_type_name is not None:
                hashable_name = f"{raw_name}{matching_type_name}"
                matching_name = is_matching(hashable_name, property_id(node))
        else:
            matching_name = None

        if owner == "EditorProperties":
            matching_name = None

        if kind == "Array":
            item_node = node.find("ItemArchetype")
            if item_node is None:
                raise TemplateError("Array has no ItemArchetype: " + owner)
            item = self.make_field(self.resolve(item_node), "item", owner + "_Item")
            return Field(
                name,
                node,
                "rstl::vector< " + item.cpp + " >",
                item=item,
                matching_name=matching_name,
            )

        if kind == "Struct":
            if archetype:
                cpp = "SLdr" + identifier(archetype)
                self.add_struct(
                    cpp, self.archetype(archetype), self.archetypes[archetype]
                )
            else:
                cpp = "SLdr" + identifier(owner + "_" + name)
                self.add_struct(cpp, node, "inline " + owner)
            return Field(name, node, cpp, matching_name, cpp)

        if kind == "Choice" and archetype in ENUM_RECORDS:
            cpp = "SLdr" + identifier(archetype)
            record = ET.fromstring(
                '<Element Type="Struct"><Atomic>true</Atomic><SubProperties>'
                '<Element Type="Choice" ID="0x0"><Name>Value</Name>'
                "<DefaultValue>0</DefaultValue></Element></SubProperties></Element>"
            )
            self.header_owners[cpp] = cpp
            self.add_struct(cpp, record, self.archetypes[archetype])
            return Field(name, node, cpp, matching_name, cpp)

        if kind == "AnimationSet":
            self.uses_animation_parameters = True
            return Field(
                name,
                node,
                "SLdrAnimationSet",
                matching_name,
                "SLdrAnimationSet",
            )
        if kind not in PRIMITIVES:
            raise TemplateError("Unsupported property type " + kind + " in " + owner)

        return Field(name, node, PRIMITIVES[kind].cpp_type, matching_name=matching_name)

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
        if node.attrib["Type"] != "Struct":
            raise TemplateError("Expected a struct template: " + path)
        struct = Struct(
            name,
            node,
            path,
            is_object=is_object,
            atomic=node.findtext("Atomic") == "true",
        )
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
            prop = self.make_field(child, member, name[4:])
            if pid in G2ME01_ABSENT_PROPERTIES.get(name, frozenset()):
                prop = replace(prop, condition="VERSION != VERSION_G2ME01")
            struct.fields.append(prop)
        if name in NATIVE_INDEXED_RECORDS:
            base, cpp = NATIVE_INDEXED_RECORDS[name]
            if (
                [prop.name for prop in struct.fields]
                != [base + str(i) for i in range(5)]
                or any(
                    prop.cpp != cpp
                    or prop.dependency
                    or prop.condition
                    or prop.node.find("DefaultValue") is None
                    for prop in struct.fields
                )
            ):
                raise TemplateError("Indexed record shape changed: " + name)
            struct.fields = [
                replace(prop, name=f"{base}s[{i}]")
                for i, prop in enumerate(struct.fields)
            ]
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
            self.apply_native_instance_defaults(cpp, node)
            self.add_struct(cpp, node, path, is_object=True)
            keys = sorted(key for key, value in self.objects.items() if value == path)
            self.loaders.append(Loader(name, cpp, tuple(keys)))
        self._assign_header_owners()

    @staticmethod
    def apply_native_instance_defaults(name: str, node: ET.Element) -> None:
        """Mark resolved instance XML defaults without changing shared archetypes."""
        for path in NATIVE_INSTANCE_DEFAULTS.get(name, ()):
            prop = node
            for pid in path:
                children = {
                    property_id(child): child
                    for child in prop.findall("SubProperties/Element")
                }
                if pid not in children:
                    raise TemplateError(
                        f"Missing native instance property 0x{pid:08x} in {name}"
                    )
                prop = children[pid]
            default = prop.find("DefaultValue")
            if default is None:
                raise TemplateError(f"Missing native instance default in {name}: {path}")
            # A generator-only annotation makes even an equal archetype default an
            # instance override for defaults(), preserving its native re-store.
            default.set("NativeInstance", "true")

    def add_duplicates(self, duplicates: dict[str, str]) -> None:
        """Reconstruct duplicate native root records without duplicating helpers."""
        for name, original in duplicates.items():
            if name in self.structs:
                raise TemplateError("Duplicate record name collision: " + name)
            struct = self.structs.get(original)
            if struct is None or not struct.is_object or original in duplicates:
                raise TemplateError("Duplicate source must be an object: " + original)
            self.structs[name] = replace(struct, name=name)
            self.header_owners[name] = self.header_owners[original]

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
        if prop.dependency and kind == "Choice":
            return []  # The enumeration record's constructor owns the default.
        if prop.dependency and kind != "AnimationSet":
            struct = self.structs[prop.dependency]
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
        if is_invalid_sound(prop):
            return []  # Initialized to -1 in the initializer list.
        default = prop.node.find("DefaultValue")
        if default is None:
            if kind in ZERO_DEFAULT_KINDS:
                return [target + " = 0;"]
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
            if kind == "Vector" and all(float(arg[:-1]) == 0.0 for arg in args):
                return []  # Already CVector3f::Zero() from the initializer list.
            if kind == "Color" and all(float(arg[:-1]) == 0.0 for arg in args):
                # The native constructor leaves the CColor::Green() placeholder in place
                # (SLdrDistanceFog::SLdrDistanceFog, G2ME01 0x800FF648).
                return []
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
            return [f"{self.loader_name(prop.cpp)}({target}, input);"]
        if kind == "Array":
            return self.read_array(prop, target, depth)
        if kind == "String" and size is not None:
            # Tagged properties provide a case scope for the owning string local.
            return ["const rstl::string value(input);", f"{target} = value;"]
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
        if struct_name == "SLdrAnimationSet":
            return "MetroidPrime/ScriptLoader/Structs/SLdrAnimationSet.hpp"
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
        for member in members.values():
            visit(member)
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
            inline = self.is_inline(member_name)
            header += [
                "",
                "struct " + member_name + " {",
                "  " + member_name + "();",
                "  ~" + member_name + "();",
                "",
            ]
            declarations: list[tuple[str | None, list[str]]] = []
            for prop in member.fields:
                if member_name in NATIVE_INDEXED_RECORDS:
                    if prop != member.fields[0]:
                        continue
                    base, cpp = NATIVE_INDEXED_RECORDS[member_name]
                    declarations.append(
                        (None, [f"  {cpp} {base}s[5]; // Guessed member name."])
                    )
                    continue
                comment_entries = []
                if not member.atomic:
                    if prop.matching_name is False:
                        comment_entries.append("non-matching name")
                    comment_entries.append(f"0x{property_id(prop.node):08x}")

                comment = " // " + ", ".join(comment_entries) if comment_entries else ""
                declarations.append(
                    (
                        prop.condition,
                        ["  " + prop.cpp + " " + prop.name + ";" + comment],
                    )
                )
            header += conditional_lines(declarations)
            header += ["};"]
            if not inline:
                header += [
                    "",
                    "void "
                    + self.loader_name(member_name)
                    + "("
                    + member_name
                    + "& data, CInputStream& input);",
                ]
            else:
                header += [""] + self.render_definitions(member, "inline ")
        header += ["", "#endif", ""]
        return "\n".join(header)

    def render_fragment(self, struct: Struct) -> str:
        """Tagged property loop for inclusion in the hand-written object loader."""
        return "\n".join(
            [
                f"// Include in the object loader after `{struct.name} sldrThis;`.",
                *self.render_tagged_reader(struct, "sldrThis", indent=""),
                "",
            ]
        )

    def render_struct_source(self, struct: Struct) -> str:
        return "\n".join(
            [
                f'#include "{self._include_path_for(struct.name)}"',
                "",
                *self.render_definitions(struct, ""),
                "",
            ]
        )

    def render_definitions(self, struct: Struct, prefix: str) -> list[str]:
        name = struct.name
        parameter = "data" if struct.is_object else "sldrThis"
        initializers: list[tuple[str | None, str]] = []
        for prop in struct.fields:
            kind = prop.node.attrib["Type"]
            value = ""
            if not prop.dependency:
                if kind == "Color":
                    value = "CColor::Green()"
                elif kind == "Vector":
                    value = "CVector3f::Zero()"
                elif kind == "Asset" and prop.node.find("DefaultValue") is None:
                    value = "kInvalidAssetId"
                elif is_invalid_sound(prop):
                    value = "-1"
                elif kind not in ("String", "Array", "Spline") and (
                    prop.node.find("DefaultValue") is not None
                    or kind in ZERO_DEFAULT_KINDS
                ):
                    # Native generated constructors assign scalar defaults once.
                    continue
            initializers.append((prop.condition, prop.name + "(" + value + ")"))
        source: list[str] = []
        if any(condition for condition, _ in initializers):
            source.append(f"{prefix}{name}::{name}()")
            source += conditional_lines(
                [
                    (condition, [(": " if i == 0 else ", ") + initializer])
                    for i, (condition, initializer) in enumerate(initializers)
                ]
            )
            source.append("{")
        else:
            source.append(
                f"{prefix}{name}::{name}()"
                + (
                    " : " + ", ".join(value for _, value in initializers)
                    if initializers
                    else ""
                )
                + " {"
            )
        source += conditional_lines(
            [
                (
                    prop.condition,
                    [
                        "  " + line
                        for line in self.defaults(prop, prop.name)
                    ],
                )
                for prop in struct.fields
            ]
        )
        source += [
            "}",
            "",
            prefix + name + "::~" + name + "() {}",
        ]
        if prefix and struct.is_object:
            return source  # The property loop is the includable fragment.
        source += [
            "",
            f"{prefix}void "
            + self.loader_name(name)
            + "("
            + name
            + f"& {parameter}, CInputStream& input) {{",
        ]
        source += (
            self.render_sequential_reader(struct, parameter)
            if struct.atomic
            else self.render_tagged_reader(struct, parameter)
        )
        source += ["}"]
        return source

    def render_sequential_reader(self, struct: Struct, parameter: str) -> list[str]:
        return conditional_lines(
            [
                (
                    prop.condition,
                    [
                        "  " + line
                        for line in self.read_field(prop, parameter + "." + prop.name)
                    ],
                )
                for prop in struct.fields
            ]
        )

    def render_tagged_reader(
        self, struct: Struct, parameter: str, indent: str = "  "
    ) -> list[str]:
        lines = [
            "  const int propertyCount = input.ReadUint16();",
            "  for (int i = 0; i < propertyCount; ++i) {",
            "    const uint propertyId = input.Get< uint >();",
            "    const u16 propertySize = input.ReadUint16();",
            "    switch (propertyId) {",
        ]
        cases: list[tuple[str | None, list[str]]] = []
        for prop in struct.fields:
            case = [f"    case 0x{property_id(prop.node):08x}: {{"]
            case.extend(
                "      " + line
                for line in self.read_field(
                    prop, parameter + "." + prop.name, "propertySize"
                )
            )
            case.extend(["      break;", "    }"])
            cases.append((prop.condition, case))
        lines.extend(conditional_lines(cases))
        lines.extend(
            [
                "    default:",
                "      input.ReadBytes(nullptr, propertySize);",
                "      break;",
                "    }",
                "  }",
            ]
        )
        return [
            line if line.startswith("#") else indent + line[2:] for line in lines
        ]

    def render_struct(self, struct: Struct) -> tuple[str, str]:
        return self.render_struct_header(struct), self.render_struct_source(struct)

    @staticmethod
    def render_animation_parameters() -> dict[str, str]:
        return {
            "Structs/SLdrAnimationSet.hpp": dedent("""\
                #ifndef _SLDRANIMATIONSET_HPP
                #define _SLDRANIMATIONSET_HPP

                #include "Kyoto/SObjectTag.hpp"
                #include "Kyoto/Streams/CInputStream.hpp"

                struct SLdrAnimationSet {
                  SLdrAnimationSet();
                  ~SLdrAnimationSet();

                  CAssetId ancs;
                  int character_index;
                  int initial_anim;
                };

                void LoadTypedefAnimationSet(SLdrAnimationSet&, CInputStream&);

                #endif
                """),
            "Structs/SLdrAnimationSet.cpp": dedent("""\
                #include "MetroidPrime/ScriptLoader/Structs/SLdrAnimationSet.hpp"

                SLdrAnimationSet::SLdrAnimationSet() : ancs(kInvalidAssetId), character_index(0), initial_anim(0) {}

                SLdrAnimationSet::~SLdrAnimationSet() {}

                void LoadTypedefAnimationSet(SLdrAnimationSet& data, CInputStream& input) {
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
            if not self.is_inline(name):
                files[f"{prefix}{name}.cpp"] = self.render_struct_source(struct)
            elif struct.is_object:
                files[f"{name}.inc"] = self.render_fragment(struct)

        if self.uses_animation_parameters:
            if "SLdrAnimationSet" in self.structs:
                raise TemplateError(
                    "AnimationSet collides with built-in wire type"
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


def read_profile(
    path: Path,
) -> tuple[str, list[str], str | None, dict[str, str]]:
    data = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(data, dict):
        raise TemplateError("Profile must contain template_ref and sources")
    ref, sources = data.get("template_ref"), data.get("sources")
    if not isinstance(ref, str) or not re.fullmatch(r"[0-9a-f]{40}", ref):
        raise TemplateError("Profile must pin a template commit")
    if (
        not isinstance(sources, list)
        or not sources
        or any(
            not isinstance(name, str) or not name.endswith(".cpp") for name in sources
        )
    ):
        raise TemplateError("Profile sources must be a nonempty list of C++ paths")
    if len(set(sources)) != len(sources):
        raise TemplateError("Duplicate profile sources")
    aggregate = data.get("aggregate")
    if aggregate is not None and (
        not isinstance(aggregate, str)
        or not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*\.cpp", aggregate)
    ):
        raise TemplateError("Profile aggregate must be a C++ filename")
    duplicates = data.get("duplicates", {})
    if not isinstance(duplicates, dict) or any(
        not isinstance(name, str)
        or not re.fullmatch(r"SLdr[A-Za-z0-9_]+", name)
        or not isinstance(original, str)
        for name, original in duplicates.items()
    ):
        raise TemplateError("Profile duplicates must map SLdr names to object types")
    return (
        ref,
        sources,
        aggregate,
        duplicates,
    )


def profile_files(
    files: dict[str, str], sources: Sequence[str], aggregate: str | None = None
) -> dict[str, str]:
    """Select reviewed sources and their headers from the complete ownership graph."""
    missing = set(sources) - files.keys()
    if missing:
        raise TemplateError(
            "Profile sources not generated: " + ", ".join(sorted(missing))
        )
    selected: dict[str, str] = {}
    pending = list(sources)
    while pending:
        name = pending.pop()
        if name in selected:
            continue
        if name not in files:
            raise TemplateError("Generated header not found: " + name)
        selected[name] = files[name]
        pending.extend(
            re.findall(
                r'^#include "MetroidPrime/ScriptLoader/([^"\n]+)"',
                files[name],
                re.MULTILINE,
            )
        )
    if aggregate is not None:
        # Preserve definition and first-include order: both affect MWCC output.
        includes: dict[str, None] = {}
        bodies: list[str] = []
        for name in sources:
            lines = selected.pop(name).splitlines()
            body: list[str] = []
            for line in lines[1:]:
                if line.startswith("#include "):
                    includes[line] = None
                else:
                    body.append(line)
            bodies.append("\n".join(body).strip())
        selected[aggregate] = (
            "// Generated by scripts/generate_script_loaders.py. Review before integration.\n"
            + "\n".join(includes)
            + "\n\n"
            + "\n\n".join(bodies)
            + "\n"
        )
    return selected


def output_differences(files: dict[str, str], output: Path) -> list[str]:
    """List missing or stale generated files without modifying the destination."""
    return [
        name
        for name, content in sorted(files.items())
        if not (output / name).is_file()
        or (output / name).read_bytes().replace(b"\r\n", b"\n")
        != content.encode("utf-8")
    ]


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
    parser.add_argument("--ref", help="remote branch, tag or commit (default: main)")
    parser.add_argument(
        "--profile",
        type=Path,
        help="reviewed module source list and pinned template commit",
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
    parser.add_argument(
        "--check",
        action="store_true",
        help="report missing/stale outputs without writing",
    )
    args = parser.parse_args(argv)
    if not args.header_output and not args.cpp_output:
        parser.error("provide --header-output and/or --cpp-output")
    if args.profile and (args.object or args.ref):
        parser.error("--profile cannot be combined with --object or --ref")
    if args.check and args.force:
        parser.error("--check cannot be combined with --force")
    try:
        ref, sources, aggregate, duplicates = (
            read_profile(args.profile)
            if args.profile
            else (args.ref or "main", [], None, {})
        )
        generator = Generator(Source(args.templates, ref))
        # Profiles select output, not input: shared-header ownership must agree with
        # full-repository generation even when only one REL is being regenerated.
        generator.collect(args.object)
        # Every profile decides which records own a source, whichever one is rendered.
        for path in sorted(PROFILE_DIRECTORY.glob("*.json")):
            _, profile_sources, _, profile_duplicates = read_profile(path)
            generator.sourced |= {PurePosixPath(name).stem for name in profile_sources}
            if not args.object:
                duplicates = {**profile_duplicates, **duplicates}
        generator.add_duplicates(duplicates)
        files = generator.render()
        if sources:
            files = profile_files(files, sources, aggregate)
        outputs: list[tuple[Path, dict[str, str]]] = []
        if args.header_output:
            outputs.append(
                (
                    args.header_output,
                    {n: c for n, c in files.items() if n.endswith((".hpp", ".inc"))},
                )
            )
        if args.cpp_output:
            outputs.append(
                (
                    args.cpp_output,
                    {n: c for n, c in files.items() if n.endswith(".cpp")},
                )
            )
        if args.check:
            stale = [
                str(path / name)
                for path, output in outputs
                for name in output_differences(output, path)
            ]
            if stale:
                print(
                    "Missing or stale generated files:\n" + "\n".join(stale),
                    file=sys.stderr,
                )
                return 1
            print("Generated files are up to date")
            return 0
        for path, output in outputs:
            write_output(output, path, args.force)
    except (
        ValueError,
        OSError,
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
