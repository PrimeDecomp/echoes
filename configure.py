#!/usr/bin/env python3
import os
import io
import sys
import argparse
import json
import typing

from pathlib import Path
from shutil import which
from tools import ninja_syntax

VERSIONS = [
    "G2ME01",  # 0
    "G2MJ01",  # 1
    "G2MP01",  # 2
    "R32J01",  # 3
    "R3ME01",  # 4
    "R3MP01",  # 5
]

if len(VERSIONS) > 1:
    versions_str = ", ".join(VERSIONS[:-1]) + f" or {VERSIONS[-1]}"
else:
    versions_str = VERSIONS[0]

parser = argparse.ArgumentParser()
parser.add_argument(
    "--version",
    dest="version",
    default=VERSIONS[0],
    help=f"version to build ({versions_str})",
)
parser.add_argument(
    "--build-dir",
    dest="build_dir",
    type=Path,
    default=Path("build"),
    help="base build directory (default: build)",
)
parser.add_argument(
    "--build-dtk",
    dest="build_dtk",
    type=Path,
    help="path to decomp-toolkit source",
)
parser.add_argument(
    "--compilers",
    dest="compilers",
    type=Path,
    default=Path("tools/mwcc_compiler"),
    help="path to compilers (default: tools/mwcc_compiler)",
)
parser.add_argument(
    "--map",
    dest="map",
    action="store_true",
    help="generate map file",
)
parser.add_argument(
    "--debug",
    dest="debug",
    action="store_true",
    help="build with debug info (non-matching)",
)
if os.name != "nt" and not "_NT-" in os.uname().sysname:
    parser.add_argument(
        "--wine",
        dest="wine",
        type=Path,
        help="path to wine (or wibo)",
    )
args = parser.parse_args()

version = args.version.upper()
if version not in VERSIONS:
    sys.exit(f"Invalid version '{version}', expected {versions_str}")
version_num = VERSIONS.index(version)

CFLAGS_BASE = [
    "-proc gekko",
    "-nodefaults",
    "-Cpp_exceptions off",
    "-RTTI off",
    "-fp hard",
    "-fp_contract on",
    "-O4,p",
    "-maxerrors 1",
    "-enum int",
    "-inline auto",
    "-str reuse",
    "-nosyspath",
    f"-DVERSION={version_num}",
    "-i include",
    "-i libc",
]
if args.debug:
    CFLAGS_BASE.extend(["-sym on", "-D_DEBUG=1"])
else:
    CFLAGS_BASE.append("-DNDEBUG=1")

CFLAGS_RETRO = [
    *CFLAGS_BASE,
    "-use_lmw_stmw on",
    "-str reuse,pool,readonly",
    "-gccinc",
    "-inline deferred,noauto",
    "-common on",
]

CFLAGS_RUNTIME = [
    *CFLAGS_BASE,
    "-use_lmw_stmw on",
    "-str reuse,pool,readonly",
    "-gccinc",
    "-common off",
]

if version_num >= 3:
    CFLAGS_RUNTIME.append("-inline auto")
    LINKER_VERSION = "3.0"
else:
    CFLAGS_RUNTIME.append("-inline deferred,auto")
    LINKER_VERSION = "2.7"

LIBS = [
    {
        "lib": "MSL_C.PPCEABI.bare.H",
        "mw_version": LINKER_VERSION,
        "cflags": CFLAGS_RUNTIME,
        "host": False,
        "objects": [
            ["Runtime/__init_cpp_exceptions.cpp", True],
            # TODO: need to implement all
            ["Runtime/Gecko_ExceptionPPC.cp", False],
            ["Runtime/global_destructor_chain.c", True],
        ],
    },
    {
        "lib": "MetroidPrime",
        "cflags": CFLAGS_RETRO,
        "mw_version": "2.7",
        "host": True,
        "objects": [
            ["MetroidPrime/main.cpp", False],
            ["MetroidPrime/ScriptLoader.cpp", False],
            ["MetroidPrime/CHealthInfo.cpp", False],
            ["MetroidPrime/CEntity.cpp", False],
            ["MetroidPrime/HUD/CHUDMemoParms.cpp", True],
            ["MetroidPrime/Player/CPlayerState.cpp", False],
            ["MetroidPrime/Player/CGameOptions.cpp", False],
            ["MetroidPrime/ScriptObjects/CScriptHUDMemo.cpp", False],
            ["MetroidPrime/ScriptObjects/CScriptPickup.cpp", False],
            ["MetroidPrime/ScriptObjects/CScriptSequenceTimer.cpp", False],
            ["MetroidPrime/ScriptObjects/CScriptAreaProperties.cpp", False],
        ],
    },
    {
        "lib": "Kyoto_CW1",
        "mw_version": "2.7",
        "cflags": CFLAGS_RETRO,
        "host": True,
        "objects": [
            ["Kyoto/Basics/CStopwatch.cpp", True],
            ["Kyoto/Basics/RAssertDolphin.cpp", False],
            ["Kyoto/Math/CTransform4f.cpp", False],
            ["Kyoto/Math/CVector2f.cpp", True],
            ["Kyoto/Math/CVector2i.cpp", True],
            ["Kyoto/Math/CVector3d.cpp", True],
            ["Kyoto/Math/CVector3f.cpp", True],
            ["Kyoto/Math/CVector3i.cpp", True],
        ],
    },
    {
        "lib": "ar",
        "mw_version": "1.2.5n",
        "cflags": CFLAGS_BASE,
        "host": False,
        "objects": [
            ["Dolphin/ar/ar.c", True],
            ["Dolphin/ar/arq.c", True],
        ],
    },
    {
        "lib": "base",
        "mw_version": "1.2.5",
        "cflags": CFLAGS_BASE,
        "host": False,
        "objects": [
            ["Dolphin/PPCArch.c", True],
        ],
    },
]

# On Windows, we need this to use && in commands
chain = "cmd /c " if os.name == "nt" else ""

out = io.StringIO()
n = ninja_syntax.Writer(out)

n.variable("ninja_required_version", "1.3")
n.newline()

n.comment("The arguments passed to configure.py, for rerunning it.")
n.variable("configure_args", sys.argv[1:])
n.variable("python", f'"{sys.executable}"')
n.newline()

###
# Variables
###
n.comment("Variables")
version = args.version
version_num = VERSIONS.index(args.version)
build_path = args.build_dir / version
config_path = Path("config") / version / "config.yml"

ldscript_path = build_path / "ldscript.lcf"
ldflags = f"-fp fmadd -nodefaults -lcf {ldscript_path}"
map_path = build_path / f"main.MAP"
if args.map:
    ldflags += f" -map {map_path} -mapunused -listclosure"
if args.debug:
    ldflags += " -g"
n.variable("ldflags", ldflags)
n.variable("mw_version", LINKER_VERSION)
if os.name == "nt":
    exe = ".exe"
    wine = ""
else:
    if "_NT-" in os.uname().sysname:
        # MSYS2
        wine = ""
    elif args.wine:
        wine = f"{args.wine} "
    elif which("wibo") is not None:
        wine = "wibo "
    else:
        wine = "wine "
    exe = ""
n.newline()


def path(
    input: typing.Union[typing.List[Path], Path, None]
) -> typing.Optional[typing.List[str]]:
    if input is None:
        return None
    elif isinstance(input, list):
        return list(map(str, input))
    else:
        return [str(input)]


###
# Tooling
###
n.comment("decomp-toolkit")

tools_path = Path("tools")
build_tools_path = args.build_dir / "tools"

if args.build_dtk:
    dtk = build_tools_path / "release" / f"dtk{exe}"
    n.rule(
        name="cargo",
        command="cargo build --release --manifest-path $in --bin $bin --target-dir $target",
        description="CARGO $bin",
        depfile=path(Path("$target") / "release" / "$bin.d"),
        deps="gcc",
    )
    n.build(
        outputs=path(dtk),
        rule="cargo",
        inputs=path(args.build_dtk / "Cargo.toml"),
        variables={
            "bin": "dtk",
            "target": build_tools_path,
        },
    )
else:
    dtk = build_tools_path / f"dtk{exe}"
    download_dtk = tools_path / "download_dtk.py"
    n.rule(
        name="download_dtk",
        command=f"$python {download_dtk} $in $out",
        description="DOWNLOAD $out",
    )
    n.build(
        outputs=path(dtk),
        rule="download_dtk",
        inputs=path(tools_path / "dtk_version"),
        implicit=path(download_dtk),
    )
n.newline()

###
# Rules
###
compiler_path = args.compilers / "$mw_version"
mwcc = compiler_path / "mwcceppc.exe"
mwld = compiler_path / "mwldeppc.exe"

mwcc_cmd = f"{chain}{wine}{mwcc} $cflags -MMD -c $in -o $basedir"
mwld_cmd = f"{wine}{mwld} $ldflags -o $out @$out.rsp"
ar_cmd = f"{dtk} ar create $out @$out.rsp"

if os.name != "nt":
    transform_dep = tools_path / "transform-dep.py"
    mwcc_cmd += f" && $python {transform_dep} $basefile.d $basefile.d"

n.comment("Link ELF file")
n.rule(
    name="link",
    command=mwld_cmd,
    description="LINK $out",
    rspfile="$out.rsp",
    rspfile_content="$in_newline",
)
n.newline()

n.comment("MWCC build")
n.rule(
    name="mwcc",
    command=mwcc_cmd,
    description="MWCC $out",
    depfile="$basefile.d",
    deps="gcc",
)
n.newline()

n.comment("Host build")
n.variable("host_cflags", "-I include -Wno-trigraphs")
n.variable(
    "host_cppflags",
    "-std=c++98 -I include -fno-exceptions -fno-rtti -D_CRT_SECURE_NO_WARNINGS -Wno-trigraphs -Wno-c++11-extensions",
)
n.rule(
    name="host_cc",
    command="clang $host_cflags -c -o $out $in",
    description="CC $out",
)
n.rule(
    name="host_cpp",
    command="clang++ $host_cppflags -c -o $out $in",
    description="CXX $out",
)
n.newline()

###
# Rules for source files
###
n.comment("Source files")
build_obj_path = build_path / "obj"
build_src_path = build_path / "src"
build_host_path = build_path / "host"
build_config_path = build_path / "config.json"

build_obj_path.mkdir(parents=True, exist_ok=True)
build_src_path.mkdir(parents=True, exist_ok=True)
objdiff_config = {
    "custom_make": "ninja",
    "target_dir": str(build_obj_path),
    "base_dir": str(build_src_path),
    "build_target": False,
    "watch_patterns": [
        "*.c",
        "*.cp",
        "*.cpp",
        "*.h",
        "*.hpp",
        "*.py",
        "*.yml",
        "*.txt",
        "*.json",
    ],
    "units": [],
}


def locate_unit(unit):
    for lib in LIBS:
        for obj in lib["objects"]:
            if obj[0] == unit:
                return [lib, obj]
    return None


has_units = False
if build_config_path.is_file():
    has_units = True

    src_path = Path("src")
    link_inputs = []
    used_compiler_versions = set()
    source_inputs = []
    host_source_inputs = []

    with open(build_config_path) as r:
        data = json.load(r)
        for unit in data["units"]:
            obj_path, unit = unit["object"], unit["name"]
            unit_path = src_path / unit
            if unit_path.exists():
                result = locate_unit(unit)
                if result:
                    lib, object = result
                    lib_name = lib["lib"]

                    completed = None
                    options = {
                        "add_to_all": True,
                        "mw_version": None,
                        "cflags": None,
                    }
                    if type(object) is list:
                        if len(object) > 1:
                            completed = object[1]
                        if len(object) > 2:
                            options.update(object[2])
                        object = object[0]

                    mw_version = options["mw_version"] or lib["mw_version"]
                    cflags = options["cflags"] or lib["cflags"]
                    if type(cflags) is list:
                        cflags_str = " ".join(cflags)
                    else:
                        cflags_str = str(cflags)
                    used_compiler_versions.add(mw_version)

                    n.comment(f"{unit}: {lib_name} (linked {completed})")

                    base_object = Path(object).with_suffix("")
                    src_obj_path = build_src_path / f"{base_object}.o"
                    n.build(
                        outputs=path(src_obj_path),
                        rule="mwcc",
                        inputs=path(unit_path),
                        variables={
                            "mw_version": mw_version,
                            "cflags": cflags_str,
                            "basedir": os.path.dirname(
                                build_src_path / f"{base_object}"
                            ),
                            "basefile": path(build_src_path / f"{base_object}"),
                        },
                    )

                    if lib["host"]:
                        host_obj_path = build_host_path / f"{base_object}.o"
                        n.build(
                            outputs=path(host_obj_path),
                            rule="host_cc" if unit_path.suffix == ".c" else "host_cpp",
                            inputs=path(unit_path),
                            variables={
                                "basedir": os.path.dirname(
                                    build_host_path / f"{base_object}"
                                ),
                                "basefile": path(build_host_path / f"{base_object}"),
                            },
                        )
                        if options["add_to_all"]:
                            host_source_inputs.append(host_obj_path)

                    if options["add_to_all"]:
                        source_inputs.append(src_obj_path)

                    reverse_fn_order = False
                    if type(cflags) is list:
                        for flag in cflags:
                            if not flag.startswith("-inline "):
                                continue
                            for value in flag.split(" ")[1].split(","):
                                if value == "deferred":
                                    reverse_fn_order = True
                                elif value == "nodeferred":
                                    reverse_fn_order = False
                    objdiff_config["units"].append(
                        {
                            "name": str(base_object),
                            "path": f"{base_object}.o",
                            "reverse_fn_order": reverse_fn_order,
                        }
                    )

                    if completed:
                        obj_path = src_obj_path
                else:
                    print(f"No configuration entry found for {unit}")
                    exit(1)
            link_inputs.append(obj_path)
    n.newline()

    # Check if all compiler versions exist
    for mw_version in used_compiler_versions:
        mw_path = args.compilers / mw_version / "mwcceppc.exe"
        if not os.path.exists(mw_path):
            print(f"Compiler {mw_path} does not exist")
            exit(1)

    # Check if linker exists
    mw_path = args.compilers / LINKER_VERSION / "mwldeppc.exe"
    if not os.path.exists(mw_path):
        print(f"Linker {mw_path} does not exist")
        exit(1)

    ###
    # Link
    ###
    n.comment("Link")
    elf_path = build_path / "main.elf"
    if args.map:
        n.build(
            outputs=path(elf_path),
            rule="link",
            inputs=path(link_inputs),
            implicit=path(ldscript_path),
            implicit_outputs=path(map_path),
        )
    else:
        n.build(
            outputs=path(elf_path),
            rule="link",
            inputs=path(link_inputs),
            implicit=path(ldscript_path),
        )
    n.newline()

    ###
    # Helper rule for building all source files
    ###
    n.comment("Build all source files")
    n.build(
        outputs="all_source",
        rule="phony",
        inputs=path(source_inputs),
    )
    n.newline()

    ###
    # Helper rule for building all source files, with a host compiler
    ###
    n.comment("Build all source files with a host compiler")
    n.build(
        outputs="all_source_host",
        rule="phony",
        inputs=path(host_source_inputs),
    )
    n.newline()

    ###
    # Generate DOL
    ###
    n.comment("Generate DOL")
    dol_path = build_path / "main.dol"
    n.rule(
        name="elf2dol",
        command=f"{dtk} elf2dol $in $out",
        description="DOL $out",
    )
    n.build(
        outputs=path(dol_path),
        rule="elf2dol",
        inputs=path(elf_path),
        implicit=path(dtk),
    )
    n.newline()

    ###
    # Check DOL hash
    ###
    n.comment("Check DOL hash")
    dol_ok_path = str(dol_path) + ".ok"
    n.rule(
        name="check",
        command=f"{dtk} shasum -c $in -o $out",
        description="CHECK $in",
    )
    n.build(
        outputs=path(dol_ok_path),
        rule="check",
        inputs=path(Path("orig") / f"{version}.sha1"),
        implicit=path([dol_path, dtk]),
    )
    n.newline()

    ###
    # Helper tools
    ###
    n.comment("Check for mismatching symbols")
    n.rule(
        name="dol_diff",
        command=f"{dtk} -L error dol diff $in",
        description=f"DIFF {elf_path}",
    )
    n.build(
        inputs=path([config_path, elf_path, map_path]),
        outputs="dol_diff",
        rule="dol_diff",
    )
    n.build(
        outputs="diff",
        rule="phony",
        inputs="dol_diff",
    )
    n.newline()

    n.comment("Apply symbols from linked ELF")
    n.rule(
        name="dol_apply",
        command=f"{dtk} dol apply $in",
        description=f"APPLY {elf_path}",
    )
    n.build(
        inputs=path([config_path, elf_path, map_path]),
        outputs="dol_apply",
        rule="dol_apply",
        implicit=path([dol_ok_path]),
    )
    n.build(
        outputs="apply",
        rule="phony",
        inputs="dol_apply",
    )

###
# DOL split
###
n.comment("Generate objects from original DOL")
n.rule(
    name="split",
    command=f"{dtk} dol split $in $out_dir",
    description="SPLIT $in",
    depfile="$out_dir/dep",
    deps="gcc",
)
n.build(
    inputs=path(config_path),
    outputs=path(build_config_path),
    rule="split",
    implicit=path(dtk),
    variables={"out_dir": path(build_path)},
)
n.newline()

###
# Regenerate on change
###
n.comment("Reconfigure on change")
script = os.path.relpath(__file__)
n.rule(
    name="configure",
    command=f"$python {script} $configure_args",
    generator=True,
    description=f"RUN {script}",
)
n.build(
    outputs="build.ninja",
    rule="configure",
    implicit=path([script, tools_path / "ninja_syntax.py", build_config_path]),
)
n.newline()

###
# Default rule
###
n.comment("Default rule")
if has_units:
    n.default(path(dol_ok_path))
else:
    n.default(path(build_config_path))

###
# Write build.ninja
###
with open("build.ninja", "w") as f:
    f.write(out.getvalue())
n.close()

###
# Write objdiff config
###
with open("objdiff.json", "w") as w:
    json.dump(objdiff_config, w, indent=4)
