#ifndef _SLDRTWEAKPLAYERRES_MAPSCREENICONS
#define _SLDRTWEAKPLAYERRES_MAPSCREENICONS

#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/string.hpp"

struct SLdrTweakPlayerRes_MapScreenIcons {
    SLdrTweakPlayerRes_MapScreenIcons();
    ~SLdrTweakPlayerRes_MapScreenIcons();

    rstl::string lStickN;
    rstl::string lStickU;
    rstl::string lStickUl;
    rstl::string lStickL;
    rstl::string lStickDl;
    rstl::string lStickD;
    rstl::string lStickDr;
    rstl::string lStickR;
    rstl::string lStickUr;
    rstl::string cStickN;
    rstl::string cStickU;
    rstl::string cStickUl;
    rstl::string cStickL;
    rstl::string cStickDl;
    rstl::string cStickD;
    rstl::string cStickDr;
    rstl::string cStickR;
    rstl::string cStickUr;
    rstl::string lTriggerOut;
    rstl::string lTriggerIn;
    rstl::string rTriggerOut;
    rstl::string rTriggerIn;
    rstl::string startButtonOut;
    rstl::string startButtonIn;
    rstl::string aButtonOut;
    rstl::string aButtonIn;
    rstl::string bButtonOut;
    rstl::string bButtonIn;
    rstl::string xButtonOut;
    rstl::string xButtonIn;
    rstl::string yButtonOut;
    rstl::string yButtonIn;
};

void LoadTypedefSLdrTweakPlayerRes_MapScreenIcons(SLdrTweakPlayerRes_MapScreenIcons&, CInputStream&);

#endif // _SLDRTWEAKPLAYERRES_MAPSCREENICONS
