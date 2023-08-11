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
    float minAttackRange;
    float maxAttackRange;
    SLdrDamageInfo damage;
    CAssetId projectile;
    CAssetId moldEffect;
    SLdrDamageInfo moldDamage;
    SLdrAudioPlaybackParms soundMold;
};

void LoadTypedefSLdrUnknownStruct13(SLdrUnknownStruct13&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT13
