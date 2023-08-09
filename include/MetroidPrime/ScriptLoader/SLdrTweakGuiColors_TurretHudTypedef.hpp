#ifndef _SLDRTWEAKGUICOLORS_TURRETHUDTYPEDEF
#define _SLDRTWEAKGUICOLORS_TURRETHUDTYPEDEF

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakGuiColors_TurretHudTypedef {
    SLdrTweakGuiColors_TurretHudTypedef();
    ~SLdrTweakGuiColors_TurretHudTypedef();

    CColor frame_color;
    CColor font_color;
    CColor font_outline_color;
    CColor energy_bar_fill_color;
    CColor energy_bar_shadow_color;
    CColor energy_bar_empty_color;
};

void LoadTypedefSLdrTweakGuiColors_TurretHudTypedef(SLdrTweakGuiColors_TurretHudTypedef&, CInputStream&);

#endif // _SLDRTWEAKGUICOLORS_TURRETHUDTYPEDEF
