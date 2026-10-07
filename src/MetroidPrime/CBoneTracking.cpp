#include "MetroidPrime/CBoneTracking.hpp"

#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/Animation/CPoseAsTransforms_Linear.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"

CBoneTracking::CBoneTracking(const CAnimData& animData, const rstl::string& bone,
                             float maxTrackingAngle, float angSpeed, uint flags)
: mRotation(CQuaternion::NoRotation())
, x10_(0.f)
, mSegId(animData.GetCharLayoutInfo()->GetSegIdFromString(bone))
, mTime(0.f)
, mMaxTrackingAngle(maxTrackingAngle)
, mAngSpeed(angSpeed)
, mDisableTrackingDistanceSquared(1000000.f)
, mTarget(kInvalidUniqueId)
, mActive(false)
, mHasTrackedRotation(false)
, mPreRendered(false)
, mNoParent(flags & kBTF_NoParent)
, mNoParentOrigin(flags & kBTF_NoParentOrigin)
, mNoHorizontalAim(flags & kBTF_NoHorizontalAim)
, mParentIk(flags & kBTF_ParentIk) {}

void CBoneTracking::PreThink(CAnimData& animData) { animData.SetPoseBuilt(false); }

void CBoneTracking::Think(float dt) {
  mTime += dt;
  mPreRendered = false;
}

void CBoneTracking::PreRender(const CStateManager& mgr, CAnimData& animData, const CTransform4f& xf,
                              const CVector3f& scale, const CBodyController& controller) {
  if (mPreRendered) {
    return;
  }

  const CPatterned* patterned = TCastToConstPtr< CPatterned >(controller.GetOwner());
  PreRender(mgr, animData, xf, scale,
            controller.GetBodyStateInfo().ApplyHeadTracking() &&
                (!patterned || patterned->ApplyBoneTracking()));
}

void CBoneTracking::PreRender(const CStateManager& mgr, CAnimData& animData, const CTransform4f& xf,
                              const CVector3f& scale, bool tracking) {
  mPreRendered = true;
  if (mSegId != CSegId::Null()) {
    CPoseAsTransforms_Linear& pose = animData.Pose();
    const CCharLayoutInfo& layout = *animData.GetCharLayoutInfo();
    const CActor* target = TCastToConstPtr< CActor >(mgr.GetObjectById(mTarget));
    if (mActive && tracking && (target || mTargetPosition.valid())) {
      mHasTrackedRotation = true;
      const CVector3f targetPosition = target ? target->GetAimPosition(mgr, 0.f) : *mTargetPosition;
      const CVector3f delta = targetPosition - xf.GetTranslation();
      if (delta.MagSquared() <= mDisableTrackingDistanceSquared) {
        UpdateTracking(xf, scale, targetPosition, layout, pose);
      } else {
        UpdateInactive(layout, pose);
      }
    } else {
      UpdateInactive(layout, pose);
    }
  }
  mTime = 0.f;
}

void CBoneTracking::SetActive(bool active) { mActive = active; }

void CBoneTracking::SetTarget(TUniqueId target) { mTarget = target; }

void CBoneTracking::SetDisableTrackingDistance(float distance) {
  mDisableTrackingDistanceSquared = distance * distance;
}

void CBoneTracking::SetTargetPosition(const CVector3f& target) { mTargetPosition = target; }

void CBoneTracking::SetMaxBoneRotation(float angle) { mMaxTrackingAngle = angle; }

void CBoneTracking::UpdateTracking(const CTransform4f& xf, const CVector3f& scale,
                                   const CVector3f& targetPosition, const CCharLayoutInfo& layout,
                                   CPoseAsTransforms_Linear& pose) {
  const CSegId parent = mNoParent ? mSegId : layout.GetOriginalParent(mSegId);
  const CTransform4f parentXf = pose.GetTransform(parent);
  const CTransform4f boneXf = pose.GetTransform(mSegId);
  CTransform4f trackingXf = parentXf;
  if (mNoParentOrigin && !mNoParent) {
    trackingXf.SetTranslation(boneXf.GetTranslation());
  }
  trackingXf.SetTranslation(CVector3f::ByElementMultiply(scale, trackingXf.GetTranslation()));

  const CTransform4f finalXf = xf * trackingXf;
  CVector3f localDir = finalXf.TransposeMultiply(targetPosition).AsNormalized();
  if (mNoHorizontalAim) {
    const float horizontalMagnitude =
        CMath::SqrtF(localDir.GetX() * localDir.GetX() + localDir.GetY() * localDir.GetY());
    localDir = CVector3f(0.f, horizontalMagnitude, localDir.GetZ());
  }
  if (mParentIk) {
    const float negativeElevation = -trackingXf.GetForward().GetZ();
    const CVector3f ikBase(0.f, CMath::SqrtF(1.f - negativeElevation * negativeElevation),
                           negativeElevation);
    const float angle = CMath::Min(CVector3f::GetAngleDiff(ikBase, localDir), mMaxTrackingAngle);
    localDir = CVector3f::Slerp(ikBase, localDir, CRelAngle::FromRadians(angle));
  } else {
    const float angle =
        CMath::Min(CVector3f::GetAngleDiff(CVector3f::Forward(), localDir), mMaxTrackingAngle);
    localDir = CVector3f::Slerp(CVector3f::Forward(), localDir, CRelAngle::FromRadians(angle));
  }

  const CVector3f currentDir = mRotation.Transform(CVector3f::Forward());
  const float angle = CVector3f::GetAngleDiff(currentDir, localDir);
  const float clampedAngle = CMath::Min(angle, mTime * mAngSpeed);
  if (clampedAngle > 1.e-5f) {
    const CQuaternion rotation =
        CQuaternion::LookAt(CUnitVector3f(CVector3f::Forward()), CUnitVector3f(localDir),
                            CRelAngle::FromDegrees(360.f));
    mRotation = CQuaternion::SlerpLocal(mRotation, rotation, clampedAngle / angle);
  }
  mRotation = mRotation.BuildNormalized();
  pose.SetRotation(layout, mSegId, parentXf.BuildMatrix3f() * mRotation.BuildTransform());
}

void CBoneTracking::UpdateInactive(const CCharLayoutInfo& layout, CPoseAsTransforms_Linear& pose) {
  if (mHasTrackedRotation) {
    const CSegId parent = mNoParent ? mSegId : layout.GetOriginalParent(mSegId);
    const CMatrix3f parentMatrix = pose.GetRotation(parent);
    const CMatrix3f boneMatrix = pose.GetRotation(mSegId);
    CQuaternion parentRotation = CQuaternion::FromMatrix(parentMatrix).BuildNormalized();
    CQuaternion boneRotation = CQuaternion::FromMatrix(boneMatrix).BuildNormalized();
    const CQuaternion animationRotation =
        mNoParent ? CQuaternion::NoRotation() : boneRotation * parentRotation.BuildInverted();
    const CVector3f currentDir = mRotation.Transform(CVector3f::Forward());
    const CVector3f animationDir = animationRotation.Transform(CVector3f::Forward());
    const float angle = CVector3f::GetAngleDiff(currentDir, animationDir);
    const float maxAngleDelta = mTime * mAngSpeed;
    const float clampedAngle = CMath::Min(angle, maxAngleDelta);
    if (clampedAngle <= 0.5f * maxAngleDelta) {
      mHasTrackedRotation = false;
      mRotation = animationRotation;
    } else {
      mRotation = CQuaternion::SlerpLocal(mRotation, animationRotation, clampedAngle / angle);
    }
    mRotation = mRotation.BuildNormalized();
    pose.SetRotation(layout, mSegId, (parentRotation * mRotation).BuildTransform());
  } else {
    const CSegId parent = mNoParent ? mSegId : layout.GetOriginalParent(mSegId);
    const CMatrix3f parentMatrix = pose.GetRotation(parent);
    const CMatrix3f boneMatrix = pose.GetRotation(mSegId);
    const CQuaternion parentRotation = CQuaternion::FromMatrix(parentMatrix);
    const CQuaternion boneRotation = CQuaternion::FromMatrix(boneMatrix);
    mRotation =
        mNoParent ? CQuaternion::NoRotation() : boneRotation * parentRotation.BuildInverted();
    mRotation = mRotation.BuildNormalized();
  }
}
