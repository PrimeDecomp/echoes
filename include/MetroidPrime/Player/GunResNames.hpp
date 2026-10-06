#ifndef _GUNRESNAMES
#define _GUNRESNAMES

// Guessed header/unit name. Echoes defines the arm/gun/weapon resource names (Prime's
// CTweakGunRes and CGunWeapon tables) once, in a data-only unit linked after CPlayerGunBase,
// and the gun, beam, grapple and player code reference them externally. Object names are
// guessed unless noted; the strings are target data.

namespace NWeaponRes {
extern const char* const kSamusArmFSM;              // Guessed name
extern const char* const kSamusGunFSM;              // Guessed name
extern const char* const kBombSet;                  // Guessed name
extern const char* const kBombExplo;                // Guessed name
extern const char* const kPowerBombExplo;           // Guessed name
extern const char* const kGunMotion;                // Guessed name
extern const char* const kRightHand;                // Guessed name
extern const char* const kHoloTransition;           // Guessed name
extern const char* const kGrappleArm;               // Guessed name
extern const char* const kGrappleSegment;           // Guessed name
extern const char* const kGrappleClaw;              // Guessed name
extern const char* const kGrappleHit;               // Guessed name
extern const char* const kGrappleMuzzle;            // Guessed name
extern const char* const kGrappleSwoosh;            // Guessed name
extern const char* const kGrappleGear[3];           // Prime's kGrappleGear, as an array here
extern const char* const kShotSmoke;                // Guessed name
extern const char* const kPower2nd1;                // Guessed name
extern const char* const kIceSmoke;                 // Guessed name
extern const char* const kIce2nd1;                  // Guessed name
extern const char* const kIce2nd2;                  // Guessed name
extern const char* const kWave2nd;                  // Guessed name
extern const char* const skWaveBallNames[3];        // Guessed name
extern const char* const kPlasma2nd1;               // Guessed name
extern const char* const kPhazon2nd1;               // Guessed name
extern const char* const skMissileMuzzleNames[5];   // Guessed name
extern const char* const kMissileAuxMuzzle;         // Guessed name
extern const char* const kMissile2nd;               // Guessed name
extern const char* const skComboNames[4];           // Prime's CAuxWeapon skComboNames
extern const char* const kCommonDependencyGroup;    // Guessed name
extern const char* const kPowerAnimDependencyGroup; // Guessed name
extern const char* const skWeaponNames[8];
extern const char* const skMuzzleNames[8];
extern const char* const skFrozenNames[8];
extern const char* const skAuxMuzzleNames[4]; // Guessed name
extern const char* const skBeamXferNames[4];
extern const char* const skDependencyNames[4];
extern const char* const kVariaArm; // Guessed name
extern const char* const skBeamNames[4];
extern const char* const kBeamThirdPersonFxGroup;
extern const char* const skThirdPersonChargeNames[4];
extern const char* const skThirdPersonMuzzleNames[4];
} // namespace NWeaponRes

#endif // _GUNRESNAMES
