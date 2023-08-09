#ifndef _SLDRUNKNOWNSTRUCT22
#define _SLDRUNKNOWNSTRUCT22

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"

struct SLdrUnknownStruct22 {
    SLdrUnknownStruct22();
    ~SLdrUnknownStruct22();

    CAssetId portal_effect;
    SLdrAnimationParameters attack_tip;
    SLdrDamageInfo stab_damage;
    int unknown_0xecfab026;
    int unknown_0x94880277;
    int sound_0x1c3e84b6;
    int sound_0xa93f0198;
};

void LoadTypedefSLdrUnknownStruct22(SLdrUnknownStruct22&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT22
