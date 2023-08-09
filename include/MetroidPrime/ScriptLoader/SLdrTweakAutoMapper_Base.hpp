#ifndef _SLDRTWEAKAUTOMAPPER_BASE
#define _SLDRTWEAKAUTOMAPPER_BASE

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakAutoMapper_Base {
    SLdrTweakAutoMapper_Base();
    ~SLdrTweakAutoMapper_Base();

    bool unknown_0xcbe595d8;
    bool unknown_0x8ecb53a6;
    bool scale_move_speed_with_camera_distance;
    float unknown_0x6bea9324;
    float unknown_0x065dd754;
    float unknown_0x57a46c09;
    float unknown_0xb54255b5;
    float unknown_0x0c64cec4;
    float unknown_0x335ebc7e;
    float map_screen_area_opacity;
    CColor unknown_0x533c5684;
    float unknown_0xeb383668;
    float unknown_0x27151ede;
    float unknown_0x434172c3;
    float unknown_0x68097036;
    CColor unknown_0x03adcf46;
    CColor unknown_0xd3fae283;
    CColor unknown_0x65d2cf45;
    CColor unknown_0xb5752c08;
    CColor unknown_0x035d01ce;
    CColor unknown_0x805d5fa3;
    CColor unknown_0x36757265;
    CColor unknown_0xe550fcbd;
    CColor unknown_0x5378d17b;
    CColor unknown_0x0ebf3cbc;
    CColor unknown_0xb897117a;
    CColor unknown_0x3e670f6a;
    CColor unknown_0x884f22ac;
    float unknown_0x7bdb0edf;
    float unknown_0x12221909;
    float unknown_0x38dbbc09;
    float unknown_0x30610062;
    float unknown_0xb6acea88;
    float unknown_0x73de4110;
    float map_screen_zoom_speed;
    float map_screen_circle_speed;
    float map_screen_move_speed;
    CColor unknown_0xab82e268;
    CColor unknown_0x1daacfae;
    CColor unknown_0xdad161a1;
    CColor unknown_0x6cf94c67;
    float unknown_0x47967404;
    float unknown_0x0ece1950;
    float unknown_0x9ac1bdde;
    float unknown_0x97a19386;
    float unknown_0xcb9e3a54;
    float unknown_0x2511a49b;
    float unknown_0x16c9f38e;
    float unknown_0xbc7e2e4d;
    float unknown_0x15564d32;
    float unknown_0xf5479260;
    float unknown_0x271b644e;
    float unknown_0x52dc08c1;
    float unknown_0x9980db64;
    float unknown_0x23f59057;
    float unknown_0xad3d5a3f;
    float unknown_0x3315d22b;
    float unknown_0x9e4007b6;
    float unknown_0x7a8d3d46;
    bool unknown_0x2b97d64c;
    float unknown_0xbdc57ce0;
    float unknown_0x7d59c854;
    float unknown_0x3c4ef7d2;
    float unknown_0x2b483e9f;
    float unknown_0x706f52fe;
    float unknown_0x62f9ebf6;
    float unknown_0xa9a53853;
    float unknown_0x722b1bc0;
    CColor player_model_color;
    CColor unknown_0x5a87c156;
    CColor player_surface_color;
    CColor player_outline_color;
    CColor text_color;
    CColor text_outline_color;
    CColor unknown_0x1a4b8068;
    CColor frame_color;
    CColor title_color;
    CColor legend_background_color;
    CColor legend_gradient_color;
};

void LoadTypedefSLdrTweakAutoMapper_Base(SLdrTweakAutoMapper_Base&, CInputStream&);

#endif // _SLDRTWEAKAUTOMAPPER_BASE
