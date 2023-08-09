#ifndef _SLDRUNKNOWNSTRUCT37
#define _SLDRUNKNOWNSTRUCT37

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrShockWaveInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSwampBossStage1Struct.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct36.hpp"

struct SLdrUnknownStruct37 {
    SLdrUnknownStruct37();
    ~SLdrUnknownStruct37();

    CAssetId dark_water_ring_effect;
    float unknown_0x27a06f6a;
    float unknown_0x233a5e40;
    CAssetId pre_jump_telegraph_effect;
    SLdrShockWaveInfo splash_shock_wave;
    CAssetId tongue_particle_effect;
    CAssetId tongue_particle_model;
    CAssetId tongue_tip_model;
    SLdrDamageInfo damage_info;
    CAssetId part;
    float unknown_0x78755da3;
    float unknown_0x74e1a041;
    float unknown_0x1f4e7c2c;
    float unknown_0xee6b6f47;
    float unknown_0x3ce96c9d;
    SLdrDamageVulnerability weak_spot_vulnerability;
    float weak_spot_damage_multiplier;
    CAssetId spit_projectile;
    SLdrDamageInfo spit_damage;
    CAssetId spit_visor_effect;
    int sound_spit_visor;
    float spit_projectile_radius;
    SLdrUnknownStruct36 unknown_struct36;
    SLdrSwampBossStage1Struct swamp_boss_stage1_struct_0x4500f774;
    SLdrSwampBossStage1Struct swamp_boss_stage1_struct_0x3e1e7597;
    SLdrSwampBossStage1Struct swamp_boss_stage1_struct_0xa1c4f609;
};

void LoadTypedefSLdrUnknownStruct37(SLdrUnknownStruct37&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT37
