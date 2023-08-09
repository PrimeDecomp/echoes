#ifndef _SLDRTWEAKBALL_CAMERA
#define _SLDRTWEAKBALL_CAMERA

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakBall_Camera {
    SLdrTweakBall_Camera();
    ~SLdrTweakBall_Camera();

    float ball_camera_angle_per_second;
    CVector3f ball_camera_offset;
    float ball_camera_min_speed_distance;
    float ball_camera_max_speed_distance;
    float ball_camera_backwards_distance;
    float ball_camera_elevation;
    float ball_camera_spring_constant;
    float ball_camera_spring_max;
    float ball_camera_spring_tardis;
    float ball_camera_centroid_spring_constant;
    float ball_camera_centroid_spring_max;
    float ball_camera_centroid_spring_tardis;
    float ball_camera_centroid_distance_spring_constant;
    float ball_camera_centroid_distance_spring_max;
    float ball_camera_centroid_distance_spring_tardis;
    float ball_camera_look_at_spring_constant;
    float ball_camera_look_at_spring_max;
    float ball_camera_look_at_spring_tardis;
    float ball_camera_transition_time;
    float ball_camera_free_look_speed;
    float ball_camera_free_look_zoom_speed;
    float ball_camera_free_look_min_distance;
    float ball_camera_free_look_max_distance;
    float ball_camera_free_look_max_vert_angle;
    float unknown_0x144db504;
    float unknown_0xee5bea64;
    float ball_camera_chase_distance;
    float ball_camera_chase_elevation;
    float ball_camera_chase_yaw_speed;
    float ball_camera_chase_dampen_angle;
    float ball_camera_chase_angle_per_second;
    CVector3f ball_camera_chase_look_at_offset;
    float ball_camera_chase_spring_constant;
    float ball_camera_chase_spring_max;
    float ball_camera_chase_spring_tardis;
    float ball_camera_boost_distance;
    float ball_camera_boost_elevation;
    float ball_camera_boost_yaw_speed;
    float ball_camera_boost_dampen_angle;
    float ball_camera_boost_angle_per_second;
    CVector3f ball_camera_boost_look_at_offset;
    float ball_camera_boost_spring_constant;
    float ball_camera_boost_spring_max;
    float ball_camera_boost_spring_tardis;
    float ball_camera_control_distance;
    float ball_camera_look_at_min_height;
    float unknown_0x50f77df0;
};

void LoadTypedefSLdrTweakBall_Camera(SLdrTweakBall_Camera&, CInputStream&);

#endif // _SLDRTWEAKBALL_CAMERA
