#ifndef _SLDRTWEAKGUICOLORS_HUDCOLORSTYPEDEF
#define _SLDRTWEAKGUICOLORS_HUDCOLORSTYPEDEF

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakGuiColors_HUDColorsTypedef {
    SLdrTweakGuiColors_HUDColorsTypedef();
    ~SLdrTweakGuiColors_HUDColorsTypedef();

    CColor unknown_0xc8ddc662;
    CColor threatGroupActiveColor;
    CColor threatGroupInactiveColor;
    CColor unknown_0xa6609cc5;
    CColor missileGroupActiveColor;
    CColor missileGroupInactiveColor;
    CColor unknown_0xdcaab836;
    CColor energyBarFilledColor;
    CColor energyBarShadowColor;
    CColor energyBarEmptyColor;
    CColor energyTanksFilledColor;
    CColor energyTanksEmptyColor;
    CColor radarWidgetColor;
    CColor activeTextForegroundColor;
    CColor inactiveTextForegroundColor;
    CColor textShadowOutlineColor;
};

void LoadTypedefSLdrTweakGuiColors_HUDColorsTypedef(SLdrTweakGuiColors_HUDColorsTypedef&, CInputStream&);

#endif // _SLDRTWEAKGUICOLORS_HUDCOLORSTYPEDEF
