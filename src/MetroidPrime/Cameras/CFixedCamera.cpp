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

CFixedCamera::CFixedCamera(const TUniqueId& uid, const CTransform4f& xf, int index,
                           int controllerIdx)
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

void CFixedCamera::UpdateTargetPosition(float dt, CStateManager& mgr) {
  const CScriptCameraHint* hint =
      TCastToConstPtr< CScriptCameraHint >(mgr.GetObjectById(mScriptCameraId));
  if (!hint) {
    return;
  }

  switch (hint->GetInfo().GetBehaviourType()) {
  case CBallCamera::kBCB_Unknown4:
    mTargetPosition = CGameCamera::GetScanObjectIndicatorPosition(mgr);
    break;
  case CBallCamera::kBCB_Unknown5: {
    const CTransform4f xf = hint->GetTransform();
    const CVector3f ballTarget =
        CameraManager(mgr).BallCamera()->GetScanObjectIndicatorPosition(mgr);
    const float distance = CMath::FastMax(
        CMath::AbsF(CVector3f::Dot(ballTarget - GetTranslation(), xf.GetForward())), 1.f);
    mTargetPosition = xf.GetTranslation() + distance * xf.GetForward();
    break;
  }
  default:
    break;
  }
}

void CFixedCamera::PreThink(float dt, CStateManager& mgr) {}

void CFixedCamera::Render(const CStateManager& mgr) const {}

void CFixedCamera::Reset(const CTransform4f& xf, CStateManager& mgr) {
  UpdateTargetPosition(0.f, mgr);
  const CScriptCameraHint* hint =
      TCastToConstPtr< CScriptCameraHint >(mgr.GetObjectById(mScriptCameraId));
  if (hint) {
    if (hint->GetInfo().GetFlags() & kHF_UseExistingTransform) {
      SetTransform(xf);
    } else {
      SetTransform(CTransform4f::LookAt(hint->GetTranslation(), mTargetPosition));
    }
    if (hint->GetInfo().GetFlags() & kHF_TeleportBallCamera) {
      mgr.CameraManager(GetControllerNumber())->BallCamera()->TeleportCamera(GetTransform(), mgr);
      mgr.CameraManager(GetControllerNumber())->BallCamera()->TeleportLookAtStuff(mgr);
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
      UpdateTargetPosition(dt, mgr);
    }
    switch (info.GetBehaviourType()) {
    case CBallCamera::kBCB_Unknown4: {
      CVector3f position = hint->GetTranslation();
      if (info.GetFlags() & kHF_UseExistingTransform) {
        position = GetTranslation();
      }
      CVector3f direction = mTargetPosition - position;
      if (direction.IsMagnitudeSafe()) {
        direction = ConstrainLookDirection(direction.AsNormalized(), mgr);
        if (info.GetFlags() & kHF_InstantLookAt) {
          SetTransform(CTransform4f::LookAt(position, position + direction));
        } else {
          const CTransform4f xf = GetTransform();
          if (!direction.DropZ().IsMagnitudeSafe()) {
            SetTranslation(position);
          } else {
            CVector3f forward = GetTransform().GetForward();
            if (!forward.IsMagnitudeSafe()) {
              SetTransform(CTransform4f::LookAt(position, position + direction));
            } else {
              forward.Normalize();
              const float dot = CMath::Limit(CVector3f::Dot(forward, direction), 1.f);
              if (CMath::AbsF(dot) >= 0.9999999f) {
                SetTransform(CTransform4f::LookAt(position, position + direction));
              } else {
                const float angle = CMath::ArcCosineR(dot);
                const float fraction = CMath::Clamp(0.f, angle / (1.0471976f * dt), 1.f);
                CRelAngle step = CRelAngle::FromRadians(
                    dt *
                    (fraction * GetCameraManager(mgr).GetBallCamera()->GetTargetAnglePerSecond()));
                const float vertical =
                    CMath::AbsF(CMath::Limit(CVector3f::Dot(direction, CVector3f::Up()), 1.f));
                const float verticalStep = (12.566371f * dt) * (1.f - vertical);
                if (step.AsRadians() > verticalStep && !Player(mgr).IsMorphBallTransitioning() &&
                    vertical > 0.999f) {
                  step = CRelAngle::FromRadians(verticalStep);
                }
                const CQuaternion rotation =
                    CQuaternion::LookAt(CUnitVector3f(forward), CUnitVector3f(direction), step);
                SetTransform(rotation.BuildTransform4f() * GetTransform().GetRotation());
              }
              SetTranslation(position);
            }
          }
        }
      }
      break;
    }
    case CBallCamera::kBCB_Unknown5:
      SetTransform(CTransform4f::LookAt(hint->GetTranslation(), mTargetPosition));
      break;
    default:
      break;
    }
    if (info.GetOverrideFlags() & kOF_OverrideFov) {
      SetFovAndTarget(info.GetFov());
    }
  }
  SetTransform(ValidateCameraTransform(GetTransform(), oldXf, dt));
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
  CVector3f hintPlanar = hintDirection;
  hintPlanar.SetZ(0.f);
  if (hintDirection.IsMagnitudeSafe() && hintPlanar.IsMagnitudeSafe()) {
    hintDirection.Normalize();
    hintPlanar.Normalize();
  } else {
    hintDirection = CVector3f(0.f, 1.f, 0.f);
    hintPlanar = CVector3f(0.f, 1.f, 0.f);
  }
  CVector3f planar = direction;
  planar.SetZ(0.f);
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
  const CQuaternion zRotation = CQuaternion::ZRotation(CRelAngle::FromRadians(azimuth));
  CVector3f rotated = zRotation.Transform(hintPlanar);
  const CUnitVector3f axis(rotated.GetY(), -rotated.GetX(), 0.f, CUnitVector3f::kN_Yes);
  const CQuaternion attitudeRotation =
      CQuaternion::AxisAngle(axis, CRelAngle::FromRadians(-attitude));
  rotated = attitudeRotation.Transform(rotated);
  return rotated;
}

void CFixedCamera::SetScriptCameraId(TUniqueId uid) { mScriptCameraId = uid; }
