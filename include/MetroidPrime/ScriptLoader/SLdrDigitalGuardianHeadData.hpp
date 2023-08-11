#ifndef _SLDRDIGITALGUARDIANHEADDATA
#define _SLDRDIGITALGUARDIANHEADDATA

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAudioPlaybackParms.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDigitalGuardianHeadStruct.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEchoParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPlasmaBeamInfo.hpp"

struct SLdrDigitalGuardianHeadData {
    SLdrDigitalGuardianHeadData();
    ~SLdrDigitalGuardianHeadData();

    CAssetId scannableInfoShieldHead;
    CAssetId scannableInfoStunnedHead;
    CAssetId scannableInfoFinalHead;
    CAssetId headArmor;
    float maxTurnSpeed;
    float maxLinearVelocity;
    float unknown_0xe7de8b82;
    float unknown_0x96e18283;
    float unknown_0xf77138d5;
    float unknown_0x8a83a097;
    float unknown_0xd919fb13;
    SLdrAudioPlaybackParms audioPlaybackParms_0x0a693a04;
    float unknown_0x1cc6d870;
    CAssetId part_0x5ff0b26c;
    SLdrAudioPlaybackParms audioPlaybackParms_0x50baee63;
    SLdrAudioPlaybackParms audioPlaybackParms_0xa5e1ec03;
    SLdrAudioPlaybackParms audioPlaybackParms_0x8d6053cb;
    CAssetId part_0xc91ef399;
    float unknown_0x7d3d44af;
    CAssetId echoTargets;
    CAssetId part_0x342ae844;
    CAssetId explodeEnergyStuff;
    SLdrAudioPlaybackParms audioPlaybackParms_0x76eedd2a;
    SLdrAudioPlaybackParms audioPlaybackParms_0xa5ecbe17;
    SLdrAudioPlaybackParms audioPlaybackParms_0xb1fc705f;
    SLdrAudioPlaybackParms audioPlaybackParms_0x2bc2677a;
    SLdrAudioPlaybackParms audioPlaybackParms_0x7f5b82b2;
    SLdrEchoParameters echoParameters;
    float unknown_0x8317610f;
    CAssetId part_0x321c97a9;
    SLdrAudioPlaybackParms audioPlaybackParms_0xd5cc7e71;
    SLdrAudioPlaybackParms audioPlaybackParms_0xa62c0ea7;
    SLdrAudioPlaybackParms audioPlaybackParms_0xfd9f5486;
    CAssetId annihilatorPulse;
    SLdrDamageInfo annihilatorPulseDamage;
    CAssetId annihilatorCharge;
    SLdrDamageInfo annihilatorChargeDamage;
    float unknown_0xff7688bf;
    int unknown_0x12ebb390;
    CAssetId frme;
    SLdrPlasmaBeamInfo plasmaBeamInfo;
    CAssetId lockOnMissiles;
    SLdrDamageInfo lockOnMissilesDamage;
    CAssetId machineGun;
    SLdrDamageInfo machineGunDamage;
    SLdrAudioPlaybackParms soundMachineGun;
    float unknown_0x4ab23ffe;
    float unknown_0x81a8474f;
    float unknown_0x71c406ac;
    float unknown_0xe5bc88b7;
    SLdrDigitalGuardianHeadStruct digitalGuardianHeadStruct_0x8f6732ea;
    SLdrDigitalGuardianHeadStruct digitalGuardianHeadStruct_0x8e128141;
    SLdrDigitalGuardianHeadStruct digitalGuardianHeadStruct_0xea54b390;
    SLdrDigitalGuardianHeadStruct digitalGuardianHeadStruct_0xbbd3e7a7;
    SLdrDigitalGuardianHeadStruct digitalGuardianHeadStruct_0x2dd88764;
    SLdrDigitalGuardianHeadStruct digitalGuardianHeadStruct_0x48b46e55;
    SLdrDamageVulnerability bombPitVulnerability;
    SLdrDamageVulnerability echoTargetVulnerability;
};

void LoadTypedefSLdrDigitalGuardianHeadData(SLdrDigitalGuardianHeadData&, CInputStream&);

#endif // _SLDRDIGITALGUARDIANHEADDATA
