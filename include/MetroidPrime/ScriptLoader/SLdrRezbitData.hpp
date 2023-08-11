#ifndef _SLDRREZBITDATA
#define _SLDRREZBITDATA

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAudioPlaybackParms.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPlasmaBeamInfo.hpp"

struct SLdrRezbitData {
    SLdrRezbitData();
    ~SLdrRezbitData();

    float hearingRadius;
    float unknown_0x4a6c4b40;
    float unknown_0x30d11671;
    CAssetId modelNoHolos;
    CAssetId skinNoHolos;
    float shieldDownTime;
    float unknown_0xffb37b81;
    float shieldUpTime;
    float unknown_0x0d1d1648;
    CAssetId shieldExplodeEffect;
    SLdrAudioPlaybackParms soundShieldExplode;
    SLdrAudioPlaybackParms soundShieldOn;
    SLdrAudioPlaybackParms soundShieldOff;
    SLdrAudioPlaybackParms soundFlinch;
    float unknown_0xad120ad7;
    float unknown_0xe70ef8a3;
    SLdrAudioPlaybackParms audioPlaybackParms;
    float unknown_0x70e597d4;
    float unknown_0x94980a67;
    float unknown_0xd02f08b0;
    float unknown_0x53e84718;
    float unknown_0x6fbc1bf9;
    float energyBoltChance;
    float cuttingLaserChance;
    float unknown_0x075491ca;
    float unknown_0x54f2892e;
    SLdrDamageInfo energyBoltDamage;
    CAssetId energyBoltProjectile;
    float energyBoltAttackDuration;
    float unknown_0x28944183;
    float unknown_0xc7a69a59;
    SLdrAudioPlaybackParms soundEnergyBolt;
    float unknown_0x9ede657f;
    float unknown_0xcd787d9b;
    float virusAttackTime;
    SLdrDamageInfo virusDamage;
    int soundAlwaysFf;
    int sound_0xbb3f8a7b;
    int sound_0x601f846d;
    float unknown_0x64c7990d;
    float unknown_0x376181e9;
    SLdrDamageInfo cuttingLaserDamage;
    SLdrAudioPlaybackParms soundCuttingLaser;
    SLdrPlasmaBeamInfo cuttingLaserBeamInfo;
    SLdrDamageVulnerability shieldVulnerability;
};

void LoadTypedefSLdrRezbitData(SLdrRezbitData&, CInputStream&);

#endif // _SLDRREZBITDATA
