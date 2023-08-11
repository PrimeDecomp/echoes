#ifndef _SLDREMPERORINGSTAGE3DATA
#define _SLDREMPERORINGSTAGE3DATA

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAudioPlaybackParms.hpp"
#include "MetroidPrime/ScriptLoader/SLdrBasicSwarmProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEmperorIngStage3StructA.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEmperorIngStage3StructB.hpp"
#include "MetroidPrime/ScriptLoader/SLdrHealthInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrShockWaveInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct26.hpp"

struct SLdrEmperorIngStage3Data {
    SLdrEmperorIngStage3Data();
    ~SLdrEmperorIngStage3Data();

    float tauntFrequency;
    SLdrHealthInfo yellowHealth;
    SLdrHealthInfo health;
    float vulnerableTime;
    float vulnerableDamageThreshold;
    SLdrDamageVulnerability redVulnerability;
    SLdrDamageVulnerability lightVulnerability;
    SLdrDamageVulnerability darkVulnerability;
    SLdrDamageInfo meleeDamage;
    SLdrDamageInfo damageInfo;
    SLdrDamageInfo jumpSlideDamage;
    SLdrDamageInfo groundPoundDamage;
    SLdrEmperorIngStage3StructA emperorIngStage3StructA_0x98e311c1;
    SLdrEmperorIngStage3StructA emperorIngStage3StructA_0x93dae216;
    CAssetId lightSwarmEffect;
    SLdrBasicSwarmProperties lightSwarmProperties;
    SLdrAudioPlaybackParms lightSwarmDeathSound;
    SLdrAudioPlaybackParms audioPlaybackParms;
    SLdrUnknownStruct26 unknownStruct26;
    SLdrShockWaveInfo jumpAttackShockWaveInfo;
    int sound;
    SLdrEmperorIngStage3StructB emperorIngStage3StructB_0xe843417f;
    SLdrEmperorIngStage3StructB emperorIngStage3StructB_0xd13bec3f;
    SLdrEmperorIngStage3StructB emperorIngStage3StructB_0xc61388ff;
    SLdrEmperorIngStage3StructB emperorIngStage3StructB_0xa3cab6bf;
};

void LoadTypedefSLdrEmperorIngStage3Data(SLdrEmperorIngStage3Data&, CInputStream&);

#endif // _SLDREMPERORINGSTAGE3DATA
