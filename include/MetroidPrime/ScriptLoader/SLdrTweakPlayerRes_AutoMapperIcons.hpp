#ifndef _SLDRTWEAKPLAYERRES_AUTOMAPPERICONS
#define _SLDRTWEAKPLAYERRES_AUTOMAPPERICONS

#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/string.hpp"

struct SLdrTweakPlayerRes_AutoMapperIcons {
    SLdrTweakPlayerRes_AutoMapperIcons();
    ~SLdrTweakPlayerRes_AutoMapperIcons();

    rstl::string saveStationIcon;
    rstl::string missileStationIcon;
    rstl::string elevatorIconIcon;
    rstl::string portalIcon;
    rstl::string unknown_0xfbf479ec;
    rstl::string unknown_0x5566b6e4;
    rstl::string unknown_0x51fe3f1f;
    rstl::string unknown_0xa4127a5a;
    rstl::string translatorDoorIcon;
};

void LoadTypedefSLdrTweakPlayerRes_AutoMapperIcons(SLdrTweakPlayerRes_AutoMapperIcons&, CInputStream&);

#endif // _SLDRTWEAKPLAYERRES_AUTOMAPPERICONS
