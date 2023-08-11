#ifndef _SLDRTWEAKBALL_BOOSTBALL
#define _SLDRTWEAKBALL_BOOSTBALL

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTDamageInfo.hpp"

struct SLdrTweakBall_BoostBall {
    SLdrTweakBall_BoostBall();
    ~SLdrTweakBall_BoostBall();

    float boostBallDrainTime;
    float boostBallMinChargeTime;
    float boostBallMinRelativeSpeedForDamage;
    float boostBallChargeTime1;
    float boostBallChargeTime2;
    float boostBallMaxChargeTime;
    float boostBallIncrementalSpeed1;
    float boostBallIncrementalSpeed2;
    float boostBallIncrementalSpeed3;
    float unknown_0xbe605660;
    SLdrTDamageInfo boostBallDamage;
    float unknown_0x6d210beb;
    float unknown_0xfdc6649d;
    float unknown_0x340be92f;
};

void LoadTypedefSLdrTweakBall_BoostBall(SLdrTweakBall_BoostBall&, CInputStream&);

#endif // _SLDRTWEAKBALL_BOOSTBALL
