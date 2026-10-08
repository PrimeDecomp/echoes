#include "MetroidPrime/CRagDoll.hpp"

#include "Collision/CCollidableSphere.hpp"
#include "Collision/CCollisionInfo.hpp"
#include "Collision/CollisionUtil.hpp"
#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/Animation/CJointData_LinearStorage.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "WorldFormat/CMetroidAreaCollider.hpp"
#include "rstl/math.hpp"

namespace {
// Guessed names for the material response settings.
struct SMaterialResponse {
  SMaterialResponse(EMaterialTypes material, float restitution, float damping)
  : mMaterial(material), mRestitution(restitution), mDamping(damping) {}

  EMaterialTypes mMaterial;
  float mRestitution;
  float mDamping;
};

float sRestitution[32];
float sDamping[32];
CMaterialList sResponseMaterials;
SMaterialResponse sMaterialResponses[] = {
    SMaterialResponse(kMT_Stone, 0.9f, 0.8f),     SMaterialResponse(kMT_Metal, 1.f, 0.7f),
    SMaterialResponse(kMT_Grass, 0.2f, 0.3f),     SMaterialResponse(kMT_Phazon, 0.1f, 0.2f),
    SMaterialResponse(kMT_Dirt, 0.3f, 0.3f),      SMaterialResponse(kMT_MudSlow, 0.2f, 0.6f),
    SMaterialResponse(kMT_Sand, 0.1f, 0.2f),      SMaterialResponse(kMT_Unknown18, 0.1f, 0.1f),
    SMaterialResponse(kMT_Unknown19, 0.1f, 0.1f), SMaterialResponse(kMT_Wood, 0.8f, 0.8f),
    SMaterialResponse(kMT_Organic, 0.3f, 0.8f),   SMaterialResponse(kMT_Unknown25, 5.f, 0.2f),
};
bool sMaterialsInitialized;

int GetMaterialIndex(const CMaterialList& material) {
  return CMath::FloorLog2(static_cast< uint >(material.GetValue() & sResponseMaterials.GetValue()));
}

void InitializeMaterialResponses() {
  if (sMaterialsInitialized) {
    return;
  }
  for (int i = 0; i < 32; ++i) {
    sDamping[i] = sRestitution[i] = 1.f;
  }
  for (uint i = 0; i < ARRAY_SIZE(sMaterialResponses); ++i) {
    sResponseMaterials.Add(sMaterialResponses[i].mMaterial);
  }
  for (uint i = 0; i < ARRAY_SIZE(sMaterialResponses); ++i) {
    const SMaterialResponse& response = sMaterialResponses[i];
    int index = GetMaterialIndex(CMaterialList(response.mMaterial));
    sDamping[index] = response.mDamping;
    sRestitution[index] = response.mRestitution;
  }
  sMaterialsInitialized = true;
}
} // namespace

CRagDoll::CRagDoll(float normalGravity, float floatingGravity, float overTime, float damping,
                   float restitution, uint flags)
: mNormalGravity(normalGravity)
, mFloatingGravity(floatingGravity)
, mDamping(damping)
, mRestitution(restitution)
, mAngTimer(0.f)
, mRenderBounds(CAABox::MakeMaxInvertedBox())
, mRenderBoundsValid(false)
, mPrimed(false)
, mContinueSmallMovements((flags & 1) != 0)
, mNoAiCollision((flags & 4) != 0)
, mHitByProjectile(false)
, mNoOverTimer((flags & 2) != 0)
, mPrevMovingSlowly(false)
, mOver(false)
, mOverTimer(overTime)
, mImpactCount(0)
, mStaticSpeedThreshold(0.5f)
, mImpactVel(0.f)
, mAverageVel(CVector3f::Zero()) {
  InitializeMaterialResponses();
}

void CRagDoll::SatisfyWorldConstraintsOnConstruction(CStateManager& mgr) {
  for (int i = 0; i < mParticles.size(); ++i) {
    mParticles[i].mImpactPending = true;
  }
  SatisfyWorldConstraints(mgr, 2);
  for (int i = 0; i < mParticles.size(); ++i) {
    mParticles[i].mPrevPos = mParticles[i].mCurPos;
  }
}

void CRagDoll::Prime(CStateManager& mgr, const CTransform4f& xf, CModelData& modelData) {
  const CVector3f scale = modelData.GetScale();
  CAnimData* animData = modelData.AnimationData();
  animData->BuildPose();
  for (int i = 0; i < mParticles.size(); ++i) {
    CSegId id = mParticles[i].GetBone();
    if (id != CSegId::Invalid()) {
      mParticles[i].mCurPos =
          xf * CVector3f::ByElementMultiply(scale, animData->Pose().GetOffset(id));
    }
  }
  SatisfyWorldConstraints(mgr, 2);
  for (int i = 0; i < mParticles.size(); ++i) {
    mParticles[i].mImpactPending = false;
  }
  mPrimed = true;
}

void CRagDoll::Verlet(float dt) {
  for (int i = 0; i < mParticles.size(); ++i) {
    CRagDollParticle& particle = mParticles[i];
    CVector3f oldPos = particle.mCurPos;
    particle.mCurPos += particle.mDamping * (particle.mCurPos - particle.mPrevPos);
    particle.mCurPos += dt * (dt * mParticles[i].mAcceleration);
    particle.mCurPos += mParticles[i].mImpactResponseDelta;
    particle.mPrevPos = oldPos;
    const CVector3f delta = particle.mCurPos - particle.mPrevPos;
    if (delta.MagSquared() > 4.f) {
      particle.mCurPos = particle.mPrevPos + 2.f * delta.AsNormalized();
    }
    mParticles[i].mImpactPending = false;
    mParticles[i].mDamping = 1.f;
    mParticles[i].mImpactResponseDelta = CVector3f::Zero();
  }
}

void CRagDoll::AccumulateForces(float dt, float waterTop) {
  float inverseDt = 1.f / dt;
  mAngTimer += dt;
  if (mAngTimer > 4.f) {
    mAngTimer -= 4.f;
  }
  float targetZ = 0.1f * CMath::FastSinR(1.5707964f * mAngTimer) + (waterTop - 0.2f);
  CVector3f centerOfVolume = CVector3f::Zero();
  float totalVolume = 0.f;
  for (int i = 0; i < mParticles.size(); ++i) {
    CRagDollParticle& particle = mParticles[i];
    float volume = particle.mRadius * particle.mRadius * particle.mRadius;
    totalVolume += volume;
    centerOfVolume += volume * particle.mCurPos;
    float fromTargetZ = particle.mCurPos.GetZ() - targetZ;
    float verticalAcc = mFloatingGravity;
    float airFraction = 0.f;
    if (CMath::AbsF(fromTargetZ) < 0.5f) {
      airFraction = 0.5f * fromTargetZ / 0.5f + 0.5f;
      verticalAcc *= -fromTargetZ / 0.5f;
    } else if (fromTargetZ > 0.f) {
      verticalAcc = mNormalGravity;
      airFraction = 1.f;
    }
    particle.mAcceleration[kDZ] += verticalAcc;
    CVector3f velocity = inverseDt * (particle.mCurPos - particle.mPrevPos);
    float speed = velocity.Magnitude();
    if (speed > FLT_EPSILON) {
      CVector3f direction = (1.f / speed) * velocity;
      float inverseMass = 1.f / (8000.f * particle.mRadius);
      float drag = (0.75f * inverseMass) * (1.2f * airFraction + 1000.f * (1.f - airFraction));
      float acceleration = speed * (speed * drag) + (3000.f * inverseMass) * CMath::SqrtF(speed);
      particle.mAcceleration -= acceleration * direction;
    }
  }
  float inverseVolume = 1.f / totalVolume;
  CVector3f averageTorque = CVector3f::Zero();
  centerOfVolume *= inverseVolume;
  for (int i = 0; i < mParticles.size(); ++i) {
    CRagDollParticle& particle = mParticles[i];
    float volume = particle.mRadius * particle.mRadius * particle.mRadius;
    averageTorque += volume * CVector3f::Cross(particle.mCurPos - centerOfVolume,
                                               particle.mCurPos - particle.mPrevPos);
  }
  averageTorque *= inverseDt * inverseVolume;
  if (averageTorque.CanBeNormalized()) {
    for (int i = 0; i < mParticles.size(); ++i) {
      CRagDollParticle& particle = mParticles[i];
      particle.mAcceleration -=
          25.f * CVector3f::Cross(averageTorque, particle.mCurPos - centerOfVolume);
    }
  }
}

bool CRagDoll::SatisfyWorldConstraints(CStateManager& mgr, int pass) {
  CAABox bounds = CAABox::MakeMaxInvertedBox();
  for (int i = 0; i < mParticles.size(); ++i) {
    if (pass == 1 || mParticles[i].mImpactPending) {
      float radius = mParticles[i].mRadius;
      CVector3f extent(radius, radius, radius);
      bounds.AccumulateBounds(mParticles[i].mPrevPos - extent);
      bounds.AccumulateBounds(mParticles[i].mPrevPos + extent);
      bounds.AccumulateBounds(mParticles[i].mCurPos - extent);
      bounds.AccumulateBounds(mParticles[i].mCurPos + extent);
    }
  }
  CAreaCollisionCache cache(bounds);
  CGameCollision::BuildAreaCollisionCache(mgr, cache);
  bool needsSecondPass = false;
  TUniqueId bestId = kInvalidUniqueId;
  CMaterialList include =
      mNoAiCollision ? CMaterialList(kMT_Solid) : CMaterialList(kMT_Solid, kMT_AIBlock);
  CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
      include, mNoAiCollision ? CMaterialList(kMT_Character, kMT_Player, kMT_AIBlock, kMT_Occluder)
                              : CMaterialList(kMT_Character, kMT_Player));
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  mgr.BuildNearList(nearList, bounds, filter, nullptr);
  for (int i = 0; i < mParticles.size(); ++i) {
    CRagDollParticle& particle = mParticles[i];
    if (pass == 1 || particle.mImpactPending) {
      CVector3f delta = particle.mCurPos - particle.mPrevPos;
      float magnitude = delta.Magnitude();
      if (magnitude > 0.0001f) {
        delta *= 1.f / magnitude;
        CSphere sphere(particle.mPrevPos, particle.mRadius);
        double distance = magnitude;
        CCollisionInfo info;
        CGameCollision::DetectCollision_Cached_Moving(
            mgr, cache, CCollidableSphere(sphere, include), CTransform4f::Identity(), filter,
            nearList, delta, bestId, info, distance);
        if (info.IsValid()) {
          needsSecondPass = true;
          switch (pass) {
          case 1: {
            int material = GetMaterialIndex(info.GetMaterialRight());
            float restitution = mRestitution * sRestitution[material];
            particle.mImpactPending = true;
            particle.mDamping = mDamping * sDamping[material];
            float dot = CVector3f::Dot(delta, info.GetNormalLeft());
            particle.mImpactFrameVel = -dot * magnitude;
            particle.mImpactResponseDelta = magnitude * (-restitution * dot) * info.GetNormalLeft();
            float penetration = (magnitude - static_cast< float >(distance)) * dot;
            particle.mCurPos += (0.0001f - penetration) * info.GetNormalLeft();
            break;
          }
          case 2:
            particle.mCurPos = particle.mPrevPos + static_cast< float >(distance - 0.0001) * delta;
            break;
          }
        }
      } else if (!mContinueSmallMovements) {
        particle.mCurPos = particle.mPrevPos;
      }
    }
  }
  return needsSecondPass;
}

void CRagDoll::SatisfyConstraints(CStateManager& mgr) {
  int i;
  for (i = 0; i < mLengthConstraints.size(); ++i) {
    mLengthConstraints[i].Update();
  }
  for (i = 0; i < mJointConstraints.size(); ++i) {
    mJointConstraints[i].Update();
  }
  for (i = 0; i < mPlaneConstraints.size(); ++i) {
    mPlaneConstraints[i].Update();
  }
  for (i = 0; i < mKneeConstraints.size(); ++i) {
    mKneeConstraints[i].Update();
  }
  if (SatisfyWorldConstraints(mgr, 1)) {
    SatisfyWorldConstraints(mgr, 2);
  }
}

void CRagDoll::ClearForces() {
  for (int i = 0; i < mParticles.size(); ++i) {
    mParticles[i].mAcceleration = CVector3f::Zero();
  }
}

void CRagDoll::CheckStatic(float dt) {
  mImpactCount = 0;
  mImpactVel = 0.f;
  float staticDistance = mStaticSpeedThreshold * dt;
  float threshold = staticDistance * staticDistance;
  mAverageVel = CVector3f::Zero();
  bool movingSlowly = true;
  for (int i = 0; i < mParticles.size(); ++i) {
    CVector3f delta = mParticles[i].mCurPos - mParticles[i].mPrevPos;
    mAverageVel += delta;
    if (delta.MagSquared() > threshold) {
      movingSlowly = false;
    }
    if (mParticles[i].mImpactPending) {
      ++mImpactCount;
      mImpactVel = rstl::max_val(mImpactVel, mParticles[i].mImpactFrameVel);
    }
    mParticles[i].mImpactFrameVel = 0.f;
  }
  if (!mParticles.empty()) {
    mAverageVel *= 1.f / (dt * mParticles.size());
  }
  mImpactVel /= dt;
  if (!mNoOverTimer) {
    mOverTimer -= dt;
    if (mOverTimer <= 0.f) {
      mOver = true;
    }
  }
  if (movingSlowly && mPrevMovingSlowly) {
    mOver = true;
  }
  mPrevMovingSlowly = movingSlowly;
}

void CRagDoll::Update(CStateManager& mgr, float dt, float waterTop) {
  if (!IsOver() || WillContinueSmallMovements()) {
    AccumulateForces(dt, waterTop);
    Verlet(dt);
    SatisfyConstraints(mgr);
    ClearForces();
    CheckStatic(dt);
    UpdateRenderBounds();
  }
  mHitByProjectile = false;
}

void CRagDoll::CRagDollLengthConstraint::Update() {
  CVector3f& p1 = mP1->Position();
  CVector3f& p2 = mP2->Position();
  const CVector3f delta = p2 - p1;
  float magSquared = delta.MagSquared();
  float lengthSquared = mLength * mLength;
  bool solve = true;
  switch (mInequality) {
  case kI_Minimum:
    solve = magSquared < lengthSquared;
    break;
  case kI_Maximum:
    solve = magSquared > lengthSquared;
    break;
  }
  if (solve) {
    const CVector3f correction = delta * (lengthSquared / (magSquared + lengthSquared) - 0.5f);
    p1 -= correction;
    p2 += correction;
  }
}

void CRagDoll::CRagDollJointConstraint::Update() {
  const CVector3f plane = CVector3f::Cross(mP3->GetPosition() - mP1->GetPosition(),
                                           mP2->GetPosition() - mP1->GetPosition());
  const CVector3f limb = mP5->GetPosition() - mP4->GetPosition();
  const CVector3f cross = CVector3f::Cross(limb, plane);
  if (cross.CanBeNormalized()) {
    const CVector3f normal = CVector3f::Cross(cross, limb).AsNormalized();
    const CVector3f delta = mP6->GetPosition() - mP5->GetPosition();
    float distance = CVector3f::Dot(delta, normal);
    if (distance > 0.f) {
      const CVector3f correction = 0.5f * distance * normal;
      mP6->Position() -= correction;
      mP5->Position() += correction;
    }
  }
}

void CRagDoll::CRagDollPlaneConstraint::Update() {
  const CVector3f normal = (mP2->GetPosition() - mP1->GetPosition()).AsNormalized();
  const CVector3f delta = mP4->GetPosition() - mP3->GetPosition();
  float distance = CVector3f::Dot(normal, delta);
  if (distance < 0.f) {
    const CVector3f correction = 0.5f * distance * normal;
    mP4->Position() -= correction;
    mP5->Position() += correction;
  }
}

void CRagDoll::CRagDollKneeConstraint::Update() {
  const CVector3f normal = (mP2->GetPosition() - mP1->GetPosition()).AsNormalized();
  const CVector3f delta = mP4->GetPosition() - mP3->GetPosition();
  float distance = CVector3f::Dot(normal, delta);
  if (distance < mMinimumDistance) {
    const CVector3f correction = (mMinimumDistance - distance) * normal;
    mP3->Position() -= correction;
    mP4->Position() += correction;
  }
}

void CRagDoll::AddParticle(const CSegId& id, const CVector3f& prevPos, const CVector3f& curPos,
                           float radius) {
  mParticles.push_back_unsafe(CRagDollParticle(id, curPos, radius, prevPos));
}

void CRagDoll::AddLengthConstraint(int i1, int i2) {
  mLengthConstraints.push_back_unsafe(CRagDollLengthConstraint(
      &mParticles[i1], &mParticles[i2],
      (mParticles[i1].GetPosition() - mParticles[i2].GetPosition()).Magnitude(),
      CRagDollLengthConstraint::kI_Equal));
}

void CRagDoll::AddMinLengthConstraint(int i1, int i2, float length) {
  mLengthConstraints.push_back_unsafe(CRagDollLengthConstraint(
      &mParticles[i1], &mParticles[i2], length, CRagDollLengthConstraint::kI_Minimum));
}

void CRagDoll::AddMaxLengthConstraint(int i1, int i2, float length) {
  mLengthConstraints.push_back_unsafe(CRagDollLengthConstraint(
      &mParticles[i1], &mParticles[i2], length, CRagDollLengthConstraint::kI_Maximum));
}

void CRagDoll::AddJointConstraint(int i1, int i2, int i3, int i4, int i5, int i6) {
  mJointConstraints.push_back_unsafe(CRagDollJointConstraint(&mParticles[i1], &mParticles[i2],
                                                             &mParticles[i3], &mParticles[i4],
                                                             &mParticles[i5], &mParticles[i6]));
}

void CRagDoll::AddKneeConstraint(int i1, int i2, int i3, int i4, float minimumDistance) {
  mKneeConstraints.push_back_unsafe(CRagDollKneeConstraint(
      &mParticles[i1], &mParticles[i2], &mParticles[i3], &mParticles[i4], minimumDistance));
}

CQuaternion CRagDoll::BoneAlign(CJointData_LinearStorage& pose, const CCharLayoutInfo& layout,
                                int i1, int i2, const CQuaternion& rotation) {
  CVector3f fromParent = layout.GetFromParentUnrotated(mParticles[i2].GetBone());
  CVector3f delta = mParticles[i2].GetPosition() - mParticles[i1].GetPosition();
  delta = rotation.BuildInverted().Transform(delta);
  CQuaternion result = CQuaternion::ShortestRotationArc(fromParent, delta);
  pose.Rotation(mParticles[i1].GetBone().val()) = result;
  return result;
}

void CRagDoll::CalfAlign(CJointData_LinearStorage& pose, int i1, int i2, int i3,
                         const CVector3f& planeNormal) {
  CVector3f limb = mParticles[i2].GetPosition() - mParticles[i1].GetPosition();
  CVector3f cross = CVector3f::Cross(limb, planeNormal);
  CVector3f normal = CVector3f::Cross(cross, limb).AsNormalized();
  CVector3f calf = (mParticles[i3].GetPosition() - mParticles[i2].GetPosition()).AsNormalized();
  CRelAngle angle =
      CRelAngle::FromRadians(static_cast< float >(asin(CVector3f::Dot(calf, normal))));
  CQuaternion& rotation = pose.Rotation(mParticles[i2].GetBone().val());
  rotation *= CQuaternion::ZRotation(angle);
}

// Guessed name.
CAABox CRagDoll::GetRenderBounds() const {
  if (!mRenderBoundsValid) {
    return CalculateRenderBounds();
  }
  return mRenderBounds;
}

// Guessed name.
void CRagDoll::UpdateRenderBounds() {
  mRenderBounds = CalculateRenderBounds();
  mRenderBoundsValid = true;
}

CAABox CRagDoll::CalculateRenderBounds() const {
  CVector3f min(3.4028235e38f, 3.4028235e38f, 3.4028235e38f);
  CVector3f max(-3.4028235e38f, -3.4028235e38f, -3.4028235e38f);
  for (int i = 0; i < mParticles.size(); ++i) {
    for (int j = 0; j < 3; ++j) {
      min[j] = rstl::min_val(mParticles[i].GetPosition()[j] - mParticles[i].GetRadius(), min[j]);
      max[j] = rstl::max_val(mParticles[i].GetPosition()[j] + mParticles[i].GetRadius(), max[j]);
    }
  }
  return CAABox(min, max);
}

// Guessed name.
CProjectileTouchResult CRagDoll::ProjectileCollision(const CGameProjectile& projectile,
                                                     TUniqueId actorId) {
  CVector3f direction = projectile.GetTranslation() - projectile.GetPreviousPos();
  if (direction.CanBeNormalized()) {
    double distance = direction.Magnitude();
    direction *= 1.f / static_cast< float >(distance);
    bool hit = false;
    int hitParticle = 0;
    CVector3f normal = -direction;
    CVector3f hitPoint = projectile.GetPreviousPos();
    for (int i = 0; i < mParticles.size(); ++i) {
      const CRagDollParticle& particle = mParticles[i];
      if (CollisionUtil::RaySphereIntersection_Double(
              CSphere(particle.GetPosition(), particle.GetRadius()), projectile.GetPreviousPos(),
              direction, distance) &&
          distance >= 0.0) {
        hit = true;
        hitPoint = projectile.GetPreviousPos() + static_cast< float >(distance) * direction;
        normal = hitPoint - particle.GetPosition();
        hitParticle = i;
      }
    }
    if (hit) {
      float impulse = 0.2f * projectile.GetCurrentDamageInfo().GetKnockBackPower(
                                 CDamageVulnerability::NormalVulnerabilty(), 0.f);
      mParticles[hitParticle].mImpactResponseDelta += impulse * direction;
      mHitByProjectile = true;
      return CProjectileTouchResult(actorId,
                                    CRayCastResult(static_cast< float >(distance), hitPoint,
                                                   CPlane(hitPoint, normal.AsNormalized()),
                                                   CMaterialList(kMT_Solid)));
    }
  }
  return CProjectileTouchResult(kInvalidUniqueId, rstl::optional_object_null());
}

void CRagDoll::PreRender(const CVector3f& pos, CModelData& modelData) {}

void CRagDoll::PreRenderAllViewports(CActor& actor, float extent) {
  const CAABox bounds = GetRenderBounds();
  const CVector3f expansion = extent * actor.GetModelData()->GetScale();
  const CAABox expanded(bounds.GetMinPoint() - expansion, bounds.GetMaxPoint() + expansion);
  actor.SetOtherBounds(expanded);
  actor.SetRenderBounds(expanded);
}
