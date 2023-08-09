#ifndef _SLDRTWEAKPLAYER_GRAPPLE
#define _SLDRTWEAKPLAYER_GRAPPLE

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_GrappleBeam.hpp"

struct SLdrTweakPlayer_Grapple {
    SLdrTweakPlayer_Grapple();
    ~SLdrTweakPlayer_Grapple();

    float grapple_distance;
    float grapple_beam_length;
    float grapple_swing_time;
    float grapple_max_velocity;
    float grapple_camera_speed;
    float grapple_pull_close_distance;
    float grapple_pull_dampen_distance;
    float grapple_pull_velocity;
    float grapple_pull_camera_speed;
    float grapple_turn_rate;
    float grapple_jump_force;
    float grapple_release_time;
    int grapple_control_scheme;
    bool grapple_hold_orbit_button;
    bool grapple_turn_controls_reversed;
    SLdrTweakPlayer_GrappleBeam beam;
};

void LoadTypedefSLdrTweakPlayer_Grapple(SLdrTweakPlayer_Grapple&, CInputStream&);

#endif // _SLDRTWEAKPLAYER_GRAPPLE
