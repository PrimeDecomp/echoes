#include "MetroidPrime/CCameraHintManager.hpp"

#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CBallCamera.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraHint.hpp"
#include "MetroidPrime/TCastTo.hpp"

CCameraHintManager::CCameraHintManager(int playerIndex, const rstl::string& name)
: CHintManager(playerIndex, name) {}

CCameraHintManager::~CCameraHintManager() {}

bool CCameraHintManager::SetHint(CHintState* hint, CStateManager& mgr, bool areaChanged,
                                 bool force) {
  // TODO: Nonfunctional scaffold. Recover behavior-specific camera overrides and transitions.
  return false;
}

void CCameraHintManager::ClearHint(CStateManager& mgr, bool areaChanged) {
  ClearCurrentHint(10000);
  // TODO: Nonfunctional scaffold. Restore the outgoing hint's camera and interpolation state.
}

void CCameraHintManager::TeleportInitialPosition(const CScriptCameraHint* hint,
                                                 CStateManager& mgr) {
  // TODO: Nonfunctional scaffold. Recover ball-camera look-at setup and trigger transfer.
}

bool CCameraHintManager::SelectHintFromStack(CHintState* hint, CStateManager& mgr,
                                             bool areaChanged) {
  // TODO: Nonfunctional scaffold. Compare equal-priority hints against the player or helper.
  return false;
}

bool CCameraHintManager::HasBallCameraInitialPositionHint(const CStateManager& mgr) const {
  if (!HasHint(mgr)) {
    return false;
  }

  const CScriptCameraHint* hint = TCastToConstPtr< CScriptCameraHint >(GetCurrentHint(mgr));
  if (!hint) {
    return false;
  }

  switch (hint->GetInfo().GetBehaviourType()) {
  case CBallCamera::kBCB_HintBallToCam:
  case CBallCamera::kBCB_Unknown4:
  case CBallCamera::kBCB_Unknown5:
  case CBallCamera::kBCB_Unknown7:
  case CBallCamera::kBCB_Unknown8:
  case CBallCamera::kBCB_Unknown9:
  case CBallCamera::kBCB_HintLocalOffset:
  case CBallCamera::kBCB_FixedTransform:
    return true;
  default:
    return false;
  }
}

void CCameraHintManager::RefreshHint(CStateManager& mgr) {
  const CScriptCameraHint* hint = TCastToConstPtr< CScriptCameraHint >(GetCurrentHint(mgr));
  if (!hint) {
    return;
  }

  switch (hint->GetInfo().GetBehaviourType()) {
  case CBallCamera::kBCB_FixedTransform:
    break;
  case CBallCamera::kBCB_HintInitializePosition:
    if ((hint->GetInfo().GetFlags() & 0x20) != 0 ||
        (hint->GetInfo().GetFlags() & 0x08000000) != 0) {
      mgr.CameraManager(GetPlayerIndex())->BallCamera()->TeleportCamera(hint->GetTransform(), mgr);
    }
    RemoveHint(GetCurrentHintId(), kInvalidUniqueId, mgr);
    break;
  default:
    if (CHintState* best = GetBestHintState()) {
      SetHint(best, mgr, false, true);
    }
    break;
  }
}

void CCameraHintManager::OnHintRemoved(CStateManager& mgr) {
  CCameraManager* cameras = mgr.CameraManager(GetPlayerIndex());
  cameras->ClearPathCamera();
  cameras->ClearSpindleCamera();
  cameras->ClearSurfaceCamera();
  cameras->ClearFixedCamera();
}
