#ifndef _SLDRTWEAKPLAYER_AIMSTUFF
#define _SLDRTWEAKPLAYER_AIMSTUFF

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakPlayer_AimStuff {
    SLdrTweakPlayer_AimStuff();
    ~SLdrTweakPlayer_AimStuff();

    float aim_min_time;
    float aim_max_time;
    float aim_max_distance;
    float aim_max_angle_left;
    float aim_max_angle_right;
    float aim_max_angle_up;
    float aim_max_angle_down;
    float aim_angle_per_second;
    float aim_threshold_distance;
    float aim_turn_angle_per_second;
    float unknown;
    float aim_box_width;
    float aim_box_height;
    float aim_target_timer;
    float aim_assist_horizontal_angle;
    float aim_assist_vertical_angle;
};

void LoadTypedefSLdrTweakPlayer_AimStuff(SLdrTweakPlayer_AimStuff&, CInputStream&);

#endif // _SLDRTWEAKPLAYER_AIMSTUFF
