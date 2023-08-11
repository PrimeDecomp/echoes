#ifndef _SLDRSPORBNEEDLE
#define _SLDRSPORBNEEDLE

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrSporbNeedle {
    SLdrSporbNeedle();
    ~SLdrSporbNeedle();

    SLdrEditorProperties editorProperties;
    SLdrActorParameters actorInformation;
    CAssetId model;
    float initialSpeed;
    float mass;
    SLdrDamageInfo attackDamage;
    float fuseTime;
    CAssetId trailEffect;
    CAssetId explosionEffect;
    int launchSound;
    int flightSound;
    int hitPlayerSound;
    int collisionSound;
    int explosionSound;
};

void LoadTypedefSLdrSporbNeedle(SLdrSporbNeedle&, CInputStream&);

#endif // _SLDRSPORBNEEDLE
