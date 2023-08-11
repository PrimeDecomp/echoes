#ifndef _SLDREYEBALL
#define _SLDREYEBALL

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrEyeBall {
    SLdrEyeBall();
    ~SLdrEyeBall();

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    float closeTime;
    float fireWaitTime;
    CAssetId projectile;
    SLdrDamageInfo rayDamage;
    CAssetId plasmaBurn;
    CAssetId plasmaPulse;
    CAssetId plasmaTexture;
    CAssetId plasmaGlow;
    CColor laserInnerColor;
    CColor laserOuterColor;
    int unknown_0x81d14be8;
    int unknown_0x6e1320d6;
    int unknown_0x85249bd5;
    int unknown_0x6ae6f0eb;
    int laserSound;
    bool shouldBeTriggered;
    float maxAudibleDistance;
    float dropOff;
};

void LoadTypedefSLdrEyeBall(SLdrEyeBall&, CInputStream&);

#endif // _SLDREYEBALL
