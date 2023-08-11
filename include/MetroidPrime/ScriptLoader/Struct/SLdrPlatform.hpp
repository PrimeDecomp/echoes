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

    SLdrEditorProperties editorProperties;
    CVector3f collisionBox;
    CVector3f collisionOffset;
    CAssetId model;
    SLdrAnimationParameters animationInformation;
    SLdrActorParameters actorInformation;
    CAssetId collisionModel;
    SLdrHealthInfo health;
    SLdrDamageVulnerability vulnerability;
    float xRayTransparency;
    int maximumSplashes;
    int splashGenerationRate;
    bool renderRainSplashes;
    bool unknown_0xf203bc81;
    SLdrPlatformMotionProperties motionProperties;
    CVector3f unknown_0x24fdeea1;
    float randomAnimationOffset;
};

void LoadTypedefSLdrPlatform(SLdrPlatform&, CInputStream&);

#endif // _SLDRPLATFORM
