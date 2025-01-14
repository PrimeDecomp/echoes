#ifndef _SLDRTWEAKPLAYERRES
#define _SLDRTWEAKPLAYERRES

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTBallTransitionResources.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTGunResources.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerRes_AutoMapperIcons.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerRes_MapScreenIcons.hpp"
#include "rstl/string.hpp"

struct SLdrTweakPlayerRes {
    SLdrTweakPlayerRes();
    ~SLdrTweakPlayerRes();

    rstl::string instanceName;
    SLdrTweakPlayerRes_AutoMapperIcons autoMapperIcons;
    SLdrTweakPlayerRes_MapScreenIcons mapScreenIcons;
    SLdrTBallTransitionResources ballTransitionResources;
    SLdrTGunResources cinematicResources;
    float unknown;
};

void LoadTypedefSLdrTweakPlayerRes(SLdrTweakPlayerRes&, CInputStream&);

#endif // _SLDRTWEAKPLAYERRES
