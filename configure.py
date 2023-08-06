#!/usr/bin/env python3
LIBS = [
    {
        "lib": "MSL_C.PPCEABI.bare.H",
        "mw_version": "1.3.2",
        "cflags": "$cflags_runtime",
        "host": False,
        "objects": [
            # TODO: need to remove from FORCEFILES
            ["Runtime/__init_cpp_exceptions.cpp", False],
            # TODO: need to implement all
            ["Runtime/Gecko_ExceptionPPC.cp", False],
        ],
    },
]
VERSIONS = [
    "G2ME01",  # 0
    "G2MJ01",  # 1
    "G2MP01",  # 2
]

import os
import io
import sys
import argparse

from pathlib import Path
from shutil import which
import typing
from tools import ninja_syntax


def path(input: typing.Union[typing.List[Path], Path, None]) -> typing.Optional[typing.List[str]]:
    if input is None:
        return None
    elif isinstance(input, list):
        return list(map(str, input))
    else:
        return [str(input)]


parser = argparse.ArgumentParser()
versions_str = ", ".join(VERSIONS)
parser.add_argument(
    "--version",
    dest="version",
    default="G2ME01",
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
    "--orig",
    dest="orig",
    type=Path,
    default=Path("orig"),
    help="path to retail files (default: orig)",
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

cflags_base = f"-proc gekko -nodefaults -Cpp_exceptions off -RTTI off -fp hard -fp_contract on -O4,p -maxerrors 1 -enum int -inline auto -str reuse -nosyspath -DVERSION={version_num} -i include -i libc"
if args.debug:
    cflags_base += " -sym on -D_DEBUG"
else:
    cflags_base += " -DNDEBUG"
n.variable("cflags_base", cflags_base)
n.variable(
    "cflags_retro",
    "$cflags_base -use_lmw_stmw on -str reuse,pool,readonly -gccinc -inline deferred,noauto -common on",
)
n.variable(
    "cflags_runtime",
    "$cflags_base -use_lmw_stmw on -str reuse,pool,readonly -gccinc -inline deferred,auto",
)
n.variable("cflags_musyx", "$cflags_base -str reuse,pool,readonly -fp_contract off")
asflags = f"-mgekko -I include --defsym version={version_num} -W --strip-local-absolute -gdwarf-2"
n.variable("asflags", asflags)
ldscript_path = build_path / "ldscript.lcf"
ldflags = f"-fp fmadd -nodefaults -lcf {ldscript_path}"
if args.map:
    map_path = args.build_dir / f"{version}.MAP"
    ldflags += f" -map {map_path}"
if args.debug:
    ldflags += " -g"
n.variable("ldflags", ldflags)
mw_link_version = "1.3.2"
n.variable("mw_version", mw_link_version)
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
    transform_dep_cmd = f" && $python {transform_dep} $basefile.d $basefile.d"
    mwcc_cmd += transform_dep_cmd

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

###
# Rules for source files
###
n.comment("Source files")
src_path = Path("src")
asm_path = Path("asm")
build_src_path = build_path / "src"
units_path = build_path / "units.txt"


def locate_unit(unit):
    for lib in LIBS:
        for obj in lib["objects"]:
            if obj[0] == unit:
                return [lib, obj]
    return None


has_units = False
if units_path.is_file():
    has_units = True

    src_path = Path("src")
    link_inputs = []
    used_compiler_versions = set()
    with open(units_path) as r:
        for line in r:
            obj_path, unit = line.rstrip().split(":", 2)
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
                            "cflags": options["cflags"] or lib["cflags"],
                            "basedir": os.path.dirname(
                                build_src_path / f"{base_object}"
                            ),
                            "basefile": path(build_src_path / f"{base_object}"),
                        },
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
    mw_path = args.compilers / mw_link_version / "mwldeppc.exe"
    if not os.path.exists(mw_path):
        print(f"Linker {mw_path} does not exist")
        exit(1)

    ###
    # Link
    ###
    n.comment("Link")
    if args.map:
        n.build(
            outputs=path(build_path / "main.elf"),
            rule="link",
            inputs=path(link_inputs),
            implicit=path(ldscript_path),
            implicit_outputs=path(map_path),
        )
    else:
        n.build(
            outputs=path(build_path / "main.elf"),
            rule="link",
            inputs=path(link_inputs),
            implicit=path(ldscript_path),
        )
    n.newline()

    ###
    # Generate DOL
    ###
    n.comment("Generate DOL")
    n.rule(
        name="elf2dol",
        command=f"{dtk} elf2dol $in $out",
        description="DOL $out",
    )
    n.build(
        outputs=path(build_path / "main.dol"),
        rule="elf2dol",
        inputs=path(build_path / "main.elf"),
        implicit=path(dtk),
    )
    n.newline()

    ###
    # Check DOL hash
    ###
    n.comment("Check DOL hash")
    n.rule(
        name="check",
        command=f"{dtk} shasum -c $in -o $out",
        description="CHECK $in",
    )
    n.build(
        outputs=path(build_path / "main.dol.ok"),
        rule="check",
        inputs=path(Path("orig") / f"{version}.sha1"),
        implicit=path([build_path / "main.dol", dtk]),
    )
    n.newline()

###
# DOL split
###
n.comment("Generate objects from original DOL")
dol_path = args.orig / version / "sys" / "main.dol"
config_path = Path("config") / version
splits_path = config_path / "splits.txt"
symbols_path = config_path / "symbols.txt"
n.rule(
    name="split",
    command=f"{dtk} dol split $in $out -p $splits -s $symbols --no-update",
    description="SPLIT $in",
)
n.build(
    inputs=path(dol_path),
    outputs=path(build_path),
    rule="split",
    implicit=path([dtk, splits_path, symbols_path]),
    implicit_outputs=path(units_path),
    variables={
        "splits": splits_path,
        "symbols": symbols_path,
    },
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
    implicit=path([script, tools_path / "ninja_syntax.py", build_path]),
)
n.newline()

###
# Default rule
###
n.comment("Default rule")
if has_units:
    n.default(path(build_path / "main.dol.ok"))
else:
    n.default(path(build_path))

###
# Write build.ninja
###
with open("build.ninja", "w") as f:
    f.write(out.getvalue())
n.close()
