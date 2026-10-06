#include "MetroidPrime/Player/GunResNames.hpp"

// Definition order follows the target's .sdata2/.rodata objects and string pool.
// kRightHand and kPhazon2nd1 are unreferenced, so the linker strips their pointers;
// only their strings remain in the pool.

namespace NWeaponRes {

const char* const kSamusArmFSM = "SamusArmFSM";
const char* const kSamusGunFSM = "SamusGunFSM";
const char* const kBombSet = "BombSet";
const char* const kBombExplo = "BombExplo";
const char* const kPowerBombExplo = "PowerBombExplo";
const char* const kGunMotion = "GunMotion";
const char* const kRightHand = "rightHand";
const char* const kHoloTransition = "holoTransition";
const char* const kGrappleArm = "grappleArm";
const char* const kGrappleSegment = "grappleSegment";
const char* const kGrappleClaw = "grappleClaw";
const char* const kGrappleHit = "grappleHit";
const char* const kGrappleMuzzle = "grappleMuzzle";
const char* const kGrappleSwoosh = "grappleSwoosh";
const char* const kGrappleGear[3] = {"GrappleGear", "GrappleGear", ""};
const char* const kShotSmoke = "ShotSmoke";
const char* const kPower2nd1 = "Power2nd_1";
const char* const kIceSmoke = "IceSmoke";
const char* const kIce2nd1 = "Ice2nd_1";
const char* const kIce2nd2 = "Ice2nd_2";
const char* const kWave2nd = "Wave2nd";
const char* const skWaveBallNames[3] = {"WaveBall_1", "WaveBall_2", "WaveBall_3"};
const char* const kPlasma2nd1 = "Plasma2nd_1";
const char* const kPhazon2nd1 = "Phazon2nd_1";
const char* const skMissileMuzzleNames[5] = {
    "MissileMuzzle", "MissileMuzzle1", "MissileMuzzle2", "MissileMuzzle3", "MissileMuzzle4",
};
const char* const kMissileAuxMuzzle = "MissileAuxMuzzle";
const char* const kMissile2nd = "Missile2nd";
const char* const skComboNames[4] = {"SuperMissile", "IceCombo", "WaveBuster", "FlameThrower"};
const char* const kCommonDependencyGroup = "Common_DGRP";
const char* const kPowerAnimDependencyGroup = "Power_Anim_DGRP";
const char* const skWeaponNames[8] = {
    "PowerBeam", "PowerBall",  "IceBeam",    "IceBall",
    "WaveBeam",  "WaveBall_1", "PlasmaBeam", "PlasmaBall",
};
const char* const skMuzzleNames[8] = {
    "PowerMuzzle", "PowerCharge", "IceMuzzle",    "IceCharge",
    "WaveMuzzle",  "WaveCharge",  "PlasmaMuzzle", "PlasmaCharge",
};
const char* const skFrozenNames[8] = {
    "powerFrozen", "Ice2nd_2", "iceFrozen",    "Ice2nd_2",
    "waveFrozen",  "Ice2nd_2", "plasmaFrozen", "Ice2nd_2",
};
const char* const skAuxMuzzleNames[4] = {"EmptyMuzzle", "IceAuxMuzzle", "WaveAuxMuzzle",
                                         "PlasmaAuxMuzzle"};
const char* const skBeamXferNames[4] = {"PowerXfer", "IceXfer", "WaveXfer", "PlasmaXfer"};
const char* const skDependencyNames[4] = {"Power_DGRP", "Ice_DGRP", "Wave_DGRP", "Plasma_DGRP"};
const char* const kVariaArm = "VariaArm";
const char* const skBeamNames[4] = {"Power", "Ice", "Wave", "Plasma"};
const char* const kBeamThirdPersonFxGroup = "BeamThirdPersonFx_DGRP";
const char* const skThirdPersonChargeNames[4] = {"PowerChargeThirdPerson", "DarkChargeThirdPerson",
                                                 "LightChargeThirdPerson",
                                                 "AnnihilatorChargeThirdPerson"};
const char* const skThirdPersonMuzzleNames[4] = {"PowerMuzzleThirdPerson", "DarkMuzzleThirdPerson",
                                                 "LightMuzzleThirdPerson",
                                                 "AnnihilatorMuzzleThirdPerson"};

} // namespace NWeaponRes
