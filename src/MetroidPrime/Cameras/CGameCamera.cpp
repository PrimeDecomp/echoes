#include "MetroidPrime/Cameras/CGameCamera.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCameraSpring.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CMath.hpp"

CGameCamera::CGameCamera(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                         const CTransform4f& xf, float fov, float nearZ, float farZ, float aspect,
                         TUniqueId watchedId, int index, int controllerIdx)
: CActor(uid, name, info, 0, xf, CModelData(), CMaterialList(kMT_NoStepLogic), CActorParameters(),
         kInvalidUniqueId)
, mWatchedObject(watchedId)
, mPerspectiveMatrix(CMatrix4f::Identity())
, mOrigXf(xf)
, mZnear(nearZ)
, mZfar(farZ)
, mAspect(aspect)
, x1d8_(index)
, mControllerIdx(controllerIdx)
, mFovInterpolation(0.f, 0.f, 0.f, fov, fov, kInvalidUniqueId)
, mPerspDirty(true) {
  SetDrawEnabled(false);
}

CGameCamera::~CGameCamera() {}

void CGameCamera::SetAspectRatio(float aspect) {
  mAspect = aspect;
  mPerspDirty = true;
}

const CMatrix4f& CGameCamera::GetPerspectiveMatrix() const {
  if (mPerspDirty) {
    mPerspectiveMatrix = CGraphics::CalculatePerspectiveMatrix(GetFov(), mAspect, mZnear, mZfar);
    mPerspDirty = false;
  }
  return mPerspectiveMatrix;
}

CVector3f CGameCamera::ConvertToScreenSpace(const CVector3f& position) const {
  const CVector3f local = GetTransform().TransposeMultiply(position);
  if (local.IsNonZero()) {
    return GetPerspectiveMatrix().MultiplyOneOverW(local);
  }
  return CVector3f(-1.f, -1.f, 1.f);
}

float CMatrix4f::Determinant() const {
  const float a = m20 * m31 - m21 * m30;
  const float b = m20 * m32 - m22 * m30;
  const float c = m20 * m33 - m23 * m30;
  const float d = m21 * m32 - m22 * m31;
  const float e = m21 * m33 - m23 * m31;
  const float f = m22 * m33 - m23 * m32;

  return m00 * (m11 * f - m12 * e + m13 * d) - m01 * (m10 * f - m12 * c + m13 * b) +
         m02 * (m10 * e - m11 * c + m13 * a) - m03 * (m10 * d - m11 * b + m12 * a);
}

CMatrix4f CMatrix4f::GetInverse() const {
  // Two-by-two minors for the adjugate matrix.
  const float a = m20 * m31 - m21 * m30;
  const float b = m20 * m32 - m22 * m30;
  const float c = m20 * m33 - m23 * m30;
  const float d = m21 * m32 - m22 * m31;
  const float e = m21 * m33 - m23 * m31;
  const float f = m22 * m33 - m23 * m32;
  const float g = m10 * m31 - m11 * m30;
  const float h = m10 * m32 - m12 * m30;
  const float i = m10 * m33 - m13 * m30;
  const float j = m11 * m32 - m12 * m31;
  const float k = m11 * m33 - m13 * m31;
  const float l = m12 * m33 - m13 * m32;
  const float m = m10 * m21 - m11 * m20;
  const float n = m10 * m22 - m12 * m20;
  const float o = m10 * m23 - m13 * m20;
  const float p = m11 * m22 - m12 * m21;
  const float q = m11 * m23 - m13 * m21;
  const float r = m12 * m23 - m13 * m22;
  const float invDet = 1.f / Determinant();

  return CMatrix4f(invDet * (m11 * f - m12 * e + m13 * d), -invDet * (m01 * f - m02 * e + m03 * d),
                   invDet * (m01 * l - m02 * k + m03 * j), -invDet * (m01 * r - m02 * q + m03 * p),
                   -invDet * (m10 * f - m12 * c + m13 * b), invDet * (m00 * f - m02 * c + m03 * b),
                   -invDet * (m00 * l - m02 * i + m03 * h), invDet * (m00 * r - m02 * o + m03 * n),
                   invDet * (m10 * e - m11 * c + m13 * a), -invDet * (m00 * e - m01 * c + m03 * a),
                   invDet * (m00 * k - m01 * i + m03 * g), -invDet * (m00 * q - m01 * o + m03 * m),
                   -invDet * (m10 * d - m11 * b + m12 * a), invDet * (m00 * d - m01 * b + m02 * a),
                   -invDet * (m00 * j - m01 * h + m02 * g), invDet * (m00 * p - m01 * n + m02 * m));
}

CVector3f CGameCamera::ConvertToWorldSpace(const CVector3f& position) const {
  return GetTransform() * GetPerspectiveMatrix().GetInverse().MultiplyOneOverW(position);
}

float CCameraSpring::ApplyDistanceSpring(float target, float current, float dt) {
  float result = current + mTardis * (mDx * dt);
  const float acceleration = mK * (target - current) - mK2Sqrt * mDx;
  mDx += mTardis * (acceleration * dt);

  if (result < target) {
    result = target;
  }
  if (result - target > mMax) {
    result = target + mMax;
  }
  return result;
}

void CCameraSpring::Reset() {
  mK2Sqrt = 2.f * CMath::SqrtF(mK);
  mDx = 0.f;
}

void CGameCamera::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CActor::AcceptScriptMsg(mgr, msg);
}

void CGameCamera::SetActive(bool active) {
  CActor::SetActive(active);
  SetDrawEnabled(false);
}

CTransform4f CGameCamera::ValidateCameraTransform(const CTransform4f& newXf,
                                                  const CTransform4f& oldXf) {
  // TODO: Recover orthonormalization and the Echoes-specific horizon/inversion corrections.
  return newXf;
}

CPlayer& CGameCamera::Player(CStateManager& mgr) const { return *mgr.GetPlayer(mControllerIdx); }

const CPlayer& CGameCamera::GetPlayer(const CStateManager& mgr) const {
  return *mgr.GetPlayer(mControllerIdx);
}

CCameraManager& CGameCamera::CameraManager(CStateManager& mgr) const {
  return *mgr.CameraManager(mControllerIdx);
}

const CCameraManager& CGameCamera::GetCameraManager(const CStateManager& mgr) const {
  return *mgr.GetCameraManager(mControllerIdx);
}

void CGameCamera::SetTargetFov(float fov) {
  mFovInterpolation.mTarget = fov;
  mPerspDirty = true;
}

float CGameCamera::GetTargetFov() const { return mFovInterpolation.mTarget; }

void CGameCamera::SetFov(float fov) {
  mFovInterpolation.mCurrent = fov;
  mPerspDirty = true;
}

float CGameCamera::GetFov() const { return mFovInterpolation.mCurrent; }

void CGameCamera::SetFovAndTarget(float fov) {
  mFovInterpolation.mCurrent = fov;
  mFovInterpolation.mTarget = fov;
  mPerspDirty = true;
}

void CGameCamera::ResetFovInterpolation(float fov) {
  mFovInterpolation.Set(0.f, 0.f, 0.f, fov, fov, kInvalidUniqueId);
  mPerspDirty = true;
}

void CGameCamera::InterpolateFOV(float fov, float duration, float delay) {
  if (duration > 0.f) {
    mFovInterpolation.Set(delay, duration, duration, GetFov(), fov, kInvalidUniqueId);
  } else {
    ResetFovInterpolation(fov);
  }
}

void CGameCamera::InterpolateFOV(float startFov, float duration, float delay, TUniqueId cameraId,
                                 CStateManager& mgr) {
  CGameCamera* camera = TCastToPtr< CGameCamera >(mgr.ObjectById(cameraId));
  if (camera != nullptr) {
    const float target = camera->GetFov();
    if (duration > 0.f) {
      mFovInterpolation.Set(delay, duration, duration, startFov, target, cameraId);
    } else {
      ResetFovInterpolation(target);
    }
  }
}

void CGameCamera::UpdatePerspective(float dt, CStateManager& mgr) {
  if (mFovInterpolation.mDelay > 0.f) {
    mFovInterpolation.mDelay -= dt;
  } else if (mFovInterpolation.mRemaining > 0.f) {
    CGameCamera* camera = TCastToPtr< CGameCamera >(mgr.ObjectById(mFovInterpolation.mCameraId));
    if (camera != nullptr && camera->GetUniqueId() != GetUniqueId()) {
      SetTargetFov(camera->GetFov());
    }

    mFovInterpolation.mRemaining -= dt;
    if (mFovInterpolation.mRemaining > 0.f) {
      const float t =
          CMath::Clamp(0.f, mFovInterpolation.mRemaining / mFovInterpolation.mDuration, 1.f);
      SetFov((GetFov() - GetTargetFov()) * t + GetTargetFov());
    } else {
      SetFov(GetTargetFov());
    }
  } else if (CMath::AbsF(GetFov() - GetTargetFov()) >= 0.00001f) {
    SetFov(GetTargetFov());
  }
}

CVector3f CGameCamera::GetScanObjectIndicatorPosition(const CStateManager& mgr) const {
  // TODO: Use the watched actor's target position, falling back to the player's ball camera.
  return GetTranslation();
}

rstl::optional_object< CAABox > CGameCamera::GetTouchBounds() const {
  return CAABox(GetTranslation(), GetTranslation());
}

void CGameCamera::UnkVtable84() {}

void CGameCamera::UnkVtable88() {}

void CGameCamera::UnkVtable20(CStateManager& mgr) {
  // TODO: Notify the camera's overlapping triggers before the inherited actor cleanup.
  CActor::UnkVtable20(mgr);
}
