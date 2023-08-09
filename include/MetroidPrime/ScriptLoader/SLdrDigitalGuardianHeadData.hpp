#ifndef _SLDRDIGITALGUARDIANHEADDATA
#define _SLDRDIGITALGUARDIANHEADDATA

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAudioPlaybackParms.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDigitalGuardianHeadStruct.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEchoParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPlasmaBeamInfo.hpp"

struct SLdrDigitalGuardianHeadData {
    SLdrDigitalGuardianHeadData();
    ~SLdrDigitalGuardianHeadData();

    CAssetId scannable_info_shield_head;
    CAssetId scannable_info_stunned_head;
    CAssetId scannable_info_final_head;
    CAssetId head_armor;
    float max_turn_speed;
    float max_linear_velocity;
    float unknown_0xe7de8b82;
    float unknown_0x96e18283;
    float unknown_0xf77138d5;
    float unknown_0x8a83a097;
    float unknown_0xd919fb13;
    SLdrAudioPlaybackParms audio_playback_parms_0x0a693a04;
    float unknown_0x1cc6d870;
    CAssetId part_0x5ff0b26c;
    SLdrAudioPlaybackParms audio_playback_parms_0x50baee63;
    SLdrAudioPlaybackParms audio_playback_parms_0xa5e1ec03;
    SLdrAudioPlaybackParms audio_playback_parms_0x8d6053cb;
    CAssetId part_0xc91ef399;
    float unknown_0x7d3d44af;
    CAssetId echo_targets;
    CAssetId part_0x342ae844;
    CAssetId explode_energy_stuff;
    SLdrAudioPlaybackParms audio_playback_parms_0x76eedd2a;
    SLdrAudioPlaybackParms audio_playback_parms_0xa5ecbe17;
    SLdrAudioPlaybackParms audio_playback_parms_0xb1fc705f;
    SLdrAudioPlaybackParms audio_playback_parms_0x2bc2677a;
    SLdrAudioPlaybackParms audio_playback_parms_0x7f5b82b2;
    SLdrEchoParameters echo_parameters;
    float unknown_0x8317610f;
    CAssetId part_0x321c97a9;
    SLdrAudioPlaybackParms audio_playback_parms_0xd5cc7e71;
    SLdrAudioPlaybackParms audio_playback_parms_0xa62c0ea7;
    SLdrAudioPlaybackParms audio_playback_parms_0xfd9f5486;
    CAssetId annihilator_pulse;
    SLdrDamageInfo annihilator_pulse_damage;
    CAssetId annihilator_charge;
    SLdrDamageInfo annihilator_charge_damage;
    float unknown_0xff7688bf;
    int unknown_0x12ebb390;
    CAssetId frme;
    SLdrPlasmaBeamInfo plasma_beam_info;
    CAssetId lock_on_missiles;
    SLdrDamageInfo lock_on_missiles_damage;
    CAssetId machine_gun;
    SLdrDamageInfo machine_gun_damage;
    SLdrAudioPlaybackParms sound_machine_gun;
    float unknown_0x4ab23ffe;
    float unknown_0x81a8474f;
    float unknown_0x71c406ac;
    float unknown_0xe5bc88b7;
    SLdrDigitalGuardianHeadStruct digital_guardian_head_struct_0x8f6732ea;
    SLdrDigitalGuardianHeadStruct digital_guardian_head_struct_0x8e128141;
    SLdrDigitalGuardianHeadStruct digital_guardian_head_struct_0xea54b390;
    SLdrDigitalGuardianHeadStruct digital_guardian_head_struct_0xbbd3e7a7;
    SLdrDigitalGuardianHeadStruct digital_guardian_head_struct_0x2dd88764;
    SLdrDigitalGuardianHeadStruct digital_guardian_head_struct_0x48b46e55;
    SLdrDamageVulnerability bomb_pit_vulnerability;
    SLdrDamageVulnerability echo_target_vulnerability;
};

void LoadTypedefSLdrDigitalGuardianHeadData(SLdrDigitalGuardianHeadData&, CInputStream&);

#endif // _SLDRDIGITALGUARDIANHEADDATA
