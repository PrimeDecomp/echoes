#ifndef _SLDRUNKNOWNSTRUCT34
#define _SLDRUNKNOWNSTRUCT34

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrHealthInfo.hpp"

struct SLdrUnknownStruct34 {
    SLdrUnknownStruct34();
    ~SLdrUnknownStruct34();

    SLdrHealthInfo health;
    SLdrDamageInfo damage;
    CAssetId explosion;
    CAssetId effect;
    CAssetId trail;
    float mass;
    float unknown_0x417f4a91;
    float min_launch_speed;
    float max_launch_speed;
    int unknown_0xfbcdb101;
    int sound_bounce;
    int sound_explode;
    float max_turn_angle;
    float unknown_0x47f99fbc;
    int min_generation;
    int max_generation;
    float unknown_0xfbf8ea0a;
    bool allow_lock_on;
};

void LoadTypedefSLdrUnknownStruct34(SLdrUnknownStruct34&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT34
