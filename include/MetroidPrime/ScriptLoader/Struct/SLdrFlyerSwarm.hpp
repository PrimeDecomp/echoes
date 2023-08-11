#ifndef _SLDRFLYERSWARM
#define _SLDRFLYERSWARM

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrBasicSwarmProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrFlyerSwarm {
    SLdrFlyerSwarm();
    ~SLdrFlyerSwarm();

    SLdrEditorProperties editorProperties;
    SLdrActorParameters actorInformation;
    SLdrAnimationParameters animationInformation;
    bool active;
    SLdrBasicSwarmProperties basicSwarmProperties;
    float unknown_0x4a85a2da;
    float unknown_0x10cccd3c;
    float unknown_0x1e8e90a4;
    float unknown_0x262e586d;
    float rollUprightSpeed;
    float rollUprightMinAngle;
};

void LoadTypedefSLdrFlyerSwarm(SLdrFlyerSwarm&, CInputStream&);

#endif // _SLDRFLYERSWARM
