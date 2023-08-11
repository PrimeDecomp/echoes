#ifndef _SLDRSANDBOSSSTRUCTA
#define _SLDRSANDBOSSSTRUCTA

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrSandBossStructA {
    SLdrSandBossStructA();
    ~SLdrSandBossStructA();

    CAssetId headArmor;
    CAssetId armorPiece2;
    CAssetId armorPiece3;
    CAssetId armorPiece4;
    CAssetId armorPiece5;
    CAssetId armorPiece6;
    CAssetId armorPiece7;
    CAssetId tailArmor;
    CAssetId soundArmorImpact;
};

void LoadTypedefSLdrSandBossStructA(SLdrSandBossStructA&, CInputStream&);

#endif // _SLDRSANDBOSSSTRUCTA
