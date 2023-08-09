#ifndef _SLDRRIFTPORTAL
#define _SLDRRIFTPORTAL

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrRiftPortal {
    SLdrRiftPortal();
    ~SLdrRiftPortal();

    SLdrEditorProperties editor_properties;
    CAssetId model;
    SLdrAnimationParameters animation_information;
    CAssetId background_model;
    SLdrAnimationParameters background_animation;
    CAssetId incandescent_model;
    SLdrAnimationParameters incandescent_animation;
    CAssetId line_model;
    SLdrAnimationParameters line_animation;
    bool rip_portal;
    int projectile_attraction;
    float projectile_box_width;
    float projectile_angle;
    float projectile_destruction_radius;
};

void LoadTypedefSLdrRiftPortal(SLdrRiftPortal&, CInputStream&);

#endif // _SLDRRIFTPORTAL
