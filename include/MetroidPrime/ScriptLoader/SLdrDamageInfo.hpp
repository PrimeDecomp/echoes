#ifndef _SLDRDAMAGEINFO
#define _SLDRDAMAGEINFO

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrWeaponType.hpp"

struct SLdrDamageInfo {
    SLdrDamageInfo();
    ~SLdrDamageInfo();

    SLdrWeaponType di_weapon_type;
    float di_damage;
    float di_radius;
    float di_knock_back_power;
};

void LoadTypedefSLdrDamageInfo(SLdrDamageInfo&, CInputStream&);

#endif // _SLDRDAMAGEINFO
