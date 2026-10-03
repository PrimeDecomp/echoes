#!/usr/bin/env python3

###
# Generates build files for the project.
# This file also includes the project configuration,
# such as compiler flags and the object matching status.
#
# Usage:
#   python3 configure.py
#   ninja
#
# Append --help to see available options.
###

import argparse
import sys
from pathlib import Path
from typing import Any, Dict, List, Optional

from tools.project import (
    Object,
    ProgressCategory,
    ProjectConfig,
    calculate_progress,
    generate_build,
    is_windows,
)

# Game versions
DEFAULT_VERSION = 0
VERSIONS = [
    "G2ME01",  # 0
    "G2MP01",  # 1
    "G2MJ01",  # 2
    "R32J01",  # 3
    "R3ME01",  # 4
    "R3MP01",  # 5
]

parser = argparse.ArgumentParser()
parser.add_argument(
    "mode",
    choices=["configure", "progress"],
    default="configure",
    help="script mode (default: configure)",
    nargs="?",
)
parser.add_argument(
    "-v",
    "--version",
    choices=VERSIONS,
    type=str.upper,
    default=VERSIONS[DEFAULT_VERSION],
    help="version to build",
)
parser.add_argument(
    "--build-dir",
    metavar="DIR",
    type=Path,
    default=Path("build"),
    help="base build directory (default: build)",
)
parser.add_argument(
    "--binutils",
    metavar="BINARY",
    type=Path,
    help="path to binutils (optional)",
)
parser.add_argument(
    "--compilers",
    metavar="DIR",
    type=Path,
    help="path to compilers (optional)",
)
parser.add_argument(
    "--map",
    action="store_true",
    help="generate map file(s)",
)
parser.add_argument(
    "--debug",
    action="store_true",
    help="build with debug info (non-matching)",
)
if not is_windows():
    parser.add_argument(
        "--wrapper",
        metavar="BINARY",
        type=Path,
        help="path to wibo or wine (optional)",
    )
parser.add_argument(
    "--dtk",
    metavar="BINARY | DIR",
    type=Path,
    help="path to decomp-toolkit binary or source (optional)",
)
parser.add_argument(
    "--objdiff",
    metavar="BINARY | DIR",
    type=Path,
    help="path to objdiff-cli binary or source (optional)",
)
parser.add_argument(
    "--sjiswrap",
    metavar="EXE",
    type=Path,
    help="path to sjiswrap.exe (optional)",
)
parser.add_argument(
    "--ninja",
    metavar="BINARY",
    type=Path,
    help="path to ninja binary (optional)",
)
parser.add_argument(
    "--verbose",
    action="store_true",
    help="print verbose output",
)
parser.add_argument(
    "--non-matching",
    dest="non_matching",
    action="store_true",
    help="builds equivalent (but non-matching) or modded objects",
)
parser.add_argument(
    "--warn",
    dest="warn",
    type=str,
    choices=["all", "off", "error"],
    help="how to handle warnings",
)
parser.add_argument(
    "--no-progress",
    dest="progress",
    action="store_false",
    help="disable progress calculation",
)
args = parser.parse_args()

config = ProjectConfig()
config.version = str(args.version)
version_num = VERSIONS.index(config.version)

# Apply arguments
config.build_dir = args.build_dir
config.dtk_path = args.dtk
config.objdiff_path = args.objdiff
config.binutils_path = args.binutils
config.compilers_path = args.compilers
config.generate_map = args.map
config.non_matching = args.non_matching
config.sjiswrap_path = args.sjiswrap
config.ninja_path = args.ninja
config.progress = args.progress
if not is_windows():
    config.wrapper = args.wrapper
# Don't build asm unless we're --non-matching
if not config.non_matching:
    config.asm_dir = None

# Tool versions
config.binutils_tag = "2.42-2"
config.compilers_tag = "20251118"
config.dtk_tag = "v1.8.4"
# v3.8.1 fails to generate the G2ME01 report due to a symbol-pairing regression.
config.objdiff_tag = "v3.7.0"
config.sjiswrap_tag = "v1.2.2"
config.wibo_tag = "1.1.0"

# Project
config.config_path = Path("config") / config.version / "config.yml"
config.check_sha_path = Path("config") / config.version / "build.sha1"
config.asflags = [
    "-mgekko",
    "--strip-local-absolute",
    "-I include",
    f"-I build/{config.version}/include",
    f"--defsym BUILD_VERSION={version_num}",
    f"--defsym VERSION_{config.version}",
]
config.ldflags = [
    "-fp hardware",
    "-nodefaults",
]
if args.debug:
    config.ldflags.append("-g")  # Or -gdwarf-2 for Wii linkers
if args.map:
    config.ldflags.append("-mapunused")
    # config.ldflags.append("-listclosure") # For Wii linkers

# Use for any additional files that should cause a re-configure when modified
config.reconfig_deps = []

# Optional numeric ID for decomp.me preset
# Can be overridden in libraries or objects
config.scratch_preset_id = None

# Base flags, common to most GC/Wii games.
# Generally leave untouched, with overrides added below.
cflags_base = [
    "-nodefaults",
    "-proc gekko",
    "-align powerpc",
    "-enum int",
    "-fp hardware",
    "-Cpp_exceptions off",
    # "-W all",
    "-O4,p",
    "-inline auto",
    '-pragma "cats off"',
    '-pragma "warn_notinlined off"',
    "-maxerrors 1",
    "-nosyspath",
    "-RTTI off",
    "-fp_contract on",
    "-str reuse",
    "-i include",
    "-i libc",
    f"-i build/{config.version}/include",
    f"-DBUILD_VERSION={version_num}",
    f"-DVERSION={version_num}",
]

# GC 3.0 and above require -enc SJIS instead of -multibyte
if version_num >= 3:
    cflags_base.append("-enc SJIS")
else:
    cflags_base.append("-multibyte")

# Debug flags
if args.debug:
    # Or -sym dwarf-2 for Wii compilers
    cflags_base.extend(["-sym on", "-DDEBUG=1"])
else:
    cflags_base.append("-DNDEBUG=1")

# Warning flags
if args.warn == "all":
    cflags_base.append("-W all")
elif args.warn == "off":
    cflags_base.append("-W off")
elif args.warn == "error":
    cflags_base.append("-W error")

# Dolphin flags
cflags_dolphin = [
    *cflags_base,
    "-multibyte",
    "-fp_contract off",
]

# Metrowerks library flags
cflags_runtime = [
    *cflags_base,
    "-use_lmw_stmw on",
    "-str reuse,pool,readonly",
    "-gccinc",
    "-common off",
    # "-inline auto",
]

# Main-game and game REL sources use the same Retro compiler and base flags.
retro_mw_version = "GC/2.7"

# Retro flags
cflags_retro = [
    *cflags_base,
    "-use_lmw_stmw on",
    "-str reuse,pool,readonly",
    "-gccinc",
    "-inline deferred,noauto",
    "-common on",
    "-i extern/musyx/include",
    "-DMUSY_TARGET=MUSY_TARGET_DOLPHIN",
    "-DMUSY_VERSION_MAJOR=2",
    "-DMUSY_VERSION_MINOR=0",
    "-DMUSY_VERSION_PATCH=3",
]

if config.version == "G2ME01":
    cflags_retro.append('-pragma "inline_max_size(125)"')

# Relocatable code cannot use the DOL's small-data bases.
cflags_rel = [
    *cflags_retro,
    "-sdata 0",
    "-sdata2 0",
]

if version_num >= 3:
    cflags_runtime.append("-inline auto")
    config.linker_version = "GC/3.0a5"
else:
    cflags_runtime.append("-inline deferred,auto")
    config.linker_version = "GC/2.7"

if version_num > 0:
    # RELs not yet set up for non-USA versions
    config.build_rels = False

# Helper function for Dolphin libraries
def DolphinLib(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/1.2.5n",
        "cflags": cflags_dolphin,
        "progress_category": "sdk",
        "host": False,
        "objects": objects,
    }


# Helper function for REL script objects
def Rel(
    lib_name: str, objects: List[Object], extra_cflags: Optional[List[str]] = None
) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": retro_mw_version,
        "cflags": cflags_rel + (extra_cflags or []),
        "progress_category": "game",
        "host": True,
        "objects": objects,
    }

# MusyX flags
cflags_musyx = [
    "-proc gekko",
    "-nodefaults",
    "-nosyspath",
    "-i include",
    "-i libc",
    "-i extern/musyx/include",
    "-inline auto,depth=4",
    "-O4,p",
    "-fp hard",
    "-enum int",
    "-sym on",
    "-Cpp_exceptions off",
    "-str reuse,pool,readonly",
    "-fp_contract off",
    "-DMUSY_TARGET=MUSY_TARGET_DOLPHIN",
    "-DM_PI=3.14159265358979323846",
]


# Helper function for MusyX objects
def MusyX(objects: List[Object], mw_version="GC/1.3.2", major=2, minor=0, patch=3) -> Dict[str, Any]:
    return {
        "lib": "musyx",
        "mw_version": mw_version,
        "src_dir": "extern/musyx/src",
        "cflags": [
            *cflags_musyx,
            f"-DMUSY_VERSION_MAJOR={major}",
            f"-DMUSY_VERSION_MINOR={minor}",
            f"-DMUSY_VERSION_PATCH={patch}",
        ],
        "progress_category": "sdk",
        "host": False,
        "objects": objects,
    }


Matching = True                   # Object matches and should be linked
NonMatching = False               # Object does not match and should not be linked
Equivalent = config.non_matching  # Object should be linked when configured with --non-matching


# Object is only matching for specific versions
def MatchingFor(*versions):
    return config.version in versions


config.warn_missing_config = True
config.warn_missing_source = False
config.libs = [
    {
        "lib": "MetroidPrime",
        "cflags": cflags_retro,
        "mw_version": retro_mw_version,
        "progress_category": "game",  # str | List[str]
        "host": True,
        "objects": [
            Object(NonMatching, "MetroidPrime/main.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/Startup.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/CControlMapper.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/CObjectList.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/CAxisAngle.cpp"),
            Object(NonMatching, "MetroidPrime/CEulerAngles.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/CMatrix3f_Ext.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/CArchMsgParmUserInput.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/CInputGenerator.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/CMainFlow.cpp"),
            Object(NonMatching, "MetroidPrime/CMFGame.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/CCredits.cpp"),
            Object(NonMatching, "MetroidPrime/CSplashScreen.cpp"),
            Object(NonMatching, "MetaRender/CCubeRenderer.cpp"),
            Object(NonMatching, "GuiSys/CGuiFrameFactory.cpp"),
            Object(NonMatching, "GuiSys/CGuiFrame.cpp"),
            Object(NonMatching, "GuiSys/CAuiMeter.cpp"),
            Object(MatchingFor("G2ME01"), "GuiSys/CGuiCompoundWidget.cpp"),
            Object(MatchingFor("G2ME01"), "GuiSys/CGuiSliderGroup.cpp"),
            Object(NonMatching, "GuiSys/CGuiTableGroup.cpp"),
            Object(MatchingFor("G2ME01"), "GuiSys/CRepeatState.cpp"),
            Object(NonMatching, "GuiSys/CGuiCamera.cpp"),
            Object(NonMatching, "GuiSys/CGuiLight.cpp"),
            Object(NonMatching, "GuiSys/CGuiObject.cpp"),
            Object(NonMatching, "GuiSys/CGuiWidget.cpp"),
            Object(NonMatching, "GuiSys/CGuiWidgetIdDB.cpp"),
            Object(NonMatching, "GuiSys/CGuiWidgetDrawParms.cpp"),
            Object(NonMatching, "GuiSys/CAuiEnergyBarT01.cpp"),
            Object(NonMatching, "GuiSys/CAuiImagePane.cpp"),
            Object(NonMatching, "GuiSys/CAuiBitmapMeter.cpp"),
            Object(MatchingFor("G2ME01"), "GuiSys/CGuiHeadWidget.cpp"),
            Object(NonMatching, "GuiSys/CGuiPane.cpp"),
            Object(NonMatching, "GuiSys/CGuiTextPane.cpp"),
            Object(NonMatching, "WorldFormat/COBBTree.cpp"),
            Object(NonMatching, "WorldFormat/CCollidableOBBTree.cpp"),
            Object(NonMatching, "WorldFormat/CCollidableOBBTreeGroup.cpp"),
            Object(NonMatching, "WorldFormat/CAreaOctTree.cpp"),
            Object(NonMatching, "WorldFormat/CMetroidAreaCollider.cpp"),
            Object(NonMatching, "WorldFormat/CAreaOctTree_Tests.cpp"),
            Object(NonMatching, "WorldFormat/CCollisionSurface.cpp"),
            Object(NonMatching, "WorldFormat/CCollisionCache.cpp"),
            Object(NonMatching, "WorldFormat/CMetroidModelInstance.cpp"),
            Object(MatchingFor("G2ME01"), "WorldFormat/CAreaBspTree.cpp"),
            Object(NonMatching, "WorldFormat/CPVSAreaSet.cpp"),
            Object(NonMatching, "WorldFormat/CAreaRenderOctTree.cpp"),
            Object(MatchingFor("G2ME01"), "WorldFormat/CWorldLight.cpp"),
            Object(NonMatching, "MetroidPrime/CStaticGeometryMap.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/CRELFileToken.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/CRELFileManager.cpp"),
            Object(NonMatching, "Collision/CCollidableAABox.cpp"),
            Object(NonMatching, "Collision/CCollidableSphere.cpp"),
            Object(MatchingFor("G2ME01"), "Collision/CCollidableCollisionSurface.cpp"),
            Object(MatchingFor("G2ME01"), "Collision/CCollisionInfo.cpp"),
            Object(MatchingFor("G2ME01"), "Collision/InternalColliders.cpp"),
            Object(MatchingFor("G2ME01"), "Collision/CCollisionPrimitive.cpp"),
            Object(NonMatching, "Collision/CMaterialList.cpp"),
            Object(NonMatching, "Collision/CollisionUtil.cpp"),
            Object(NonMatching, "Collision/COBBox.cpp"),
            Object(MatchingFor("G2ME01"), "Collision/CMRay.cpp"),
            Object(NonMatching, "Collision/CSpatialPrimitive.cpp"),
            Object(NonMatching, "MetroidPrime/CStateManager.cpp"),
            Object(NonMatching, "MetroidPrime/CVisorFlare.cpp"),
            Object(NonMatching, "MetroidPrime/CWorldTransManager.cpp"),
            Object(NonMatching, "MetroidPrime/CRagDoll.cpp"),
            Object(NonMatching, "MetroidPrime/CSortedLists.cpp"),
            Object(NonMatching, "MetroidPrime/CProjectedShadow.cpp"),
            Object(NonMatching, "MetroidPrime/CSlideShow.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/CPreFrontEnd.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptProjectedShadow.cpp"),
            Object(NonMatching, "MetroidPrime/CSteeringBehaviors.cpp"),
            Object(NonMatching, "MetroidPrime/CEntity.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/CArchMsgParmInt32.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/CArchMsgParmInt32Int32VoidPtr.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/CArchMsgParmNull.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/CArchMsgParmReal32.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/Decode.cpp"),
            Object(NonMatching, "MetroidPrime/CIOWinManager.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/CIOWin.cpp"),
            Object(NonMatching, "MetroidPrime/CWorld.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/CArchMsgParmControllerStatus.cpp"),
            Object(NonMatching, "MetroidPrime/CGameArea.cpp"),
            Object(NonMatching, "MetroidPrime/CWorldLayerState.cpp"),
            Object(NonMatching, "MetroidPrime/CMemoryCard.cpp"),
            Object(NonMatching, "MetroidPrime/CMemoryCardDriver.cpp"),
            Object(NonMatching, "MetroidPrime/CSaveGameScreen.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/Weapons/CElectricBeamProjectile.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CDamageEffect.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/CPauseScreenBlur.cpp"),
            Object(NonMatching, "MetroidPrime/CGameHintInfo.cpp"),
            Object(NonMatching, "MetroidPrime/CErrorOutputWindow.cpp"),
            Object(NonMatching, "MetroidPrime/CRainSplashGenerator.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/CWorldSaveGameInfo.cpp"),
            Object(NonMatching, "MetroidPrime/Cameras/CGameCamera.cpp"),
            Object(NonMatching, "MetroidPrime/Cameras/CCameraShakerData.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptCameraFilterKeyframe.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/ScriptObjects/CScriptCameraBlurKeyframe.cpp"),
            Object(NonMatching, "MetroidPrime/Cameras/CCameraFilter.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptCameraShaker.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptActorKeyframe.cpp"),
            Object(NonMatching, "MetroidPrime/CConsoleOutputWindow.cpp"),
            Object(NonMatching, "MetroidPrime/Cameras/CBallCamera.cpp"),
            Object(NonMatching, "MetroidPrime/Cameras/CCameraColliderGroup.cpp"),
            Object(NonMatching, "MetroidPrime/Cameras/CBallCameraTransitions.cpp"),
            Object(NonMatching, "MetroidPrime/Cameras/CFirstPersonCamera.cpp"),
            Object(NonMatching, "MetroidPrime/Cameras/CCameraManager.cpp"),
            Object(NonMatching, "MetroidPrime/Cameras/CCinematicCamera.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptCamera.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/Cameras/CBallCameraTransitionState.cpp"),
            Object(NonMatching, "MetroidPrime/Cameras/CSpindleCamera.cpp"),
            Object(NonMatching, "MetroidPrime/TypesMatch.cpp"),
            Object(NonMatching, "MetroidPrime/Player/CPlayerState.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptTimer.cpp"),
            Object(NonMatching, "MetroidPrime/CAutoMapper.cpp"),
            Object(NonMatching, "MetroidPrime/Player/CPlayerGun.cpp"),
            Object(NonMatching, "MetroidPrime/Player/CPlayerGunBase.cpp"),
            Object(NonMatching, "MetroidPrime/Player/CGrappleArm.cpp"),
            Object(NonMatching, "MetroidPrime/Player/CFidget.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptPickup.cpp"),
            Object(Matching, "MetroidPrime/HUD/CHUDMemoParms.cpp"),
            Object(NonMatching, "MetroidPrime/HUD/CSamusHud.cpp"),
            Object(NonMatching, "MetroidPrime/HUD/CHudRadarInterface.cpp"),
            Object(NonMatching, "MetroidPrime/HUD/CHudBossEnergyInterface.cpp"),
            Object(NonMatching, "MetroidPrime/HUD/CHudDecoInterfaceScan.cpp"),
            Object(NonMatching, "MetroidPrime/HUD/CHudVisorBeamMenu.cpp"),
            Object(NonMatching, "MetroidPrime/CQuitGameScreen.cpp"),
            Object(NonMatching, "MetroidPrime/CPauseScreen.cpp"),
            Object(NonMatching, "MetroidPrime/CInGameGuiManager.cpp"),
            Object(NonMatching, "MetroidPrime/CInGameGuiManagerSet.cpp"),
            Object(NonMatching, "MetroidPrime/CMultiplayerGui.cpp"),
            Object(NonMatching, "MetroidPrime/CSimpleShadow.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/CActorParameters.cpp"),
            Object(NonMatching, "MetroidPrime/CWorldShadow.cpp"),
            Object(Matching, "MetroidPrime/CAudioStateWin.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptRepulsor.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptSound.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptPlatform.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptActor.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptDoor.cpp"),
            Object(NonMatching, "MetroidPrime/CDamageInfo.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptDock.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptDebris.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptEffect.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptTrigger.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptSteam.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptTargetingPoint.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptSpiderBallAttractionSurface.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptRipple.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptTriggerOrientated.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptBallTrigger.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptDamageableTrigger.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptDamageableTriggerOrientated.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptCoverPoint.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptGrapplePoint.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptWater.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptGenerator.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptColorModulate.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptSpecialFunction.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptActorRotate.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptPickupGenerator.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptPointOfInterest.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptEMPulse.cpp"),
            Object(NonMatching, "MetroidPrime/CMapArea.cpp"),
            Object(NonMatching, "MetroidPrime/CMappableObject.cpp"),
            Object(NonMatching, "MetroidPrime/CMapWorldInfo.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptCounter.cpp"),
            Object(NonMatching, "MetroidPrime/CMapWorld.cpp"),
            Object(NonMatching, "MetroidPrime/CMemoryDrawEnum.cpp"),
            Object(NonMatching, "MetroidPrime/CMapUniverse.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptTeamAiMgr.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptMemoryRelay.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptCameraWaypoint.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptCameraHint.cpp"),
            Object(Matching, "MetroidPrime/CAnimRes.cpp"),
            Object(NonMatching, "MetroidPrime/Player/CSamusFaceReflection.cpp"),
            Object(Matching, "MetroidPrime/ScriptObjects/CScriptPlayerHint.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptRoomAcoustics.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptCameraPitch.cpp"),
            Object(NonMatching, "MetroidPrime/CHintState.cpp"),
            Object(NonMatching, "MetroidPrime/CHintManager.cpp"),
            Object(NonMatching, "MetroidPrime/CPlayerHintManager.cpp"),
            Object(NonMatching, "MetroidPrime/CGameHint.cpp"),
            Object(NonMatching, "MetroidPrime/CGameLight.cpp"),
            Object(NonMatching, "MetroidPrime/CExplosion.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/CEffect.cpp"),
            Object(NonMatching, "MetroidPrime/CParticleGenInfoGeneric.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/CParticleGenInfo.cpp"),
            Object(NonMatching, "MetroidPrime/CParticleDatabase.cpp"),
            Object(NonMatching, "MetroidPrime/CAnimData.cpp"),
            Object(NonMatching, "MetroidPrime/Factories/CCharacterFactory.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/Factories/CAssetFactory.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/CAnimationDatabaseGame.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/CTransitionDatabaseGame.cpp"),
            Object(NonMatching, "MetroidPrime/CTargetReticles.cpp"),
            Object(NonMatching, "MetroidPrime/CWeaponMgr.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptHUDMemo.cpp"),
            Object(NonMatching, "MetroidPrime/GameObjectLists.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptAreaProperties.cpp"),
            Object(NonMatching, "MetroidPrime/Player/CPlayerEnergyDrain.cpp"),
            Object(NonMatching, "MetroidPrime/Weapons/CIceImpact.cpp"),
            Object(NonMatching, "MetroidPrime/Player/CStaticInterference.cpp"),
            Object(NonMatching, "MetroidPrime/PathFinding/CPathFindSearch.cpp"),
            Object(NonMatching, "MetroidPrime/PathFinding/CPathFindRegion.cpp"),
            Object(NonMatching, "MetroidPrime/PathFinding/CPathFindArea.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/PathFinding/CPathFindSpline.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/CHealthInfo.cpp"),
            Object(NonMatching, "MetroidPrime/Player/CGameState.cpp"),
            Object(NonMatching, "MetroidPrime/Tweaks/CTweakAutoMapper.cpp"),
            Object(NonMatching, "MetroidPrime/Tweaks/CTweakBall.cpp"),
            Object(NonMatching, "MetroidPrime/Tweaks/CTweakPlayer.cpp"),
            Object(NonMatching, "MetroidPrime/Tweaks/CTweakPlayerGun.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/Tweaks/CTweakPlayerRes.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/CRelFile.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/Tweaks/CTweakTargeting.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/Tweaks/CTweakGuiColors.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/Tweaks/CTweakGui.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/Player/CFrontEndGameMode.cpp"),
            Object(NonMatching, "MetroidPrime/Player/CGMDeathMatch.cpp"),
            Object(NonMatching, "MetroidPrime/Player/CGMCoin.cpp"),
            Object(NonMatching, "MetroidPrime/Player/CGMMultiplayer.cpp"),
            Object(NonMatching, "MetroidPrime/CBoneTracking.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/Player/CFaceplateDecoration.cpp"),
            Object(NonMatching, "MetroidPrime/Player/CGameOptions.cpp"),
            Object(NonMatching, "MetroidPrime/CEnvFxManager.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/CRumbleManager.cpp"),
            Object(NonMatching, "MetroidPrime/CFluidUVMotion.cpp"),
            Object(NonMatching, "MetroidPrime/CFluidPlane.cpp"),
            Object(NonMatching, "MetroidPrime/CFluidPlaneManager.cpp"),
            Object(NonMatching, "MetroidPrime/CFluidPlaneCPU.cpp"),
            Object(NonMatching, "MetroidPrime/CCollisionActorManager.cpp"),
            Object(NonMatching, "MetroidPrime/CCollisionActor.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/Enemies/CBurstFire.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptSequenceTimer.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptTriggerEllipsoid.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptSpindleCamera.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptPathCamera.cpp"),
            Object(NonMatching, "MetroidPrime/Cameras/CPathCamera.cpp"),
            Object(NonMatching, "MetroidPrime/Cameras/CInterpolationCamera.cpp"),
            Object(NonMatching, "MetroidPrime/Cameras/CCameraSurface.cpp"),
            Object(NonMatching, "MetroidPrime/Cameras/CCylinderCameraSurface.cpp"),
            Object(NonMatching, "MetroidPrime/Cameras/CSplineCylinderCameraSurface.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/Cameras/CSplinePlaneCameraSurface.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptMidi.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptStreamedMusic.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptLoaderRel.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptLoader.cpp"),
            Object(NonMatching, "MetroidPrime/Weapons/CBomb.cpp"),
            Object(NonMatching, "MetroidPrime/Weapons/CPowerBomb.cpp"),
            Object(NonMatching, "MetroidPrime/Weapons/CDarkBeam.cpp"),
            Object(NonMatching, "MetroidPrime/Weapons/CAnnihilatorBeam.cpp"),
            Object(NonMatching, "MetroidPrime/Weapons/CPowerBeam.cpp"),
            Object(NonMatching, "MetroidPrime/Weapons/CAuxWeapon.cpp"),
            Object(NonMatching, "MetroidPrime/Weapons/CLightBeam.cpp"),
            Object(NonMatching, "MetroidPrime/Weapons/GunController/CGunMotion.cpp"),
            Object(NonMatching, "MetroidPrime/Weapons/CGunWeapon.cpp"),
            Object(NonMatching, "MetroidPrime/Weapons/GunController/CGunController.cpp"),
            Object(NonMatching, "MetroidPrime/Weapons/GunController/CGSComboFire.cpp"),
            Object(NonMatching, "MetroidPrime/Weapons/GunController/CGSFidget.cpp"),
            Object(NonMatching, "MetroidPrime/Weapons/GunController/CGSFreeLook.cpp"),
            Object(NonMatching, "MetroidPrime/Weapons/CWeapon.cpp"),
            Object(NonMatching, "MetroidPrime/Weapons/CGameProjectile.cpp"),
            Object(NonMatching, "MetroidPrime/Weapons/CEnergyProjectile.cpp"),
            Object(NonMatching, "MetroidPrime/Weapons/CTargetableProjectile.cpp"),
            Object(NonMatching, "MetroidPrime/Weapons/CAnnihilatorProjectile.cpp"),
            Object(NonMatching, "MetroidPrime/Weapons/CProjectileInfo.cpp"),
            Object(NonMatching, "MetroidPrime/CInGameTweakManager.cpp"),
            Object(NonMatching, "MetroidPrime/CIkChain.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/RumbleFxTable.cpp"),
            Object(NonMatching, "MetroidPrime/Weapons/CBeamProjectile.cpp"),
            Object(NonMatching, "MetroidPrime/Enemies/CBouncyGrenade.cpp"),
            Object(NonMatching, "MetroidPrime/Enemies/CElitePirateGrenadeLauncher.cpp"),
            Object(NonMatching, "MetroidPrime/Weapons/CShockWave.cpp"),
            Object(NonMatching, "MetroidPrime/Weapons/CPlasmaProjectile.cpp"),
            Object(NonMatching, "Weapons/CProjectileWeapon.cpp"),
            Object(NonMatching, "Weapons/CCollisionResponseData.cpp"),
            Object(Matching, "Weapons/IWeaponRenderer.cpp"),
            Object(Matching, "Weapons/CDecalDataFactory.cpp"),
            Object(NonMatching, "Weapons/CDecal.cpp"),
            Object(Matching, "Weapons/CDecalDescription.cpp"),
            Object(NonMatching, "MetroidPrime/CDecalManager.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptSpiderBallWaypoint.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/TGameTypes.cpp"),
            Object(NonMatching, "MetroidPrime/CPhysicsActor.cpp"),
            Object(NonMatching, "MetroidPrime/Player/CPlayerVisor.cpp"),
            Object(NonMatching, "MetroidPrime/CModelData.cpp"),
            Object(NonMatching, "MetroidPrime/CDamageVulnerability.cpp"),
            Object(NonMatching, "MetroidPrime/CActorLights.cpp"),
            Object(NonMatching, "MetroidPrime/CGroundMovement.cpp"),
            Object(NonMatching, "MetroidPrime/CGameCollision.cpp"),
            Object(NonMatching, "MetroidPrime/Enemies/CAmbientAI.cpp"),
            Object(NonMatching, "MetroidPrime/Enemies/CAi.cpp"),
            Object(NonMatching, "MetroidPrime/Enemies/CStateMachine.cpp"),
            Object(NonMatching, "MetroidPrime/Factories/CStateMachineFactory.cpp"),
            Object(NonMatching, "MetroidPrime/Enemies/CAiKnockBackMgr.cpp"),
            Object(NonMatching, "MetroidPrime/Enemies/CKnockBackMgr.cpp"),
            Object(NonMatching, "MetroidPrime/Player/CPlayerKnockBackMgr.cpp"),
            Object(NonMatching, "MetroidPrime/Player/CPlayerBodyLocomotion.cpp"),
            Object(NonMatching, "MetroidPrime/Player/CPlayerBodyJump.cpp"),
            Object(NonMatching, "MetroidPrime/Player/CPlayerRagDoll.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptWaypoint.cpp"),
            Object(NonMatching, "MetroidPrime/Enemies/CPatterned.cpp"),
            Object(NonMatching, "MetroidPrime/Enemies/CPatternedAiFunctions.cpp"),
            Object(NonMatching, "MetroidPrime/BodyState/CBodyController.cpp"),
            Object(NonMatching, "MetroidPrime/BodyState/CBodyStateCmdMgr.cpp"),
            Object(NonMatching, "MetroidPrime/BodyState/CBodyStateInfo.cpp"),
            Object(NonMatching, "MetroidPrime/BodyState/CBSLocomotion.cpp"),
            Object(NonMatching, "MetroidPrime/BodyState/CBSHurled.cpp"),
            Object(NonMatching, "MetroidPrime/BodyState/CBSJump.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/BodyState/CBSScripted.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/BodyState/CBSAttack.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/BodyState/CBSLoopAttack.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/BodyState/CBSCover.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/BodyState/CBSLoopReaction.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/BodyState/CBSGenerate.cpp"),
            Object(NonMatching, "MetroidPrime/BodyState/CBSKnockBack.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/BodyState/CBSFall.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/BodyState/CBSGetup.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/BodyState/CBSLieOnGround.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/BodyState/CBSDie.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/BodyState/CBSGroundHit.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/BodyState/CBSSlide.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/BodyState/CBSStep.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/BodyState/CBSTaunt.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptDistanceFog.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/BodyState/CBSProjectileAttack.cpp"),
            Object(NonMatching, "MetroidPrime/BodyState/CBSTurn.cpp"),
            Object(NonMatching, "MetroidPrime/BodyState/CBSWallHang.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptVisorFlare.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CHUDBillboardEffect.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptWorldTeleporter.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptVisorGoo.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptControllerAction.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/ScriptObjects/CScriptSwitch.cpp"),
            Object(NonMatching, "MetroidPrime/BodyState/CABSIdle.cpp"),
            Object(NonMatching, "MetroidPrime/BodyState/CABSFlinch.cpp"),
            Object(NonMatching, "MetroidPrime/BodyState/CABSAim.cpp"),
            Object(NonMatching, "MetroidPrime/BodyState/CABSReaction.cpp"),
            Object(NonMatching, "MetroidPrime/CActor.cpp"),
            Object(NonMatching, "MetroidPrime/CEchoEmitter.cpp"),
            Object(NonMatching, "MetroidPrime/SwarmRenderHelpers.cpp"),
            Object(NonMatching, "MetroidPrime/CScriptObjectLoaderHelper.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptSoundModifier.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptAdvancedCounter.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptLayerController.cpp"),
            Object(NonMatching, "MetroidPrime/Player/CPlayerTargeting.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptPlayerStateChange.cpp"),
            Object(NonMatching, "MetroidPrime/CActorModelParticles.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptAiJumpPoint.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/CMessageScreen.cpp"),
            Object(NonMatching, "MetroidPrime/CDamageInfoModifiers.cpp"),
            Object(NonMatching, "MetroidPrime/Weapons/WeaponTypes.cpp"),
            Object(Matching, "MetroidPrime/CScriptMailbox.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptRelay.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptSpawnPoint.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptRandomRelay.cpp"),
            Object(MatchingFor("G2ME01"), "MetroidPrime/CRuleSet.cpp"),
            Object(NonMatching, "MetroidPrime/CRuleSetEvaluator.cpp"),
            Object(NonMatching, "MetroidPrime/CLineOfSightTracker.cpp"),
            Object(NonMatching, "MetroidPrime/CSurfaceAlignmentHelper.cpp"),
            Object(NonMatching, "MetroidPrime/Player/CPlayer.cpp"),
            Object(NonMatching, "MetroidPrime/Player/CPlayerDynamics.cpp"),
            Object(NonMatching, "MetroidPrime/Player/CPlayerOrbit.cpp"),
            Object(NonMatching, "MetroidPrime/Player/CPlayerHints.cpp"),
            Object(NonMatching, "MetroidPrime/Player/CPlayerCameraBob.cpp"),
            Object(NonMatching, "MetroidPrime/Factories/CScannableObjectInfo.cpp"),
            Object(NonMatching, "MetroidPrime/Player/CScanDisplay.cpp"),
            Object(NonMatching, "MetroidPrime/Player/CMorphBall.cpp"),
            Object(NonMatching, "MetroidPrime/Player/CMorphBallShadow.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScanTreeInventory.cpp"),
        ],
    },
    {
        "lib": "Kyoto_CW",
        "mw_version": "GC/2.7",
        "cflags": cflags_retro,
        "progress_category": "game",  # str | List[str]
        "host": True,
        "objects": [
            Object(Matching, "Kyoto/Basics/CStopwatch.cpp"),
            Object(NonMatching, "Kyoto/Basics/CBasicsDolphin.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Alloc/CCallStackDolphin.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Basics/COsContextDolphin.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Basics/CSWDataDolphin.cpp"),
            Object(Matching, "Kyoto/Basics/RAssertDolphin.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/CDvdRequest.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/CDvdRequestManager.cpp"),
            Object(NonMatching, "Kyoto/Graphics/CLight.cpp"),  # Float literal order
            Object(NonMatching, "Kyoto/Graphics/CCubeModel.cpp"),
            Object(NonMatching, "Kyoto/Graphics/CGX.cpp"),
            Object(NonMatching, "Kyoto/Graphics/DolphinCGraphics.cpp"),
            Object(NonMatching, "Kyoto/Graphics/DolphinCTexture.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Math/CloseEnough.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Math/CMatrix3f.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Math/CMatrix4f.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Math/CNUQuaternion.cpp"),
            Object(NonMatching, "Kyoto/Math/CQuaternion.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/CRandom16.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/CObjectReference.cpp"),
            Object(NonMatching, "Kyoto/CSimplePool.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/CToken.cpp"),
            Object(NonMatching, "Kyoto/Math/CTransform4f.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Math/CUnitVector3f.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Math/CAABox.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Math/CTri.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Math/CQuad.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Math/CCylinder.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Math/CLine.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Math/CPlane.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Math/CSphere.cpp"),
            Object(NonMatching, "Kyoto/CFactoryMgr.cpp"),
            Object(NonMatching, "Kyoto/CResFactory.cpp"),
            Object(Matching, "Kyoto/CResLoader.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/CARAMManager.cpp"),
            Object(NonMatching, "Kyoto/Math/CFrustumPlanes.cpp"),
            Object(NonMatching, "Kyoto/Graphics/CCubeMaterial.cpp"),
            Object(NonMatching, "Kyoto/Graphics/CDisplayListReader.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Graphics/CCubeSurface.cpp"),
            Object(Matching, "Kyoto/Math/CVector2f.cpp"),
            Object(Matching, "Kyoto/Math/CVector2i.cpp"),
            Object(Matching, "Kyoto/Math/CVector3d.cpp"),
            Object(Matching, "Kyoto/Math/CVector3f.cpp"),
            Object(Matching, "Kyoto/Math/CVector3i.cpp"),
            Object(NonMatching, "Kyoto/Math/RMathUtils.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/CCrc32.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Alloc/CCircularBuffer.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Alloc/CMemory.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Alloc/LockedCache.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Alloc/CMediumAllocPool.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Alloc/CSmallAllocPool.cpp"),
            Object(NonMatching, "Kyoto/Alloc/CGameAllocator.cpp"),
            Object(NonMatching, "Kyoto/Animation/DolphinCSkinnedModel.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Graphics/CGXTransientBuffer.cpp"),
            Object(NonMatching, "Kyoto/Animation/DolphinCSkinRules.cpp"),
            Object(NonMatching, "Kyoto/Animation/DolphinCVirtualBone.cpp"),
            Object(NonMatching, "Kyoto/Graphics/DolphinCModel.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Alloc/IAllocator.cpp"),
            Object(NonMatching, "Kyoto/PVS/CPVSVisOctree.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/PVS/CPVSVisSet.cpp"),
            Object(NonMatching, "Kyoto/DolphinCMemoryCardSys.cpp"),
            Object(Matching, "Kyoto/Input/DolphinIController.cpp"),
            Object(Matching, "Kyoto/Input/CDolphinController.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Animation/CSegIdList.cpp"),
            Object(NonMatching, "Kyoto/Animation/CAnimSource.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Graphics/DolphinCPalette.cpp"),
            Object(NonMatching, "Kyoto/Animation/CAnimMathUtils.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Animation/CAdvancementDeltas.cpp"),
            Object(NonMatching, "Kyoto/Animation/CAnimSourceReader.cpp"),
            Object(NonMatching, "Kyoto/Animation/CAnimSourceReaderBase.cpp"),
            Object(NonMatching, "Kyoto/Animation/CAnimTreeAnimReaderContainer.cpp"),
            Object(NonMatching, "Kyoto/Animation/CAnimTreeDoubleChild.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Animation/CAnimTreeLoopIn.cpp"),
            Object(NonMatching, "Kyoto/Animation/CAnimTreeNode.cpp"),
            Object(NonMatching, "Kyoto/Animation/CAnimTreeSequence.cpp"),
            Object(NonMatching, "Kyoto/Animation/CAnimTreeSingleChild.cpp"),
            Object(NonMatching, "Kyoto/Animation/CAnimTreeBlend.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Animation/CAnimTreeTimeScale.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Animation/CTimeScaleFunctions.cpp"),
            Object(NonMatching, "Kyoto/Animation/CAnimTreeTransition.cpp"),
            Object(NonMatching, "Kyoto/Animation/CAnimTreeTweenBase.cpp"),
            Object(NonMatching, "Kyoto/Animation/IAnimReader.cpp"),
            Object(NonMatching, "Kyoto/Animation/CAnimation.cpp"),
            Object(NonMatching, "Kyoto/Animation/CAnimationSet.cpp"),
            Object(NonMatching, "Kyoto/Animation/CAnimCharacterSet.cpp"),
            Object(NonMatching, "Kyoto/Animation/CCharacterInfo.cpp"),
            Object(NonMatching, "Kyoto/Animation/CCharacterSet.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Animation/CAnimPOIData.cpp"),
            Object(NonMatching, "Kyoto/Animation/CCharLayoutInfo.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Animation/CHierarchyPoseBuilder.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Animation/CMetaAnimBlend.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Animation/CMetaAnimPhaseBlend.cpp"),
            Object(NonMatching, "Kyoto/Animation/CMetaAnimRandom.cpp"),
            Object(NonMatching, "Kyoto/Animation/CMetaAnimSequence.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Animation/CMetaAnimFactory.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Animation/CMetaAnimPlay.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Animation/CMetaTransFactory.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Animation/CMetaTransMetaAnim.cpp"),
            Object(NonMatching, "Kyoto/Animation/CMetaTransPhaseTrans.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Animation/CMetaTransSnap.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Animation/CMetaTransTrans.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Animation/IMetaAnim.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Animation/CPrimitive.cpp"),
            Object(NonMatching, "Kyoto/Animation/CSequenceHelper.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Animation/CTransition.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Animation/CTransitionManager.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Animation/CTreeUtils.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Animation/CAllFormatsAnimSource.cpp"),
            Object(NonMatching, "Kyoto/Animation/CJointData_LinearStorage.cpp"),
            Object(NonMatching, "Kyoto/Animation/CFBStreamedAnimReader.cpp"),
            Object(NonMatching, "Kyoto/Animation/CFBStreamedCompression.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Animation/CBoolPOINode.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Animation/CCharAnimMemoryMetrics.cpp"),
            Object(NonMatching, "Kyoto/Animation/CInt32POINode.cpp"),
            Object(NonMatching, "Kyoto/Animation/CParticlePOINode.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Animation/CPASAnimParm.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Animation/CPASAnimInfo.cpp"),
            Object(NonMatching, "Kyoto/Animation/CPASAnimState.cpp"),
            Object(NonMatching, "Kyoto/Animation/CPASDatabase.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Animation/CPASParmInfo.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Animation/CPOINode.cpp"),
            Object(NonMatching, "Kyoto/Animation/CSoundPOINode.cpp"),
            Object(NonMatching, "Kyoto/Animation/CPoseAsTransforms_Linear.cpp"),
            Object(NonMatching, "Kyoto/Particles/CColorElement.cpp"),
            Object(NonMatching, "Kyoto/Particles/CDeferredParticleEffect.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/CRelFileDebugInfo.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Animation/CSegId.cpp"),
            Object(NonMatching, "Kyoto/Animation/CSegStatementSet.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Graphics/CTevCombiners.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Input/CFinalInput.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Graphics/CColor.cpp"),
            Object(NonMatching, "Kyoto/Graphics/DolphinCColor.cpp"),
            Object(NonMatching, "Kyoto/CDependencyGroup.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Input/CRumbleVoice.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Input/RumbleAdsr.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Input/CRumbleGenerator.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Animation/CCharAnimTime.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Animation/CTimeRemainderAndFraction.cpp"),
            Object(NonMatching, "Kyoto/DolphinCDvdFile.cpp"),
            Object(NonMatching, "Kyoto/Graphics/CCubeMoviePlayer.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Animation/CAdditiveAnimPlayback.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Particles/CParticleElectricDataFactory.cpp"),
            Object(NonMatching, "Kyoto/Particles/CParticleSpawnSystemDataFactory.cpp"),
            Object(NonMatching, "Kyoto/Particles/CSortedParticleSystemDataFactory.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Particles/CSpawnSystemDescription.cpp"),
            Object(NonMatching, "Kyoto/Particles/CParticleElectric.cpp"),
            Object(NonMatching, "Kyoto/Particles/CElementGen.cpp"),
            Object(NonMatching, "Kyoto/Particles/CParticleSpawnSystem.cpp"),
            Object(NonMatching, "Kyoto/Particles/CSortedParticleSystem.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Particles/CSortedParticleSystemDescription.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Particles/CParticleSwooshDataFactory.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Particles/CRealElement.cpp"),
            Object(NonMatching, "Kyoto/Particles/CSpawnSystemKeyframeData.cpp"),
            Object(NonMatching, "Kyoto/Particles/CUVElement.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Particles/CVectorElement.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Audio/g721.cpp"),
            Object(NonMatching, "Kyoto/Audio/CStaticAudioPlayer.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Audio/CSfxPitchBend.cpp"),
            Object(NonMatching, "Kyoto/Audio/DolphinCAudioGroupSet.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Audio/DolphinCAudioSys.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Audio/CStreamAudioManager.cpp"),
            Object(NonMatching, "Kyoto/Audio/CDSPStreamManager.cpp"),
            Object(NonMatching, "Kyoto/CFrameDelayedKiller.cpp"),
            Object(NonMatching, "Kyoto/Text/CStringTable.cpp"),
            Object(NonMatching, "Kyoto/Text/CRasterFont.cpp"),
            Object(NonMatching, "Kyoto/Text/CTextExecuteBuffer.cpp"),
            Object(NonMatching, "Kyoto/Text/CGuiTextSupport.cpp"),
            Object(NonMatching, "Kyoto/Text/CTextParser.cpp"),
            Object(NonMatching, "Kyoto/Particles/CEmitterElement.cpp"),
            Object(NonMatching, "Kyoto/Particles/CEffectComponent.cpp"),
            Object(NonMatching, "Kyoto/Particles/CIntElement.cpp"),
            Object(Matching, "Kyoto/Particles/CModVectorElement.cpp"),
            Object(Matching, "Kyoto/Particles/CParticleDataFactory.cpp"),
            Object(NonMatching, "Kyoto/Particles/CParticleGen.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Particles/CParticleGlobals.cpp"),
            Object(NonMatching, "Kyoto/Particles/CParticleSwoosh.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Particles/CParticleData.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/CTimeProvider.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/CARAMToken.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Particles/CElectricDescription.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Particles/CSwooshDescription.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Particles/CGenDescription.cpp"),
            Object(NonMatching, "Kyoto/CPakFile.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Audio/CMidiManager.cpp"),
            Object(NonMatching, "Kyoto/Audio/CSfxHandle.cpp"),
            Object(NonMatching, "Kyoto/Audio/CSfxManager.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Text/CFontImageDef.cpp"),
            Object(NonMatching, "Kyoto/Text/CTextRenderBuffer.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Text/CDrawStringOptions.cpp"),
            Object(NonMatching, "Kyoto/Text/CFontRenderState.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Text/CBlockInstruction.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Text/CFont.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Text/CLineInstruction.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Text/CWordInstruction.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Text/CWordBreakTables.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Text/CTextInstruction.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Text/CFontInstruction.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Text/CImageInstruction.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Text/CColorInstruction.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Text/CColorOverrideInstruction.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Text/CRemoveColorOverrideInstruction.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Text/CLineSpacingInstruction.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Text/CLineExtraSpaceInstruction.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Text/CPushStateInstruction.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Text/CPopStateInstruction.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Text/CSaveableState.cpp"),
            Object(NonMatching, "Kyoto/Math/CMayaSpline.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Math/CGameCameraSpline.cpp"),
            Object(NonMatching, "Kyoto/Math/CGameSpline.cpp"),
            Object(NonMatching, "Kyoto/Math/CMotionSpline.cpp"),
            Object(NonMatching, "Kyoto/Audio/CAuxEffect.cpp"),
            Object(NonMatching, "Kyoto/Audio/CAudioSys.cpp"),
            Object(NonMatching, "Kyoto/Audio/CBitcrusher.cpp"),
            Object(NonMatching, "Kyoto/Audio/CAuxEffectManager.cpp"),
            Object(NonMatching, "Kyoto/Audio/CPhaser.cpp"),
            Object(NonMatching, "Kyoto/Audio/CFlanger.cpp"),
            Object(NonMatching, "Kyoto/Audio/AudioEffect.cpp"),
            Object(NonMatching, "Kyoto/Audio/AudioEffectX.cpp"),
            Object(NonMatching, "Kyoto/Audio/CCustomAudioAux.cpp"),
            Object(NonMatching, "Kyoto/Audio/CFilteredDelayAux.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Streams/CInputStream.cpp"),
            Object(Matching, "Kyoto/Streams/CBufferedDvdRequest.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Streams/CBitStreamReader.cpp"),
            Object(MatchingFor("G2ME01"), "rstl/rstl_map.cpp"),
            Object(
                MatchingFor("G2ME01"),
                "rstl/rstl_strings.cpp",
                extra_cflags=["-inline deferred"] if config.version == "G2ME01" else [],
            ),
            Object(MatchingFor("G2ME01"), "rstl/rstl_misc.cpp"),
            Object(NonMatching, "rstl/RstlExtras.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Streams/COutputStream.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Streams/CMemoryStreamOut.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Streams/CBitStreamWriter.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Streams/CMemoryInStream.cpp"),
            Object(NonMatching, "Kyoto/Streams/DolphinCLZOInputStream.cpp"),
            Object(NonMatching, "Kyoto/Streams/CFilePreload.cpp"),
            Object(MatchingFor("G2ME01"), "Kyoto/Streams/CLZOSupport.cpp", extra_cflags=["-i include/LZO"]),
        ],
    },
    {
        "lib": "LZO",
        "mw_version": "GC/2.7",
        "cflags": cflags_runtime + ["-i include/LZO"],
        "progress_category": "sdk",
        "host": False,
        "objects": [
            Object(MatchingFor("G2ME01"), "LZO/lzo_init.c"),
            Object(MatchingFor("G2ME01"), "LZO/lzo_ptr.c"),
            Object(MatchingFor("G2ME01"), "LZO/lzo1x_d1.c"),
        ],
    },
    {
        "lib": "Runtime.PPCEABI.H",
        "mw_version": config.linker_version,
        "cflags": cflags_runtime,
        "progress_category": "sdk",  # str | List[str]
        "host": False,
        "objects": [
            Object(Matching, "Runtime/global_destructor_chain.c"),
            Object(MatchingFor("G2ME01"), "Runtime/__va_arg.c"),
            Object(MatchingFor("G2ME01"), "Runtime/CPlusLibPPC.cpp"),
            Object(NonMatching, "Runtime/NMWException.cp", extra_cflags=["-RTTI on", "-Cpp_exceptions on"]),
            Object(MatchingFor("G2ME01"), "Runtime/ptmf.c"),
            Object(MatchingFor("G2ME01"), "Runtime/runtime.c"),
            Object(Matching, "Runtime/__init_cpp_exceptions.cpp"),
            # TODO: need to implement all
            Object(NonMatching, "Runtime/Gecko_ExceptionPPC.cp"),
        ],
    },
    {
        "lib": "MSL_C.PPCEABI.bare.H",
        "mw_version": config.linker_version,
        "cflags": [*cflags_runtime, "-DMSL_OLD_FP_CLASSIFY", "-DMSL_NO_INLINE_SQRT"],
        "progress_category": "sdk",
        "host": False,
        "objects": [
            Object(MatchingFor("G2ME01"), "Runtime/arith.c"),
            Object(MatchingFor("G2ME01"), "Runtime/buffer_io.c"),
            Object(MatchingFor("G2ME01"), "Runtime/critical_regions.gamecube.c"),
            Object(MatchingFor("G2ME01"), "Runtime/ctype.c"),
            Object(MatchingFor("G2ME01"), "Runtime/abort_exit.c"),
            Object(MatchingFor("G2ME01"), "Runtime/alloc.c"),
            Object(MatchingFor("G2ME01"), "Runtime/errno.c"),
            Object(MatchingFor("G2ME01"), "Runtime/ansi_files.c"),
            Object(MatchingFor("G2ME01"), "Runtime/ansi_fp.c"),
            Object(MatchingFor("G2ME01"), "Runtime/locale.c"),
            Object(MatchingFor("G2ME01"), "Runtime/direct_io.c"),
            Object(MatchingFor("G2ME01"), "Runtime/file_io.c"),
            Object(MatchingFor("G2ME01"), "Runtime/FILE_POS.c"),
            Object(MatchingFor("G2ME01"), "Runtime/mbstring.c"),
            Object(MatchingFor("G2ME01"), "Runtime/printf.c"),
            Object(MatchingFor("G2ME01"), "Runtime/qsort.c"),
            Object(MatchingFor("G2ME01"), "Runtime/rand.c"),
            Object(MatchingFor("G2ME01"), "Runtime/float.c"),
            Object(MatchingFor("G2ME01"), "Runtime/sscanf.c"),
            Object(MatchingFor("G2ME01"), "Runtime/signal.c"),
            Object(MatchingFor("G2ME01"), "Runtime/strtold.c"),
            Object(MatchingFor("G2ME01"), "Runtime/uart_console_io.c"),
            Object(MatchingFor("G2ME01"), "Runtime/mem.c"),
            Object(MatchingFor("G2ME01"), "Runtime/mem_funcs.c"),
            Object(MatchingFor("G2ME01"), "Runtime/misc_io.c"),
            Object(MatchingFor("G2ME01"), "Runtime/wchar_io.c"),
            Object(MatchingFor("G2ME01"), "Runtime/string.c"),
            Object(MatchingFor("G2ME01"), "Runtime/e_acos.c"),
            Object(MatchingFor("G2ME01"), "Runtime/e_asin.c"),
            Object(MatchingFor("G2ME01"), "Runtime/e_atan2.c"),
            Object(MatchingFor("G2ME01"), "Runtime/e_fmod.c"),
            Object(MatchingFor("G2ME01"), "Runtime/e_log.c"),
            Object(MatchingFor("G2ME01"), "Runtime/e_pow.c"),
            Object(MatchingFor("G2ME01"), "Runtime/e_rem_pio2.c"),
            Object(MatchingFor("G2ME01"), "Runtime/k_cos.c"),
            Object(MatchingFor("G2ME01"), "Runtime/k_rem_pio2.c"),
            Object(MatchingFor("G2ME01"), "Runtime/k_sin.c"),
            Object(MatchingFor("G2ME01"), "Runtime/k_tan.c"),
            Object(MatchingFor("G2ME01"), "Runtime/s_atan.c"),
            Object(MatchingFor("G2ME01"), "Runtime/s_copysign.c"),
            Object(MatchingFor("G2ME01"), "Runtime/s_cos.c"),
            Object(MatchingFor("G2ME01"), "Runtime/s_floor.c"),
            Object(MatchingFor("G2ME01"), "Runtime/s_frexp.c"),
            Object(MatchingFor("G2ME01"), "Runtime/s_ldexp.c"),
            Object(MatchingFor("G2ME01"), "Runtime/s_modf.c"),
            Object(MatchingFor("G2ME01"), "Runtime/s_sin.c"),
            Object(MatchingFor("G2ME01"), "Runtime/s_tan.c"),
            Object(MatchingFor("G2ME01"), "Runtime/w_acos.c"),
            Object(MatchingFor("G2ME01"), "Runtime/w_asin.c"),
            Object(MatchingFor("G2ME01"), "Runtime/w_atan2.c"),
            Object(MatchingFor("G2ME01"), "Runtime/w_fmod.c"),
            Object(MatchingFor("G2ME01"), "Runtime/e_log10.c"),
            Object(MatchingFor("G2ME01"), "Runtime/w_pow.c"),
            Object(MatchingFor("G2ME01"), "Runtime/w_log10.c"),
            Object(MatchingFor("G2ME01"), "Runtime/e_sqrt.c"),
            Object(MatchingFor("G2ME01"), "Runtime/math_ppc.c"),
            Object(MatchingFor("G2ME01"), "Runtime/w_sqrt.c"),
        ],
    },
    DolphinLib(
        "dsp",
        [
            Object(MatchingFor("G2ME01"), "Dolphin/dsp/dsp.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/dsp/dsp_debug.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/dsp/dsp_task.c"),
        ],
    ),
    DolphinLib(
        "dtk",
        [
            Object(MatchingFor("G2ME01"), "Dolphin/dtk.c"),
        ],
    ),
    DolphinLib(
        "dvd",
        [
            Object(MatchingFor("G2ME01"), "Dolphin/dvd/dvdlow.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/dvd/dvdfs.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/dvd/dvd.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/dvd/dvdqueue.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/dvd/dvderror.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/dvd/dvdidutils.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/dvd/dvdfatal.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/dvd/fstload.c"),
        ],
    ),
    DolphinLib(
        "exi",
        [
            Object(MatchingFor("G2ME01"), "Dolphin/exi/EXIBios.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/exi/EXIUart.c"),
        ],
    ),
    DolphinLib(
        "gx",
        [
            Object(MatchingFor("G2ME01"), "Dolphin/gx/GXInit.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/gx/GXFifo.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/gx/GXAttr.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/gx/GXMisc.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/gx/GXGeometry.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/gx/GXFrameBuf.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/gx/GXLight.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/gx/GXTexture.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/gx/GXBump.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/gx/GXTev.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/gx/GXPixel.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/gx/GXDisplayList.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/gx/GXTransform.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/gx/GXPerf.c"),
        ],
    ),
    DolphinLib(
        "mtx",
        [
            Object(MatchingFor("G2ME01"), "Dolphin/mtx/mtx.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/mtx/mtxvec.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/mtx/mtx44.c"),
        ],
    ),
    DolphinLib(
        "pad",
        [
            Object(MatchingFor("G2ME01"), "Dolphin/pad/Pad.c", extra_cflags=["-char unsigned"]),
            Object(MatchingFor("G2ME01"), "Dolphin/pad/PadClamp.c"),
        ],
    ),
    DolphinLib(
        "vi",
        [
            Object(MatchingFor("G2ME01"), "Dolphin/vi/vi.c"),
        ],
    ),
    DolphinLib(
        "ai",
        [
            Object(MatchingFor("G2ME01"), "Dolphin/ai.c"),
        ],
    ),
    DolphinLib(
        "si",
        [
            Object(MatchingFor("G2ME01"), "Dolphin/si/SIBios.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/si/SISamplingRate.c"),
        ],
    ),
    DolphinLib(
        "thp",
        [
            Object(MatchingFor("G2ME01"), "Dolphin/thp/THPDec.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/thp/THPAudio.c"),
        ],
    ),
    DolphinLib(
        "ar",
        [
            Object(Matching, "Dolphin/ar/ar.c"),
            Object(Matching, "Dolphin/ar/arq.c"),
        ],
    ),
    DolphinLib(
        "card",
        [
            Object(Matching, "Dolphin/card/CARDBios.c"),
            Object(Matching, "Dolphin/card/CARDUnlock.c"),
            Object(Matching, "Dolphin/card/CARDRdwr.c"),
            Object(Matching, "Dolphin/card/CARDBlock.c"),
            Object(Matching, "Dolphin/card/CARDDir.c"),
            Object(Matching, "Dolphin/card/CARDCheck.c"),
            Object(Matching, "Dolphin/card/CARDMount.c"),
            Object(Matching, "Dolphin/card/CARDFormat.c"),
            Object(Matching, "Dolphin/card/CARDOpen.c"),
            Object(Matching, "Dolphin/card/CARDCreate.c"),
            Object(Matching, "Dolphin/card/CARDRead.c"),
            Object(Matching, "Dolphin/card/CARDWrite.c"),
            Object(Matching, "Dolphin/card/CARDDelete.c"),
            Object(Matching, "Dolphin/card/CARDStat.c"),
            Object(Matching, "Dolphin/card/CARDRename.c"),
            Object(Matching, "Dolphin/card/CARDStatEx.c"),
            Object(Matching, "Dolphin/card/CARDRaw.c"),
            Object(Matching, "Dolphin/card/CARDNet.c"),
            Object(Matching, "Dolphin/card/CARDErase.c"),
            Object(Matching, "Dolphin/card/CARDProgram.c"),
        ],
    ),
    DolphinLib(
        "base",
        [
            Object(Matching, "Dolphin/PPCArch.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/db.c"),
        ],
    ),
    DolphinLib(
        "os",
        [
            Object(Matching, "Dolphin/os/OSCache.c"),
            Object(Matching, "Dolphin/os/OSContext.c"),
            Object(Matching, "Dolphin/os/OSError.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/os/OSExec.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/os/OSFatal.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/os/OSFont.c", extra_cflags=["-char unsigned"]),
            Object(Matching, "Dolphin/os/OSInterrupt.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/os/OS.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/os/OSAlarm.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/os/OSArena.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/os/OSAudioSystem.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/os/OSLink.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/os/OSMemory.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/os/OSMutex.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/os/OSReboot.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/os/OSReset.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/os/OSResetSW.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/os/OSRtc.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/os/OSSync.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/os/OSThread.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/os/OSTime.c"),
            Object(MatchingFor("G2ME01"), "Dolphin/os/__ppc_eabi_init.cpp"),
        ],
    ),
    MusyX(
        [
            Object(MatchingFor("G2ME01"), "musyx/runtime/seq.c"),
            Object(MatchingFor("G2ME01"), "musyx/runtime/synth.c"),
            Object(MatchingFor("G2ME01"), "musyx/runtime/seq_api.c"),
            Object(MatchingFor("G2ME01"), "musyx/runtime/snd_synthapi.c"),
            Object(NonMatching, "musyx/runtime/stream.c"),
            Object(MatchingFor("G2ME01"), "musyx/runtime/synthdata.c"),
            Object(MatchingFor("G2ME01"), "musyx/runtime/synthmacros.c"),
            Object(MatchingFor("G2ME01"), "musyx/runtime/synthvoice.c"),
            Object(MatchingFor("G2ME01"), "musyx/runtime/synth_ac.c"),
            Object(MatchingFor("G2ME01"), "musyx/runtime/synth_dbtab.c"),
            Object(MatchingFor("G2ME01"), "musyx/runtime/synth_adsr.c"),
            Object(MatchingFor("G2ME01"), "musyx/runtime/synth_vsamples.c"),
            Object(MatchingFor("G2ME01"), "musyx/runtime/s_data.c"),
            Object(NonMatching, "musyx/runtime/hw_dspctrl.c"),
            Object(MatchingFor("G2ME01"), "musyx/runtime/hw_volconv.c"),
            Object(NonMatching, "musyx/runtime/snd3d.c"),
            Object(MatchingFor("G2ME01"), "musyx/runtime/snd_init.c"),
            Object(MatchingFor("G2ME01"), "musyx/runtime/snd_math.c"),
            Object(MatchingFor("G2ME01"), "musyx/runtime/snd_midictrl.c"),
            Object(MatchingFor("G2ME01"), "musyx/runtime/snd_service.c"),
            Object(MatchingFor("G2ME01"), "musyx/runtime/hardware.c"),
            Object(MatchingFor("G2ME01"), "musyx/runtime/hw_aramdma.c"),
            Object(MatchingFor("G2ME01"), "musyx/runtime/dsp_import.c"),
            Object(MatchingFor("G2ME01"), "musyx/runtime/hw_dolphin.c"),
            Object(MatchingFor("G2ME01"), "musyx/runtime/hw_memory.c"),
            Object(MatchingFor("G2ME01"), "musyx/runtime/CheapReverb/creverb_fx.c"),
            Object(MatchingFor("G2ME01"), "musyx/runtime/CheapReverb/creverb.c"),
            Object(MatchingFor("G2ME01"), "musyx/runtime/StdReverb/reverb_fx.c"),
            Object(MatchingFor("G2ME01"), "musyx/runtime/StdReverb/reverb.c"),
            Object(MatchingFor("G2ME01"), "musyx/runtime/Delay/delay_fx.c"),
            Object(MatchingFor("G2ME01"), "musyx/runtime/Chorus/chorus_fx.c"),
        ],
    ),
    # Begin RELs
    {
        "lib": "REL",
        "mw_version": "GC/2.7",
        "cflags": cflags_rel,
        "progress_category": "game",  # str | List[str]
        "host": False,
        "objects": [
            Object(Matching, "REL/REL_Setup.cpp"),
            Object(
                Matching,
                "REL/global_destructor_chain.c",
                source="Runtime/global_destructor_chain.c",
            ),
        ],
    },
    Rel(
        "ForgottenObject",
        [
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptForgottenObject.cpp"),
        ],
    ),
    Rel(
        "ScriptCannonBall",
        [
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptCannonBall.cpp"),
        ],
    ),
    Rel(
        "Tweaks",
        [
            Object(NonMatching, "MetroidPrime/Tweaks/Tweaks.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptLoader/Tweaks.cpp"),
        ],
        # Native generated constructors address each float constant separately.
        extra_cflags=["-pool off"],
    ),
]


# Optional callback to adjust link order. This can be used to add, remove, or reorder objects.
# This is called once per module, with the module ID and the current link order.
#
# For example, this adds "dummy.c" to the end of the DOL link order if configured with --non-matching.
# "dummy.c" *must* be configured as a Matching (or Equivalent) object in order to be linked.
def link_order_callback(module_id: int, objects: List[str]) -> List[str]:
    # Don't modify the link order for matching builds
    if not config.non_matching:
        return objects
    if module_id == 0:  # DOL
        return objects + ["dummy.c"]
    return objects


# Uncomment to enable the link order callback.
# config.link_order_callback = link_order_callback


# Optional extra categories for progress tracking
# Adjust as desired for your project
config.progress_categories = [
    ProgressCategory("game", "Game Code"),
    ProgressCategory("sdk", "SDK Code"),
]
config.progress_each_module = args.verbose
# Optional extra arguments to `objdiff-cli report generate`
config.progress_report_args = [
    # Marks relocations as mismatching if the target value is different
    # Default is "functionRelocDiffs=none", which is most lenient
    # "--config functionRelocDiffs=data_value",
]
config.extra_clang_flags = ["-DCLANGD"]

if args.mode == "configure":
    # Write build.ninja and objdiff.json
    generate_build(config)
elif args.mode == "progress":
    # Print progress information
    calculate_progress(config)
else:
    sys.exit("Unknown mode: " + args.mode)
