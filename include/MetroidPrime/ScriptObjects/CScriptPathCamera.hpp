#ifndef _CSCRIPTPATHCAMERA
#define _CSCRIPTPATHCAMERA

#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/Cameras/CScriptCameraSpline.hpp"

// Echoes separates the script settings from the runtime path camera.
class CScriptPathCamera : public CEntity {
public:
  CScriptPathCamera(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                    float distance, float speed, float angularSpeed, float dampenDistance,
                    uint flags, uint splineFlags, int initialPosition,
                    CMotionSpline::ESplineType positionType, CMotionSpline::ESplineType lookAtType,
                    const CMayaSpline& positionTimeSpline, const CMayaSpline& lookAtTimeSpline,
                    const CMayaSpline& fovSpline, const CMayaSpline& speedControlSpline,
                    CMotionSpline::ESplineType playerType, bool playerLoops,
                    const CMayaSpline& perpendicularDistanceSpline,
                    const CMayaSpline& perpendicularInterpSpline);

  // CEntity
  ~CScriptPathCamera() override;
  CEntity* TypesMatch(int typeId) const override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // Guessed names.
  void TranslateSplines(const CVector3f& offset);
  void RotateSplines(const CQuaternion& rotation, const CVector3f& origin);

  // Evaluation updates spline caches, not the scripted settings.
  CScriptCameraSpline& GetSpline() const { return mSpline; }
  const CMotionSpline& GetPlayerSpline() const { return mPlayerSpline; }
  CMayaSpline& GetSpeedControlSpline() const { return mSpeedControlSpline; }
  CMayaSpline& GetPerpendicularDistanceControlSpline() const {
    return mPerpendicularDistanceControlSpline;
  }
  float GetDistance() const { return mDistance; }
  float GetSpeed() const { return mSpeed; }
  float GetDampenDistance() const { return mDampenDistance; }
  uint GetFlags() const { return mFlags; }

private:
  mutable CScriptCameraSpline mSpline;
  CMotionSpline mPlayerSpline;
  mutable CMayaSpline mSpeedControlSpline;
  float mDistance;
  float mSpeed;
  float mDampenDistance;
  float mAngularSpeed;
  int mInitialPosition;
  uint mFlags;
  mutable CMayaSpline mPerpendicularDistanceControlSpline;
  CMayaSpline mPerpendicularInterpControlSpline;
  TUniqueId mTimeKeyframeId;
};
CHECK_SIZEOF(CScriptPathCamera, 0x308)

#endif // _CSCRIPTPATHCAMERA
