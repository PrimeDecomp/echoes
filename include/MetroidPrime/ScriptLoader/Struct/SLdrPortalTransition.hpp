#ifndef _SLDRPORTALTRANSITION
#define _SLDRPORTALTRANSITION

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrPortalTransition {
    SLdrPortalTransition();
    ~SLdrPortalTransition();

    SLdrEditorProperties editor_properties;
    SLdrAnimationParameters animation_information;
    CVector3f player_scale;
    int volume;
    int pan;
    CAssetId agsc_0xe08e2172;
    CAssetId agsc_0xb3e6c4e3;
    int start_portal;
    int in_portal1;
    int in_portal2;
    int direction;
};

void LoadTypedefSLdrPortalTransition(SLdrPortalTransition&, CInputStream&);

#endif // _SLDRPORTALTRANSITION
