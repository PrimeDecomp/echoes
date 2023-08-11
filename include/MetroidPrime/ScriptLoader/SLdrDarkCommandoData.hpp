#ifndef _SLDRDARKCOMMANDODATA
#define _SLDRDARKCOMMANDODATA

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAudioPlaybackParms.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct12.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct13.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct14.hpp"

struct SLdrDarkCommandoData {
    SLdrDarkCommandoData();
    ~SLdrDarkCommandoData();

    float lurkChance;
    float tauntChance;
    float unknown;
    float chargeBeamAttackChance;
    SLdrDamageInfo bladeDamage;
    SLdrAudioPlaybackParms soundImpactRagDoll;
    SLdrAudioPlaybackParms soundHurledDeath;
    SLdrUnknownStruct12 unknownStruct12;
    SLdrUnknownStruct13 unknownStruct13;
    SLdrUnknownStruct14 unknownStruct14;
};

void LoadTypedefSLdrDarkCommandoData(SLdrDarkCommandoData&, CInputStream&);

#endif // _SLDRDARKCOMMANDODATA
