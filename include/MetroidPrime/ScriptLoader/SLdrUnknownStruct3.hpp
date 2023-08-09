#ifndef _SLDRUNKNOWNSTRUCT3
#define _SLDRUNKNOWNSTRUCT3

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrHealthInfo.hpp"

struct SLdrUnknownStruct3 {
    SLdrUnknownStruct3();
    ~SLdrUnknownStruct3();

    float unknown_0x17cd8b2a;
    float unknown_0x1473dad2;
    float unknown_0x3650ce75;
    float unknown_0x78520e6e;
    float damage_angle;
    float horiz_speed;
    float vert_speed;
    float fire_rate;
    float unknown_0xf9bd253e;
    float max_attack_angle;
    float max_attack_range;
    float start_attack_range;
    float attack_leash_timer;
    SLdrDamageInfo weapon_damage;
    CAssetId weapon_effect;
    SLdrHealthInfo health;
    SLdrDamageVulnerability vulnerability;
    CAssetId state_machine;
    CAssetId telegraph_effect;
};

void LoadTypedefSLdrUnknownStruct3(SLdrUnknownStruct3&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT3
