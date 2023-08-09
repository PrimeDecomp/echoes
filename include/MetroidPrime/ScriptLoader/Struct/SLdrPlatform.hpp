#ifndef _SLDRPLATFORM
#define _SLDRPLATFORM

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrHealthInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPlatformMotionProperties.hpp"

struct SLdrPlatform {
    SLdrPlatform();
    ~SLdrPlatform();

    SLdrEditorProperties editor_properties;
    CVector3f collision_box;
    CVector3f collision_offset;
    CAssetId model;
    SLdrAnimationParameters animation_information;
    SLdrActorParameters actor_information;
    CAssetId collision_model;
    SLdrHealthInfo health;
    SLdrDamageVulnerability vulnerability;
    float x_ray_transparency;
    int maximum_splashes;
    int splash_generation_rate;
    bool render_rain_splashes;
    bool unknown_0xf203bc81;
    SLdrPlatformMotionProperties motion_properties;
    CVector3f unknown_0x24fdeea1;
    float random_animation_offset;
};

void LoadTypedefSLdrPlatform(SLdrPlatform&, CInputStream&);

#endif // _SLDRPLATFORM
