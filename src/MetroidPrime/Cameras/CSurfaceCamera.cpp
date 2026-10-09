#include "MetroidPrime/Cameras/CSurfaceCamera.hpp"

#include "Kyoto/Math/CRelAngle.hpp"

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

  CVector3f playerPosition = Player(mgr).GetBallPosition();
  CTransform4f xf = GetTransform();
  const CScriptSurfaceCamera* camera =
      TCastToConstPtr< CScriptSurfaceCamera >(mgr.ObjectById(mScriptCameraId));
  if (camera != nullptr && camera->GetSurface() != nullptr) {
    CCameraSurface* surface = camera->GetSurface();

    if (!(camera->GetFlags() & CScriptSurfaceCamera::kSF_OffsetAlongSurface)) {
      playerPosition += camera->GetPlayerOffset();
    }

    if (camera->GetFlags() & CScriptSurfaceCamera::kSF_FollowPlayerDirectly) {
      mTrackedPlayerPosition = playerPosition;
    } else {
      const CVector3f toTracked = mTrackedPlayerPosition - playerPosition;
      if (toTracked.CanBeNormalized()) {
        const float distance = toTracked.Magnitude();
        if (!CMath::IsEpsilon(distance, 0.1f, 1.0e-5f)) {
          const float spring = mPlayerPositionSpring.ApplyDistanceSpring(0.f, distance, dt);
          mTrackedPlayerPosition = playerPosition + spring * toTracked.AsNormalized();
        } else {
          mTrackedPlayerPosition = playerPosition;
        }
      }
    }

    CVector3f position = surface->GetSurfacePoint(mTrackedPlayerPosition);
    if (camera->GetFlags() & CScriptSurfaceCamera::kSF_OffsetAlongSurface) {
      switch (camera->GetSurfaceType()) {
      case CScriptSurfaceCamera::kST_Cylinder:
      case CScriptSurfaceCamera::kST_SplineCylinder: {
        position +=
            camera->GetTransform().Rotate(CVector3f(0.f, 0.f, camera->GetPlayerOffset().GetZ()));
        if (const CCylinderCameraSurface* cylinderSurface =
                static_cast< CCylinderCameraSurface* >(camera->GetSurface())) {
          const CCylinder cylinder = cylinderSurface->GetCylinder();
          const CVector3f axisPoint = cylinder.GetAxisPoint(position);
          const CVector3f radial = position - axisPoint;
          const float radius = radial.Magnitude();
          if (radius > 0.001f) {
            const float offsetX = camera->GetPlayerOffset().GetX();
            float angle = offsetX / radius;
            if (camera->GetFlags() & CScriptSurfaceCamera::kSF_OffsetIsDegrees) {
              angle = CRelAngle::FromDegrees(offsetX).AsRadians();
            }
            const CQuaternion rotation =
                CQuaternion::AxisAngle(cylinder.GetAxis().GetNormal(), CRelAngle::FromRadians(angle));
            position = surface->GetSurfacePoint(axisPoint + rotation.Transform(radial));
          }
        }
        break;
      }
      case CScriptSurfaceCamera::kST_Sphere:
        if (const CSphereCameraSurface* sphereSurface =
                static_cast< CSphereCameraSurface* >(camera->GetSurface())) {
          const CSphere sphere = sphereSurface->GetSphere();
          CVector3f axis = CVector3f::Zero();
          CVector3f center = sphere.GetCenter();
          float angle = 0.f;
          if (!CMath::IsEpsilon(camera->GetPlayerOffset().GetX(), 0.f, 1.0e-5f)) {
            axis = CVector3f::Up();
            angle = camera->GetPlayerOffset().GetX();
            center.SetZ(position.GetZ());
          }
          if (!CMath::IsEpsilon(camera->GetPlayerOffset().GetY(), 0.f, 1.0e-5f)) {
            axis = CVector3f::Forward();
            angle = camera->GetPlayerOffset().GetY();
            center.SetY(position.GetY());
          }
          if (!CMath::IsEpsilon(camera->GetPlayerOffset().GetZ(), 0.f, 1.0e-5f)) {
            axis = CVector3f::Right();
            angle = -camera->GetPlayerOffset().GetZ();
            center.SetX(position.GetX());
          }
          if (angle != 0.f) {
            const CVector3f radial = position - center;
            const float radius = radial.Magnitude();
            if (radius > 0.001f) {
              float theta = angle / radius;
              if (camera->GetFlags() & CScriptSurfaceCamera::kSF_OffsetIsDegrees) {
                theta = CRelAngle::FromDegrees(angle).AsRadians();
              }
              const CQuaternion rotation =
                  CQuaternion::AxisAngle(CUnitVector3f(axis), CRelAngle::FromRadians(theta));
              position = surface->GetSurfacePoint(center + rotation.Transform(radial));
            }
          }
        }
        break;
      default:
        position = surface->GetSurfacePoint(
            position + camera->GetTransform().Rotate(camera->GetPlayerOffset()));
        break;
      }
    }
    xf.SetTranslation(position);
  }

  const CVector3f ballPosition = Player(mgr).GetBallPosition();
  camera = TCastToConstPtr< CScriptSurfaceCamera >(mgr.GetObjectById(mScriptCameraId));
  if (camera) {
    const CMotionSpline& playerSpline = camera->GetPlayerSpline();
    if (playerSpline.GetControlPointCount() != 0) {
      mPlayerSplineDistance =
          playerSpline.FindClosestLengthOnSpline(mPlayerSplineDistance, ballPosition);
    }
    const CMotionSpline& targetSpline = camera->GetTargetSpline();
    if (targetSpline.GetControlPointCount() != 0) {
      mTargetSplineDistance =
          targetSpline.FindClosestLengthOnSpline(mTargetSplineDistance, ballPosition);
    }
  }

  const CVector3f lookTarget = GetScanObjectIndicatorPosition(mgr);
  CVector3f lookDirection = lookTarget - GetTranslation();
  lookDirection.SetZ(0.f);
  if (lookDirection.IsMagnitudeSafe()) {
    xf = CTransform4f::LookAt(xf.GetTranslation(), lookTarget, CVector3f::Up());
  }

  const CMayaSpline& fovSpline = camera->GetFovSpline();
  if (fovSpline.GetKnotCount() != 0) {
    if (camera->GetPlayerSpline().GetControlPointCount() != 0) {
      float progress = mPlayerSplineDistance / camera->GetPlayerSpline().GetLength();
      SetTargetFov(fovSpline.EvaluateAt(CMath::Clamp(0.f, progress, 1.f)));
    } else if (camera->GetTargetSpline().GetControlPointCount() != 0) {
      float progress = mTargetSplineDistance / camera->GetTargetSpline().GetLength();
      SetTargetFov(fovSpline.EvaluateAt(CMath::Clamp(0.f, progress, 1.f)));
    } else {
      SetTargetFov(fovSpline.EvaluateAt(0.f));
    }
  }

  if (camera->GetFlags() & CScriptSurfaceCamera::kSF_ProjectTargetAlongHintForward) {
    const CGameHint* hint = CameraManager(mgr).GetHintManager()->GetCurrentHint(mgr);
    if (hint) {
      const CVector3f forward = hint->GetTransform().GetForward();
      xf = CTransform4f::LookAt(xf.GetTranslation(), xf.GetTranslation() + forward,
                                CVector3f::Up());
    }
  }

  SetTransform(xf);
  CActor::Think(dt, mgr);
}

void CSurfaceCamera::ProcessInput(const CFinalInput& input, CStateManager& mgr) {}

void CSurfaceCamera::Render(const CStateManager& mgr) const {}

void CSurfaceCamera::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CGameCamera::AcceptScriptMsg(mgr, msg);
}

CVector3f CSurfaceCamera::GetScanObjectIndicatorPosition(const CStateManager& mgr) const {
  // The native helpers used here take a mutable manager.
  CStateManager& stateMgr = const_cast< CStateManager& >(mgr);
  CVector3f target = CameraManager(stateMgr).GetBallCamera()->GetScanObjectIndicatorPosition(mgr);
  const CScriptSurfaceCamera* camera =
      TCastToConstPtr< CScriptSurfaceCamera >(mgr.GetObjectById(mScriptCameraId));
  if (camera) {
    if (camera->GetFlags() & CScriptSurfaceCamera::kSF_TargetBallPosition) {
      return Player(stateMgr).GetBallPosition();
    }
    const CMotionSpline& targetSpline = camera->GetTargetSpline();
    if (targetSpline.GetControlPointCount() != 0) {
      float progress = CMath::Clamp(0.f, mTargetSplineDistance / targetSpline.GetLength(), 1.f);
      if (camera->GetPlayerSpline().GetControlPointCount() != 0) {
        progress =
            CMath::Clamp(0.f, mPlayerSplineDistance / camera->GetPlayerSpline().GetLength(), 1.f);
      }
      const float time = camera->GetTargetControlSpline().EvaluateAt(progress);
      target = targetSpline.GetPositionByTime(time * targetSpline.GetDuration());
    } else if (const CActor* actor =
                   TCastToConstPtr< CActor >(mgr.GetObjectById(camera->GetTargetId()))) {
      target = actor->GetTranslation();
    }
    if (camera->GetFlags() & CScriptSurfaceCamera::kSF_ProjectTargetAlongHintForward) {
      const CGameHint* hint = CameraManager(stateMgr).GetHintManager()->GetCurrentHint(mgr);
      if (hint) {
        const CVector3f direction = hint->GetTransform().GetForward();
        const CVector3f delta = Player(stateMgr).GetBallPosition() - GetTranslation();
        target = GetTranslation() + CVector3f::Dot(direction, delta) * direction;
      }
    }
  }
  return target;
}
