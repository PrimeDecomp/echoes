#ifndef _SLDRSHOCKWAVEINFO
#define _SLDRSHOCKWAVEINFO

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"

struct SLdrShockWaveInfo {
    SLdrShockWaveInfo();
    ~SLdrShockWaveInfo();

    CAssetId shock_wave_effect;
    SLdrDamageInfo damage;
    float radius;
    float height;
    float unknown;
    float radial_velocity;
    float radial_velocity_acceleration;
    CAssetId visor_electric_effect;
    int sound_visor_electric;
};

void LoadTypedefSLdrShockWaveInfo(SLdrShockWaveInfo&, CInputStream&);

#endif // _SLDRSHOCKWAVEINFO
