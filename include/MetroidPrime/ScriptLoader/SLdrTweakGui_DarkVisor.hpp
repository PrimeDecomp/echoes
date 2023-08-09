#ifndef _SLDRTWEAKGUI_DARKVISOR
#define _SLDRTWEAKGUI_DARKVISOR

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakGui_DarkVisor {
    SLdrTweakGui_DarkVisor();
    ~SLdrTweakGui_DarkVisor();

    CColor dark_world_base_color;
    CColor dark_visor_static_color;
    CColor dark_visor_palette_modulate;
    float dark_visor_blur_speed;
    int dark_visor_frame_top;
    int dark_visor_frame_height;
    int dark_visor_frame_left;
    int dark_visor_frame_width;
};

void LoadTypedefSLdrTweakGui_DarkVisor(SLdrTweakGui_DarkVisor&, CInputStream&);

#endif // _SLDRTWEAKGUI_DARKVISOR
