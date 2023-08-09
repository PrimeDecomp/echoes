#ifndef _SLDRTDAMAGEINFO
#define _SLDRTDAMAGEINFO

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTDamageInfo {
    SLdrTDamageInfo();
    ~SLdrTDamageInfo();

    int weapon_type;
    float damage_amount;
    float radius_damage_amount;
    float damage_radius;
    float knock_back_power;
};

void LoadTypedefSLdrTDamageInfo(SLdrTDamageInfo&, CInputStream&);

#endif // _SLDRTDAMAGEINFO
