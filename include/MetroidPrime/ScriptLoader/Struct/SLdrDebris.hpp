#ifndef _SLDRDEBRIS
#define _SLDRDEBRIS

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrDebris {
    SLdrDebris();
    ~SLdrDebris();

    SLdrEditorProperties editorProperties;
    float impulse;
    CVector3f impulseVariance;
    CColor fadeOutColor;
    float mass;
    float unknown_0x417f4a91;
    float lifeTime;
    int scaleType;
    bool randomSpin;
    CAssetId model;
    SLdrActorParameters actorInformation;
    CAssetId particle;
    CVector3f particleSystemScale;
    bool isCollider;
    bool unknown_0x4edb1d0e;
};

void LoadTypedefSLdrDebris(SLdrDebris&, CInputStream&);

#endif // _SLDRDEBRIS
