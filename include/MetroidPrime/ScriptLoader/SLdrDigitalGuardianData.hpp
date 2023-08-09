#ifndef _SLDRDIGITALGUARDIANDATA
#define _SLDRDIGITALGUARDIANDATA

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAudioPlaybackParms.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEchoParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrShockWaveInfo.hpp"

struct SLdrDigitalGuardianData {
    SLdrDigitalGuardianData();
    ~SLdrDigitalGuardianData();

    CAssetId scannable_info_crippled;
    float unknown_0x0faf6a8e;
    float unknown_0xd3056808;
    float unknown_0x304b47ee;
    SLdrDamageInfo leg_stab_damage;
    float unknown_0xb4561f28;
    CAssetId toe_target_model;
    CAssetId part_0x783635a6;
    SLdrAudioPlaybackParms sound_toe_target;
    SLdrAudioPlaybackParms sound_toe_target_attack;
    SLdrAudioPlaybackParms sound_toe_target_explosion;
    SLdrAudioPlaybackParms sound_toe_target_hit;
    SLdrAudioPlaybackParms sound_shock_wave;
    SLdrShockWaveInfo shock_wave_info;
    float vortex_attack_duration;
    float vortex_attraction_force;
    float unknown_0x348bff02;
    float vortex_linear_velocity;
    float vortex_linear_acceleration;
    SLdrDamageInfo vortex_damage;
    int unknown_0xfb5263e8;
    int unknown_0x6aaf33e3;
    float unknown_0x4f5d725c;
    SLdrAudioPlaybackParms sound_vortex_flash;
    CAssetId leg_model;
    CAssetId shin_armor;
    float unknown_0xe3dd61e6;
    SLdrAudioPlaybackParms sound_knee_armor_hit;
    SLdrAudioPlaybackParms sound_knee_vulnerable;
    CAssetId knee_armor;
    SLdrEchoParameters echo_parameters_0x7b5b7312;
    float unknown_0xa324e26c;
    float unknown_0x6a754ebd;
    float jump_timer;
    float unknown_0x8106cda9;
    float unknown_0x9e1b8105;
    float unknown_0xa08fcc70;
    float unknown_0x3254a16b;
    CAssetId transmission_beacon;
    CAssetId part_0x3fa7df1c;
    SLdrEchoParameters echo_parameters_0x021b6f9d;
    SLdrAudioPlaybackParms sound_transmission_beacon;
    SLdrAudioPlaybackParms audio_playback_parms;
    SLdrAudioPlaybackParms sound_beacon_retract;
    float unknown_0x4f6d27d3;
    CAssetId part_0x71f0c674;
    CAssetId part_0xc8ec315b;
    SLdrAudioPlaybackParms sound_beacon_explode;
    SLdrAudioPlaybackParms sound_beacon_hit;
    SLdrDamageVulnerability knee_vulnerability;
    SLdrDamageVulnerability vortex_vulnerability;
    SLdrDamageVulnerability toe_target_vulnerability;
};

void LoadTypedefSLdrDigitalGuardianData(SLdrDigitalGuardianData&, CInputStream&);

#endif // _SLDRDIGITALGUARDIANDATA
