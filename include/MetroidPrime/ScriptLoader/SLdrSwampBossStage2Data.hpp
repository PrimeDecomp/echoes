#ifndef _SLDRSWAMPBOSSSTAGE2DATA
#define _SLDRSWAMPBOSSSTAGE2DATA

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAudioPlaybackParms.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrShockWaveInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSwampBossStage2Struct.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct38.hpp"

struct SLdrSwampBossStage2Data {
    SLdrSwampBossStage2Data();
    ~SLdrSwampBossStage2Data();

    float hover_speed;
    CAssetId upper_left_wing_target;
    CAssetId lower_left_wing_target;
    CAssetId upper_right_wing_target;
    CAssetId lower_right_wing_target;
    float unknown_0xcabe6b96;
    SLdrSwampBossStage2Struct swamp_boss_stage2_struct_0x7fa9256a;
    SLdrSwampBossStage2Struct swamp_boss_stage2_struct_0x8b884b8e;
    SLdrSwampBossStage2Struct swamp_boss_stage2_struct_0x04b7a789;
    SLdrSwampBossStage2Struct swamp_boss_stage2_struct_0xf096c96d;
    float stun_time;
    int unknown_0x96ce7897;
    CAssetId spit_projectile;
    SLdrDamageInfo spit_damage;
    CAssetId spit_visor_effect;
    int sound_spit_visor;
    float spit_projectile_radius;
    SLdrDamageInfo swoop_damage;
    float swoop_push;
    float swoop_damage_time;
    CAssetId splash;
    float unknown_0x7fc50ac2;
    float unknown_0x13448a4a;
    float unknown_0xf55924da;
    float unknown_0x83bc1de7;
    float unknown_0x5a844633;
    float unknown_0x78e22d1b;
    float unknown_0x9e116385;
    float radar_range;
    float unknown_0xfe97e835;
    SLdrShockWaveInfo splash_shock_wave;
    float unknown_0x9807497c;
    float unknown_0xe57ca27c;
    CAssetId scan_info_light;
    CAssetId scan_info_dark;
    CAssetId bubble_telegraph_effect;
    CAssetId wing_damage_effect;
    SLdrUnknownStruct38 unknown_struct38;
    CAssetId blow_effect;
    SLdrDamageInfo blow_damage;
    float unknown_0x6d89649b;
    float blow_push;
    float break_stun_damage;
    SLdrAudioPlaybackParms stunned_sound;
    SLdrAudioPlaybackParms audio_playback_parms_0x427a116a;
    SLdrAudioPlaybackParms audio_playback_parms_0xc05d5c7a;
    SLdrAudioPlaybackParms audio_playback_parms_0x2b3c923a;
    float unknown_0x8fe0bf01;
    SLdrAudioPlaybackParms flinch_sound;
    float flinch_sound_chance;
    SLdrAudioPlaybackParms stunned_flinch_sound;
    SLdrAudioPlaybackParms audio_playback_parms_0x878a6522;
    float unknown_0x19849710;
    SLdrAudioPlaybackParms audio_playback_parms_0x692fa63c;
    SLdrAudioPlaybackParms audio_playback_parms_0xbe3d39aa;
};

void LoadTypedefSLdrSwampBossStage2Data(SLdrSwampBossStage2Data&, CInputStream&);

#endif // _SLDRSWAMPBOSSSTAGE2DATA
