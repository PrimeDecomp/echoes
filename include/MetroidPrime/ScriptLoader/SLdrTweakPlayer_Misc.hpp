#ifndef _SLDRTWEAKPLAYER_MISC
#define _SLDRTWEAKPLAYER_MISC

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakPlayer_Misc {
    SLdrTweakPlayer_Misc();
    ~SLdrTweakPlayer_Misc();

    float eye_offset;
    float normal_turn_factor;
    float free_look_turn_factor;
    float free_look_max_x;
    float free_look_max_z;
    float free_look_speed;
    float free_look_snap_speed;
    float free_look_fade_angle;
    float free_look_min_angle;
    float free_look_centered_time;
    float free_look_dampen_factor;
    bool null_analog_scales;
    float unknown;
    float left_analog_max;
    float right_analog_max;
};

void LoadTypedefSLdrTweakPlayer_Misc(SLdrTweakPlayer_Misc&, CInputStream&);

#endif // _SLDRTWEAKPLAYER_MISC
