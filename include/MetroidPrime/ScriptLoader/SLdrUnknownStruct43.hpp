#ifndef _SLDRUNKNOWNSTRUCT43
#define _SLDRUNKNOWNSTRUCT43

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrIngPossessionData.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPlasmaBeamInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct42.hpp"

struct SLdrUnknownStruct43 {
    SLdrUnknownStruct43();
    ~SLdrUnknownStruct43();

    int unknown_0xbd80fd94;
    float max_linear_velocity;
    float max_turn_speed;
    float scanning_turn_speed;
    float unknown_0xe32fcae9;
    float unknown_0xc5e0b92c;
    float unknown_0xc17a8806;
    int unknown_0xe75bae9e;
    CAssetId laser_pulse_projectile;
    SLdrDamageInfo laser_pulse_damage;
    int unknown_0xeda45014;
    int unknown_0x7dd740fe;
    float dodge_chance;
    float reset_shield_time;
    float split_destroyed_priority;
    float laser_sweep_turn_speed;
    SLdrDamageInfo laser_sweep_damage;
    SLdrPlasmaBeamInfo laser_sweep_beam_info;
    SLdrUnknownStruct42 unknown_struct42;
    int sound_laser_sweep;
    int sound_laser_charge_up;
    int sound_docking;
    int sound_scanning;
    int sound_light_shield;
    int sound_dark_shield;
    int sound_shield_on;
    SLdrIngPossessionData ing_possession_data;
    SLdrDamageVulnerability light_shield_vulnerability;
    SLdrDamageVulnerability dark_shield_vulnerability;
};

void LoadTypedefSLdrUnknownStruct43(SLdrUnknownStruct43&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT43
