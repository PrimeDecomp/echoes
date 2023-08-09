#ifndef _SLDRUNKNOWNSTRUCT39
#define _SLDRUNKNOWNSTRUCT39

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"

struct SLdrUnknownStruct39 {
    SLdrUnknownStruct39();
    ~SLdrUnknownStruct39();

    int start_state;
    SLdrDamageInfo explosion_damage;
    float min_height;
    float max_height;
    float min_down_height;
    float max_down_height;
    float separation_distance;
    float min_life_time;
    float max_life_time;
    float normal_knockback;
    float heavy_knockback;
    float knockback_decline;
    bool is_dark_shredder;
    float desired_distance;
};

void LoadTypedefSLdrUnknownStruct39(SLdrUnknownStruct39&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT39
