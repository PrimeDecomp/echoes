#ifndef _SLDRTWEAKBALL_CANNONBALL
#define _SLDRTWEAKBALL_CANNONBALL

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTDamageInfo.hpp"

struct SLdrTweakBall_CannonBall {
    SLdrTweakBall_CannonBall();
    ~SLdrTweakBall_CannonBall();

    SLdrTDamageInfo cannon_ball_damage;
};

void LoadTypedefSLdrTweakBall_CannonBall(SLdrTweakBall_CannonBall&, CInputStream&);

#endif // _SLDRTWEAKBALL_CANNONBALL
