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

    float taunt_frequency;
    SLdrHealthInfo yellow_health;
    SLdrHealthInfo health;
    float vulnerable_time;
    float vulnerable_damage_threshold;
    SLdrDamageVulnerability red_vulnerability;
    SLdrDamageVulnerability light_vulnerability;
    SLdrDamageVulnerability dark_vulnerability;
    SLdrDamageInfo melee_damage;
    SLdrDamageInfo damage_info;
    SLdrDamageInfo jump_slide_damage;
    SLdrDamageInfo ground_pound_damage;
    SLdrEmperorIngStage3StructA emperor_ing_stage3_struct_a_0x98e311c1;
    SLdrEmperorIngStage3StructA emperor_ing_stage3_struct_a_0x93dae216;
    CAssetId light_swarm_effect;
    SLdrBasicSwarmProperties light_swarm_properties;
    SLdrAudioPlaybackParms light_swarm_death_sound;
    SLdrAudioPlaybackParms audio_playback_parms;
    SLdrUnknownStruct26 unknown_struct26;
    SLdrShockWaveInfo jump_attack_shock_wave_info;
    int sound;
    SLdrEmperorIngStage3StructB emperor_ing_stage3_struct_b_0xe843417f;
    SLdrEmperorIngStage3StructB emperor_ing_stage3_struct_b_0xd13bec3f;
    SLdrEmperorIngStage3StructB emperor_ing_stage3_struct_b_0xc61388ff;
    SLdrEmperorIngStage3StructB emperor_ing_stage3_struct_b_0xa3cab6bf;
};

void LoadTypedefSLdrEmperorIngStage3Data(SLdrEmperorIngStage3Data&, CInputStream&);

#endif // _SLDREMPERORINGSTAGE3DATA
