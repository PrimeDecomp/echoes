#include "MetroidPrime/ScriptObjects/CScriptCameraPitch.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Cameras/CScriptCameraSpline.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"

CScriptCameraPitch::CScriptCameraPitch(TUniqueId uid, const rstl::string& name,
                                       const CEntityInfo& info, const CTransform4f& xf,
                                       const CMayaSpline& forwardsPitch,
                                       const CMayaSpline& backwardsPitch, bool playerSplineLoops,
                                       const CMotionSpline::ESplineType& playerSplineType)
: CActor(uid, name, info, 0, xf, CModelData::CModelDataNull(), CMaterialList(kMT_Trigger),
         CActorParameters::None(), kInvalidUniqueId)
, mForwardsPitch(forwardsPitch)
, mBackwardsPitch(backwardsPitch)
, mPlayerSpline(playerSplineLoops, 1.f, playerSplineType) {}

CScriptCameraPitch::~CScriptCameraPitch() {}

float CScriptCameraPitch::GetPitch(const CTransform4f& playerXf) {
  const CVector3f position = playerXf.GetTranslation();
  CVector3f forward = playerXf.GetForward();
  forward.SetZ(0.f);
  if (forward.CanBeNormalized()) {
    forward.Normalize();
  } else {
    forward = CVector3f(0.f, 1.f, 0.f);
  }

  float distance = 0.f;
  if (mPlayerSpline.GetControlPointCount() != 0u) {
    distance = mPlayerSpline.FindClosestLengthOnSpline(0.f, position);
  }

  if (mPlayerSpline.GetKnotCount() == 0u) {
    CVector3f toTarget = GetTranslation() - position;
    toTarget.SetZ(0.f);
    if (toTarget.CanBeNormalized()) {
      const float alignment = CMath::Limit(CVector3f::Dot(toTarget.AsNormalized(), forward), 1.f);
      const float time = CMath::Clamp(0.f, CMath::AbsF(alignment), 1.f);
      if (alignment > 0.f) {
        return mForwardsPitch.EvaluateAt(time);
      }
      return mBackwardsPitch.EvaluateAt(time);
    }
    return 0.f;
  }

  const CVector3f start = mPlayerSpline.GetKnot(mPlayerSpline.GetKnotIndexByLength(distance));
  const CVector3f end = mPlayerSpline.GetKnot(mPlayerSpline.GetKnotIndexByLength(distance) + 1);
  CVector3f direction = end - start;
  direction.SetZ(0.f);
  if (direction.CanBeNormalized()) {
    direction.Normalize();
  } else {
    direction = forward;
  }

  const float alignment = CMath::Limit(CVector3f::Dot(direction, forward), 1.f);
  if (alignment > 0.f) {
    return alignment *
           mForwardsPitch.EvaluateAt(CMath::Clamp(0.f, distance / mPlayerSpline.GetLength(), 1.f));
  }
  return -alignment *
         mBackwardsPitch.EvaluateAt(CMath::Clamp(0.f, distance / mPlayerSpline.GetLength(), 1.f));
}

void CScriptCameraPitch::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CActor::AcceptScriptMsg(mgr, msg);
  if (!GetActive()) {
    return;
  }

  switch (message) {
  case kSM_XALD: {
    rstl::vector< CVector3f > positions;
    rstl::vector< CQuaternion > orientations;
    const TUniqueId waypointId = FindConnectedObject(mgr, kSS_CameraPlayer, kSM_Attach);
    if (TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(waypointId)) != nullptr) {
      ScriptCameraSpline::CollectWaypoints(*this, kSS_CameraPlayer, kSM_Attach, positions,
                                           orientations, mgr);
      mPlayerSpline.Initialise(positions);
    }
    break;
  }
  case kSM_Increment: {
    CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(msg.GetOriginator()));
    if (player == nullptr) {
      player = TCastToPtr< CPlayer >(mgr.GetPlayer(0));
    }
    if (player != nullptr) {
      mgr.CameraManager(mgr.MaskUIdNumPlayers(player->GetUniqueId()))
          ->FirstPersonCamera()
          ->SetScriptPitchId(GetUniqueId());
    }
    break;
  }
  case kSM_Decrement: {
    CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(msg.GetOriginator()));
    if (player == nullptr) {
      player = TCastToPtr< CPlayer >(mgr.GetPlayer(0));
    }
    if (player != nullptr) {
      mgr.CameraManager(mgr.MaskUIdNumPlayers(player->GetUniqueId()))
          ->FirstPersonCamera()
          ->SetScriptPitchId(kInvalidUniqueId);
    }
    break;
  }
  default:
    break;
  }
}
