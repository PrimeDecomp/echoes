#ifndef _SLDRPICKUP
#define _SLDRPICKUP

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEchoParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPlayerItem.hpp"

struct SLdrPickup {
    SLdrPickup();

    SLdrEditorProperties editor_properties;
    CVector3f collision_size;
    CVector3f collision_offset;
    SLdrPlayerItem item_to_give;
    int capacity_increase;
    int item_percentage_increase;
    int amount;
    float respawn_time;
    float pickup_effect_lifetime;
    float lifetime;
    float fadetime;
    CAssetId model;
    SLdrAnimationParameters animation_information;
    SLdrActorParameters actor_information;
    SLdrEchoParameters echo_information;
    float activation_delay;
    CAssetId pickup_effect;
    bool absolute_value;
    bool calculate_visibility;
    bool unknown;
    float auto_home_range;
    float delay_until_home;
    float homing_speed;
    bool auto_spin;
    bool blink_out;
    CVector3f orbit_offset;
};

void LoadTypedefSLdrPickup(SLdrPickup&, CInputStream&);

#endif // _SLDRPICKUP
