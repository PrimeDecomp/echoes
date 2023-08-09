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
    float mass;
    float max_attack_height;
    CVector3f attack_aim_offset;
};

void LoadTypedefSLdrIngBlobSwarm(SLdrIngBlobSwarm&, CInputStream&);

#endif // _SLDRINGBLOBSWARM
