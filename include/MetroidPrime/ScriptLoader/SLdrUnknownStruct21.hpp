#ifndef _SLDRUNKNOWNSTRUCT21
#define _SLDRUNKNOWNSTRUCT21

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"

struct SLdrUnknownStruct21 {
    SLdrUnknownStruct21();
    ~SLdrUnknownStruct21();

    CAssetId projectile;
    SLdrDamageInfo projectileDamage;
    CAssetId projectileVisorEffect;
};

void LoadTypedefSLdrUnknownStruct21(SLdrUnknownStruct21&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT21
