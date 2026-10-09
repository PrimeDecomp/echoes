#include "MetroidPrime/ScriptObjects/CScriptPathCamera.hpp"

#include "Kyoto/Math/CTransform4f.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPathCamera.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"

// Guessed name, supported by the time-keyframe loader and connected-object updates.
class CScriptTimeKeyframe;

CScriptCameraSpline::CScriptCameraSpline(float duration, uint flags,
                                         const CMayaSpline& positionTimeSpline,
                                         const CMayaSpline& lookAtTimeSpline,
                                         const CMayaSpline& fovSpline,
                                         const CMayaSpline& secondScalarSpline,
                                         CMotionSpline::ESplineType positionType,
                                         CMotionSpline::ESplineType lookAtType)
: CGameCameraSpline(duration, flags, positionTimeSpline, lookAtTimeSpline, fovSpline,
                    secondScalarSpline, positionType, lookAtType)
, mPositionId(kInvalidUniqueId)
, mTargetId(kInvalidUniqueId) {}

CScriptCameraSpline::~CScriptCameraSpline() {}

CVector3f CScriptCameraSpline::GetPositionByTime(float time, const CTransform4f& xf,
                                                 const CStateManager& mgr) {
  CVector3f position;
  if (GetPositionKnotCount() != 0) {
    position = CSpline::GetPositionByTime(time);
  } else if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mPositionId))) {
    position = actor->GetTranslation();
  } else {
    position = xf.GetTranslation();
  }
  return position;
}

CQuaternion CScriptCameraSpline::GetOrientationByTime(float time, const CTransform4f& xf,
                                                      const CStateManager& mgr) {
  const CVector3f position = xf.GetTranslation();
  CQuaternion orientation = CQuaternion::FromMatrix(xf);
  if (GetPositionKnotCount() == 0) {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mPositionId))) {
      orientation = CQuaternion::FromMatrix(actor->GetTransform());
    }
  }

  if (GetLookAtKnotCount() == 0) {
    if (const CActor* target = TCastToConstPtr< CActor >(mgr.GetObjectById(mTargetId))) {
      orientation =
          CQuaternion::FromMatrix(CTransform4f::LookAt(position, target->GetTranslation()));
    } else if (GetPositionKnotCount() != 0) {
      orientation = CSpline::GetOrientationByTime(time);
    }
  } else {
    const CVector3f target = CSpline::GetLookAtByTime(time);
    if ((target - position).IsMagnitudeSafe()) {
      orientation = CQuaternion::FromMatrix(CTransform4f::LookAt(position, target));
    } else {
      orientation = CQuaternion::FromMatrix(xf);
    }
  }
  return orientation;
}

CVector3f CScriptCameraSpline::GetPositionByLength(float distance, const CTransform4f& xf,
                                                   const CStateManager& mgr) {
  CVector3f position;
  if (GetPositionKnotCount() != 0) {
    position = CSpline::GetPositionByLength(distance);
  } else if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mPositionId))) {
    position = actor->GetTranslation();
  } else {
    position = xf.GetTranslation();
  }
  return position;
}

CQuaternion CScriptCameraSpline::GetOrientationByLength(float positionDistance,
                                                        float targetDistance,
                                                        const CTransform4f& xf,
                                                        const CStateManager& mgr) {
  const CVector3f position = xf.GetTranslation();
  CQuaternion orientation = CQuaternion::FromMatrix(xf);
  if (GetPositionKnotCount() == 0) {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mPositionId))) {
      orientation = CQuaternion::FromMatrix(actor->GetTransform());
    }
  }

  if (GetLookAtKnotCount() == 0) {
    if (const CActor* target = TCastToConstPtr< CActor >(mgr.GetObjectById(mTargetId))) {
      orientation =
          CQuaternion::FromMatrix(CTransform4f::LookAt(position, target->GetTranslation()));
    } else if (GetPositionKnotCount() != 0) {
      orientation = CSpline::GetOrientationByLength(positionDistance);
    }
  } else {
    const CVector3f target = CSpline::GetLookAtByLength(targetDistance);
    if ((target - position).IsMagnitudeSafe()) {
      orientation = CQuaternion::FromMatrix(CTransform4f::LookAt(position, target));
    } else {
      orientation = CQuaternion::FromMatrix(xf);
    }
  }
  return orientation;
}

CScriptPathCamera::CScriptPathCamera(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info, float distance, float speed,
    float angularSpeed, float dampenDistance, uint flags, uint splineFlags, int initialPosition,
    CMotionSpline::ESplineType positionType, CMotionSpline::ESplineType lookAtType,
    const CMayaSpline& positionTimeSpline, const CMayaSpline& lookAtTimeSpline,
    const CMayaSpline& fovSpline, const CMayaSpline& speedControlSpline,
    CMotionSpline::ESplineType playerType, bool playerLoops,
    const CMayaSpline& perpendicularDistanceSpline, const CMayaSpline& perpendicularInterpSpline)
: CEntity(uid, info, name, 0)
, mSpline(1.f, splineFlags, positionTimeSpline, lookAtTimeSpline, fovSpline,
          SLdrSpline::CreateFor(0.f, 0.f, 1.f, 1.f), positionType, lookAtType)
, mPlayerSpline(playerLoops, 1.f, playerType)
, mSpeedControlSpline(speedControlSpline)
, mDistance(distance)
, mSpeed(speed)
, mDampenDistance(dampenDistance)
, mAngularSpeed(angularSpeed)
, mInitialPosition(initialPosition)
, mFlags(flags)
, mPerpendicularDistanceControlSpline(perpendicularDistanceSpline)
, mPerpendicularInterpControlSpline(perpendicularInterpSpline)
, mTimeKeyframeId(kInvalidUniqueId) {}

CScriptPathCamera::~CScriptPathCamera() {}

void CScriptPathCamera::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CEntity::AcceptScriptMsg(mgr, msg);
  if (GetActive() && message == kSM_AreaLoaded) {
    ScriptCameraSpline::Initialise(*this, kSS_CameraPath, kSM_Attach, kSS_CameraTarget, kSM_Attach,
                                   mgr, mSpline);
    if (mSpline.GetLookAtKnotCount() == 0) {
      mSpline.SetTargetId(FindConnectedObject(mgr, kSS_CameraTarget, kSM_Attach));
    }
    if (mSpline.GetPositionKnotCount() == 0) {
      mSpline.SetPositionId(FindConnectedObject(mgr, kSS_CameraPath, kSM_Attach));
    }

    rstl::vector< CVector3f > playerPoints;
    rstl::vector< CQuaternion > playerOrientations;
    const TUniqueId player = FindConnectedObject(mgr, kSS_CameraPlayer, kSM_Attach);
    if (TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(player))) {
      ScriptCameraSpline::CollectWaypoints(*this, kSS_CameraPlayer, kSM_Attach, playerPoints,
                                           playerOrientations, mgr);
      mPlayerSpline.Initialise(playerPoints);
    }

    const TUniqueId timeKeyframe = FindConnectedObject(mgr, kSS_CameraTime, kSM_Attach);
    mTimeKeyframeId = TCastToConstPtr< CScriptTimeKeyframe >(mgr.GetObjectById(timeKeyframe))
                          ? timeKeyframe
                          : kInvalidUniqueId;
  }
}

void CScriptPathCamera::TranslateSplines(const CVector3f& offset) {
  mSpline.PositionSpline().Translate(offset);
  mSpline.LookAtSpline().Translate(offset);
  mPlayerSpline.Translate(offset);
}

void CScriptPathCamera::RotateSplines(const CQuaternion& rotation, const CVector3f& origin) {
  mSpline.PositionSpline().Rotate(rotation, origin);
  mSpline.LookAtSpline().Rotate(rotation, origin);
  mPlayerSpline.Rotate(rotation, origin);
}

CEntity* LoadPathCamera(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrPathCamera sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrPathCamera.inc"
  return rs_new CScriptPathCamera(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), sldrThis.distance, sldrThis.speed,
      sldrThis.angularSpeed * (M_PIF / 180.f), sldrThis.dampenDistance, sldrThis.flagsPathCamera,
      sldrThis.unknown_0xd4b29446, sldrThis.initialPosition,
      static_cast< CMotionSpline::ESplineType >(sldrThis.motionSplineType.type),
      static_cast< CMotionSpline::ESplineType >(sldrThis.targetSplineType.type),
      sldrThis.motionControlSpline, sldrThis.targetControlSpline, sldrThis.fOVSpline,
      sldrThis.speedControlSpline,
      static_cast< CMotionSpline::ESplineType >(sldrThis.playerSplineType.type),
      sldrThis.playerSplineLoops, CMayaSpline(sldrThis.perpendicularDistanceControlSpline),
      CMayaSpline(sldrThis.perpendicularInterpControlSpline));
}
