#include "MetroidPrime/Cameras/CGameCamera.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CBallCamera.hpp"
#include "MetroidPrime/Cameras/CCameraSpring.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

CGameCamera::SFovInterpolation::SFovInterpolation(float delay, float remaining, float duration,
                                                  float current, float target, TUniqueId cameraId)
: mDelay(delay)
, mRemaining(remaining)
, mDuration(duration)
, mCurrent(current)
, mTarget(target)
, mCameraId(cameraId) {}

void CGameCamera::SFovInterpolation::Set(float delay, float remaining, float duration,
                                         float current, float target, TUniqueId cameraId) {
  mDelay = delay;
  mRemaining = remaining;
  mDuration = duration;
  mCurrent = current;
  mTarget = target;
  mCameraId = cameraId;
}

CGameCamera::CGameCamera(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                         const CTransform4f& xf, float fov, float nearZ, float farZ, float aspect,
                         TUniqueId watchedId, int index, int controllerIdx)
: CActor(uid, name, info, 0, xf, CModelData::CModelDataNull(), CMaterialList(kMT_NoStepLogic),
         CActorParameters::None(), kInvalidUniqueId)
, mWatchedObject(watchedId)
, mPerspectiveMatrix(CMatrix4f::Identity())
, mOrigXf(xf)
, mZnear(nearZ)
, mZfar(farZ)
, mAspect(aspect)
, mInputIndex(index)
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
  if (mPerspDirty == true) {
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
  return m00 * (m13 * (m21 * m32 - m22 * m31) - m11 * (m23 * m32 - m22 * m33) +
                m12 * (m23 * m31 - m21 * m33)) -
         m01 * (m10 * (m22 * m33 - m23 * m32) - m12 * (m20 * m33 - m23 * m30) +
                m13 * (m20 * m32 - m22 * m30)) +
         m02 * (m10 * (m21 * m33 - m23 * m31) - m11 * (m20 * m33 - m23 * m30) +
                m13 * (m20 * m31 - m21 * m30)) -
         m03 * (m10 * (m21 * m32 - m22 * m31) - m11 * (m20 * m32 - m22 * m30) +
                m12 * (m20 * m31 - m21 * m30));
}

CMatrix4f CMatrix4f::GetInverse() const {
  const float invDet = 1.f / Determinant();
  // minorRC is the determinant of this matrix without row R and column C.
  const float minor00 =
      m13 * (m21 * m32 - m22 * m31) - m11 * (m23 * m32 - m22 * m33) + m12 * (m23 * m31 - m21 * m33);
  const float minor10 =
      m01 * (m22 * m33 - m23 * m32) - m02 * (m21 * m33 - m23 * m31) + m03 * (m21 * m32 - m22 * m31);
  const float minor20 =
      m01 * (m12 * m33 - m13 * m32) - m02 * (m11 * m33 - m13 * m31) + m03 * (m11 * m32 - m12 * m31);
  const float minor30 =
      m01 * (m12 * m23 - m13 * m22) - m02 * (m11 * m23 - m13 * m21) + m03 * (m11 * m22 - m12 * m21);
  const float minor01 =
      m10 * (m22 * m33 - m23 * m32) - m12 * (m20 * m33 - m23 * m30) + m13 * (m20 * m32 - m22 * m30);
  const float minor11 =
      m00 * (m22 * m33 - m23 * m32) - m02 * (m20 * m33 - m23 * m30) + m03 * (m20 * m32 - m22 * m30);
  const float minor21 =
      m00 * (m12 * m33 - m13 * m32) - m02 * (m10 * m33 - m13 * m30) + m03 * (m10 * m32 - m12 * m30);
  const float minor31 =
      m00 * (m12 * m23 - m13 * m22) - m02 * (m10 * m23 - m13 * m20) + m03 * (m10 * m22 - m12 * m20);
  const float minor02 =
      m10 * (m21 * m33 - m23 * m31) - m11 * (m20 * m33 - m23 * m30) + m13 * (m20 * m31 - m21 * m30);
  const float minor12 =
      m00 * (m21 * m33 - m23 * m31) - m01 * (m20 * m33 - m23 * m30) + m03 * (m20 * m31 - m21 * m30);
  const float minor22 =
      m00 * (m11 * m33 - m13 * m31) - m01 * (m10 * m33 - m13 * m30) + m03 * (m10 * m31 - m11 * m30);
  const float minor32 =
      m00 * (m11 * m23 - m13 * m21) - m01 * (m10 * m23 - m13 * m20) + m03 * (m10 * m21 - m11 * m20);
  const float minor03 =
      m10 * (m21 * m32 - m22 * m31) - m11 * (m20 * m32 - m22 * m30) + m12 * (m20 * m31 - m21 * m30);
  const float minor13 =
      m00 * (m21 * m32 - m22 * m31) - m01 * (m20 * m32 - m22 * m30) + m02 * (m20 * m31 - m21 * m30);
  const float minor23 =
      m00 * (m11 * m32 - m12 * m31) - m01 * (m10 * m32 - m12 * m30) + m02 * (m10 * m31 - m11 * m30);
  const float minor33 =
      m00 * (m11 * m22 - m12 * m21) - m01 * (m10 * m22 - m12 * m20) + m02 * (m10 * m21 - m11 * m20);

  return CMatrix4f(invDet * minor00, invDet * -minor10, invDet * minor20, invDet * -minor30,
                   invDet * -minor01, invDet * minor11, invDet * -minor21, invDet * minor31,
                   invDet * minor02, invDet * -minor12, invDet * minor22, invDet * -minor32,
                   invDet * -minor03, invDet * minor13, invDet * -minor23, invDet * minor33);
}

CVector3f CGameCamera::ConvertToWorldSpace(const CVector3f& position) const {
  const CVector3f viewPos = GetPerspectiveMatrix().GetInverse().MultiplyOneOverW(position);
  const CVector3f result = GetTransform() * viewPos;
  return result;
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

void CGameCamera::SetActive(const bool active) {
  CActor::SetActive(active);
  SetDrawEnabled(false);
}

CTransform4f CGameCamera::ValidateCameraTransform(const CTransform4f& newXf,
                                                  const CTransform4f& oldXf, float dt) {
  CTransform4f xf(newXf);
  if (!close_enough(newXf.GetColumn(kDX).Magnitude(), 1.f, FLT_EPSILON * 1000.f) ||
      !close_enough(newXf.GetColumn(kDY).Magnitude(), 1.f, FLT_EPSILON * 1000.f) ||
      !close_enough(newXf.GetColumn(kDZ).Magnitude(), 1.f, FLT_EPSILON * 1000.f)) {
    xf.Orthonormalize();
  }
  const float dot = CMath::Limit(CVector3f::Dot(newXf.GetColumn(kDY), CVector3f::Up()), 1.f);
  if (CMath::AbsF(dot) > 0.999f) {
    xf = oldXf;
  }
  CVector3f forward = xf.GetColumn(kDY);
  forward.SetZ(0.f);
  if (xf.GetColumn(kDZ).GetZ() < -0.2f) {
    if (forward.CanBeNormalized()) {
      xf = CTransform4f::LookAt(CUnitVector3f(CVector3f::Zero()), forward);
    } else {
      xf = oldXf;
    }
  }
  if (!close_enough(xf.GetColumn(kDX).GetZ(), 0.f, 0.01f) &&
      close_enough(xf.GetColumn(kDZ).GetZ(), 0.f, 0.01f)) {
    if (forward.IsMagnitudeSafe()) {
      xf = CTransform4f::LookAt(CUnitVector3f(CVector3f::Zero()), forward);
    } else {
      xf = oldXf;
    }
  }
  xf.SetTranslation(newXf.GetTranslation());
  return xf;
}

const CPlayer& CGameCamera::GetPlayer(const CStateManager& mgr) const {
  return *mgr.GetPlayer(mControllerIdx);
}

CPlayer& CGameCamera::Player(CStateManager& mgr) const { return *mgr.GetPlayer(mControllerIdx); }

const CCameraManager& CGameCamera::GetCameraManager(const CStateManager& mgr) const {
  return *mgr.GetCameraManager(mControllerIdx);
}

CCameraManager& CGameCamera::CameraManager(CStateManager& mgr) const {
  return *mgr.CameraManager(mControllerIdx);
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
  if (duration <= 0.f) {
    ResetFovInterpolation(fov);
  } else {
    mFovInterpolation.Set(delay, duration, duration, GetFov(), fov, kInvalidUniqueId);
  }
}

void CGameCamera::InterpolateFOV(float startFov, float duration, float delay, TUniqueId cameraId,
                                 CStateManager& mgr) {
  CGameCamera* camera =
      TCastToPtr< CGameCamera >(const_cast< CEntity* >(mgr.GetObjectById(cameraId)));
  if (camera != nullptr) {
    const float target = camera->GetFov();
    if (duration <= 0.f) {
      ResetFovInterpolation(target);
    } else {
      mFovInterpolation.Set(delay, duration, duration, startFov, target, cameraId);
    }
  }
}

void CGameCamera::UpdatePerspective(float dt, CStateManager& mgr) {
  if (mFovInterpolation.mDelay > 0.f) {
    mFovInterpolation.mDelay -= dt;
  } else if (mFovInterpolation.mRemaining > 0.f) {
    CGameCamera* camera = TCastToPtr< CGameCamera >(
        const_cast< CEntity* >(mgr.GetObjectById(mFovInterpolation.mCameraId)));
    if (camera != nullptr && camera->GetUniqueId() != GetUniqueId()) {
      SetTargetFov(camera->GetFov());
    }

    mFovInterpolation.mRemaining -= dt;
    if (mFovInterpolation.mRemaining <= 0.f) {
      SetFov(GetTargetFov());
    } else {
      const float delta = GetFov() - GetTargetFov();
      const float t =
          CMath::Clamp(0.f, mFovInterpolation.mRemaining / mFovInterpolation.mDuration, 1.f);
      SetFov(delta * t + GetTargetFov());
    }
  } else if (!(CMath::AbsF(GetFov() - GetTargetFov()) < 0.00001f)) {
    SetFov(GetTargetFov());
  }
}

CVector3f CGameCamera::GetScanObjectIndicatorPosition(const CStateManager& mgr) const {
  if (TCastToConstPtr< CPlayer >(mgr.GetObjectById(mWatchedObject))) {
    return GetCameraManager(mgr).BallCamera()->GetScanObjectIndicatorPosition(mgr);
  }
  const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mWatchedObject));
  if (actor == nullptr) {
    return GetCameraManager(mgr).BallCamera()->GetScanObjectIndicatorPosition(mgr);
  }
  return actor->GetScanObjectIndicatorPosition(mgr);
}

rstl::optional_object< CAABox > CGameCamera::GetTouchBounds() const {
  return CAABox(GetTranslation(), GetTranslation());
}

void CGameCamera::UnkVtable84(TUniqueId fluidId, CStateManager& mgr) {}

void CGameCamera::UnkVtable88(TUniqueId fluidId, CStateManager& mgr) {}

void CGameCamera::ClearFluidList(CStateManager& mgr) {
  const rstl::reserved_vector< TUniqueId, 4 > fluids = GetFluidList();
  for (int i = 0; i < fluids.size(); ++i) {
    if (CScriptWater* water = TCastToPtr< CScriptWater >(mgr.ObjectById(fluids[i]))) {
      water->RemoveInhabitant(GetUniqueId(), mgr);
    }
  }
  CActor::ClearFluidList(mgr);
}
