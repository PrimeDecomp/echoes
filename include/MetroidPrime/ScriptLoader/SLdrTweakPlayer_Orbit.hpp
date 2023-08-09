#ifndef _SLDRTWEAKPLAYER_ORBIT
#define _SLDRTWEAKPLAYER_ORBIT

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakPlayer_Orbit {
    SLdrTweakPlayer_Orbit();
    ~SLdrTweakPlayer_Orbit();

    float orbit_close_min_distance;
    float orbit_close_normal_distance;
    float orbit_close_max_distance;
    float orbit_far_min_distance;
    float orbit_far_normal_distance;
    float orbit_far_max_distance;
    float orbit_carcass_min_distance;
    float orbit_carcass_normal_distance;
    float orbit_carcass_max_distance;
    float orbit_max_angular_change;
    float orbit_mode_timer;
    float orbit_camera_speed;
    float orbit_upper_angle;
    float orbit_lower_angle;
    float orbit_horiz_angle;
    float orbit_upper_camera_angle;
    float orbit_lower_camera_angle;
    float orbit_max_target_distance;
    float orbit_max_lock_distance;
    float unknown_0x55f7d145;
    float orbit_distance_threshold;
    int orbit_zone_width;
    int orbit_zone_height;
    int unknown_0x58ee9d03;
    int unknown_0xe052fa66;
    int unknown_0xc452b61e;
    int unknown_0x7ceed17b;
    int orbit_scan_zone_width;
    int orbit_scan_zone_height;
    int unknown_0xec529a5e;
    int unknown_0x54eefd3b;
    int unknown_0x73ebdce2;
    int unknown_0xcb57bb87;
    float orbit_box_width;
    float orbit_box_height;
    float orbit_min_camera_pitch_distance;
    float orbit_max_camera_pitch_distance;
    float unknown_0x478c15f9;
    float orbit_z_range;
    float orbit_selection_close_angle;
    float orbit_selection_max_angle;
    float unknown_0x90b71b2e;
    float orbit_prevention_time;
    bool orbit_dash;
    bool orbit_dash_uses_tap;
    float orbit_dash_tap_time;
    float orbit_dash_stick_threshold;
    float orbit_dash_double_jump_impulse;
    float unknown_0x75a00cfb;
    float unknown_0xc4775e5f;
};

void LoadTypedefSLdrTweakPlayer_Orbit(SLdrTweakPlayer_Orbit&, CInputStream&);

#endif // _SLDRTWEAKPLAYER_ORBIT
