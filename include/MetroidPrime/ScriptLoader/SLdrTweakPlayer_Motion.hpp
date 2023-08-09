#ifndef _SLDRTWEAKPLAYER_MOTION
#define _SLDRTWEAKPLAYER_MOTION

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakPlayer_Motion {
    SLdrTweakPlayer_Motion();
    ~SLdrTweakPlayer_Motion();

    float forward_accel_normal;
    float forward_accel_air;
    float forward_accel_ice;
    float forward_accel_organic;
    float forward_accel_water;
    float forward_accel_lava;
    float forward_accel_phazon;
    float forward_accel_shrubbery;
    float rotational_accel_normal;
    float rotational_accel_air;
    float rotational_accel_ice;
    float rotational_accel_organic;
    float rotational_accel_water;
    float rotational_accel_lava;
    float rotational_accel_phazon;
    float rotational_accel_shrubbery;
    float movement_friction_normal;
    float movement_friction_air;
    float movement_friction_ice;
    float movement_friction_organic;
    float movement_friction_water;
    float movement_friction_lava;
    float movement_friction_phazon;
    float movement_friction_shrubbery;
    float rotation_friction_normal;
    float rotation_friction_air;
    float rotation_friction_ice;
    float rotation_friction_organic;
    float rotation_friction_water;
    float rotation_friction_lava;
    float rotation_friction_phazon;
    float rotation_friction_shrubbery;
    float rotation_max_speed_normal;
    float rotation_max_speed_air;
    float rotation_max_speed_ice;
    float rotation_max_speed_organic;
    float rotation_max_speed_water;
    float rotation_max_speed_lava;
    float rotation_max_speed_phazon;
    float rotation_max_speed_shrubbery;
    float forward_max_speed_normal;
    float forward_max_speed_air;
    float forward_max_speed_ice;
    float forward_max_speed_organic;
    float forward_max_speed_water;
    float forward_max_speed_lava;
    float forward_max_speed_phazon;
    float forward_max_speed_shrubbery;
    float gravitational_accel;
    float fluid_gravitational_accel;
    float vertical_jump_accel;
    float horizontal_jump_accel;
    float vertical_double_jump_accel;
    float horizontal_double_jump_accel;
    float water_jump_factor;
    float water_ball_jump_factor;
    float lava_jump_factor;
    float lava_ball_jump_factor;
    float phazon_jump_factor;
    float phazon_ball_jump_factor;
    float allowed_jump_time;
    float allowed_double_jump_time;
    float min_double_jump_window;
    float max_double_jump_window;
    float unknown;
    float min_jump_time;
    float min_double_jump_time;
    float ledge_fall_time;
    float double_jump_impulse;
    float backwards_force_multiplier;
    float bomb_jump_height;
    float bomb_jump_radius;
    float gravity_boost_time;
    float gravity_boost_force;
    float gravity_boost_cancel_dampening;
    bool gravity_boost_multiple_allowed;
};

void LoadTypedefSLdrTweakPlayer_Motion(SLdrTweakPlayer_Motion&, CInputStream&);

#endif // _SLDRTWEAKPLAYER_MOTION
