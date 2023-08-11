#ifndef _SLDRDIGITALGUARDIANDATA
#define _SLDRDIGITALGUARDIANDATA

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAudioPlaybackParms.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEchoParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrShockWaveInfo.hpp"

struct SLdrDigitalGuardianData {
    SLdrDigitalGuardianData();
    ~SLdrDigitalGuardianData();

    CAssetId scannableInfoCrippled;
    float unknown_0x0faf6a8e;
    float unknown_0xd3056808;
    float unknown_0x304b47ee;
    SLdrDamageInfo legStabDamage;
    float unknown_0xb4561f28;
    CAssetId toeTargetModel;
    CAssetId part_0x783635a6;
    SLdrAudioPlaybackParms soundToeTarget;
    SLdrAudioPlaybackParms soundToeTargetAttack;
    SLdrAudioPlaybackParms soundToeTargetExplosion;
    SLdrAudioPlaybackParms soundToeTargetHit;
    SLdrAudioPlaybackParms soundShockWave;
    SLdrShockWaveInfo shockWaveInfo;
    float vortexAttackDuration;
    float vortexAttractionForce;
    float unknown_0x348bff02;
    float vortexLinearVelocity;
    float vortexLinearAcceleration;
    SLdrDamageInfo vortexDamage;
    int unknown_0xfb5263e8;
    int unknown_0x6aaf33e3;
    float unknown_0x4f5d725c;
    SLdrAudioPlaybackParms soundVortexFlash;
    CAssetId legModel;
    CAssetId shinArmor;
    float unknown_0xe3dd61e6;
    SLdrAudioPlaybackParms soundKneeArmorHit;
    SLdrAudioPlaybackParms soundKneeVulnerable;
    CAssetId kneeArmor;
    SLdrEchoParameters echoParameters_0x7b5b7312;
    float unknown_0xa324e26c;
    float unknown_0x6a754ebd;
    float jumpTimer;
    float unknown_0x8106cda9;
    float unknown_0x9e1b8105;
    float unknown_0xa08fcc70;
    float unknown_0x3254a16b;
    CAssetId transmissionBeacon;
    CAssetId part_0x3fa7df1c;
    SLdrEchoParameters echoParameters_0x021b6f9d;
    SLdrAudioPlaybackParms soundTransmissionBeacon;
    SLdrAudioPlaybackParms audioPlaybackParms;
    SLdrAudioPlaybackParms soundBeaconRetract;
    float unknown_0x4f6d27d3;
    CAssetId part_0x71f0c674;
    CAssetId part_0xc8ec315b;
    SLdrAudioPlaybackParms soundBeaconExplode;
    SLdrAudioPlaybackParms soundBeaconHit;
    SLdrDamageVulnerability kneeVulnerability;
    SLdrDamageVulnerability vortexVulnerability;
    SLdrDamageVulnerability toeTargetVulnerability;
};

void LoadTypedefSLdrDigitalGuardianData(SLdrDigitalGuardianData&, CInputStream&);

#endif // _SLDRDIGITALGUARDIANDATA
