#include "MetroidPrime/ScriptObjects/CScriptSpindleCamera.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CScriptCameraSpline.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrSplineType.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
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
    SLdrSplineType targetType, const CMayaSpline& targetControlSpline, bool targetLoops,
    SLdrSplineType playerType, bool playerLoops)
: CActor(uid, name, info, 0, xf, CModelData::CModelDataNull(), CMaterialList(kMT_NoStepLogic),
         CActorParameters::None(), kInvalidUniqueId)
, mParameters(flags, angularSpeed, linearSpeed, motionRadius, radialOffset, desiredAngularOffset,
              minAngularOffset, maxAngularOffset, lookAtAngularOffset, lookAtZOffset, zOffset,
              angularConstraint, angularDampening, desiredAngularSpeed, deactivateRadius,
              constraintFlipAngle, fov)
, mTargetSpline(targetLoops, 1.f, static_cast< CMotionSpline::ESplineType >(targetType.type))
, mTargetControlSpline(targetControlSpline)
, mPlayerSpline(playerLoops, 1.f, static_cast< CMotionSpline::ESplineType >(playerType.type))
, mOrigXf(xf) {}

CScriptSpindleCamera::~CScriptSpindleCamera() {}

void CScriptSpindleCamera::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CActor::AcceptScriptMsg(mgr, msg);
  if (GetActive() && message == kSM_XALD) {
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
  }
}

CEntity* LoadSpindleCamera(CStateManager& mgr, CInputStream& in, CEntityInfo& info) {}
