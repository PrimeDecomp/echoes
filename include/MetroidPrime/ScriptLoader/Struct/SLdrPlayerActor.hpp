#ifndef _SLDRPLAYERACTOR
#define _SLDRPLAYERACTOR

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrHealthInfo.hpp"

struct SLdrPlayerActor {
    SLdrPlayerActor();
    ~SLdrPlayerActor();

    SLdrEditorProperties editor_properties;
    CVector3f collision_box;
    CVector3f collision_offset;
    float mass;
    float gravity;
    SLdrHealthInfo health;
    SLdrDamageVulnerability vulnerability;
    CAssetId no_model;
    SLdrAnimationParameters animation_information;
    SLdrActorParameters actor_information;
    bool is_loop;
    bool immovable;
    bool is_solid;
    int flags_player_actor;
    int render_gun_override;
};

void LoadTypedefSLdrPlayerActor(SLdrPlayerActor&, CInputStream&);

#endif // _SLDRPLAYERACTOR
