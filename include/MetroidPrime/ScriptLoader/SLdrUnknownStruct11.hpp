#ifndef _SLDRUNKNOWNSTRUCT11
#define _SLDRUNKNOWNSTRUCT11

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"

struct SLdrUnknownStruct11 {
    SLdrUnknownStruct11();
    ~SLdrUnknownStruct11();

    SLdrDamageInfo shield_charge_damage;
    SLdrDamageVulnerability shield_vulnerability;
    float unknown_0xc1d9dbc6;
    float unknown_0x927fc322;
    float shield_charge_speed;
    CAssetId shield_explode_effect;
    int sound_shield_explode;
    float unknown_0x6cb0da5a;
    float unknown_0xc3938663;
    float arm_shield_chance;
    float arm_shield_time;
    float unknown_0xe1b0efa0;
    CAssetId arm_shield_explode_effect;
    CAssetId shield_charge_effect;
    CAssetId arm_shield_effect;
    int sound_0xbf3c59b6;
    int sound_0x78be3b8d;
};

void LoadTypedefSLdrUnknownStruct11(SLdrUnknownStruct11&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT11
