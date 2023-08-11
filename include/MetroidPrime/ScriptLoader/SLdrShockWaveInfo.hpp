#ifndef _SLDRSHOCKWAVEINFO
#define _SLDRSHOCKWAVEINFO

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"

struct SLdrShockWaveInfo {
    SLdrShockWaveInfo();
    ~SLdrShockWaveInfo();

    CAssetId shockWaveEffect;
    SLdrDamageInfo damage;
    float radius;
    float height;
    float unknown;
    float radialVelocity;
    float radialVelocityAcceleration;
    CAssetId visorElectricEffect;
    int soundVisorElectric;
};

void LoadTypedefSLdrShockWaveInfo(SLdrShockWaveInfo&, CInputStream&);

#endif // _SLDRSHOCKWAVEINFO
