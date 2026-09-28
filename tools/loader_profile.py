"""Reviewed generated-loader source lists shared by configure and generation."""

from __future__ import annotations

import json
import re
from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class LoaderProfile:
    name: str
    template_ref: str
    sources: tuple[str, ...]

    @classmethod
    def read(cls, path: Path) -> LoaderProfile:
        data = json.loads(path.read_text(encoding="utf-8"))
        if not isinstance(data, dict) or set(data) != {
            "version",
            "name",
            "template_ref",
            "sources",
        }:
            raise ValueError(f"Invalid loader profile fields: {path}")
        if type(data["version"]) is not int or data["version"] != 1:
            raise ValueError(f"Unsupported loader profile version: {path}")
        if not isinstance(data["name"], str) or not data["name"].strip():
            raise ValueError(f"Missing loader profile name: {path}")
        ref = data["template_ref"]
        if not isinstance(ref, str) or not re.fullmatch(r"[0-9a-f]{40}", ref):
            raise ValueError(f"Loader profile must pin a template commit: {path}")
        sources = data["sources"]
        if (
            not isinstance(sources, list)
            or not sources
            or any(
                not isinstance(source, str)
                or not re.fullmatch(r"(?:Structs/)?SLdr[A-Za-z0-9_]+\.cpp", source)
                for source in sources
            )
        ):
            raise ValueError(f"Invalid loader profile source paths: {path}")
        if len(set(sources)) != len(sources):
            raise ValueError(f"Duplicate loader profile sources: {path}")
        return cls(data["name"], ref, tuple(sources))
