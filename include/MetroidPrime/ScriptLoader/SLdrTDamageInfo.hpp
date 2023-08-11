#ifndef _SLDRTDAMAGEINFO
#define _SLDRTDAMAGEINFO

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTDamageInfo {
    SLdrTDamageInfo();
    ~SLdrTDamageInfo();

    int weaponType;
    float damageAmount;
    float radiusDamageAmount;
    float damageRadius;
    float knockBackPower;
};

void LoadTypedefSLdrTDamageInfo(SLdrTDamageInfo&, CInputStream&);

#endif // _SLDRTDAMAGEINFO
