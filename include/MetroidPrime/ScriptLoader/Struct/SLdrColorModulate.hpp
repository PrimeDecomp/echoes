#ifndef _SLDRCOLORMODULATE
#define _SLDRCOLORMODULATE

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrColorModulate {
    SLdrColorModulate();
    ~SLdrColorModulate();

    SLdrEditorProperties editor_properties;
    CColor color_a;
    CColor color_b;
    int blend_mode;
    float time_a2_b;
    float time_b2_a;
    bool do_reverse;
    bool reset_target_when_done;
    bool depth_compare;
    bool depth_update;
    bool depth_backwards;
    bool auto_start;
    bool update_time;
    bool loop_forever;
    bool external_time;
    bool copy_model_color_to_color_a;
    spline control_spline;
};

void LoadTypedefSLdrColorModulate(SLdrColorModulate&, CInputStream&);

#endif // _SLDRCOLORMODULATE
