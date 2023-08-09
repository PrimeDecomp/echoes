#ifndef _SLDRTWEAKBALL_DEATHBALL
#define _SLDRTWEAKBALL_DEATHBALL

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTDamageInfo.hpp"

struct SLdrTweakBall_DeathBall {
    SLdrTweakBall_DeathBall();
    ~SLdrTweakBall_DeathBall();

    float death_ball_damage_delay;
    SLdrTDamageInfo death_ball_damage;
};

void LoadTypedefSLdrTweakBall_DeathBall(SLdrTweakBall_DeathBall&, CInputStream&);

#endif // _SLDRTWEAKBALL_DEATHBALL
