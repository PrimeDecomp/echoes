#ifndef _SLDRTWEAKBALL_MOVEMENT
#define _SLDRTWEAKBALL_MOVEMENT

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakBall_Movement {
    SLdrTweakBall_Movement();
    ~SLdrTweakBall_Movement();

    float forward_accel_normal;
    float forward_accel_air;
    float forward_accel_ice;
    float forward_accel_organic;
    float forward_accel_water;
    float forward_accel_lava;
    float forward_accel_phazon;
    float forward_accel_shrubbery;
    float movement_friction_normal;
    float movement_friction_air;
    float movement_friction_ice;
    float movement_friction_organic;
    float movement_friction_water;
    float movement_friction_lava;
    float movement_friction_phazon;
    float movement_friction_shrubbery;
    float forward_max_speed_normal;
    float forward_max_speed_air;
    float forward_max_speed_ice;
    float forward_max_speed_organic;
    float forward_max_speed_water;
    float forward_max_speed_lava;
    float forward_max_speed_phazon;
    float forward_max_speed_shrubbery;
    float unknown_0x85ee51ed;
    float unknown_0x6d7811f5;
    float ball_up_gravity;
    float ball_down_gravity;
    float ball_forward_braking_accel_normal;
    float ball_forward_braking_accel_air;
    float ball_forward_braking_accel_ice;
    float ball_forward_braking_accel_organic;
    float ball_forward_braking_accel_water;
    float ball_forward_braking_accel_lava;
    float ball_forward_braking_accel_phazon;
    float ball_forward_braking_accel_shrubbery;
    float ball_gravity;
    float ball_water_gravity;
    float unknown_0xeeb74968;
    float ball_boost_height;
    float unknown_0xe2bb0298;
    float minimum_alignment_speed;
    float tireness;
    float max_lean_angle;
    float tire_to_marble_threshold_speed;
    float marble_to_tire_threshold_speed;
    float force_to_lean_gain;
    float lean_tracking_gain;
};

void LoadTypedefSLdrTweakBall_Movement(SLdrTweakBall_Movement&, CInputStream&);

#endif // _SLDRTWEAKBALL_MOVEMENT
