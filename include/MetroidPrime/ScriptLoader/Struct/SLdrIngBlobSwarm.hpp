#ifndef _SLDRINGBLOBSWARM
#define _SLDRINGBLOBSWARM

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrBasicSwarmProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrIngBlobSwarm {
    SLdrIngBlobSwarm();
    ~SLdrIngBlobSwarm();

    SLdrEditorProperties editorProperties;
    SLdrActorParameters actorInformation;
    SLdrAnimationParameters animationInformation;
    bool active;
    SLdrBasicSwarmProperties basicSwarmProperties;
    int unknown_0x7399abbb;
    int unknown_0x734d923b;
    float maxAttackAngle;
    float intoAttackSpeed;
    float attackSpeed;
    float mass;
    float maxAttackHeight;
    CVector3f attackAimOffset;
};

void LoadTypedefSLdrIngBlobSwarm(SLdrIngBlobSwarm&, CInputStream&);

#endif // _SLDRINGBLOBSWARM
