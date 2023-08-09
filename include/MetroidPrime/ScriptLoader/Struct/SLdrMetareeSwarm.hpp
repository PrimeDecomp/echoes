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

    SLdrEditorProperties editor_properties;
    SLdrActorParameters actor_information;
    SLdrAnimationParameters animation_information;
    bool active;
    SLdrBasicSwarmProperties basic_swarm_properties;
    int unknown_0x7399abbb;
    int unknown_0x734d923b;
    float max_attack_angle;
    float into_attack_speed;
    float attack_speed;
};

void LoadTypedefSLdrMetareeSwarm(SLdrMetareeSwarm&, CInputStream&);

#endif // _SLDRMETAREESWARM
