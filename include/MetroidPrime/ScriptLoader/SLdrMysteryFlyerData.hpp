#ifndef _SLDRMYSTERYFLYERDATA
#define _SLDRMYSTERYFLYERDATA

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"

struct SLdrMysteryFlyerData {
    SLdrMysteryFlyerData();
    ~SLdrMysteryFlyerData();

    CAssetId shot_projectile;
    SLdrDamageInfo shot_damage;
    float hover_speed;
    float hover_height;
    float separation_distance;
    bool unknown;
};

void LoadTypedefSLdrMysteryFlyerData(SLdrMysteryFlyerData&, CInputStream&);

#endif // _SLDRMYSTERYFLYERDATA
