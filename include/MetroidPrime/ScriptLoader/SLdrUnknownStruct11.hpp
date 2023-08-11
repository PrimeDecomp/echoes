#ifndef _SLDRUNKNOWNSTRUCT11
#define _SLDRUNKNOWNSTRUCT11

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"

struct SLdrUnknownStruct11 {
    SLdrUnknownStruct11();
    ~SLdrUnknownStruct11();

    SLdrDamageInfo shieldChargeDamage;
    SLdrDamageVulnerability shieldVulnerability;
    float unknown_0xc1d9dbc6;
    float unknown_0x927fc322;
    float shieldChargeSpeed;
    CAssetId shieldExplodeEffect;
    int soundShieldExplode;
    float unknown_0x6cb0da5a;
    float unknown_0xc3938663;
    float armShieldChance;
    float armShieldTime;
    float unknown_0xe1b0efa0;
    CAssetId armShieldExplodeEffect;
    CAssetId shieldChargeEffect;
    CAssetId armShieldEffect;
    int sound_0xbf3c59b6;
    int sound_0x78be3b8d;
};

void LoadTypedefSLdrUnknownStruct11(SLdrUnknownStruct11&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT11
