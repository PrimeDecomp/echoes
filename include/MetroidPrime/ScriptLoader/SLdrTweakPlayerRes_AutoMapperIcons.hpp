#ifndef _SLDRTWEAKPLAYERRES_AUTOMAPPERICONS
#define _SLDRTWEAKPLAYERRES_AUTOMAPPERICONS

#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/string.hpp"

struct SLdrTweakPlayerRes_AutoMapperIcons {
    SLdrTweakPlayerRes_AutoMapperIcons();
    ~SLdrTweakPlayerRes_AutoMapperIcons();

    rstl::string save_station_icon;
    rstl::string missile_station_icon;
    rstl::string elevator_icon_icon;
    rstl::string portal_icon;
    rstl::string unknown_0xfbf479ec;
    rstl::string unknown_0x5566b6e4;
    rstl::string unknown_0x51fe3f1f;
    rstl::string unknown_0xa4127a5a;
    rstl::string translator_door_icon;
};

void LoadTypedefSLdrTweakPlayerRes_AutoMapperIcons(SLdrTweakPlayerRes_AutoMapperIcons&, CInputStream&);

#endif // _SLDRTWEAKPLAYERRES_AUTOMAPPERICONS
