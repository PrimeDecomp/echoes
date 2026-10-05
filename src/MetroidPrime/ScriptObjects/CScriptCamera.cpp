#include "MetroidPrime/ScriptObjects/CScriptCamera.hpp"

#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CCinematicCamera.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/TCastTo.hpp"

class CScriptTimeKeyframe; // Guessed name; shared with the path-camera connection code.

CScriptCamera::CScriptCamera(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                             const CTransform4f& xf, float duration, uint flags, uint splineFlags,
                             const CMayaSpline& positionTimeSpline,
                             const CMayaSpline& lookAtTimeSpline, const CMayaSpline& fovSpline,
                             const CMayaSpline& rollSpline, CMotionSpline::ESplineType positionType,
                             CMotionSpline::ESplineType lookAtType,
                             const CMayaSpline& slowMotionSpline)
: CActor(uid, name, info, 0, xf, CModelData::CModelDataNull(), CMaterialList(kMT_NoStepLogic),
         CActorParameters::None(), kInvalidUniqueId)
, mSpline(duration, splineFlags, positionTimeSpline, lookAtTimeSpline, fovSpline, rollSpline,
          positionType, lookAtType)
, mDuration(duration)
, mFlags(flags)
, mCameraActorId(kInvalidUniqueId)
, mTimeKeyframeId(kInvalidUniqueId)
, mSlowMotionSpline(slowMotionSpline)
, mHasBeenViewed(false) {}

CScriptCamera::~CScriptCamera() {}

void CScriptCamera::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CActor::AcceptScriptMsg(mgr, msg);
  switch (message) {
  case kSM_Activate: {
    const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(msg.GetOriginator()));
    const int index = player ? mgr.MaskUIdNumPlayers(msg.GetOriginator()) : 0;
    mgr.CameraManager(index)->AddCinemaCamera(GetUniqueId(), mgr);
    mHasBeenViewed = WasViewed(mgr);
    break;
  }
  case kSM_Deactivate: {
    CPlayer* player = TCastToPtr< CPlayer >(mgr.GetObjectByIdFromListAll(msg.GetOriginator()));
    const int index = player ? mgr.MaskUIdNumPlayers(msg.GetOriginator()) : 0;
    CCameraManager& cameras = *mgr.CameraManager(index);
    if (cameras.GetCinematicCamera()->GetScriptCameraId() == GetUniqueId()) {
      cameras.StopCinematics(mgr);
      if (player) {
        player->GetMorphBall()->LoadMorphBallModel();
      }
    }
    if ((mFlags & kF_CinematicPause) != 0) {
      mgr.SetCinematicPause(false);
    }
    break;
  }
  case kSM_Stop:
  case kSM_Start: {
    const CPlayer* player =
        TCastToPtr< CPlayer >(mgr.GetObjectByIdFromListAll(msg.GetOriginator()));
    const int index = player ? mgr.MaskUIdNumPlayers(msg.GetOriginator()) : 0;
    mgr.CameraManager(index)->SetCinematicPaused(message == kSM_Stop);
    break;
  }
  case kSM_AreaLoaded: {
    ScriptCameraSpline::Initialise(*this, kSS_CameraPath, kSM_Attach, kSS_CameraTarget, kSM_Attach,
                                   mgr, mSpline);
    if (mSpline.GetPositionKnotCount() == 0) {
      mSpline.SetPositionId(FindConnectedObject(mgr, kSS_CameraPath, kSM_Attach));
    }
    if (mSpline.GetLookAtKnotCount() == 0) {
      mSpline.SetTargetId(FindConnectedObject(mgr, kSS_CameraTarget, kSM_Attach));
    }
    const TUniqueId timeKeyframe = FindConnectedObject(mgr, kSS_CameraTime, kSM_Attach);
    mTimeKeyframeId = TCastToConstPtr< CScriptTimeKeyframe >(mgr.GetObjectById(timeKeyframe))
                          ? timeKeyframe
                          : kInvalidUniqueId;
    break;
  }
  default:
    break;
  }
}

bool CScriptCamera::WasViewed(const CStateManager& mgr) const {
  return gpGameState->SystemOptions().GetCinematicState(
      rstl::pair< CAssetId, TEditorId >(mgr.GetWorld()->GetWorldAssetId(), GetEditorId()));
}

void CScriptCamera::MarkViewed(const CStateManager& mgr) const {
  gpGameState->SystemOptions().SetCinematicState(
      rstl::pair< CAssetId, TEditorId >(mgr.GetWorld()->GetWorldAssetId(), GetEditorId()), true);
}

void CScriptCamera::TranslateSplines(const CVector3f& offset) {
  mSpline.PositionSpline().Translate(offset);
  mSpline.LookAtSpline().Translate(offset);
}

void CScriptCamera::RotateSplines(const CQuaternion& rotation, const CVector3f& origin) {
  mSpline.PositionSpline().Rotate(rotation, origin);
  mSpline.LookAtSpline().Rotate(rotation, origin);
}

CEntity* LoadCamera(CStateManager& mgr, CInputStream& in, CEntityInfo& info) {}
