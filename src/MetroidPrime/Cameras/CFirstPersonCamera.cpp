#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerCameraBob.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptObjects/CHUDBillboardEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraPitch.hpp"
#include "MetroidPrime/ScriptObjects/CScriptGrapplePoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerControls.hpp"

CFirstPersonCamera::CFirstPersonCamera(const TUniqueId& uid, const CTransform4f& xf,
                                       TUniqueId watchedId, float orbitCameraSpeed, float fov,
                                       float nearZ, float farZ, float aspect, int index,
                                       int controllerIdx)
: CGameCamera(uid, rstl::string_l("First Person Camera"),
              CEntityInfo(kInvalidAreaId, NullConnectionList, true), xf, fov, nearZ, farZ, aspect,
              watchedId, index, controllerIdx)
, mOrbitCameraSpeed(orbitCameraSpeed)
, mLockCamera(false)
, mGunFollowXf(xf)
, mPitch(0.f)
, mPitchId(kInvalidUniqueId)
, mPitchTransitionTimer(0.f)
, mPendingFluidId(kInvalidUniqueId)
, mCloseInVec(CVector3f::Zero())
, mCloseInTimer(0.f)
, mInitialFov(fov)
, mDeferBallTransitionProcessing(false)
, mFluidEffectsPending(false) {}

CFirstPersonCamera::~CFirstPersonCamera() {}

void CFirstPersonCamera::ProcessInput(const CFinalInput& input, CStateManager& mgr) {}

void CFirstPersonCamera::UpdateElevation(CStateManager& mgr) {
  mPitch = 0.f;
  if (CameraManager(mgr).IsInCinematicCamera()) {
    return;
  }
  CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(GetWatchedObject()));
  if (player && mPitchId != kInvalidUniqueId) {
    if (CScriptCameraPitch* pitch = TCastToPtr< CScriptCameraPitch >(mgr.ObjectById(mPitchId))) {
      mPitch = CRelAngle::FromDegrees(pitch->GetPitch(player->GetTransform())).AsRadians();
    }
  }
}

void CFirstPersonCamera::UpdateTransform(CStateManager& mgr, float dt) {
  if (gpMain->IsMaxSpeed()) {
    return;
  }
  CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(GetWatchedObject()));
  if (player == nullptr) {
    SetTransform(CTransform4f::Identity());
    return;
  }

  CTweakPlayer* tweak = player->GetTweakPlayer();
  const CTransform4f playerXf = player->GetTransform();
  float sinPitch = sinf(mPitch);
  sinPitch = CMath::Limit(sinPitch, 1.f);
  float cosPitch = cosf(mPitch);
  cosPitch = CMath::Limit(cosPitch, 1.f);
  CVector3f lookDir = playerXf.Rotate(CVector3f(0.f, cosPitch, sinPitch));
  if (player->IsInFreeLook()) {
    const float freeLookPitch = player->GetFreeLookAngleX();
    float angle = mPitch + freeLookPitch;
    if (!CMath::IsEpsilon(freeLookPitch, 0.f, 0.00001f)) {
      if (freeLookPitch <= 0.f) {
        angle = (mPitch + tweak->GetVerticalFreeLookAngleVel()) *
                    (freeLookPitch / tweak->GetVerticalFreeLookAngleVel()) +
                mPitch;
      } else {
        angle = (tweak->GetVerticalFreeLookAngleVel() - mPitch) *
                    (freeLookPitch / tweak->GetVerticalFreeLookAngleVel()) +
                mPitch;
      }
    }
    const float maxAngle = tweak->GetVerticalFreeLookAngleVel();
    if (fabs(angle) > maxAngle) {
      angle = maxAngle * CMath::Sign(angle);
    }
    CVector3f freeLookDir(sinf(-player->GetFreeLookAngleZ()) * cosf(angle),
                          cosf(-player->GetFreeLookAngleZ()) * cosf(angle), sinf(angle));
    if (player->GetTweakPlayerControls()->GetFreeLookTurnsPlayer()) {
      freeLookDir[kDX] = 0.f;
      if (!close_enough(freeLookDir, CVector3f::Zero())) {
        freeLookDir.Normalize();
      }
    }
    lookDir = playerXf.Rotate(freeLookDir);
  }

  CVector3f eyePos = player->GetEyePosition();
  if (mCloseInTimer > 0.f) {
    eyePos += CMath::Clamp(0.f, mCloseInTimer / 2.f, 1.f) * mCloseInVec;
    CPlayerCameraBob* bob = player->CameraBobObject();
    bob->ResetCameraBobTime();
    bob->SetCameraBobTransform(CTransform4f::Identity());
  }

  switch (player->GetOrbitState()) {
  case CPlayer::kOS_OrbitObject:
  case CPlayer::kOS_ForcedOrbitObject: {
    const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(player->GetOrbitTargetId()));
    if (actor != nullptr && actor->GetMaterialList().HasMaterial(kMT_Orbit)) {
      CVector3f orbitDir = player->GetOrbitPoint() - eyePos;
      if (orbitDir.CanBeNormalized()) {
        orbitDir.Normalize();
      }
      lookDir = orbitDir;
    } else {
      lookDir = player->GetOrbitPoint() - eyePos;
    }
    break;
  }
  case CPlayer::kOS_OrbitPoint:
  case CPlayer::kOS_OrbitCarcass:
    if (!player->IsLookButtonHeld()) {
      lookDir = player->GetOrbitPoint() - eyePos;
    }
    break;
  case CPlayer::kOS_NoOrbit:
    if (player->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed &&
        !player->IsInFreeLook() && mPitchId == kInvalidUniqueId) {
      if (player->GetJumpCameraTimer() > 0.f) {
        float t = (player->GetJumpCameraTimer() - tweak->GetJumpCameraPitchDownStart()) /
                  tweak->GetJumpCameraPitchDownFull();
        t = CMath::Clamp(0.f, t, 1.f);
        float angle = t * tweak->GetJumpCameraPitchDownAngle();
        angle += mPitch;
        lookDir = CVector3f(0.f, cosf(angle), -sinf(angle));
        lookDir = playerXf.Rotate(lookDir);
      } else if (player->GetFallCameraTimer() > 0.f) {
        float t = (player->GetFallCameraTimer() - tweak->GetFallCameraPitchDownStart()) /
                  tweak->GetFallCameraPitchDownFull();
        t = CMath::Clamp(0.f, t, 1.f);
        const float angle = t * tweak->GetFallCameraPitchDownAngle();
        lookDir = CVector3f(0.f, cosf(angle), -sinf(angle));
        lookDir = playerXf.Rotate(lookDir);
      }
    }
    break;
  case CPlayer::kOS_Grapple:
  default:
    break;
  }

  if (lookDir.CanBeNormalized()) {
    lookDir.Normalize();
  }
  float angularStep = dt;
  CQuaternion gunRotation = CQuaternion::NoRotation();
  CTransform4f gunXf = mGunFollowXf;
  if (!player->IsInFreeLook()) {
    switch (player->GetOrbitState()) {
    default: {
      CVector3f gunFront = mGunFollowXf.GetForward();
      gunFront[kDZ] = 0.f;
      if (gunFront.CanBeNormalized()) {
        gunFront.Normalize();
      }
      CVector3f flatLookDir = lookDir;
      flatLookDir[kDZ] = 0.f;
      if (flatLookDir.CanBeNormalized()) {
        flatLookDir.Normalize();
      }
      const CQuaternion yawRotation =
          CQuaternion::LookAt(gunFront, flatLookDir, CRelAngle::FromRadians(M_2PIF));
      gunXf = yawRotation.BuildTransform4f() * mGunFollowXf.GetRotation();
      CVector3f newFront = gunXf.GetForward();
      if (newFront.CanBeNormalized()) {
        newFront.Normalize();
      }
      angularStep *= tweak->GetFirstPersonCameraSpeed();
      if (mPitchTransitionTimer > 0.f) {
        angularStep *= 0.2f;
      }
      float angle = CMath::Limit(CVector3f::Dot(newFront, lookDir), 1.f);
      float t = acosf(angle) / angularStep;
      t = CMath::Clamp(0.f, t, 1.f);
      gunRotation = CQuaternion::LookAt(newFront, lookDir, CRelAngle::FromRadians(angularStep * t));
      break;
    }
    case CPlayer::kOS_Grapple: {
      CVector3f gunFront = mGunFollowXf.GetForward();
      gunFront[kDZ] = 0.f;
      if (gunFront.CanBeNormalized()) {
        gunFront.Normalize();
      }
      CVector3f flatLookDir = lookDir;
      flatLookDir[kDZ] = 0.f;
      if (flatLookDir.CanBeNormalized()) {
        flatLookDir.Normalize();
      }
      const CQuaternion yawRotation =
          CQuaternion::LookAt(gunFront, flatLookDir, CRelAngle::FromRadians(M_2PIF));
      gunXf = yawRotation.BuildTransform4f() * mGunFollowXf.GetRotation();
      CVector3f newFront = gunXf.GetForward();
      if (newFront.CanBeNormalized()) {
        newFront.Normalize();
      }
      angularStep *= tweak->GetGrappleCameraSpeed();
      float angle = CMath::Limit(CVector3f::Dot(newFront, lookDir), 1.f);
      float t = acosf(angle) / angularStep;
      t = CMath::Clamp(0.f, t, 1.f);
      gunRotation = CQuaternion::LookAt(newFront, lookDir, CRelAngle::FromRadians(angularStep * t));
      break;
    }
    case CPlayer::kOS_OrbitPoint:
    case CPlayer::kOS_OrbitCarcass: {
      CVector3f gunFront = mGunFollowXf.GetForward();
      gunFront[kDZ] = 0.f;
      if (gunFront.CanBeNormalized()) {
        gunFront.Normalize();
      }
      CVector3f flatLookDir = lookDir;
      flatLookDir[kDZ] = 0.f;
      if (flatLookDir.CanBeNormalized()) {
        flatLookDir.Normalize();
      }
      const CQuaternion yawRotation =
          CQuaternion::LookAt(gunFront, flatLookDir, CRelAngle::FromRadians(M_2PIF));
      gunXf = yawRotation.BuildTransform4f() * mGunFollowXf.GetRotation();
      CVector3f newFront = gunXf.GetForward();
      if (newFront.CanBeNormalized()) {
        newFront.Normalize();
      }
      const CRelAngle scaledAngle = CRelAngle::FromRadians(
          player->GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Scan
              ? angularStep * tweak->GetScanCameraSpeed()
              : angularStep * tweak->GetOrbitCameraSpeed() * 0.25f);
      float angle = CMath::Limit(CVector3f::Dot(newFront, lookDir), 1.f);
      float t = acosf(angle) / scaledAngle.AsRadians();
      t = CMath::Clamp(0.f, t, 1.f);
      gunRotation = CQuaternion::LookAt(newFront, lookDir,
                                        CRelAngle::FromRadians(scaledAngle.AsRadians() * t));
      break;
    }
    case CPlayer::kOS_ForcedOrbitObject:
    case CPlayer::kOS_OrbitObject: {
      CVector3f gunFront = mGunFollowXf.GetForward();
      if (gunFront.CanBeNormalized()) {
        gunFront.Normalize();
      }
      angularStep *= player->GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Scan
                         ? tweak->GetScanCameraSpeed()
                         : tweak->GetOrbitCameraSpeed();
      float angle = CMath::Limit(CVector3f::Dot(gunFront, lookDir), 1.f);
      float t = acosf(angle) / angularStep;
      t = CMath::Clamp(0.f, t, 1.f);
      if (angle > 0.9999f || mLockCamera || player->GetOrbitLockAcquired()) {
        gunRotation = CQuaternion::LookAt(gunFront, lookDir, CRelAngle::FromRadians(M_2PIF));
      } else {
        gunRotation =
            CQuaternion::LookAt(gunFront, lookDir, CRelAngle::FromRadians(angularStep * t));
      }

      const CScriptGrapplePoint* grapple =
          TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(player->GetOrbitTargetId()));
      if (grapple != nullptr && player->GetFallCameraTimer() > 0.f) {
        CVector3f flatGunFront = mGunFollowXf.GetForward();
        flatGunFront[kDZ] = 0.f;
        if (flatGunFront.CanBeNormalized()) {
          flatGunFront.Normalize();
        }
        CVector3f flatLookDir = lookDir;
        flatLookDir[kDZ] = 0.f;
        if (flatLookDir.CanBeNormalized()) {
          flatLookDir.Normalize();
        }
        const CQuaternion yawRotation =
            CQuaternion::LookAt(flatGunFront, flatLookDir, CRelAngle::FromRadians(M_2PIF));
        gunXf = yawRotation.BuildTransform4f() * mGunFollowXf.GetRotation();
        CVector3f newFront = gunXf.GetForward();
        if (newFront.CanBeNormalized()) {
          newFront.Normalize();
        }
        // Retail evaluates this interpolation even though it uses a full rotation below.
        float grappleDt = dt * tweak->GetGrappleCameraSpeed();
        float grappleAngle = CMath::Limit(CVector3f::Dot(newFront, lookDir), 1.f);
        float grappleT = acosf(grappleAngle) / grappleDt;
        t = CMath::Clamp(0.f, grappleT, 1.f);
        gunRotation = CQuaternion::LookAt(newFront, flatLookDir, CRelAngle::FromRadians(M_2PIF));
      }
      break;
    }
    }
  } else {
    CVector3f gunFront = mGunFollowXf.GetForward();
    gunFront[kDZ] = 0.f;
    if (gunFront.CanBeNormalized()) {
      gunFront.Normalize();
    }
    CVector3f flatLookDir = lookDir;
    flatLookDir[kDZ] = 0.f;
    if (flatLookDir.CanBeNormalized()) {
      flatLookDir.Normalize();
    }
    const CQuaternion yawRotation =
        CQuaternion::LookAt(gunFront, flatLookDir, CRelAngle::FromRadians(M_2PIF));
    gunXf = yawRotation.BuildTransform4f() * mGunFollowXf.GetRotation();
    CVector3f newFront = gunXf.GetForward();
    if (newFront.CanBeNormalized()) {
      newFront.Normalize();
    }
    angularStep *= tweak->GetFreeLookSpeed();
    float angle = CMath::Limit(CVector3f::Dot(newFront, lookDir), 1.f);
    if (CMath::AbsF(angle) < 0.999999f) {
      const float damping =
          CMath::EaseInOut(1.f - angle, CMath::kET_Quadratic, 0.f, 0.8f, 0.4f, 2.f, 4.f);
      gunRotation =
          CQuaternion::LookAt(newFront, lookDir, CRelAngle::FromRadians(angularStep * damping));
    }
  }

  CPlayerCameraBob* bob = player->CameraBobObject();
  CTransform4f bobXf = bob->GetCameraBobTransformation();
  if (player->GetMorphballTransitionState() == CPlayer::kMS_Morphed ||
      player->GetOrbitState() == CPlayer::kOS_Grapple ||
      player->GetGrappleState() != CPlayer::kGS_None ||
      mgr.GetGameState() == CStateManager::kGS_SoftPaused ||
      CameraManager(mgr).IsInCinematicCamera() || mCloseInTimer > 0.f) {
    bobXf = CTransform4f::Identity();
    bob->SetCameraBobTransform(bobXf);
  }
  mGunFollowXf = gunRotation.BuildTransform4f() * gunXf;
  SetTransform(mGunFollowXf * bobXf.GetRotation());
  mGunFollowXf.SetTranslation(eyePos);
  SetTranslation(eyePos + player->GetTransform().Rotate(bobXf.GetTranslation()));
  mGunFollowXf.Orthonormalize();
}

void CFirstPersonCamera::PreThink(float dt, CStateManager& mgr) {}

void CFirstPersonCamera::Render(const CStateManager& mgr) const {}

void CFirstPersonCamera::Reset(const CTransform4f& xf, CStateManager& mgr) {
  SetTransform(xf);
  SetTranslation(Player(mgr).GetEyePosition());
  mGunFollowXf = GetTransform();
  mPitchId = kInvalidUniqueId;
  mPitchTransitionTimer = 0.f;
}

void CFirstPersonCamera::SkipCinematic() {
  mCloseInVec = CVector3f::Zero();
  mCloseInTimer = 0.f;
}

void CFirstPersonCamera::Think(float dt, CStateManager& mgr) {
  CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(GetWatchedObject()));
  if (!player || player->HealthInfo()->GetHP() <= 0.f) {
    return;
  }
  if (mFluidEffectsPending) {
    UpdateFluidEffects(mgr);
    mFluidEffectsPending = false;
  }
  if (mDeferBallTransitionProcessing) {
    mDeferBallTransitionProcessing = false;
  } else {
    if (player->GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
      if (player->GetCameraState() != CPlayer::kCS_Spawned) {
        return;
      }
      SetTransform(player->CreateTransformFromMovementDirection());
      SetTranslation(player->GetEyePosition());
      return;
    }
    if (player->GetMorphballTransitionState() != CPlayer::kMS_Unmorphed) {
      if (player->GetMorphballTransitionState() != CPlayer::kMS_Unmorphing ||
          !CMath::IsEpsilon(player->GetMorphBallTransitionFactor(), 1.f, 0.00001f)) {
        return;
      }
    }
  }
  if (mPitchTransitionTimer > 0.f) {
    mPitchTransitionTimer -= dt;
  }
  const CTransform4f previous = GetTransform();
  UpdateElevation(mgr);
  UpdateTransform(mgr, dt);
  SetTransform(ValidateCameraTransform(GetTransform(), previous));
  if (mCloseInTimer > 0.f) {
    mCloseInTimer -= dt;
  }
  if (player->GetTurretState() == CPlayer::kTS_Entering) {
    CTransform4f turret = player->GetTurretTransform(mgr);
    const float t = 1.f - CMath::Clamp(0.f, player->GetTurretTimer() / 0.5f, 1.f);
    turret.SetTranslation(turret.GetTranslation() +
                          t * (GetTranslation() - turret.GetTranslation()));
    SetTransform(turret);
  } else if (player->GetTurretState() == CPlayer::kTS_Active) {
    SetTransform(player->GetTurretTransform(mgr));
  }
  CActor::Think(dt, mgr);
}

const CTransform4f& CFirstPersonCamera::GetGunFollowTransform() const { return mGunFollowXf; }

void CFirstPersonCamera::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CGameCamera::AcceptScriptMsg(mgr, msg);
  if (msg.GetMessage() == kSM_AreaLoaded) {
    mPitchId = kInvalidUniqueId;
  }
}

CVector3f CFirstPersonCamera::GetScanObjectIndicatorPosition(const CStateManager& mgr) const {
  return GetTranslation() + 5.f * GetTransform().GetForward();
}

void CFirstPersonCamera::UnkVtable84() {}

void CFirstPersonCamera::UnkVtable88(TUniqueId fluidId) {
  mFluidEffectsPending = true;
  mPendingFluidId = fluidId;
}

void CFirstPersonCamera::UpdateFluidEffects(CStateManager& mgr) {
  const CScriptWater* water = TCastToConstPtr< CScriptWater >(mgr.GetObjectById(mPendingFluidId));
  if ((Player(mgr).GetMorphballTransitionState() == CPlayer::kMS_Unmorphing ||
       Player(mgr).GetMorphballTransitionState() == CPlayer::kMS_Morphed) &&
      water) {
    // The original checks the visor effect before using the unmorph effect here.
    if (water->GetVisorRunoffEffect()) {
      mgr.AddObject(rs_new CHUDBillboardEffect(
          rstl::optional_object< TToken< CGenDescription > >(*water->GetUnmorphVisorRunoffEffect()),
          rstl::optional_object_null(), mgr.AllocateUniqueId(), true, rstl::string_l("WaterSheets"),
          CHUDBillboardEffect::GetNearClipDistance(mgr, GetControllerNumber()),
          CHUDBillboardEffect::GetScaleForPOV(mgr), GetControllerNumber(), CColor::White(),
          CVector3f::One(), CVector3f::Zero(), false));
    }
    Player(mgr).ApplySubmergedPitchBend(CSfxManager::SfxStart(
        water->GetUnmorphVisorRunoffSfx(), 127, Player(mgr).GetSoundPan(CPlayer::kMSP_4),
        CSfxManager::kAllAreas, false, false, CSfxManager::kMedPriority));
  }
  if (Player(mgr).GetMorphballTransitionState() == CPlayer::kMS_Unmorphed && water) {
    if (water->GetVisorRunoffEffect()) {
      mgr.AddObject(rs_new CHUDBillboardEffect(
          rstl::optional_object< TToken< CGenDescription > >(*water->GetVisorRunoffEffect()),
          rstl::optional_object_null(), mgr.AllocateUniqueId(), true, rstl::string_l("WaterSheets"),
          CHUDBillboardEffect::GetNearClipDistance(mgr, GetControllerNumber()),
          CHUDBillboardEffect::GetScaleForPOV(mgr), GetControllerNumber(), CColor::White(),
          CVector3f::One(), CVector3f::Zero(), false));
    }
    Player(mgr).ApplySubmergedPitchBend(CSfxManager::SfxStart(
        water->GetVisorRunoffSfx(), 127, Player(mgr).GetSoundPan(CPlayer::kMSP_4),
        CSfxManager::kAllAreas, false, false, CSfxManager::kMedPriority));
  }
  mPendingFluidId = kInvalidUniqueId;
}

void CFirstPersonCamera::SetScriptPitchId(TUniqueId uid) {
  mPitchId = uid;
  mPitchTransitionTimer = 1.f;
}
