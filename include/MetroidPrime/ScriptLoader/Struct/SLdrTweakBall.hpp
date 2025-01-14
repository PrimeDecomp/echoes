#ifndef _SLDRTWEAKBALL
#define _SLDRTWEAKBALL

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakBall_BoostBall.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakBall_Camera.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakBall_CannonBall.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakBall_DeathBall.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakBall_Misc.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakBall_Movement.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakBall_ScrewAttack.hpp"
#include "rstl/string.hpp"

struct SLdrTweakBall {
    SLdrTweakBall();
    ~SLdrTweakBall();

    rstl::string instanceName;
    SLdrTweakBall_Movement movement;
    SLdrTweakBall_Camera camera;
    SLdrTweakBall_Misc misc;
    SLdrTweakBall_BoostBall boostBall;
    SLdrTweakBall_CannonBall cannonBall;
    SLdrTweakBall_ScrewAttack screwAttack;
    SLdrTweakBall_DeathBall deathBall;
};

void LoadTypedefSLdrTweakBall(SLdrTweakBall&, CInputStream&);

#endif // _SLDRTWEAKBALL
