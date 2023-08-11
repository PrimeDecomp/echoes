#ifndef _SLDRPUFFER
#define _SLDRPUFFER

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrPuffer {
    SLdrPuffer();
    ~SLdrPuffer();

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    float hoverSpeed;
    CAssetId cloudEffect;
    SLdrDamageInfo cloudDamage;
    CAssetId cloudSteam;
    float cloudSteamAlpha;
    bool orbitInterpolantFollowed;
    bool cloudInDark;
    bool cloudInEcho;
    SLdrDamageInfo explosionDamage;
    int soundTurn;
};

void LoadTypedefSLdrPuffer(SLdrPuffer&, CInputStream&);

#endif // _SLDRPUFFER
