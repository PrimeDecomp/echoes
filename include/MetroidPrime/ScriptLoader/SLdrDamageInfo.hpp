#ifndef _SLDRDAMAGEINFO
#define _SLDRDAMAGEINFO

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrWeaponType.hpp"

struct SLdrDamageInfo {
    SLdrDamageInfo();
    ~SLdrDamageInfo();

    SLdrWeaponType diWeaponType;
    float diDamage;
    float diRadius;
    float diKnockBackPower;
};

void LoadTypedefSLdrDamageInfo(SLdrDamageInfo&, CInputStream&);

#endif // _SLDRDAMAGEINFO
