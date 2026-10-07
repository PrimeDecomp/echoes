#include "MetroidPrime/Player/CPlayerRagDoll.hpp"

#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/Animation/CJointData_LinearStorage.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "rstl/math.hpp"

static const rstl::string skParts[] = {
    rstl::string_l("Collar"),
    rstl::string_l("Neck_1"),
    rstl::string_l("R_shoulder"),
    rstl::string_l("R_elbow"),
    rstl::string_l("GUN_Particle_LCTR"),
    rstl::string_l("L_shoulder"),
    rstl::string_l("L_elbow"),
    rstl::string_l("L_wrist"),
    rstl::string_l("R_hip"),
    rstl::string_l("R_knee"),
    rstl::string_l("R_ankle"),
    rstl::string_l("L_hip"),
    rstl::string_l("L_knee"),
    rstl::string_l("L_ankle"),
};

static const float skRadii[] = {0.25f, 0.32f, 0.25f, 0.1f,  0.15f, 0.25f, 0.1f,
                                0.15f, 0.15f, 0.15f, 0.15f, 0.15f, 0.15f, 0.15f};

CPlayerRagDoll::CPlayerRagDoll(CStateManager& mgr, CPlayer* player, ushort thudSfx, uint flags)
: CRagDoll(-50.f, 3.f, 8.f, 0.9f, 0.125f, flags)
, mPlayer(player)
, mThudSfx(thudSfx)
, mSfxTimer(0.f)
, mLastSfxPos(CVector3f::Zero())
, mTorsoImpulse(CVector3f::Zero())
, mOriginalBounds(player->GetBaseBoundingBox())
, mInitSfx(true) {
  mPlayer->RemoveMaterial(kMT_Unknown59, kMT_AIBlock, kMT_GroundCollider, mgr);
  mPlayer->HealthInfo()->SetHP(-1.f);
  SetNumParticles(14);
  SetNumLengthConstraints(47);
  SetNumJointConstraints(4);

  const CVector3f& scale = player->GetModelData()->GetScale();
  const CTransform4f& xf = player->GetTransform();
  CAnimData* animData = player->AnimationData();
  animData->BuildPose();
  const CVector3f center = player->GetBoundingBox().GetCenterPoint();
  for (int i = 0; i < 14; ++i) {
    CSegId id = animData->GetLocatorSegId(skParts[i]);
    CVector3f pos = xf * CVector3f::ByElementMultiply(scale, animData->Pose().GetOffset(id));
    AddParticle(id, center, pos, skRadii[i] * scale.GetZ());
  }

  AddLengthConstraint(0, 1);
  AddLengthConstraint(0, 2);
  AddLengthConstraint(0, 8);
  AddLengthConstraint(0, 11);
  AddLengthConstraint(0, 5);
  AddLengthConstraint(2, 3);
  AddLengthConstraint(3, 4);
  AddLengthConstraint(5, 6);
  AddLengthConstraint(6, 7);
  AddLengthConstraint(2, 5);
  AddLengthConstraint(2, 8);
  AddLengthConstraint(2, 11);
  AddLengthConstraint(5, 8);
  AddLengthConstraint(5, 11);
  AddLengthConstraint(8, 11);
  AddLengthConstraint(8, 9);
  AddLengthConstraint(9, 10);
  AddLengthConstraint(11, 12);
  AddLengthConstraint(12, 13);

  AddMinLengthConstraint(1, 8, GetConstraintLength(2));
  AddMinLengthConstraint(1, 11, GetConstraintLength(3));
  AddMinLengthConstraint(1, 2, GetConstraintLength(1) * 0.9f);
  AddMinLengthConstraint(1, 5, GetConstraintLength(4) * 0.9f);
  AddMinLengthConstraint(1, 4, GetConstraintLength(0) * 2.5f);
  AddMinLengthConstraint(1, 7, GetConstraintLength(0) * 2.5f);
  AddMinLengthConstraint(4, 2, GetConstraintLength(5));
  AddMinLengthConstraint(7, 5, GetConstraintLength(7));
  AddMinLengthConstraint(3, 5, GetConstraintLength(5) * 0.5f + GetConstraintLength(9));
  AddMinLengthConstraint(6, 2, GetConstraintLength(7) * 0.5f + GetConstraintLength(9));
  AddMinLengthConstraint(4, 5, GetConstraintLength(5) * 0.5f + GetConstraintLength(9));
  AddMinLengthConstraint(7, 2, GetConstraintLength(7) * 0.5f + GetConstraintLength(9));
  AddMinLengthConstraint(4, 7, GetConstraintLength(9));
  AddMinLengthConstraint(4, 8, GetConstraintLength(14));
  AddMinLengthConstraint(7, 11, GetConstraintLength(14));
  AddMinLengthConstraint(10, 8, GetConstraintLength(15));
  AddMinLengthConstraint(13, 11, GetConstraintLength(17));
  AddMinLengthConstraint(9, 2, GetConstraintLength(15) * 0.707f + GetConstraintLength(10));
  AddMinLengthConstraint(12, 5, GetConstraintLength(17) * 0.707f + GetConstraintLength(13));
  AddMinLengthConstraint(9, 11, GetConstraintLength(15));
  AddMinLengthConstraint(12, 8, GetConstraintLength(17));
  AddMinLengthConstraint(10, 0, GetConstraintLength(2) + GetConstraintLength(15));
  AddMinLengthConstraint(13, 0, GetConstraintLength(3) + GetConstraintLength(17));
  AddMinLengthConstraint(10, 13, GetConstraintLength(14));
  AddMinLengthConstraint(9, 12, GetConstraintLength(14));
  AddMinLengthConstraint(10, 12, GetConstraintLength(14));
  AddMinLengthConstraint(13, 9, GetConstraintLength(14));
  AddMaxLengthConstraint(10, 13, GetConstraintLength(14) * 5.f);

  AddJointConstraint(8, 2, 5, 8, 9, 10);
  AddJointConstraint(11, 2, 5, 11, 12, 13);
  AddJointConstraint(2, 11, 5, 2, 3, 4);
  AddJointConstraint(5, 2, 8, 5, 6, 7);
  SatisfyWorldConstraintsOnConstruction(mgr);
}

void CPlayerRagDoll::Prime(CStateManager& mgr, const CTransform4f& xf, CModelData& modelData) {
  const CAABox& bounds = mPlayer->GetBaseBoundingBox();
  CVector3f max = bounds.GetMaxPoint();
  max.SetZ((bounds.GetMaxPoint().GetZ() - bounds.GetMinPoint().GetZ()) * 0.5f +
           bounds.GetMinPoint().GetZ());
  mOriginalBounds = mPlayer->GetBaseBoundingBox();
  mPlayer->SetBoundingBox(CAABox(bounds.GetMinPoint(), max));

  const CVector3f force = mPlayer->GetConstantForceWR();
  for (int i = 0; i < 14; ++i) {
    mParticles[i].Acceleration() += force;
  }
  CRagDoll::Prime(mgr, xf, modelData);
}

void CPlayerRagDoll::Update(CStateManager& mgr, float dt, float waterTop) {
  if (!IsOver() || WillContinueSmallMovements()) {
    if (mPlayer->GetAttachedActorId() != kInvalidUniqueId) {
      float delta = mParticles[2].GetPosition().GetZ() - mParticles[5].GetPosition().GetZ();
      if (delta * delta > 0.0625f) {
        CVector3f adjustment(0.f, 0.f, (delta > 0.f ? delta - 0.25f : delta + 0.25f) * 0.1f);
        mParticles[2].Position() = mParticles[2].GetPosition() - adjustment;
        mParticles[5].Position() = mParticles[5].GetPosition() + adjustment;
      }
      delta = mParticles[0].GetPosition().GetZ() -
              (mParticles[8].GetPosition().GetZ() + mParticles[11].GetPosition().GetZ()) * 0.5f;
      if (delta * delta > 0.0625f) {
        CVector3f adjustment(0.f, 0.f, (delta > 0.f ? delta - 0.25f : delta + 0.25f) * 0.1f);
        mParticles[0].Position() = mParticles[0].GetPosition() - adjustment;
        adjustment[kDZ] *= 0.5f;
        mParticles[8].Position() = mParticles[8].GetPosition() + adjustment;
        mParticles[11].Position() = mParticles[11].GetPosition() + adjustment;
      }
    }

    CVector3f oldCenter = mParticles[8].GetPosition() * 0.25f +
                          mParticles[11].GetPosition() * 0.25f + mParticles[0].GetPosition() * 0.5f;
    oldCenter[kDZ] = rstl::min_val(mParticles[8].GetPosition().GetZ() - mParticles[8].GetRadius(),
                                   mParticles[11].GetPosition().GetZ() - mParticles[11].GetRadius());
    oldCenter[kDZ] =
        rstl::min_val(oldCenter[kDZ], mParticles[0].GetPosition().GetZ() - mParticles[0].GetRadius());
    if (oldCenter.GetZ() < 0.5f + waterTop) {
      mTorsoImpulse *= 1000.f;
    }
    const CVector3f acceleration = mTorsoImpulse * 0.333f * (1.f / mPlayer->GetMass());
    mParticles[11].Acceleration() += acceleration;
    mParticles[8].Acceleration() += acceleration;
    mParticles[0].Acceleration() += acceleration;
    mTorsoImpulse = CVector3f::Zero();
    CRagDoll::Update(mgr, dt, waterTop);

    CVector3f newCenter = mParticles[8].GetPosition() * 0.25f +
                          mParticles[11].GetPosition() * 0.25f + mParticles[0].GetPosition() * 0.5f;
    newCenter[kDZ] = rstl::min_val(mParticles[8].GetPosition().GetZ() - mParticles[8].GetRadius(),
                                   mParticles[11].GetPosition().GetZ() - mParticles[11].GetRadius());
    newCenter[kDZ] =
        rstl::min_val(newCenter[kDZ], mParticles[0].GetPosition().GetZ() - mParticles[0].GetRadius());
    const CVector3f velocity = (1.f / dt) * (newCenter - oldCenter);
    mPlayer->SetTransform(CTransform4f::Identity());
    mPlayer->SetTranslation(newCenter);
    mPlayer->SetVelocityWR(velocity);

    mSfxTimer -= dt;
    const float impactVelocity = mImpactVel;
    if (impactVelocity > 2.5f && mSfxTimer < 0.f) {
      const CVector3f delta = mPlayer->GetTranslation() - mLastSfxPos;
      if (mInitSfx || delta.MagSquared() > 0.1f) {
        float volume = rstl::min_val(127.f, 25.f * impactVelocity);
        CSfxManager::AddEmitter(mThudSfx, mPlayer->GetTranslation(), CCast::ToUint8(volume),
                                mPlayer->GetCurrentAreaId().Value(), true, false,
                                CSfxManager::kMedPriority);
        mSfxTimer = mgr.Random()->Float() * 0.222f + 0.222f;
        mInitSfx = false;
        mLastSfxPos = mPlayer->GetTranslation();
      }
    }
  } else {
    mPlayer->SetMomentumWR(CVector3f::Zero());
    mPlayer->Stop();
  }
}

void CPlayerRagDoll::PreRender(const CVector3f& pos, CModelData& modelData) {
  if (!IsOver() || WillContinueSmallMovements()) {
    CAnimData* animData = modelData.AnimationData();
    const CCharLayoutInfo& layout = *animData->GetCharLayoutInfo();
    CJointData_LinearStorage pose(layout.GetBodyPartSegIds().GetCount(),
                                  CJointData_LinearStorage::kAF_Pool);
    pose.SetReferenceOffsets(layout);
    pose.SetZeroRotation();

    CSegId rootId = animData->GetLocatorSegId(rstl::string_l("Skeleton_Root"));
    CVector3f rootOffset =
        0.5f * (mParticles[8].GetPosition() + mParticles[11].GetPosition()) - pos;
    const CVector3f scale = modelData.GetScale();
    pose.Translation(rootId.val()) =
        CVector3f(rootOffset.GetX() / scale.GetX(), rootOffset.GetY() / scale.GetY(),
                  rootOffset.GetZ() / scale.GetZ());
    CVector3f right = mParticles[2].GetPosition() - mParticles[5].GetPosition();
    CVector3f up = (mParticles[0].GetPosition() -
                    (mParticles[8].GetPosition() + mParticles[11].GetPosition()) * 0.5f)
                       .AsNormalized();
    CVector3f forward = CVector3f::Cross(up, right).AsNormalized();
    right = CVector3f::Cross(forward, up);
    CMatrix3f matrix(right, forward, up);
    CQuaternion rootRotation = CQuaternion::FromMatrix(matrix.GetTranspose());
    pose.Rotation(rootId.val()) = rootRotation;

    if (mPlayer->GetAttachedActorId() == kInvalidUniqueId) {
      CVector3f rest = layout.GetFromParentUnrotated(mParticles[1].GetBone());
      CVector3f neck = mParticles[1].GetPosition() - mParticles[0].GetPosition();
      neck = rootRotation.BuildInverted().Transform(neck);
      CQuaternion neckRotation = CQuaternion::ShortestRotationArc(rest, neck);
      pose.Rotation(mParticles[1].GetBone().val()) = neckRotation;
    }
    CQuaternion jointRotation = BoneAlign(pose, layout, 2, 3, rootRotation);
    BoneAlign(pose, layout, 3, 4, rootRotation * jointRotation);
    jointRotation = BoneAlign(pose, layout, 5, 6, rootRotation);
    BoneAlign(pose, layout, 6, 7, rootRotation * jointRotation);
    jointRotation = BoneAlign(pose, layout, 8, 9, rootRotation);
    BoneAlign(pose, layout, 9, 10, rootRotation * jointRotation);
    jointRotation = BoneAlign(pose, layout, 11, 12, rootRotation);
    BoneAlign(pose, layout, 12, 13, rootRotation * jointRotation);

    CQuaternion ankleRotation = CQuaternion::XRotation(CRelAngle::FromDegrees(-70.f));
    pose.Rotation(mParticles[10].GetBone().val()) = ankleRotation;
    pose.Rotation(mParticles[13].GetBone().val()) = ankleRotation;
    animData->Pose().BuildPose(layout, pose);
  }
}

void CPlayerRagDoll::RestoreActorCollision(CStateManager& mgr) {
  mPlayer->SetBoundingBox(mOriginalBounds);
  mPlayer->AddMaterial(kMT_Unknown59, kMT_GroundCollider, mgr);
}
