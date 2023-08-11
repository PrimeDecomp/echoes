#ifndef _SLDRFISHCLOUD
#define _SLDRFISHCLOUD

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrFishCloud {
    SLdrFishCloud();
    ~SLdrFishCloud();

    SLdrEditorProperties editorProperties;
    bool active;
    CAssetId fishModel;
    SLdrAnimationParameters animationInformation;
    float fishCount;
    float speed;
    float influenceDistance;
    float unknown_0x61959f0d;
    float alignmentPriority;
    float separationPriority;
    float projectilePriority;
    float playerPriority;
    float containmentPriority;
    float wanderPriority;
    float wanderAmount;
    float playerBallPriority;
    float playerBallDistance;
    float projectileDecayRate;
    float playerDecayRate;
    float lookAheadTime;
    int updateFrame;
    CColor materialColor;
    bool canBeKilled;
    float collisionRadius;
    CAssetId deathEffect0;
    int deathEffect0Count;
    CAssetId deathEffect1;
    int deathEffect1Count;
    CAssetId deathEffect2;
    int deathEffect2Count;
    CAssetId deathEffect3;
    int deathEffect3Count;
    int deathSound;
    bool unknown_0xc320a050;
    bool unknown_0xcd4c81a1;
};

void LoadTypedefSLdrFishCloud(SLdrFishCloud&, CInputStream&);

#endif // _SLDRFISHCLOUD
