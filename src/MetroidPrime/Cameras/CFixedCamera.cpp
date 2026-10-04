#include "MetroidPrime/Cameras/CFixedCamera.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CBallCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraHint.hpp"
#include "MetroidPrime/TCastTo.hpp"

namespace {
// Guessed names for the supported roles in the two separate flag domains.
enum EHintFlags {
  kHF_TeleportBallCamera = 0x20,
  kHF_InstantLookAt = 0x40,
  kHF_UseExistingTransform = 0x400,
  kHF_FreezeTargetPosition = 0x800
};

enum EOverrideFlags {
  kOF_OverrideFov = 0x10,
  kOF_ConstrainAttitude = 0x20,
  kOF_ConstrainAzimuth = 0x40
};
} // namespace

CFixedCamera::CFixedCamera(TUniqueId uid, const CTransform4f& xf, int index, int controllerIdx)
: CGameCamera(uid, rstl::string_l("Fixed Camera"),
              CEntityInfo(kInvalidAreaId, NullConnectionList, false), xf,
              CCameraManager::GetDefaultThirdPersonVerticalFOV(),
              CCameraManager::GetDefaultFirstPersonNearClipDistance(),
              CCameraManager::GetDefaultFirstPersonFarClipDistance(),
              CCameraManager::GetDefaultAspectRatio(), kInvalidUniqueId, index, controllerIdx)
, mTargetPosition(CVector3f::Zero())
, mScriptCameraId(kInvalidUniqueId) {}

CFixedCamera::~CFixedCamera() {}

void CFixedCamera::ProcessInput(const CFinalInput& input, CStateManager& mgr) {}

void CFixedCamera::UpdateTargetPosition(CStateManager& mgr) {
  const CScriptCameraHint* hint =
      TCastToConstPtr< CScriptCameraHint >(mgr.GetObjectById(mScriptCameraId));
  if (!hint) {
    return;
  }

  switch (hint->GetInfo().GetBehaviourType()) {
  case CBallCamera::kBCB_Unknown5: {
    const CTransform4f xf = hint->GetTransform();
    const CVector3f ballTarget =
        CameraManager(mgr).BallCamera()->GetScanObjectIndicatorPosition(mgr);
    const float distance = CMath::FastMax(
        CMath::AbsF(CVector3f::Dot(ballTarget - GetTranslation(), xf.GetForward())), 1.f);
    mTargetPosition = xf.GetTranslation() + distance * xf.GetForward();
    break;
  }
  case CBallCamera::kBCB_Unknown4:
    mTargetPosition = CGameCamera::GetScanObjectIndicatorPosition(mgr);
    break;
  default:
    break;
  }
}

void CFixedCamera::PreThink(float dt, CStateManager& mgr) {}

void CFixedCamera::Render(const CStateManager& mgr) const {}

void CFixedCamera::Reset(const CTransform4f& xf, CStateManager& mgr) {
  UpdateTargetPosition(mgr);
  const CScriptCameraHint* hint =
      TCastToConstPtr< CScriptCameraHint >(mgr.GetObjectById(mScriptCameraId));
  if (hint) {
    if (!(hint->GetInfo().GetFlags() & kHF_UseExistingTransform)) {
      SetTransform(CTransform4f::LookAt(hint->GetTranslation(), mTargetPosition));
    } else {
      SetTransform(xf);
    }
    if (hint->GetInfo().GetFlags() & kHF_TeleportBallCamera) {
      CBallCamera* ballCamera = mgr.CameraManager(GetControllerNumber())->BallCamera();
      ballCamera->TeleportCamera(GetTransform(), mgr);
      ballCamera->TeleportLookAtStuff(mgr);
    }
  } else {
    SetTransform(xf);
  }
  SetFovAndTarget(CCameraManager::GetDefaultThirdPersonVerticalFOV());
}

void CFixedCamera::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  const CTransform4f oldXf = GetTransform();
  const CScriptCameraHint* hint =
      TCastToConstPtr< CScriptCameraHint >(mgr.GetObjectById(mScriptCameraId));
  if (hint) {
    const CCameraOverrideInfo& info = hint->GetInfo();
    if (!(info.GetFlags() & kHF_FreezeTargetPosition)) {
      UpdateTargetPosition(mgr);
    }
    switch (info.GetBehaviourType()) {
    case CBallCamera::kBCB_Unknown5:
      SetTransform(CTransform4f::LookAt(hint->GetTranslation(), mTargetPosition));
      break;
    case CBallCamera::kBCB_Unknown4: {
      const CVector3f position =
          (info.GetFlags() & kHF_UseExistingTransform) ? GetTranslation() : hint->GetTranslation();
      CVector3f direction = mTargetPosition - position;
      if (direction.IsMagnitudeSafe()) {
        direction = ConstrainLookDirection(direction.AsNormalized(), mgr);
        if (info.GetFlags() & kHF_InstantLookAt) {
          SetTransform(CTransform4f::LookAt(position, position + direction));
        } else if (direction.DropZ().IsMagnitudeSafe()) {
          CVector3f forward = GetTransform().GetForward();
          if (!forward.IsMagnitudeSafe()) {
            SetTransform(CTransform4f::LookAt(position, position + direction));
            return;
          }
          forward.Normalize();
          const float dot = CMath::Limit(CVector3f::Dot(forward, direction), 1.f);
          if (CMath::AbsF(dot) >= 0.9999999f) {
            SetTransform(CTransform4f::LookAt(position, position + direction));
          } else {
            const float angle = CMath::ArcCosineR(dot);
            const float fraction = CMath::Clamp(0.f, angle / (1.0471976f * dt), 1.f);
            float step =
                dt * (fraction * GetCameraManager(mgr).GetBallCamera()->GetTargetAnglePerSecond());
            const float vertical =
                CMath::AbsF(CMath::Limit(CVector3f::Dot(direction, CVector3f::Up()), 1.f));
            const float verticalStep = (12.566371f * dt) * (1.f - vertical);
            if (step > verticalStep && !GetPlayer(mgr).IsMorphBallTransitioning() &&
                vertical > 0.999f) {
              step = verticalStep;
            }
            const CQuaternion rotation = CQuaternion::LookAt(
                CUnitVector3f(forward), CUnitVector3f(direction), CRelAngle::FromRadians(step));
            SetTransform(rotation.BuildTransform4f() * GetTransform().GetRotation());
          }
          SetTranslation(position);
        } else {
          SetTranslation(position);
        }
      }
      break;
    }
    default:
      break;
    }
    if (info.GetOverrideFlags() & kOF_OverrideFov) {
      SetFovAndTarget(info.GetFov());
    }
  }
  SetTransform(ValidateCameraTransform(GetTransform(), oldXf));
  CActor::Think(dt, mgr);
}

void CFixedCamera::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CGameCamera::AcceptScriptMsg(mgr, msg);
}

CVector3f CFixedCamera::GetScanObjectIndicatorPosition(const CStateManager& mgr) const {
  return mTargetPosition;
}

CVector3f CFixedCamera::ConstrainLookDirection(const CVector3f& direction, CStateManager& mgr) {
  const CScriptCameraHint* hint =
      TCastToConstPtr< CScriptCameraHint >(mgr.GetObjectById(mScriptCameraId));
  if (!hint) {
    return direction;
  }

  CVector3f hintDirection = hint->GetTransform().GetForward();
  CVector3f hintPlanar = hintDirection.DropZ();
  if (hintDirection.IsMagnitudeSafe() && hintPlanar.IsMagnitudeSafe()) {
    hintDirection.Normalize();
    hintPlanar.Normalize();
  } else {
    hintDirection = CVector3f::Forward();
    hintPlanar = CVector3f::Forward();
  }
  CVector3f planar = direction.DropZ();
  if (direction.IsMagnitudeSafe() && planar.IsMagnitudeSafe()) {
    planar.Normalize();
  } else {
    planar = hintPlanar;
  }

  const CCameraOverrideInfo& info = hint->GetInfo();
  float attitude = CMath::ArcCosineR(CMath::Limit(CVector3f::Dot(direction, planar), 1.f));
  if (info.GetOverrideFlags() & kOF_ConstrainAttitude) {
    const float hintAttitude =
        CMath::ArcCosineR(CMath::Limit(CVector3f::Dot(hintDirection, hintPlanar), 1.f));
    attitude = hintAttitude + CMath::Limit(attitude - hintAttitude, info.GetAttitudeRange());
  }
  if (direction.GetZ() >= 0.f) {
    attitude = -attitude;
  }

  float azimuth = CMath::ArcCosineR(CMath::Limit(CVector3f::Dot(planar, hintPlanar), 1.f));
  if (info.GetOverrideFlags() & kOF_ConstrainAzimuth) {
    azimuth = CMath::Limit(azimuth, info.GetAzimuthRange());
  }
  if (CVector3f::Cross(planar, hintPlanar).GetZ() >= 0.f) {
    azimuth = -azimuth;
  }
  const CVector3f rotated =
      CQuaternion::ZRotation(CRelAngle::FromRadians(azimuth)).Transform(hintPlanar);
  const CUnitVector3f axis(CVector3f(rotated.GetY(), -rotated.GetX(), 0.f));
  return CQuaternion::AxisAngle(axis, CRelAngle::FromRadians(-attitude)).Transform(rotated);
}

void CFixedCamera::SetScriptCameraId(TUniqueId uid) { mScriptCameraId = uid; }
