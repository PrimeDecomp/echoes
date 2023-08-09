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

    float lurk_chance;
    float taunt_chance;
    float unknown;
    float charge_beam_attack_chance;
    SLdrDamageInfo blade_damage;
    SLdrAudioPlaybackParms sound_impact_rag_doll;
    SLdrAudioPlaybackParms sound_hurled_death;
    SLdrUnknownStruct12 unknown_struct12;
    SLdrUnknownStruct13 unknown_struct13;
    SLdrUnknownStruct14 unknown_struct14;
};

void LoadTypedefSLdrDarkCommandoData(SLdrDarkCommandoData&, CInputStream&);

#endif // _SLDRDARKCOMMANDODATA
