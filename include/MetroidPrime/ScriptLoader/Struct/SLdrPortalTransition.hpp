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

    SLdrEditorProperties editorProperties;
    SLdrAnimationParameters animationInformation;
    CVector3f playerScale;
    int volume;
    int pan;
    CAssetId agsc_0xe08e2172;
    CAssetId agsc_0xb3e6c4e3;
    int startPortal;
    int inPortal1;
    int inPortal2;
    int direction;
};

void LoadTypedefSLdrPortalTransition(SLdrPortalTransition&, CInputStream&);

#endif // _SLDRPORTALTRANSITION
