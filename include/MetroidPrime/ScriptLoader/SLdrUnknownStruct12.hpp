#ifndef _SLDRUNKNOWNSTRUCT12
#define _SLDRUNKNOWNSTRUCT12

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"

struct SLdrUnknownStruct12 {
    SLdrUnknownStruct12();
    ~SLdrUnknownStruct12();

    float unknown_0xb33a0cbc;
    float min_attack_range;
    float max_attack_range;
    SLdrDamageInfo grenade_damage;
    CAssetId grenade_explosion;
    CAssetId grenade_effect;
    CAssetId grenade_trail;
    float grenade_mass;
    float unknown_0xed086ce0;
    float unknown_0x00fc6646;
    float unknown_0xa7c8e63f;
    int unknown_0x454f16b1;
    float unknown_0x2d4706e8;
    int sound_grenade_bounce;
    int sound_grenade_explode;
};

void LoadTypedefSLdrUnknownStruct12(SLdrUnknownStruct12&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT12
