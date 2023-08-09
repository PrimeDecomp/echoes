#ifndef _SLDRFOGOVERLAY
#define _SLDRFOGOVERLAY

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrFogOverlay {
    SLdrFogOverlay();
    ~SLdrFogOverlay();

    SLdrEditorProperties editor_properties;
    float full_alpha;
    float fade_down_time;
    float fade_up_time;
    bool start_faded_out;
    CColor color;
    float ambient_radius_x;
    float ambient_radius_y;
    float ambient_speed;
    float ambient_speed_target;
    float unknown_0x6a111b96;
    float unknown_0xff226ea3;
    CVector3f unknown_0x2190ab0a;
    float unknown_0x9f19f0af;
    float unknown_0x90c10fe7;
    float unknown_0xd8daff1d;
};

void LoadTypedefSLdrFogOverlay(SLdrFogOverlay&, CInputStream&);

#endif // _SLDRFOGOVERLAY
