#ifndef _SLDRTWEAKBALL_CANNONBALL
#define _SLDRTWEAKBALL_CANNONBALL

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTDamageInfo.hpp"

struct SLdrTweakBall_CannonBall {
    SLdrTweakBall_CannonBall();
    ~SLdrTweakBall_CannonBall();

    SLdrTDamageInfo cannonBallDamage;
};

void LoadTypedefSLdrTweakBall_CannonBall(SLdrTweakBall_CannonBall&, CInputStream&);

#endif // _SLDRTWEAKBALL_CANNONBALL
