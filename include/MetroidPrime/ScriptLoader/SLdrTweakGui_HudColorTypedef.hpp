#ifndef _SLDRTWEAKGUI_HUDCOLORTYPEDEF
#define _SLDRTWEAKGUI_HUDCOLORTYPEDEF

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakGui_HudColorTypedef {
    SLdrTweakGui_HudColorTypedef();
    ~SLdrTweakGui_HudColorTypedef();

    CColor energyBarFilledColor;
    CColor energyBarEmptyColor;
    CColor energyBarShadowColor;
    CColor energyTanksFilledColor;
    CColor energyTanksEmptyColor;
    CColor unknown_0x1fd3d43a;
    CColor unknown_0xe1ff2a4f;
    CColor unknown_0x9cfb8a36;
    CColor unknown_0x89a45bcb;
    CColor unknown_0xc2a8ccc6;
};

void LoadTypedefSLdrTweakGui_HudColorTypedef(SLdrTweakGui_HudColorTypedef&, CInputStream&);

#endif // _SLDRTWEAKGUI_HUDCOLORTYPEDEF
