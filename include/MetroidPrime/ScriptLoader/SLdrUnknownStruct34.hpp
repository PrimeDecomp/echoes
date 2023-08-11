#ifndef _SLDRUNKNOWNSTRUCT34
#define _SLDRUNKNOWNSTRUCT34

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrHealthInfo.hpp"

struct SLdrUnknownStruct34 {
    SLdrUnknownStruct34();
    ~SLdrUnknownStruct34();

    SLdrHealthInfo health;
    SLdrDamageInfo damage;
    CAssetId explosion;
    CAssetId effect;
    CAssetId trail;
    float mass;
    float unknown_0x417f4a91;
    float minLaunchSpeed;
    float maxLaunchSpeed;
    int unknown_0xfbcdb101;
    int soundBounce;
    int soundExplode;
    float maxTurnAngle;
    float unknown_0x47f99fbc;
    int minGeneration;
    int maxGeneration;
    float unknown_0xfbf8ea0a;
    bool allowLockOn;
};

void LoadTypedefSLdrUnknownStruct34(SLdrUnknownStruct34&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT34
