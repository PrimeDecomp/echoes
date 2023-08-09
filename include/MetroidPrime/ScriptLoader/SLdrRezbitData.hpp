#ifndef _SLDRREZBITDATA
#define _SLDRREZBITDATA

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAudioPlaybackParms.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPlasmaBeamInfo.hpp"

struct SLdrRezbitData {
    SLdrRezbitData();
    ~SLdrRezbitData();

    float hearing_radius;
    float unknown_0x4a6c4b40;
    float unknown_0x30d11671;
    CAssetId model_no_holos;
    CAssetId skin_no_holos;
    float shield_down_time;
    float unknown_0xffb37b81;
    float shield_up_time;
    float unknown_0x0d1d1648;
    CAssetId shield_explode_effect;
    SLdrAudioPlaybackParms sound_shield_explode;
    SLdrAudioPlaybackParms sound_shield_on;
    SLdrAudioPlaybackParms sound_shield_off;
    SLdrAudioPlaybackParms sound_flinch;
    float unknown_0xad120ad7;
    float unknown_0xe70ef8a3;
    SLdrAudioPlaybackParms audio_playback_parms;
    float unknown_0x70e597d4;
    float unknown_0x94980a67;
    float unknown_0xd02f08b0;
    float unknown_0x53e84718;
    float unknown_0x6fbc1bf9;
    float energy_bolt_chance;
    float cutting_laser_chance;
    float unknown_0x075491ca;
    float unknown_0x54f2892e;
    SLdrDamageInfo energy_bolt_damage;
    CAssetId energy_bolt_projectile;
    float energy_bolt_attack_duration;
    float unknown_0x28944183;
    float unknown_0xc7a69a59;
    SLdrAudioPlaybackParms sound_energy_bolt;
    float unknown_0x9ede657f;
    float unknown_0xcd787d9b;
    float virus_attack_time;
    SLdrDamageInfo virus_damage;
    int sound_always_ff;
    int sound_0xbb3f8a7b;
    int sound_0x601f846d;
    float unknown_0x64c7990d;
    float unknown_0x376181e9;
    SLdrDamageInfo cutting_laser_damage;
    SLdrAudioPlaybackParms sound_cutting_laser;
    SLdrPlasmaBeamInfo cutting_laser_beam_info;
    SLdrDamageVulnerability shield_vulnerability;
};

void LoadTypedefSLdrRezbitData(SLdrRezbitData&, CInputStream&);

#endif // _SLDRREZBITDATA
