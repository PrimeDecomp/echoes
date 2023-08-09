#ifndef _SLDRTWEAKGUICOLORS_HUDCOLORSTYPEDEF
#define _SLDRTWEAKGUICOLORS_HUDCOLORSTYPEDEF

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakGuiColors_HUDColorsTypedef {
    SLdrTweakGuiColors_HUDColorsTypedef();
    ~SLdrTweakGuiColors_HUDColorsTypedef();

    CColor unknown_0xc8ddc662;
    CColor threat_group_active_color;
    CColor threat_group_inactive_color;
    CColor unknown_0xa6609cc5;
    CColor missile_group_active_color;
    CColor missile_group_inactive_color;
    CColor unknown_0xdcaab836;
    CColor energy_bar_filled_color;
    CColor energy_bar_shadow_color;
    CColor energy_bar_empty_color;
    CColor energy_tanks_filled_color;
    CColor energy_tanks_empty_color;
    CColor radar_widget_color;
    CColor active_text_foreground_color;
    CColor inactive_text_foreground_color;
    CColor text_shadow_outline_color;
};

void LoadTypedefSLdrTweakGuiColors_HUDColorsTypedef(SLdrTweakGuiColors_HUDColorsTypedef&, CInputStream&);

#endif // _SLDRTWEAKGUICOLORS_HUDCOLORSTYPEDEF
