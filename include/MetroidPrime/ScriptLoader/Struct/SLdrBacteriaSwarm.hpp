#ifndef _SLDRBACTERIASWARM
#define _SLDRBACTERIASWARM

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrBasicSwarmProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrBacteriaSwarm {
    SLdrBacteriaSwarm();
    ~SLdrBacteriaSwarm();

    SLdrEditorProperties editorProperties;
    SLdrActorParameters actorInformation;
    SLdrAnimationParameters animationInformation;
    bool active;
    SLdrBasicSwarmProperties basicSwarmProperties;
    float unknown_0x4a85a2da;
    float containmentPriority;
    float bacteriaPatrolSpeed;
    float unknown_0x7de56d56;
    float unknown_0x39098c47;
    float bacteriaAcceleration;
    float bacteriaDeceleration;
    float patrolTurnSpeed;
    float unknown_0xbdcdb9c0;
    CAssetId bacteriaParticleEffect;
    CColor bacteriaPatrolColor;
    CColor bacteriaPlayerPursuitColor;
    float colorChangeTime;
    int patrolSound;
    int pursuitSound;
    float unknown_0xad4ce8f3;
    float unknown_0xa9d6d9d9;
    float patrolSoundWeight;
    float unknown_0x90f8e29f;
    float unknown_0x4b47b178;
    float pursuitSoundWeight;
    float unknown_0xd2986c43;
    float maxAudibleDistance;
    int minVolume;
    int maxVolume;
    CAssetId bacteriaScanModel;
    bool spawnInstantly;
};

void LoadTypedefSLdrBacteriaSwarm(SLdrBacteriaSwarm&, CInputStream&);

#endif // _SLDRBACTERIASWARM
