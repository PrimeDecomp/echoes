#ifndef _SLDRGRAPPLEPARAMETERS
#define _SLDRGRAPPLEPARAMETERS

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrGrappleParameters {
    SLdrGrappleParameters();
    ~SLdrGrappleParameters();

    float grapple_length;
    float grapple_attach_length;
    float grapple_spring_constant;
    float grapple_spring_length;
    float unknown_0x987f68c7;
    float swing_force;
    float swing_max_force;
    float swing_arc_angle;
    float swing_turn_angle;
    float swing_camera_pitch;
    float unknown_0xf9e45827;
    bool unknown_0x11b6a17a;
};

void LoadTypedefSLdrGrappleParameters(SLdrGrappleParameters&, CInputStream&);

#endif // _SLDRGRAPPLEPARAMETERS
