#ifndef _CSPINDLECAMERA
#define _CSPINDLECAMERA

#include "Kyoto/Math/CMayaSpline.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "rstl/reserved_vector.hpp"

enum ESpindleInput {
  kSI_HintToBallDist,
  kSI_HintToBallVOff,
  kSI_HintBallAngle,
  kSI_HintBallRightAngle,
  kSI_HintBallLeftAngle,
  kSI_HintDeltaDist,
  kSI_HintDeltaVOff,
  kSI_SplineProgress,
};

class CSpindleCameraInterpolant {
public:
  CSpindleCameraInterpolant(ESpindleInput input, const CMayaSpline& spline);
  float InterpolateValue(float input);
  ESpindleInput GetInput() const { return mInput; }

private:
  ESpindleInput mInput;
  CMayaSpline mSpline;
};
CHECK_SIZEOF(CSpindleCameraInterpolant, 0x48)

// Guessed name: runtime settings embedded in the separate spindle script actor.
class CSpindleCameraParameters {
public:
  CSpindleCameraParameters(uint flags, const CSpindleCameraInterpolant& angularSpeed,
                           const CSpindleCameraInterpolant& linearSpeed,
                           const CSpindleCameraInterpolant& motionRadius,
                           const CSpindleCameraInterpolant& radialOffset,
                           const CSpindleCameraInterpolant& desiredAngularOffset,
                           const CSpindleCameraInterpolant& minAngularOffset,
                           const CSpindleCameraInterpolant& maxAngularOffset,
                           const CSpindleCameraInterpolant& lookAtAngularOffset,
                           const CSpindleCameraInterpolant& lookAtZOffset,
                           const CSpindleCameraInterpolant& zOffset,
                           const CSpindleCameraInterpolant& angularConstraint,
                           const CSpindleCameraInterpolant& angularDampening,
                           const CSpindleCameraInterpolant& desiredAngularSpeed,
                           const CSpindleCameraInterpolant& deactivateRadius,
                           const CSpindleCameraInterpolant& constraintFlipAngle,
                           const CSpindleCameraInterpolant& fov);
  virtual ~CSpindleCameraParameters();

private:
  uint mFlags;
  CSpindleCameraInterpolant mAngularSpeed;
  CSpindleCameraInterpolant mLinearSpeed;
  CSpindleCameraInterpolant mMotionRadius;
  CSpindleCameraInterpolant mRadialOffset;
  CSpindleCameraInterpolant mDesiredAngularOffset;
  CSpindleCameraInterpolant mMinAngularOffset;
  CSpindleCameraInterpolant mMaxAngularOffset;
  CSpindleCameraInterpolant mLookAtAngularOffset;
  CSpindleCameraInterpolant mLookAtZOffset;
  CSpindleCameraInterpolant mZOffset;
  CSpindleCameraInterpolant mAngularConstraint;
  CSpindleCameraInterpolant mAngularDampening;
  CSpindleCameraInterpolant mDesiredAngularSpeed;
  CSpindleCameraInterpolant mDeactivateRadius;
  CSpindleCameraInterpolant mConstraintFlipAngle;
  CSpindleCameraInterpolant mFov;
};
CHECK_SIZEOF(CSpindleCameraParameters, 0x488)

// Guessed name: Echoes separates Prime's CScriptSpindleCamera behavior from its script actor.
class CSpindleCamera : public CGameCamera {
public:
  CSpindleCamera(TUniqueId uid, const CTransform4f& xf, bool active, int index, int controllerIdx);

  // CEntity
  ~CSpindleCamera() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;

  // CActor
  void Render(const CStateManager& mgr) const override;
  CVector3f GetScanObjectIndicatorPosition(const CStateManager& mgr) const override;

  // CGameCamera
  void ProcessInput(const CFinalInput& input, CStateManager& mgr) override;
  void Reset(const CTransform4f& xf, CStateManager& mgr) override;

private:
  float CalculateTargetSplineDistance(CStateManager& mgr) const;
  float GetInVar(const CSpindleCameraInterpolant& interpolant) const;
  float GetInterpolant(CSpindleCameraInterpolant& interpolant) const;

  TUniqueId mSpindleCameraId;
  rstl::reserved_vector< float, 8 > mInVars;
  float mMaxAzimuthInterpTimer;
  CVector3f mLookDir;
  float mTargetSplineDistance;
  float mPlayerSplineDistance;
  CVector3f mLookPosition;
  bool mOutsideClampedAzimuth : 1;
  bool mInResetThink : 1;
  bool mFixedPositionInitialized : 1;
};
CHECK_SIZEOF(CSpindleCamera, 0x250)

#endif // _CSPINDLECAMERA
