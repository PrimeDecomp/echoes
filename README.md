Metroid Prime 2: Echoes  
[![Build Status]][actions] [![Code Progress]][progress] [![Data Progress]][progress] [![Discord Badge]][discord]
=============

[Build Status]: https://github.com/PrimeDecomp/echoes/actions/workflows/build.yml/badge.svg
[actions]: https://github.com/PrimeDecomp/echoes/actions/workflows/build.yml
[Code Progress]: https://decomp.dev/PrimeDecomp/echoes.svg?mode=shield&measure=code&label=Code
[Data Progress]: https://decomp.dev/PrimeDecomp/echoes.svg?mode=shield&measure=data&label=Data
[progress]: https://decomp.dev/PrimeDecomp/echoes
[DOL Progress]: https://decomp.dev/PrimeDecomp/echoes.svg?mode=shield&measure=code&label=DOL&version=G2ME01&category=dol
[RELs Progress]: https://decomp.dev/PrimeDecomp/echoes.svg?mode=shield&measure=code&label=RELs&version=G2ME01&category=modules
[DOL progress page]: https://decomp.dev/PrimeDecomp/echoes?version=G2ME01&category=dol
[RELs progress page]: https://decomp.dev/PrimeDecomp/echoes?version=G2ME01&category=modules

[Code Progress G2ME01]: https://decomp.dev/PrimeDecomp/echoes.svg?mode=shield&measure=code&label=Code&version=G2ME01
[Data Progress G2ME01]: https://decomp.dev/PrimeDecomp/echoes.svg?mode=shield&measure=data&label=Data&version=G2ME01
[Progress G2ME01]: https://decomp.dev/PrimeDecomp/echoes?version=G2ME01

[Code Progress G2MJ01]: https://decomp.dev/PrimeDecomp/echoes.svg?mode=shield&measure=code&label=Code&version=G2MJ01
[Data Progress G2MJ01]: https://decomp.dev/PrimeDecomp/echoes.svg?mode=shield&measure=data&label=Data&version=G2MJ01
[Progress G2MJ01]: https://decomp.dev/PrimeDecomp/echoes?version=G2MJ01

[Code Progress G2MP01]: https://decomp.dev/PrimeDecomp/echoes.svg?mode=shield&measure=code&label=Code&version=G2MP01
[Data Progress G2MP01]: https://decomp.dev/PrimeDecomp/echoes.svg?mode=shield&measure=data&label=Data&version=G2MP01
[Progress G2MP01]: https://decomp.dev/PrimeDecomp/echoes?version=G2MP01

[Code Progress R32J01]: https://decomp.dev/PrimeDecomp/echoes.svg?mode=shield&measure=code&label=Code&version=R32J01
[Data Progress R32J01]: https://decomp.dev/PrimeDecomp/echoes.svg?mode=shield&measure=data&label=Data&version=R32J01
[Progress R32J01]: https://decomp.dev/PrimeDecomp/echoes?version=R32J01

[Code Progress R3ME01]: https://decomp.dev/PrimeDecomp/echoes.svg?mode=shield&measure=code&label=Code&version=R3ME01
[Data Progress R3ME01]: https://decomp.dev/PrimeDecomp/echoes.svg?mode=shield&measure=data&label=Data&version=R3ME01
[Progress R3ME01]: https://decomp.dev/PrimeDecomp/echoes?version=R3ME01

[Code Progress R3MP01]: https://decomp.dev/PrimeDecomp/echoes.svg?mode=shield&measure=code&label=Code&version=R3MP01
[Data Progress R3MP01]: https://decomp.dev/PrimeDecomp/echoes.svg?mode=shield&measure=data&label=Data&version=R3MP01
[Progress R3MP01]: https://decomp.dev/PrimeDecomp/echoes?version=R3MP01

[Discord Badge]: https://img.shields.io/discord/727908905392275526?color=%237289DA&logo=discord&logoColor=%23FFFFFF
[discord]: https://discord.gg/hKx3FJJgrV

A decompilation of Metroid Prime 2: Echoes.

Supported versions:

| Version | Release | Code | Data |
|---------|---------|------|------|
| `G2ME01` | GameCube (USA) | [![Code Progress G2ME01]][Progress G2ME01] | [![Data Progress G2ME01]][Progress G2ME01] |
| `G2MJ01` | GameCube (Japan) | [![Code Progress G2MJ01]][Progress G2MJ01] | [![Data Progress G2MJ01]][Progress G2MJ01] |
| `G2MP01` | GameCube (PAL) | [![Code Progress G2MP01]][Progress G2MP01] | [![Data Progress G2MP01]][Progress G2MP01] |
| `R32J01` | Wii: New Play Control! (Japan) | [![Code Progress R32J01]][Progress R32J01] | [![Data Progress R32J01]][Progress R32J01] |
| `R3ME01` | Wii: Metroid Prime Trilogy (USA) | [![Code Progress R3ME01]][Progress R3ME01] | [![Data Progress R3ME01]][Progress R3ME01] |
| `R3MP01` | Wii: Metroid Prime Trilogy (PAL) | [![Code Progress R3MP01]][Progress R3MP01] | [![Data Progress R3MP01]][Progress R3MP01] |

RELs are built for `G2ME01` only: [![DOL Progress]][DOL progress page] [![RELs Progress]][RELs progress page].

This repository builds the following DOLs:

```text
6ef9b491d0cc08bc81a124fdedb8bfaec34d0010  build/G2ME01/main.dol
7f24a768f7b1a687adb88e56559ad8637ed80589  build/G2MJ01/main.dol
5a670d5da3d181e86a0df7cf7751c7055eee35fb  build/G2MP01/main.dol
442947ba57dce414917feab0e75a8227690b3e4b  build/R32J01/rs5mp2jpn_p.dol
2375606f4e9429a699cfa02728b0bb1176421226  build/R3ME01/rs5mp2_p.dol
077712e46eb7cf2488942f337d215636edbc972d  build/R3MP01/rs5mp2_p.dol
```

Dependencies
============

Windows:
--------

- Install [ninja](https://github.com/ninja-build/ninja/releases) and add it to `%PATH%`.
- Install [Python 3.9+](https://www.python.org/downloads/) and add it to `%PATH%`.

macOS:
------

- Install Python and ninja:

  ```sh
  brew install python ninja
  ```

- On Apple Silicon, install Rosetta 2 if prompted.

Linux:
------

- Install Python 3.9+ and [ninja](https://github.com/ninja-build/ninja/wiki/Pre-built-Ninja-packages) from your package manager.
- On non-x86 platforms, install Wine with 32-bit x86 support and configure with `--wrapper /path/to/wine`.

Build tools are downloaded automatically, including the compilers and [wibo 1.1.0](https://github.com/decompals/wibo/releases/tag/1.1.0) on macOS and x86 Linux.

Building
========

- Checkout the repository:

  ```sh
  git clone https://github.com/PrimeDecomp/echoes.git
  cd echoes
  ```

- Copy your game's disc image to `orig/G2ME01` (or the appropriate version).
  - Supported formats: ISO (GCM), RVZ, WIA, WBFS, CISO, NFS, GCZ, TGC.
  - Required files are extracted automatically. The image can be deleted after the first build.
- Configure:

  ```sh
  python configure.py
  ```

  For another version, add `--version G2MP01` or similar. Use `python3` if needed.
- Build:

  ```sh
  ninja
  ```

Diffing
========

Open the project directory in [objdiff 3.7.0](https://github.com/encounter/objdiff/releases/tag/v3.7.0) after the first build. Select an object to compare; edits rebuild automatically.
