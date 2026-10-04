#include "MetroidPrime/Cameras/CSurfaceCamera.hpp"

#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CGameHint.hpp"
#include "MetroidPrime/CHintManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CBallCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSurfaceCamera.hpp"
#include "MetroidPrime/TCastTo.hpp"

CSurfaceCamera::CSurfaceCamera(TUniqueId uid, const CTransform4f& xf, bool active, int index,
                               int controllerIdx)
: CGameCamera(uid, rstl::string_l("Surface Camera"),
              CEntityInfo(kInvalidAreaId, NullConnectionList, active), xf,
              CCameraManager::GetDefaultThirdPersonVerticalFOV(),
              CCameraManager::GetDefaultFirstPersonNearClipDistance(),
              CCameraManager::GetDefaultFirstPersonFarClipDistance(),
              CCameraManager::GetDefaultAspectRatio(), kInvalidUniqueId, index, controllerIdx)
, mScriptCameraId(kInvalidUniqueId)
, mTrackedPlayerPosition(CVector3f::Zero())
, mPlayerPositionSpring(1.f, 5.f, 3.f)
, mTargetSplineDistance(0.f)
, mPlayerSplineDistance(0.f) {}

CSurfaceCamera::~CSurfaceCamera() {}

void CSurfaceCamera::SetScriptCameraId(TUniqueId uid) { mScriptCameraId = uid; }

void CSurfaceCamera::Reset(const CTransform4f& xf, CStateManager& mgr) {
  mTrackedPlayerPosition = Player(mgr).GetBallPosition();
  mPlayerPositionSpring.Reset();
  mTargetSplineDistance = 0.f;
  mPlayerSplineDistance = 0.f;
  Think(0.01f, mgr);
}

void CSurfaceCamera::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  // TODO: Recover surface projection, spring tracking, type-specific offsets and spline/FOV
  // updates.
  CActor::Think(dt, mgr);
}

void CSurfaceCamera::ProcessInput(const CFinalInput& input, CStateManager& mgr) {}

void CSurfaceCamera::Render(const CStateManager& mgr) const {}

void CSurfaceCamera::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CGameCamera::AcceptScriptMsg(mgr, msg);
}

CVector3f CSurfaceCamera::GetScanObjectIndicatorPosition(const CStateManager& mgr) const {
  CVector3f target = GetCameraManager(mgr).GetBallCamera()->GetScanObjectIndicatorPosition(mgr);
  const CScriptSurfaceCamera* camera =
      TCastToConstPtr< CScriptSurfaceCamera >(mgr.GetObjectById(mScriptCameraId));
  if (!camera) {
    return target;
  }
  if (camera->GetFlags() & CScriptSurfaceCamera::kSF_TargetBallPosition) {
    return GetPlayer(mgr).GetBallPosition();
  }
  const CMotionSpline& targetSpline = camera->GetTargetSpline();
  if (targetSpline.GetControlPointCount() == 0) {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(camera->GetTargetId()))) {
      target = actor->GetTranslation();
    }
  } else {
    float progress = mTargetSplineDistance / targetSpline.GetLength();
    if (camera->GetPlayerSpline().GetControlPointCount() != 0) {
      progress = mPlayerSplineDistance / camera->GetPlayerSpline().GetLength();
    }
    // Native ordered comparisons map an unordered progress value to zero.
    if (!(0.f <= progress)) {
      progress = 0.f;
    } else if (!(1.f >= progress)) {
      progress = 1.f;
    }
    const float time = camera->GetTargetControlSpline().EvaluateAt(progress);
    target = targetSpline.GetPositionByTime(time * targetSpline.GetDuration());
  }
  if (camera->GetFlags() & CScriptSurfaceCamera::kSF_ProjectTargetAlongHintForward) {
    const CGameHint* hint = GetCameraManager(mgr).GetHintManager()->GetCurrentHint(mgr);
    if (hint) {
      const CVector3f direction = hint->GetTransform().GetForward();
      const CVector3f position = GetTranslation();
      target = position +
               CVector3f::Dot(direction, GetPlayer(mgr).GetBallPosition() - position) * direction;
    }
  }
  return target;
}
