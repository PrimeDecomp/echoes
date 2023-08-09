#ifndef _SLDRBRIZGEE
#define _SLDRBRIZGEE

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrBrizgee {
    SLdrBrizgee();
    ~SLdrBrizgee();

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    float waypoint_approach_distance;
    float wall_turn_speed;
    float floor_turn_speed;
    float down_turn_speed;
    float visible_distance;
    float forward_moving_priority;
    CAssetId no_shell_model;
    CAssetId no_shell_skin;
    SLdrDamageVulnerability shell_vulnerability;
    float shell_health;
    SLdrDamageInfo shell_contact_damage;
    float unknown;
    SLdrDamageInfo poison_damage;
    float poison_time;
    int shell_break_sound;
    int poison_hit_sound;
    int player_poison_sound;
};

void LoadTypedefSLdrBrizgee(SLdrBrizgee&, CInputStream&);

#endif // _SLDRBRIZGEE
