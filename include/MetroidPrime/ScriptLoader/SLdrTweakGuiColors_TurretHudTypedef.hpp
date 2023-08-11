#ifndef _SLDRTWEAKGUICOLORS_TURRETHUDTYPEDEF
#define _SLDRTWEAKGUICOLORS_TURRETHUDTYPEDEF

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakGuiColors_TurretHudTypedef {
    SLdrTweakGuiColors_TurretHudTypedef();
    ~SLdrTweakGuiColors_TurretHudTypedef();

    CColor frameColor;
    CColor fontColor;
    CColor fontOutlineColor;
    CColor energyBarFillColor;
    CColor energyBarShadowColor;
    CColor energyBarEmptyColor;
};

void LoadTypedefSLdrTweakGuiColors_TurretHudTypedef(SLdrTweakGuiColors_TurretHudTypedef&, CInputStream&);

#endif // _SLDRTWEAKGUICOLORS_TURRETHUDTYPEDEF
