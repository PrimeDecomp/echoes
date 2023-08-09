#ifndef _SLDRTWEAPONDAMAGE
#define _SLDRTWEAPONDAMAGE

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTDamageInfo.hpp"

struct SLdrTWeaponDamage {
    SLdrTWeaponDamage();
    ~SLdrTWeaponDamage();

    SLdrTDamageInfo normal;
    SLdrTDamageInfo charged;
};

void LoadTypedefSLdrTWeaponDamage(SLdrTWeaponDamage&, CInputStream&);

#endif // _SLDRTWEAPONDAMAGE
