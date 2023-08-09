#ifndef _SLDRPLANTSCARABSWARM
#define _SLDRPLANTSCARABSWARM

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrBasicSwarmProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrPlantScarabSwarm {
    SLdrPlantScarabSwarm();
    ~SLdrPlantScarabSwarm();

    SLdrEditorProperties editor_properties;
    SLdrActorParameters actor_information;
    SLdrAnimationParameters animation_information;
    bool active;
    SLdrBasicSwarmProperties basic_swarm_properties;
    int unknown_0x7399abbb;
    int unknown_0x734d923b;
    float max_attack_angle;
    float into_attack_speed;
    float attack_speed;
    float grenade_mass;
    float grenade_launch_speed;
    float unknown_0xed086ce0;
    int unknown_0x454f16b1;
    SLdrDamageInfo grenade_damage;
    float grenade_explosion_proximity;
    CAssetId grenade_explosion_effect;
    CAssetId part;
    CAssetId grenade_trail_effect;
    CAssetId grenade_effect;
    int grenade_bounce_sound;
    float grenade_bounce_sound_fall_off;
    float unknown_0x15e0c159;
    int grenade_explosion_sound;
    float grenade_explosion_sound_fall_off;
    float unknown_0xab84892e;
};

void LoadTypedefSLdrPlantScarabSwarm(SLdrPlantScarabSwarm&, CInputStream&);

#endif // _SLDRPLANTSCARABSWARM
