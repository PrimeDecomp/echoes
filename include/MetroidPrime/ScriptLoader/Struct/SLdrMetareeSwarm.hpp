#ifndef _SLDRMETAREESWARM
#define _SLDRMETAREESWARM

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrBasicSwarmProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrMetareeSwarm {
    SLdrMetareeSwarm();
    ~SLdrMetareeSwarm();

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
};

void LoadTypedefSLdrMetareeSwarm(SLdrMetareeSwarm&, CInputStream&);

#endif // _SLDRMETAREESWARM
