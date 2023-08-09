#ifndef _SLDRSANDBOSSDATA
#define _SLDRSANDBOSSDATA

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSandBossStructA.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct40.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct41.hpp"

struct SLdrSandBossData {
    SLdrSandBossData();
    ~SLdrSandBossData();

    CAssetId scannable_info1;
    int command_index;
    CAssetId cracked_sphere1;
    CAssetId cracked_sphere2;
    CAssetId cracked_sphere3;
    SLdrDamageInfo snap_jaw_damage;
    SLdrDamageInfo spit_out_damage;
    float unknown_0xbf88fe4f;
    float unknown_0x74c702b3;
    CAssetId dark_beam_projectile;
    SLdrDamageInfo dark_beam_damage;
    float unknown_0x2b42dddf;
    float unknown_0x1562e0d6;
    float unknown_0xd0db2574;
    float suck_air_time;
    float suck_morphball_range;
    float spit_morphball_time;
    CAssetId part;
    SLdrUnknownStruct40 unknown_struct40;
    SLdrUnknownStruct41 unknown_struct41;
    SLdrSandBossStructA sand_boss_struct_a_0x8b452a19;
    SLdrSandBossStructA sand_boss_struct_a_0x0cf8c54c;
    CAssetId model_with_tail_armor;
    CAssetId skin_for_armored_tail;
    SLdrDamageVulnerability damage_vulnerability;
    SLdrDamageVulnerability stampede_vulnerability;
    SLdrDamageVulnerability suck_air_vulnerability;
};

void LoadTypedefSLdrSandBossData(SLdrSandBossData&, CInputStream&);

#endif // _SLDRSANDBOSSDATA
