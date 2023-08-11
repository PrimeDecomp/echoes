#ifndef _SLDRRIFTPORTAL
#define _SLDRRIFTPORTAL

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrRiftPortal {
    SLdrRiftPortal();
    ~SLdrRiftPortal();

    SLdrEditorProperties editorProperties;
    CAssetId model;
    SLdrAnimationParameters animationInformation;
    CAssetId backgroundModel;
    SLdrAnimationParameters backgroundAnimation;
    CAssetId incandescentModel;
    SLdrAnimationParameters incandescentAnimation;
    CAssetId lineModel;
    SLdrAnimationParameters lineAnimation;
    bool ripPortal;
    int projectileAttraction;
    float projectileBoxWidth;
    float projectileAngle;
    float projectileDestructionRadius;
};

void LoadTypedefSLdrRiftPortal(SLdrRiftPortal&, CInputStream&);

#endif // _SLDRRIFTPORTAL
