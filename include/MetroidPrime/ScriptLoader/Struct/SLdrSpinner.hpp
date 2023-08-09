#ifndef _SLDRSPINNER
#define _SLDRSPINNER

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrSpinner {
    SLdrSpinner();
    ~SLdrSpinner();

    SLdrEditorProperties editor_properties;
    float forward_speed;
    float backward_speed;
    float unknown_0x449dd059;
    float unknown_0xfc849759;
    bool shot_spinner;
    bool allow_wrap;
    bool no_backward;
    bool spline_control;
    int loop_sound;
    int start_sound;
    int stop_sound;
};

void LoadTypedefSLdrSpinner(SLdrSpinner&, CInputStream&);

#endif // _SLDRSPINNER
