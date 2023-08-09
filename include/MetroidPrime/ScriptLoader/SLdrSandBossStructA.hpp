#ifndef _SLDRSANDBOSSSTRUCTA
#define _SLDRSANDBOSSSTRUCTA

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrSandBossStructA {
    SLdrSandBossStructA();
    ~SLdrSandBossStructA();

    CAssetId head_armor;
    CAssetId armor_piece2;
    CAssetId armor_piece3;
    CAssetId armor_piece4;
    CAssetId armor_piece5;
    CAssetId armor_piece6;
    CAssetId armor_piece7;
    CAssetId tail_armor;
    CAssetId sound_armor_impact;
};

void LoadTypedefSLdrSandBossStructA(SLdrSandBossStructA&, CInputStream&);

#endif // _SLDRSANDBOSSSTRUCTA
