#ifndef _SLDRCOIN
#define _SLDRCOIN

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrCoin {
    SLdrCoin();
    ~SLdrCoin();

    SLdrEditorProperties editorProperties;
    float coneSpread;
    float minimumSpeed;
    float maximumSpeed;
    float minimumSpinSpeed;
    float maximumSpinSpeed;
    float minimumLifeTime;
    float maximumLifeTime;
    float disableCollisionTime;
    float fadeInEndPercentage;
    float fadeOutStartPercentage;
    CColor startColor;
    CColor endColor;
    float scaleStartPercentage;
    CVector3f finalScale;
    float unknown_0x417f4a91;
    float gravity;
    CVector3f positionOffset;
    CAssetId model;
    SLdrActorParameters actorInformation;
    CAssetId particle1;
    int bounceSound;
    int maxBounceSounds;
    float unknown_0x76c79503;
    float unknown_0x310dfac8;
    CVector3f particleSystem1Scale;
    bool particleSystem1UsesGlobalTranslation;
    bool particleSystem1WaitForParticlesToDie;
    int particleSystem1Orientation;
    CAssetId particle2;
    CVector3f particleSystem2Scale;
    bool particleSystem2UsesGlobalTranslation;
    bool particleSystem2WaitForParticlesToDie;
    int particleSystem2Orientation;
    CAssetId deathParticle;
    CVector3f deathParticleSystemScale;
    int deathParticleSystemOrientation;
    bool isCollider;
    bool isShootable;
    bool dieOnCollision;
    bool unknown_0xdcaa0f22;
    bool unknown_0xbfd82a19;
    bool unknown_0x723d42d6;
    float disablePhysicsThreshold;
};

void LoadTypedefSLdrCoin(SLdrCoin&, CInputStream&);

#endif // _SLDRCOIN
