#ifndef _CSCRIPTSPINDLECAMERA
#define _CSCRIPTSPINDLECAMERA

#include "Kyoto/Math/CMotionSpline.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/Cameras/CSpindleCamera.hpp"

class CScriptSpindleCamera : public CActor {
public:
  CScriptSpindleCamera(
      TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
      uint flags, const CSpindleCameraInterpolant& angularSpeed,
      const CSpindleCameraInterpolant& linearSpeed, const CSpindleCameraInterpolant& motionRadius,
      const CSpindleCameraInterpolant& radialOffset,
      const CSpindleCameraInterpolant& desiredAngularOffset,
      const CSpindleCameraInterpolant& minAngularOffset,
      const CSpindleCameraInterpolant& maxAngularOffset,
      const CSpindleCameraInterpolant& lookAtAngularOffset,
      const CSpindleCameraInterpolant& lookAtZOffset, const CSpindleCameraInterpolant& zOffset,
      const CSpindleCameraInterpolant& angularConstraint,
      const CSpindleCameraInterpolant& angularDampening,
      const CSpindleCameraInterpolant& desiredAngularSpeed,
      const CSpindleCameraInterpolant& deactivateRadius,
      const CSpindleCameraInterpolant& constraintFlipAngle, const CSpindleCameraInterpolant& fov,
      const CMotionSpline::ESplineType& targetType, const CMayaSpline& targetControlSpline,
      bool targetLoops, const CMotionSpline::ESplineType& playerType, bool playerLoops);

  // CEntity
  ~CScriptSpindleCamera() override;
  CEntity* TypesMatch(int typeId) const override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  const CSpindleCameraParameters& GetParameters() const { return mParameters; }
  const CMotionSpline& GetTargetSpline() const { return mTargetSpline; }
  const CMayaSpline& GetTargetControlSpline() const { return mTargetControlSpline; }
  const CMotionSpline& GetPlayerSpline() const { return mPlayerSpline; }
  CTransform4f GetOrigXf() const { return mOrigXf; }

private:
  CSpindleCameraParameters mParameters;
  CMotionSpline mTargetSpline;
  CMayaSpline mTargetControlSpline;
  CMotionSpline mPlayerSpline;
  CTransform4f mOrigXf;
};
CHECK_SIZEOF(CScriptSpindleCamera, 0x6e0)

#endif // _CSCRIPTSPINDLECAMERA
