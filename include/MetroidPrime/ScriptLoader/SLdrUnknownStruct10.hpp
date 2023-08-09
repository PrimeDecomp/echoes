#ifndef _SLDRUNKNOWNSTRUCT10
#define _SLDRUNKNOWNSTRUCT10

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"

struct SLdrUnknownStruct10 {
    SLdrUnknownStruct10();
    ~SLdrUnknownStruct10();

    float grenade_min_attack_interval;
    float unknown_0xb7994ea1;
    float grenade_attack_chance;
    float unknown_0x25f822c4;
    float unknown_0x765e3a20;
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

void LoadTypedefSLdrUnknownStruct10(SLdrUnknownStruct10&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT10
