#ifndef _CGRAPPLEPARAMETERS
#define _CGRAPPLEPARAMETERS

#include "types.h"

// Original Wii constructor parameter type. Fields follow validated SLdrGrappleParameters data.
class CGrappleParameters {
public:
  CGrappleParameters(float grappleLength, float grappleAttachLength, float grappleSpringConstant,
                     float grappleSpringLength, float grappleSpringTardis, float swingForce,
                     float swingMaxForce, float swingArcAngle, float swingTurnAngle,
                     float swingCameraPitch, float swingCameraMaxPitch, bool constrainToAxis)
  : mGrappleLength(grappleLength)
  , mGrappleAttachLength(grappleAttachLength)
  , mGrappleSpringConstant(grappleSpringConstant)
  , mGrappleSpringLength(grappleSpringLength)
  , mGrappleSpringTardis(grappleSpringTardis)
  , mSwingForce(swingForce)
  , mSwingMaxForce(swingMaxForce)
  , mSwingArcAngle(swingArcAngle)
  , mSwingTurnAngle(swingTurnAngle)
  , mSwingCameraPitch(swingCameraPitch)
  , mSwingCameraMaxPitch(swingCameraMaxPitch)
  , mConstrainToAxis(constrainToAxis) {}

  bool GetConstrainToAxis() const { return mConstrainToAxis; }

private:
  float mGrappleLength;
  float mGrappleAttachLength;
  float mGrappleSpringConstant;
  float mGrappleSpringLength;
  float mGrappleSpringTardis;
  float mSwingForce;
  float mSwingMaxForce;
  float mSwingArcAngle;
  float mSwingTurnAngle;
  float mSwingCameraPitch;
  float mSwingCameraMaxPitch;
  bool mConstrainToAxis : 1;
};
CHECK_SIZEOF(CGrappleParameters, 0x30)

#endif // _CGRAPPLEPARAMETERS
