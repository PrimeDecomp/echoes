#ifndef _SLDRTWEAKGUI_LOGBOOK
#define _SLDRTWEAKGUI_LOGBOOK

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakGui_LogBook {
    SLdrTweakGui_LogBook();
    ~SLdrTweakGui_LogBook();

    CColor main_window_border_color;
    CColor main_window_text_color;
    CColor main_window_selected_text_color;
    CColor unknown_0xfe98f30e;
    CColor unknown_0x39e57ac0;
    CColor legend_background_color;
    CColor node_color;
    CColor selected_node_color;
    CColor parent_node_color;
    CColor unknown_0x56843943;
    CColor selected_text_cursor_color;
    float unknown_0x905ce2c0;
    float text_scale;
    float selected_text_scale;
    float transition_time;
    spline node_collapse_motion;
    spline selected_node_collapse_motion;
    spline node_expand_motion;
    float rotation_speed;
    float node_scale;
    float selected_node_scale;
    float selected_text_cursor_scale;
    float scan_model_scale;
    spline unknown_0x925b9d4a;
    CColor unknown_0x075a0734;
    float unknown_0x8ef43309;
    float fog_near;
    float fog_far;
    CColor fog_color;
    float unknown_0xeef15783;
    float unknown_0x78055ab2;
    float unknown_0x09e3197f;
    float unknown_0xfaffce1f;
    float unknown_0xe5280e7c;
    CColor background_sweep_color;
    float background_sweep_radius;
    float background_sweep_time;
    CColor scan_text_window_background_color;
    CColor scan_text_window_border_color;
    CColor scan_text_window_font_color;
    CColor legend_window_background_color;
    CColor legend_window_border_color;
    CColor legend_window_font_color;
    float legend_hide_time;
    CColor unknown_0x7f35c573;
    CColor unknown_0xe0d77cf1;
    CColor unknown_0x903d9b7e;
    CColor unknown_0x0fdf22fc;
    CColor unknown_0xbeaf9d6b;
    CColor unknown_0x7e8247d6;
    CColor unknown_0x8b3f7902;
    CColor unknown_0xdc1abb82;
    CColor unknown_0x306b2b24;
    CColor frame_color;
    CColor unknown_0x1a4b8068;
    CColor slider_background_color;
    CColor slider_selection_color;
    float slider_scale;
    float slider_speed;
    float unknown_0x58795865;
    float unknown_0x35e0bd31;
    CColor menu_option_color;
    CColor menu_option_enabled_arrow_color;
    CColor menu_option_disabled_arrow_color;
    float menu_option_scale;
    float menu_option_arrow_scale;
    float model_rotation_speed;
    float unknown_0x60914b79;
    float unknown_0x1fd44d9b;
    CVector3f model_light1_position;
    CColor model_light1_color;
    CVector3f model_light2_position;
    CColor model_light2_color;
    CColor model_ambient_light_color;
};

void LoadTypedefSLdrTweakGui_LogBook(SLdrTweakGui_LogBook&, CInputStream&);

#endif // _SLDRTWEAKGUI_LOGBOOK
