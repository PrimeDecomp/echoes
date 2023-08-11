#ifndef _SLDROCTAPEDESEGMENT
#define _SLDROCTAPEDESEGMENT

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrOctapedeSegment {
    SLdrOctapedeSegment();
    ~SLdrOctapedeSegment();

    SLdrEditorProperties editorProperties;
    int flavor;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    float waypointApproachDistance;
    float visibleDistance;
    float wallTurnSpeed;
    float floorTurnSpeed;
    float downTurnSpeed;
    float unknown_0xd5c25506;
    float projectileBoundsMultiplier;
    float collisionLookAhead;
    float animSpeedScalar;
    float maxAudibleDistance;
    bool initiallyPaused;
    float unknown_0x4fb8747e;
    CAssetId betweenSegmentsEffect;
    float unknown_0x9b9c46fc;
    float unknown_0x9f0677d6;
    float unknown_0xc0241fc1;
    float unknown_0xc4be2eeb;
    float unknown_0x99778599;
    float unknown_0xff92e3ed;
    float unknown_0xb8a1f0d5;
    float unknown_0xabe4167e;
    float unknown_0x2caddcbe;
    float unknown_0x4d320455;
    int unknown_0xd6f71bb3;
    int unknown_0x96b863c5;
    float unknown_0x417f4a91;
    SLdrDamageInfo explosionDamage;
    int walkSound;
    int idleSound;
    int seperateSound;
    int bounceSound;
    int explodeSound;
    float unknown_0x0c4763d7;
};

void LoadTypedefSLdrOctapedeSegment(SLdrOctapedeSegment&, CInputStream&);

#endif // _SLDROCTAPEDESEGMENT
