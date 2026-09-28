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

private:
  CScriptCameraSpline mSpline;
  CMotionSpline mPlayerSpline;
  CMayaSpline mSpeedControlSpline;
  float mDistance;
  float mSpeed;
  float mDampenDistance;
  float mAngularSpeed;
  int mInitialPosition;
  uint mFlags;
  CMayaSpline mPerpendicularDistanceControlSpline;
  CMayaSpline mPerpendicularInterpControlSpline;
  TUniqueId mTimeKeyframeId;
};
CHECK_SIZEOF(CScriptPathCamera, 0x308)

#endif // _CSCRIPTPATHCAMERA
