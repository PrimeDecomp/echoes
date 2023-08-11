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
    SLdrUnknownStruct20 unknownStruct20;
    SLdrUnknownStruct21 unknownStruct21;
    SLdrUnknownStruct22 unknownStruct22;
    SLdrUnknownStruct23 unknownStruct23;
    SLdrUnknownStruct24 unknownStruct24;
    float heartExposedTime;
    float unknown_0x905938b8;
    float unknown_0xb826317a;
    SLdrAudioPlaybackParms heartDamageSound;
    float turnSpeedAccel;
    float maxTurnSpeedNormal;
    float maxTurnSpeedMelee;
    float unknown_0xe5a7c358;
    int vulnerabilityChangeSound;
    float tauntFrequency;
    float attackIntervalMin;
    float attackIntervalMax;
};

void LoadTypedefSLdrEmperorIngStage1Data(SLdrEmperorIngStage1Data&, CInputStream&);

#endif // _SLDREMPERORINGSTAGE1DATA
