#ifndef _SLDRUNKNOWNSTRUCT13
#define _SLDRUNKNOWNSTRUCT13

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAudioPlaybackParms.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"

struct SLdrUnknownStruct13 {
    SLdrUnknownStruct13();
    ~SLdrUnknownStruct13();

    float unknown;
    float min_attack_range;
    float max_attack_range;
    SLdrDamageInfo damage;
    CAssetId projectile;
    CAssetId mold_effect;
    SLdrDamageInfo mold_damage;
    SLdrAudioPlaybackParms sound_mold;
};

void LoadTypedefSLdrUnknownStruct13(SLdrUnknownStruct13&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT13
