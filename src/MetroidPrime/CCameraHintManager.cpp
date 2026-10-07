#include "MetroidPrime/CCameraHintManager.hpp"

#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CBallCamera.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Cameras/CFixedCamera.hpp"
#include "MetroidPrime/Cameras/CInterpolationCamera.hpp"
#include "MetroidPrime/Cameras/CPathCamera.hpp"
#include "MetroidPrime/Cameras/CScriptCameraSpline.hpp"
#include "MetroidPrime/Cameras/CSpindleCamera.hpp"
#include "MetroidPrime/Cameras/CSurfaceCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPathCamera.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/TCastTo.hpp"

CCameraHintManager::CCameraHintManager(int playerIndex, const rstl::string& name)
: CHintManager(playerIndex, name) {}

CCameraHintManager::~CCameraHintManager() {}

bool CCameraHintManager::SetHint(CHintState* hint, CStateManager& mgr, bool areaChanged,
                                 bool force) {
  const CScriptCameraHint* oldHint = TCastToConstPtr< CScriptCameraHint >(GetCurrentHint(mgr));
  CGameCamera* currentCamera = const_cast< CGameCamera* >(
      mgr.GetCameraManager(GetPlayerIndex())->GetCurrentCamera(mgr, false));
  const CTransform4f oldXf =
      mgr.GetCameraManager(GetPlayerIndex())->GetCurrentCamera(mgr, false)->GetTransform();
  const CScriptCameraHint* newHint =
      TCastToConstPtr< CScriptCameraHint >(mgr.GetObjectById(hint->GetHintId()));
  if (newHint == nullptr) {
    return false;
  }
  if (!CHintManager::SetHint(hint, mgr, areaChanged, force)) {
    return false;
  }

  bool interpolate = true;
  if (mgr.GetCameraManager(GetPlayerIndex())->IsBallCameraTransitioning(mgr)) {
    interpolate = false;
    const CScriptTrigger* trigger =
        TCastToConstPtr< CScriptTrigger >(mgr.GetObjectById(GetBestHintState()->GetFirstSender()));
    if (trigger != nullptr && (trigger->GetTriggerFlags() & 0x10000006) == 0x10000000) {
      interpolate = true;
    }
    if (!interpolate) {
      return false;
    }
  }

  CCameraManager* cameras = mgr.CameraManager(GetPlayerIndex());
  if (cameras->IsInFPCamera()) {
    interpolate = false;
  }
  CBallCamera* ballCamera = cameras->BallCamera();
  if (newHint->GetInfo().GetBehaviourType() == CBallCamera::kBCB_HintInitializePosition) {
    if (interpolate) {
      cameras->TransferCameraState(*currentCamera, *ballCamera, mgr);
      TeleportInitialPosition(newHint, mgr);
      return true;
    }
    return false;
  }

  if (interpolate) {
    ballCamera->OverrideCameraInfo(mgr);
    if ((newHint->GetInfo().GetFlags() & 0x20) != 0 ||
        (newHint->GetInfo().GetFlags() & 0x08000000) != 0) {
      ballCamera->TeleportLookAtStuff(mgr);
    }
  }

  CGameCamera* targetCamera = cameras->BallCamera();
  switch (newHint->GetInfo().GetBehaviourType()) {
  case CBallCamera::kBCB_Unknown6:
    cameras->SetPathCamera(newHint->GetDelegatedCameraId(), mgr);
    break;
  case CBallCamera::kBCB_Unknown7: {
    cameras->SetPathCamera(newHint->GetDelegatedCameraId(), mgr);
    targetCamera = cameras->PathCamera();
    const CScriptPathCamera* pathScript = TCastToConstPtr< CScriptPathCamera >(
        mgr.GetObjectById(cameras->PathCamera()->GetScriptCameraId()));
    if (pathScript != nullptr && newHint->GetInfo().GetInterpolateOnType() == 1) {
      CMotionSpline spline(false, newHint->GetInfo().GetInterpolateOnTime(),
                           CMotionSpline::kST_Bezier);
      spline.Reset(5);
      const float clampedLength =
          ScriptCameraSpline::ClampLength(pathScript->GetSpline().GetPositionSpline(),
                                          oldXf.GetTranslation(), false, CMaterialFilter(), mgr);
      const float closestLength = pathScript->GetSpline().FindClosestLengthOnSpline(
          clampedLength, targetCamera->GetTranslation());
      spline.AddKnotAndControlPoint(oldXf.GetTranslation());
      spline.AddKnotAndControlPoint(pathScript->GetSpline().GetPositionByLength(clampedLength));
      spline.AddKnotAndControlPoint(pathScript->GetSpline().GetPositionByLength(
          clampedLength + 0.5f * (closestLength - clampedLength)));
      spline.AddKnotAndControlPoint(pathScript->GetSpline().GetPositionByLength(closestLength));
      spline.AddKnotAndControlPoint(targetCamera->GetTranslation());
      spline.CalculateLength();
      cameras->InterpolationCamera()->SetSpline(spline);
    }
    break;
  }
  case CBallCamera::kBCB_Unknown8:
    cameras->SetSpindleCamera(newHint->GetDelegatedCameraId(), mgr);
    targetCamera = cameras->SpindleCamera();
    break;
  case CBallCamera::kBCB_Unknown9:
    cameras->SetSurfaceCamera(newHint->GetDelegatedCameraId(), mgr);
    targetCamera = cameras->SurfaceCamera();
    break;
  case CBallCamera::kBCB_FixedTransform:
    ballCamera->TeleportCamera(oldXf, mgr);
    ballCamera->TeleportLookAtStuff(mgr);
    ballCamera->SetFixedTransform(ballCamera->GetTransform());
    cameras->ClearPathCamera();
    cameras->ClearSpindleCamera();
    cameras->ClearSurfaceCamera(mgr);
    cameras->ClearFixedCamera();
    break;
  case CBallCamera::kBCB_Unknown4:
  case CBallCamera::kBCB_Unknown5:
    cameras->SetFixedCamera(newHint->GetUniqueId(), oldXf, mgr);
    targetCamera = cameras->FixedCamera();
    cameras->ClearPathCamera();
    cameras->ClearSpindleCamera();
    cameras->ClearSurfaceCamera(mgr);
    break;
  default:
    cameras->ClearPathCamera();
    cameras->ClearSpindleCamera();
    cameras->ClearSurfaceCamera(mgr);
    cameras->ClearFixedCamera();
    if (interpolate && !cameras->IsInterpolationCameraActive()) {
      cameras->SetCurrentCameraId(targetCamera->GetUniqueId(), mgr);
    }
    break;
  }

  if ((newHint->GetInfo().GetFlags() & 0x20) != 0) {
    const CVector3f position = targetCamera->GetTranslation();
    if (interpolate) {
      cameras->TransferCameraState(*currentCamera, *targetCamera, mgr);
    }
    targetCamera->SetTranslation(position);
  }

  if ((newHint->GetInfo().GetFlags() & 0x2000) != 0) {
    if (cameras->IsInCinematicCamera()) {
      cameras->CinematicCut(mgr);
      ForceRemoveHint(newHint->GetUniqueId(), mgr, kInvalidUniqueId);
      return true;
    }
  } else if (ballCamera->GetState() == CBallCamera::kBCS_FromBall) {
    cameras->SetCurrentCameraId(cameras->BallCamera()->GetUniqueId(), mgr);
    cameras->BallCamera()->TeleportCamera(oldXf, mgr);
    return true;
  }

  if ((newHint->GetInfo().GetFlags() & 0x20) == 0 &&
      ((newHint->GetInfo().GetFlags() & 0x08000000) == 0 || !force)) {
    if ((newHint->GetInfo().GetBehaviourType() == CBallCamera::kBCB_Default ||
         newHint->GetInfo().GetBehaviourType() == CBallCamera::kBCB_FreezeLookPosition) &&
        (oldHint == nullptr || oldHint->GetInfo().GetBehaviourType() == CBallCamera::kBCB_Default ||
         oldHint->GetInfo().GetBehaviourType() == CBallCamera::kBCB_FreezeLookPosition)) {
      if (interpolate && !cameras->IsInterpolationCameraActive() &&
          cameras->InterpolationCamera()->GetTargetId() == targetCamera->GetUniqueId()) {
        cameras->SetCurrentCameraId(targetCamera->GetUniqueId(), mgr);
      }
    } else {
      if (targetCamera->GetUniqueId() == cameras->BallCamera()->GetUniqueId()) {
        ballCamera->Reset(oldXf, mgr);
        ballCamera->OverrideCameraInfo(mgr);
      }
      CInterpolationCamera::EPositionMode positionMode =
          static_cast< CInterpolationCamera::EPositionMode >(
              newHint->GetInfo().GetInterpolateOnType());
      if (areaChanged == true) {
        positionMode = CInterpolationCamera::kPM_Direct;
      }
      bool toBallCamera = false;
      if (targetCamera->GetUniqueId() == cameras->BallCamera()->GetUniqueId()) {
        toBallCamera = true;
      }
      cameras->UpdateCameraTriggers(targetCamera->GetUniqueId(), mgr);
      cameras->SetupInterpolation(oldXf, currentCamera->GetUniqueId(), targetCamera->GetUniqueId(),
                                  (newHint->GetInfo().GetFlags() & 0x200000) != 0, positionMode,
                                  static_cast< CInterpolationCamera::ERotationMode >(
                                      newHint->GetInfo().GetInterpolationMode()),
                                  mgr, toBallCamera, newHint->GetInfo().GetInterpolateOnTime(),
                                  currentCamera->GetFov());
    }
  } else {
    if (currentCamera->GetUniqueId() != cameras->FirstPersonCamera()->GetUniqueId() &&
        newHint->GetInfo().GetBehaviourType() != CBallCamera::kBCB_Default &&
        newHint->GetInfo().GetBehaviourType() != CBallCamera::kBCB_FreezeLookPosition) {
      cameras->SetCurrentCameraId(targetCamera->GetUniqueId(), mgr);
      if ((newHint->GetInfo().GetFlags() & 0x1000000) == 0) {
        cameras->StartScreenFlash();
      }
    }
    ballCamera->InvalidateSpline();
    cameras->SetPlayerCamera(mgr, targetCamera->GetUniqueId());
    ballCamera->ResetFovInterpolation(ballCamera->GetTargetFov());
  }

  cameras->UpdateCameraTriggers(targetCamera->GetUniqueId(), mgr);
  if (TCastToPtr< CActor >(mgr.ObjectById(newHint->GetCameraTargetId()))) {
    targetCamera->SetWatchedObject(newHint->GetCameraTargetId());
  }
  return true;
}

void CCameraHintManager::ClearHint(CStateManager& mgr, bool areaChanged) {
  CScriptCameraHint* hint =
      TCastToPtr< CScriptCameraHint >(const_cast< CGameHint* >(GetCurrentHint(mgr)));
  CCameraManager* cameras = mgr.CameraManager(GetPlayerIndex());
  CBallCamera* ballCamera = cameras->BallCamera();
  const CTransform4f oldXf = cameras->GetCurrentCameraTransform(mgr, false);
  ClearCurrentHint(10000);

  if (cameras->IsBallCameraTransitioning(mgr)) {
    ballCamera->ResetToTweaks(mgr);
    return;
  }

  if (hint) {
    CInterpolationCamera::EPositionMode positionMode =
        static_cast< CInterpolationCamera::EPositionMode >(hint->GetInfo().GetInterpolateOffType());
    if (areaChanged == true) {
      positionMode = CInterpolationCamera::kPM_Direct;
    }

    const CVector3f toPlayer =
        mgr.GetPlayer(GetPlayerIndex())->GetTranslation() - oldXf.GetTranslation();
    CVector3f direction = toPlayer;
    direction.SetZ(0.f);
    if (direction.IsMagnitudeSafe() && positionMode == CInterpolationCamera::kPM_Direct) {
      direction.Normalize();
    } else {
      direction = mgr.GetPlayer(GetPlayerIndex())->GetMovementDirection();
    }
    ballCamera->ResetToTweaks(mgr);
    ballCamera->UpdateLookAtPosition(0.f, mgr, false);

    if (hint->GetInfo().GetBehaviourType() != CBallCamera::kBCB_Default &&
        hint->GetInfo().GetBehaviourType() != CBallCamera::kBCB_FreezeLookPosition) {
      if ((hint->GetInfo().GetFlags() & 0x1000) != 0) {
        ballCamera->SetClampVelTimer(hint->GetInfo().GetInterpolateOffTime());
      } else {
        if (!mgr.GetPlayer(GetPlayerIndex())->IsMorphBallTransitioning()) {
          ballCamera->TeleportCamera(ballCamera->FindDesiredTransform(direction, mgr), mgr);
        }

        if (positionMode != CInterpolationCamera::kPM_Direct) {
          CMotionSpline spline(false, hint->GetInfo().GetInterpolateOffTime(),
                               CMotionSpline::kST_Bezier);
          const CVector3f ballPosition = ballCamera->GetTranslation();
          CVector3f delta = ballPosition - oldXf.GetTranslation();
          delta.SetZ(0.f);
          const CVector3f cameraPosition = oldXf.GetTranslation();
          spline.Reset(5);
          spline.AddKnotAndControlPoint(cameraPosition);
          spline.AddKnotAndControlPoint(cameraPosition + delta);
          CVector3f forward = ballCamera->GetTransform().GetColumn(kDY);
          forward.SetZ(0.f);
          forward.Normalize();
          spline.AddKnotAndControlPoint(ballPosition - forward * delta.Magnitude());
          spline.AddKnotAndControlPoint(ballPosition);
          spline.AddKnotAndControlPoint(ballPosition);
          cameras->InterpolationCamera()->SetSpline(spline);
        }

        const float duration = hint->GetInfo().GetInterpolateOffTime();
        const TUniqueId ballId = ballCamera->GetUniqueId();
        const TUniqueId currentId = cameras->GetCurrentCameraId(false);
        mgr.CameraManager(GetPlayerIndex())
            ->SetupInterpolation(oldXf, currentId, ballId, false, positionMode,
                                 CInterpolationCamera::kRM_LinearSlerp, mgr, true, duration,
                                 ballCamera->GetFov());
      }
    }
  } else {
    const CVector3f toPlayer =
        mgr.GetPlayer(GetPlayerIndex())->GetTranslation() - oldXf.GetTranslation();
    CVector3f direction = toPlayer;
    direction.SetZ(0.f);
    if (direction.IsMagnitudeSafe()) {
      direction.Normalize();
    } else {
      direction = mgr.GetPlayer(GetPlayerIndex())->GetMovementDirection();
    }
    ballCamera->ResetToTweaks(mgr);
    ballCamera->UpdateLookAtPosition(0.f, mgr, false);
    ballCamera->TeleportCamera(ballCamera->FindDesiredTransform(direction, mgr), mgr);
    const TUniqueId ballId = ballCamera->GetUniqueId();
    const TUniqueId currentId = cameras->GetCurrentCameraId(false);
    mgr.CameraManager(GetPlayerIndex())
        ->SetupInterpolation(oldXf, currentId, ballId, false, CInterpolationCamera::kPM_Direct,
                             CInterpolationCamera::kRM_LinearSlerp, mgr, true, 2.f,
                             ballCamera->GetFov());
  }
}

void CCameraHintManager::TeleportInitialPosition(const CScriptCameraHint* hint,
                                                 CStateManager& mgr) {
  if (!hint) {
    return;
  }

  CCameraManager* cameras = mgr.CameraManager(GetPlayerIndex());
  CBallCamera* ballCamera = cameras->BallCamera();
  cameras->ClearPathCamera();
  cameras->ClearSpindleCamera();
  cameras->ClearSurfaceCamera(mgr);
  cameras->ClearFixedCamera();
  cameras->SetPlayerCamera(mgr, ballCamera->GetUniqueId());
  ballCamera->ResetToTweaks(mgr);
  ballCamera->UpdateLookAtPosition(0.f, mgr, false);

  const uint flags = hint->GetInfo().GetFlags();
  if ((flags & 0x40000) != 0) {
    ballCamera->Reset(CTransform4f::LookAt(CVector3f(0.f, 0.f, 0.f),
                                           mgr.GetPlayer(GetPlayerIndex())->GetMovementDirection(),
                                           CVector3f::Up()),
                      mgr);
  } else if ((flags & 0x20) != 0 || (flags & 0x08000000) != 0) {
    ballCamera->TeleportCamera(CTransform4f::LookAt(hint->GetTranslation(),
                                                    ballCamera->GetScanObjectIndicatorPosition(mgr),
                                                    CVector3f::Up()),
                               mgr);
    ballCamera->TeleportLookAtStuff(mgr);
  }
  ballCamera->InvalidateSpline();

  ForceRemoveHint(hint->GetUniqueId(), mgr, kInvalidUniqueId);
  if ((hint->GetInfo().GetFlags() & 0x2000) != 0) {
    cameras->CinematicCut(mgr);
  }
  cameras->SetCurrentCameraId(ballCamera->GetUniqueId(), mgr);
  ballCamera->SetWatchedObject(mgr.GetPlayer(GetPlayerIndex())->GetUniqueId());
  if ((hint->GetInfo().GetFlags() & 0x1000000) == 0) {
    cameras->StartScreenFlash();
  }
  cameras->UpdateCameraTriggers(ballCamera->GetUniqueId(), mgr);
}

bool CCameraHintManager::SelectHintFromStack(CHintState* hint, CStateManager& mgr,
                                             bool areaChanged) {
  bool changed = false;
  const CScriptCameraHint* newHint =
      TCastToConstPtr< CScriptCameraHint >(mgr.GetObjectById(hint->GetHintId()));
  const CScriptCameraHint* best = newHint;

  if ((newHint->GetInfo().GetFlags() & 0x80) != 0 && GetHints().size() > 1) {
    const CVector3f ballPosition = mgr.GetPlayer(GetPlayerIndex())->GetBallPosition();
    if ((newHint->GetInfo().GetFlags() & 0x100) != 0) {
      const CTransform4f cameraXf =
          mgr.CameraManager(GetPlayerIndex())->GetCurrentCameraTransform(mgr, false);
      CVector3f cameraDir = ballPosition - cameraXf.GetTranslation();
      if (cameraDir.IsMagnitudeSafe()) {
        cameraDir.Normalize();
      } else {
        cameraDir = cameraXf.GetColumn(kDY);
      }

      for (rstl::vector< SHint >::iterator it = GetHints().begin(); it != GetHints().end(); ++it) {
        const CScriptCameraHint* other =
            TCastToPtr< CScriptCameraHint >(mgr.ObjectById(it->mState.GetHintId()));
        if (other && other->GetUniqueId() != best->GetUniqueId() &&
            (other->GetInfo().GetFlags() & 0x80) != 0 &&
            other->GetPriority() == best->GetPriority() &&
            other->GetCurrentAreaId() == best->GetCurrentAreaId()) {
          CVector3f bestDir = ballPosition - best->GetTranslation();
          if (bestDir.IsMagnitudeSafe()) {
            bestDir.Normalize();
          } else {
            bestDir = best->GetTransform().GetColumn(kDY);
          }
          const float bestDot = CMath::Limit(CVector3f::Dot(cameraDir, bestDir), 1.f);

          CVector3f otherDir = ballPosition - other->GetTranslation();
          if (otherDir.IsMagnitudeSafe()) {
            otherDir.Normalize();
          } else {
            otherDir = other->GetTransform().GetColumn(kDY);
          }
          const float otherDot = CMath::Limit(CVector3f::Dot(cameraDir, otherDir), 1.f);
          if (otherDot > bestDot) {
            best = other;
          }
        }
      }
    } else {
      const CActor* sender = TCastToConstPtr< CActor >(mgr.GetObjectById(hint->GetFirstSender()));
      if (sender) {
        const CVector3f senderPosition = sender->GetTranslation();
        const CVector3f ballPosition2 = mgr.GetPlayer(GetPlayerIndex())->GetBallPosition();
        CVector3f senderDir = senderPosition - ballPosition2;
        if (senderDir.IsMagnitudeSafe()) {
          senderDir.Normalize();
        } else {
          senderDir = newHint->GetTransform().GetColumn(kDY);
        }

        for (rstl::vector< SHint >::iterator it = GetHints().begin(); it != GetHints().end();
             ++it) {
          const CScriptCameraHint* other =
              TCastToPtr< CScriptCameraHint >(mgr.ObjectById(it->mState.GetHintId()));
          if (!other || (other->GetInfo().GetFlags() & 0x80) == 0 ||
              other->GetPriority() != best->GetPriority() ||
              other->GetCurrentAreaId() != best->GetCurrentAreaId()) {
            break;
          }
          CVector3f bestDir = senderPosition - best->GetTranslation();
          if (bestDir.IsMagnitudeSafe()) {
            bestDir.Normalize();
          } else {
            bestDir = best->GetTransform().GetColumn(kDY);
          }
          const float bestDot = CMath::Limit(CVector3f::Dot(bestDir, senderDir), 1.f);

          CVector3f otherSenderDir = senderPosition - ballPosition2;
          if (otherSenderDir.IsMagnitudeSafe()) {
            otherSenderDir.Normalize();
          } else {
            otherSenderDir = other->GetTransform().GetColumn(kDY);
          }
          CVector3f otherDir = senderPosition - other->GetTranslation();
          if (otherDir.IsMagnitudeSafe()) {
            otherDir.Normalize();
          } else {
            otherDir = other->GetTransform().GetColumn(kDY);
          }
          const float otherDot = CMath::Limit(CVector3f::Dot(otherDir, otherSenderDir), 1.f);
          if (otherDot > bestDot) {
            best = other;
          }
        }
      }
    }
    if (best->GetUniqueId() != GetCurrentHintId()) {
      changed = true;
    }
  } else if (GetCurrentHintId() != hint->GetHintId()) {
    if (newHint->GetInfo().GetBehaviourType() == CBallCamera::kBCB_HintInitializePosition) {
      TeleportInitialPosition(newHint, mgr);
      changed = false;
    } else {
      changed = true;
    }
  }

  if (changed) {
    SetHint(GetHintState(best->GetUniqueId()), mgr, areaChanged, false);
  }
  return changed;
}

bool CCameraHintManager::HasBallCameraInitialPositionHint(const CStateManager& mgr) const {
  if (HasHint(mgr)) {
    const CScriptCameraHint* hint = TCastToConstPtr< CScriptCameraHint >(GetCurrentHint(mgr));
    if (hint) {
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
        break;
      }
    }
  }
  return false;
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
  cameras->ClearSurfaceCamera(mgr);
  cameras->ClearFixedCamera();
}
