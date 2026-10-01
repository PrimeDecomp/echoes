"""Generate the G2ME01 embedded font include from the user's original DOL.

These are compressed resource payloads, not reconstructed C++ or committed binaries.
"""

import argparse
import hashlib
import struct
from pathlib import Path


def extract(dol: bytes, address: int, size: int) -> bytes:
    offsets = struct.unpack_from(">18I", dol, 0)
    addresses = struct.unpack_from(">18I", dol, 0x48)
    sizes = struct.unpack_from(">18I", dol, 0x90)
    for offset, start, length in zip(offsets, addresses, sizes):
        if start <= address and address + size <= start + length:
            return dol[offset + address - start : offset + address - start + size]
    raise ValueError(f"Resource at {address:#x} is outside the DOL sections")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("dol", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    dol = args.dol.read_bytes()
    expected = "7621e7571e0ee4e098a40bc8a0a3647811bd944e43025229426c7deae1760684"
    if hashlib.sha256(dol).hexdigest() != expected:
        raise ValueError("Expected the original G2ME01 DOL")
    lines = ["// Generated from G2ME01. Do not edit."]
    for name, address, size in (
        ("sDefaultFontData", 0x803AB1C0, 0xA2E),
        ("sDefaultFontTexture", 0x803ABBF0, 0x7BE),
    ):
        data = extract(dol, address, size)
        lines.append(f"static const uchar {name}[] = {{")
        for i in range(0, len(data), 16):
            lines.append("  " + ", ".join(f"0x{b:02x}" for b in data[i : i + 16]) + ",")
        lines.append("};\n")
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text("\n".join(lines) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
