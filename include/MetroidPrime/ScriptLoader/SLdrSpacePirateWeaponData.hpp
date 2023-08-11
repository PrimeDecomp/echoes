#ifndef _SLDRSPACEPIRATEWEAPONDATA
#define _SLDRSPACEPIRATEWEAPONDATA

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"

struct SLdrSpacePirateWeaponData {
    SLdrSpacePirateWeaponData();
    ~SLdrSpacePirateWeaponData();

    int unknown_0x647670ac;
    CAssetId grenadeLauncher;
    int unknown_0xa95a025b;
    float unknown_0x25f822c4;
    float unknown_0x765e3a20;
    SLdrDamageInfo grenadeDamage;
    CAssetId grenadeExplosion;
    CAssetId grenadeEffect;
    CAssetId grenadeTrail;
    float grenadeMass;
    float unknown_0xed086ce0;
    float unknown_0x00fc6646;
    float unknown_0xa7c8e63f;
    int unknown_0x454f16b1;
    int soundGrenadeBounce;
    int soundGrenadeExplode;
};

void LoadTypedefSLdrSpacePirateWeaponData(SLdrSpacePirateWeaponData&, CInputStream&);

#endif // _SLDRSPACEPIRATEWEAPONDATA
