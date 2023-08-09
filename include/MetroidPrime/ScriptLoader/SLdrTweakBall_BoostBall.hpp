#ifndef _SLDRTWEAKBALL_BOOSTBALL
#define _SLDRTWEAKBALL_BOOSTBALL

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTDamageInfo.hpp"

struct SLdrTweakBall_BoostBall {
    SLdrTweakBall_BoostBall();
    ~SLdrTweakBall_BoostBall();

    float boost_ball_drain_time;
    float boost_ball_min_charge_time;
    float boost_ball_min_relative_speed_for_damage;
    float boost_ball_charge_time1;
    float boost_ball_charge_time2;
    float boost_ball_max_charge_time;
    float boost_ball_incremental_speed1;
    float boost_ball_incremental_speed2;
    float boost_ball_incremental_speed3;
    float unknown_0xbe605660;
    SLdrTDamageInfo boost_ball_damage;
    float unknown_0x6d210beb;
    float unknown_0xfdc6649d;
    float unknown_0x340be92f;
};

void LoadTypedefSLdrTweakBall_BoostBall(SLdrTweakBall_BoostBall&, CInputStream&);

#endif // _SLDRTWEAKBALL_BOOSTBALL
