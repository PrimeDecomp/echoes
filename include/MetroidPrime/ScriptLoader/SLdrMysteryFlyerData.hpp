#ifndef _SLDRMYSTERYFLYERDATA
#define _SLDRMYSTERYFLYERDATA

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"

struct SLdrMysteryFlyerData {
    SLdrMysteryFlyerData();
    ~SLdrMysteryFlyerData();

    CAssetId shotProjectile;
    SLdrDamageInfo shotDamage;
    float hoverSpeed;
    float hoverHeight;
    float separationDistance;
    bool unknown;
};

void LoadTypedefSLdrMysteryFlyerData(SLdrMysteryFlyerData&, CInputStream&);

#endif // _SLDRMYSTERYFLYERDATA
