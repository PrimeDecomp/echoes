#ifndef _SLDRSNAKEWEEDSWARM
#define _SLDRSNAKEWEEDSWARM

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrSnakeWeedSwarm {
    SLdrSnakeWeedSwarm();
    ~SLdrSnakeWeedSwarm();

    SLdrEditorProperties editorProperties;
    SLdrAnimationParameters animationInformation;
    SLdrActorParameters actorInformation;
    float density;
    float maxDepth;
    float locationVariance;
    float detectionRadius;
    float grabRadius;
    float unknown_0x723737bc;
    float unknown_0x57452dd9;
    float retreatDepth;
    float moveSpeed;
    float unknown_0x11f854e2;
    float maxSlope;
    float minSize;
    float maxSize;
    float heightOffset;
    SLdrDamageInfo contactDamage;
    float damageWaitTime;
    int soundLooped;
    int soundIntoGround;
    int soundOutofGround;
};

void LoadTypedefSLdrSnakeWeedSwarm(SLdrSnakeWeedSwarm&, CInputStream&);

#endif // _SLDRSNAKEWEEDSWARM
