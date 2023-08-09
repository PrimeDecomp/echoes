#ifndef _SLDRWEAPONTYPE
#define _SLDRWEAPONTYPE

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrWeaponType {
    SLdrWeaponType();
    ~SLdrWeaponType();

    int value;
};

void LoadTypedefSLdrWeaponType(SLdrWeaponType&, CInputStream&);

#endif // _SLDRWEAPONTYPE
