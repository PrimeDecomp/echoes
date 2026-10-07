#include "MetroidPrime/ScriptObjects/CScriptSpindleCamera.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CScriptCameraSpline.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSpindleCamera.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrSplineType.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"

CScriptSpindleCamera::CScriptSpindleCamera(
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
    bool targetLoops, const CMotionSpline::ESplineType& playerType, bool playerLoops)
: CActor(uid, name, info, 0, xf, CModelData::CModelDataNull(), CMaterialList(kMT_NoStepLogic),
         CActorParameters::None(), kInvalidUniqueId)
, mParameters(flags, angularSpeed, linearSpeed, motionRadius, radialOffset, desiredAngularOffset,
              minAngularOffset, maxAngularOffset, lookAtAngularOffset, lookAtZOffset, zOffset,
              angularConstraint, angularDampening, desiredAngularSpeed, deactivateRadius,
              constraintFlipAngle, fov)
, mTargetSpline(targetLoops, 1.f, targetType)
, mTargetControlSpline(targetControlSpline)
, mPlayerSpline(playerLoops, 1.f, playerType)
, mOrigXf(xf) {}

CScriptSpindleCamera::~CScriptSpindleCamera() {}

void CScriptSpindleCamera::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CActor::AcceptScriptMsg(mgr, msg);
  if (GetActive()) {
    switch (message) {
    case kSM_AreaLoaded: {
      rstl::vector< CVector3f > targetPoints;
      rstl::vector< CQuaternion > targetOrientations;
      const TUniqueId target = FindConnectedObject(mgr, kSS_CameraTarget, kSM_Attach);
      if (TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(target))) {
        ScriptCameraSpline::CollectWaypoints(*this, kSS_CameraTarget, kSM_Attach, targetPoints,
                                             targetOrientations, mgr);
        mTargetSpline.Initialise(targetPoints);
      }

      rstl::vector< CVector3f > playerPoints;
      rstl::vector< CQuaternion > playerOrientations;
      const TUniqueId player = FindConnectedObject(mgr, kSS_CameraPlayer, kSM_Attach);
      if (TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(player))) {
        ScriptCameraSpline::CollectWaypoints(*this, kSS_CameraPlayer, kSM_Attach, playerPoints,
                                             playerOrientations, mgr);
        mPlayerSpline.Initialise(playerPoints);
      }
      break;
    }
    default:
      break;
    }
  }
}

CEntity* LoadSpindleCamera(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrSpindleCamera sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrSpindleCamera.inc"

  CSpindleCameraInterpolant angularSpeed(
      static_cast< ESpindleInput >(sldrThis.angularSpeed.interpolantType),
      sldrThis.angularSpeed.interpolantSpline);
  CSpindleCameraInterpolant linearSpeed(
      static_cast< ESpindleInput >(sldrThis.linearSpeed.interpolantType),
      sldrThis.linearSpeed.interpolantSpline);
  CSpindleCameraInterpolant motionRadius(
      static_cast< ESpindleInput >(sldrThis.motionRadius.interpolantType),
      sldrThis.motionRadius.interpolantSpline);
  CSpindleCameraInterpolant radialOffset(
      static_cast< ESpindleInput >(sldrThis.radialOffset.interpolantType),
      sldrThis.radialOffset.interpolantSpline);
  CSpindleCameraInterpolant desiredAngularOffset(
      static_cast< ESpindleInput >(sldrThis.desiredAngularOffset.interpolantType),
      sldrThis.desiredAngularOffset.interpolantSpline);
  CSpindleCameraInterpolant minAngularOffset(
      static_cast< ESpindleInput >(sldrThis.minAngularOffset.interpolantType),
      sldrThis.minAngularOffset.interpolantSpline);
  CSpindleCameraInterpolant maxAngularOffset(
      static_cast< ESpindleInput >(sldrThis.maxAngularOffset.interpolantType),
      sldrThis.maxAngularOffset.interpolantSpline);
  CSpindleCameraInterpolant lookAtAngularOffset(
      static_cast< ESpindleInput >(sldrThis.lookAtAngularOffset.interpolantType),
      sldrThis.lookAtAngularOffset.interpolantSpline);
  CSpindleCameraInterpolant lookAtZOffset(
      static_cast< ESpindleInput >(sldrThis.lookAtZOffset.interpolantType),
      sldrThis.lookAtZOffset.interpolantSpline);
  CSpindleCameraInterpolant zOffset(static_cast< ESpindleInput >(sldrThis.zOffset.interpolantType),
                                    sldrThis.zOffset.interpolantSpline);
  CSpindleCameraInterpolant angularConstraint(
      static_cast< ESpindleInput >(sldrThis.angularConstraint.interpolantType),
      sldrThis.angularConstraint.interpolantSpline);
  CSpindleCameraInterpolant angularDampening(
      static_cast< ESpindleInput >(sldrThis.angularDampening.interpolantType),
      sldrThis.angularDampening.interpolantSpline);
  CSpindleCameraInterpolant desiredAngularSpeed(
      static_cast< ESpindleInput >(sldrThis.desiredAngularSpeed.interpolantType),
      sldrThis.desiredAngularSpeed.interpolantSpline);
  CSpindleCameraInterpolant deactivateRadius(
      static_cast< ESpindleInput >(sldrThis.deactivateRadius.interpolantType),
      sldrThis.deactivateRadius.interpolantSpline);
  CSpindleCameraInterpolant constraintFlipAngle(
      static_cast< ESpindleInput >(sldrThis.constraintFlipAngle.interpolantType),
      sldrThis.constraintFlipAngle.interpolantSpline);
  CSpindleCameraInterpolant fov(static_cast< ESpindleInput >(sldrThis.fOV.interpolantType),
                                sldrThis.fOV.interpolantSpline);

  return rs_new CScriptSpindleCamera(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      sldrThis.flagsSpindleCamera, angularSpeed, linearSpeed, motionRadius, radialOffset,
      desiredAngularOffset, minAngularOffset, maxAngularOffset, lookAtAngularOffset, lookAtZOffset,
      zOffset, angularConstraint, angularDampening, desiredAngularSpeed, deactivateRadius,
      constraintFlipAngle, fov,
      static_cast< CMotionSpline::ESplineType >(sldrThis.targetSplineType.type),
      sldrThis.targetControlSpline, sldrThis.targetSplineLoops,
      static_cast< CMotionSpline::ESplineType >(sldrThis.playerSplineType.type),
      sldrThis.playerSplineLoops);
}
