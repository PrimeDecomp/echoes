#ifndef _SLDRACTOR
#define _SLDRACTOR

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEchoParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrHealthInfo.hpp"

struct SLdrActor {
    SLdrActor();
    ~SLdrActor();

    SLdrEditorProperties editor_properties;
    CVector3f collision_box;
    CVector3f collision_offset;
    float mass;
    float gravity;
    SLdrHealthInfo health;
    SLdrDamageVulnerability vulnerability;
    CAssetId model;
    CAssetId collision_model;
    SLdrAnimationParameters animation_information;
    SLdrActorParameters actor_information;
    SLdrEchoParameters echo_information;
    bool is_loop;
    bool immovable;
    bool is_solid;
    bool is_camera_through;
    bool is_scan_through;
    int render_texture_set;
    bool draws_shadow;
    bool scale_animation;
    bool ai_shoot_through;
    float random_animation_offset;
    CAssetId projectile;
    SLdrDamageInfo projectile_damage;
};

void LoadTypedefSLdrActor(SLdrActor&, CInputStream&);

#endif // _SLDRACTOR
