#ifndef _SLDRTBEAMINFO
#define _SLDRTBEAMINFO

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTWeaponDamage.hpp"

struct SLdrTBeamInfo {
    SLdrTBeamInfo();
    ~SLdrTBeamInfo();

    float cooldown;
    SLdrTWeaponDamage damage_info;
};

void LoadTypedefSLdrTBeamInfo(SLdrTBeamInfo&, CInputStream&);

#endif // _SLDRTBEAMINFO
