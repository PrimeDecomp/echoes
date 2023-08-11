#ifndef _SLDRGLOWBUG
#define _SLDRGLOWBUG

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrGlowbug {
    SLdrGlowbug();
    ~SLdrGlowbug();

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    CAssetId deathFlashEffect;
    CAssetId part;
    CAssetId attackEffect;
    CAssetId attackTelegraphEffect;
    CAssetId attackEchoEffect;
    float attackDuration;
    float attackTelegraphDuration;
    CVector3f attackAimOffset;
    int attackTelegraphSound;
    int attackSound;
    CAssetId scanModel;
    bool isInLightWorld;
};

void LoadTypedefSLdrGlowbug(SLdrGlowbug&, CInputStream&);

#endif // _SLDRGLOWBUG
