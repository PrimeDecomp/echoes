#ifndef _CINTERPOLATIONCAMERA
#define _CINTERPOLATIONCAMERA

#include "Kyoto/Math/CMotionSpline.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"

class CInterpolationCamera : public CGameCamera {
public:
  // Guessed names; values and behavior are recovered from the interpolation paths.
  enum EPositionMode {
    kPM_Direct = 0,
    kPM_Spline = 1,
  };

  enum ERotationMode {
    kRM_Linear = 0,
    kRM_LinearSlerp = 1,
    kRM_Sine = 2,
    kRM_SinusoidalEase = 3,
    kRM_SinusoidalEaseFull = 4,
    kRM_QuadraticEase = 5,
    kRM_QuadraticEaseFull = 6,
  };

  enum EEndReason {
    kER_Completed = 0,
    kER_Obstruction = 2,
    kER_TargetUnavailable = 3,
  };

  CInterpolationCamera(TUniqueId uid, const CTransform4f& xf, int index, int controllerIdx);

  // CEntity
  ~CInterpolationCamera() override;
  void Think(float dt, CStateManager& mgr) override;

  // CActor
  void Render(const CStateManager& mgr) const override;
  CVector3f GetScanObjectIndicatorPosition(const CStateManager& mgr) const override;

  // CGameCamera
  void ProcessInput(const CFinalInput& input, CStateManager& mgr) override;
  void Reset(const CTransform4f& xf, CStateManager& mgr) override;

  void SetInterpolation(const CTransform4f& xf, TUniqueId from, TUniqueId to,
                        bool interpolateRotation, EPositionMode positionMode,
                        ERotationMode rotationMode, CStateManager& mgr, bool flag, float duration,
                        float fov);
  void EndInterpolation(EEndReason reason, CStateManager& mgr);
  void SetSpline(const CMotionSpline& spline); // Guessed name

private:
  // Guessed names for the Echoes-specific interpolation paths.
  CTransform4f CalculateOrientation(float dt, const CVector3f& position, bool& done,
                                    const CStateManager& mgr);
  bool InterpolatePosition(float dt, CTransform4f& xf, const CVector3f& target,
                           const CStateManager& mgr);
  bool InterpolateSpline(float dt, CTransform4f& xf, const CVector3f& target,
                         const CStateManager& mgr);

  TUniqueId mTargetId;
  float mTime;
  float mDuration;
  CTransform4f mStartTransform;
  CVector3f mLookPosition;
  float mInitialDistance;
  float mInitialAngle;
  float mAngularSpeed;
  EPositionMode mPositionMode;
  ERotationMode mRotationMode;
  CMotionSpline mSpline;
  float x2a0_; // Reset to zero; no consumer established.
  bool mInterpolateRotation : 1;
  bool x2a4_1_ : 1; // Copied from setup; purpose unresolved.
  bool mRotationFinished : 1;
};
CHECK_SIZEOF(CInterpolationCamera, 0x2a8)

#endif // _CINTERPOLATIONCAMERA
