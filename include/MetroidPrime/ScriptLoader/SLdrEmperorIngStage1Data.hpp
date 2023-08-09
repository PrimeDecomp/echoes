#ifndef _SLDREMPERORINGSTAGE1DATA
#define _SLDREMPERORINGSTAGE1DATA

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAudioPlaybackParms.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEmperorIngStage1TentacleData.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct20.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct21.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct22.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct23.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct24.hpp"

struct SLdrEmperorIngStage1Data {
    SLdrEmperorIngStage1Data();
    ~SLdrEmperorIngStage1Data();

    SLdrEmperorIngStage1TentacleData tentacle;
    SLdrUnknownStruct20 unknown_struct20;
    SLdrUnknownStruct21 unknown_struct21;
    SLdrUnknownStruct22 unknown_struct22;
    SLdrUnknownStruct23 unknown_struct23;
    SLdrUnknownStruct24 unknown_struct24;
    float heart_exposed_time;
    float unknown_0x905938b8;
    float unknown_0xb826317a;
    SLdrAudioPlaybackParms heart_damage_sound;
    float turn_speed_accel;
    float max_turn_speed_normal;
    float max_turn_speed_melee;
    float unknown_0xe5a7c358;
    int vulnerability_change_sound;
    float taunt_frequency;
    float attack_interval_min;
    float attack_interval_max;
};

void LoadTypedefSLdrEmperorIngStage1Data(SLdrEmperorIngStage1Data&, CInputStream&);

#endif // _SLDREMPERORINGSTAGE1DATA
