#ifndef _SLDRGRAPPLEPARAMETERS
#define _SLDRGRAPPLEPARAMETERS

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrGrappleParameters {
    SLdrGrappleParameters();
    ~SLdrGrappleParameters();

    float grappleLength;
    float grappleAttachLength;
    float grappleSpringConstant;
    float grappleSpringLength;
    float unknown_0x987f68c7;
    float swingForce;
    float swingMaxForce;
    float swingArcAngle;
    float swingTurnAngle;
    float swingCameraPitch;
    float unknown_0xf9e45827;
    bool unknown_0x11b6a17a;
};

void LoadTypedefSLdrGrappleParameters(SLdrGrappleParameters&, CInputStream&);

#endif // _SLDRGRAPPLEPARAMETERS
