#include "MetroidPrime/Enemies/CPirateRagDoll.hpp"

#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/Animation/CJointData_LinearStorage.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CValidEntityPredicate.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "REL/REL_Setup.h"
#include "rstl/math.hpp"
#include <float.h>

static const rstl::string skParts[] = {
    rstl::string_l("Collar"),  rstl::string_l("Neck_1"),  rstl::string_l("R_shoulder"),
    rstl::string_l("R_elbow"), rstl::string_l("R_wrist"), rstl::string_l("L_shoulder"),
    rstl::string_l("L_elbow"), rstl::string_l("L_wrist"), rstl::string_l("R_hip"),
    rstl::string_l("R_knee"),  rstl::string_l("R_ankle"), rstl::string_l("L_hip"),
    rstl::string_l("L_knee"),  rstl::string_l("L_ankle"),
};

// Guessed name. Selects the AI waypoints that pin particles of the ragdoll.
class CAIWaypointPredicate : public CValidEntityPredicate {
public:
  ~CAIWaypointPredicate() override {}

  bool IsValid(const CStateManager& mgr, TUniqueId id) const override {
    return TCastToConstPtr< CScriptAIWaypoint >(mgr.GetObjectById(id)) != nullptr;
  }
};

CPirateRagDoll::CPirateRagDoll(CStateManager& mgr, CPatterned* actor, ushort thudSfx, uint flags,
                               float gravity, float floatingGravity,
                               const rstl::reserved_vector< float, 14 >& radii)
: CRagDoll(-gravity, -floatingGravity, 8.f, 0.85f, 0.125f, flags)
, mActor(actor)
, mThudSfx(thudSfx)
, mSfxTimer(0.f)
, mLastSfxPos(CVector3f::Zero())
, mTorsoImpulse(CVector3f::Zero())
, mMinImpactVelocity(18.f)
, mImpactVolumeScale(25.f)
, mMaxImpactVolume(105.f)
, mSfxInterval(0.222f)
, mElapsedTime(0.f)
, mLeftShoulderPadId()
, mRightShoulderPadId()
, mPrevWaterTop(-FLT_MAX * 0.5f)
, mInitSfx(true)
, mActorAttached(false) {
  mActor->RemoveMaterial(kMT_Solid, kMT_AIBlock, kMT_GroundCollider, mgr);
  mActor->RemoveMaterial(kMT_Target, kMT_Orbit, mgr);
  mActor->HealthInfo()->SetHP(-1.f);
  SetNumParticles(14);
  SetNumLengthConstraints(47);
  SetNumJointConstraints(4);
  SetNumKneeConstraints(2);

  const CVector3f scale = actor->GetModelData()->GetScale();
  const CTransform4f& xf = actor->GetTransform();
  CAnimData* animData = actor->AnimationData();
  animData->BuildPose();
  const CVector3f center = actor->GetBoundingBox().GetCenterPoint();
  for (int i = 0; i < 14; ++i) {
    CSegId id = animData->GetLocatorSegId(skParts[i]);
    if (id == CSegId::Invalid()) {
      break;
    }
    CVector3f pos = xf * CVector3f::ByElementMultiply(scale, animData->Pose().GetOffset(id));
    AddParticle(id, center, pos, radii[i] * scale.GetZ());
  }
  mLeftShoulderPadId = animData->GetLocatorSegId(rstl::string_l("L_varia_SDK"));
  mRightShoulderPadId = animData->GetLocatorSegId(rstl::string_l("R_varia_SDK"));

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
  AddMinLengthConstraint(10, 8, GetConstraintLength(15) * 1.414f);
  AddMinLengthConstraint(13, 11, GetConstraintLength(17) * 1.414f);
  AddMinLengthConstraint(9, 2, GetConstraintLength(15) * 0.707f + GetConstraintLength(10));
  AddMinLengthConstraint(12, 5, GetConstraintLength(17) * 0.707f + GetConstraintLength(13));
  AddMinLengthConstraint(9, 11, GetConstraintLength(15));
  AddMinLengthConstraint(12, 8, GetConstraintLength(17));
  AddMinLengthConstraint(10, 0, GetConstraintLength(2) + GetConstraintLength(15));
  AddMinLengthConstraint(13, 0, GetConstraintLength(3) + GetConstraintLength(17));
  AddMinLengthConstraint(10, 13, GetConstraintLength(14));
  AddMinLengthConstraint(9, 12, GetConstraintLength(14) * 1.1f);
  AddMinLengthConstraint(10, 12, GetConstraintLength(14));
  AddMinLengthConstraint(13, 9, GetConstraintLength(14));
  AddMaxLengthConstraint(10, 13, GetConstraintLength(15) * 1.5f);

  AddJointConstraint(8, 2, 5, 8, 9, 10);
  AddJointConstraint(11, 2, 5, 11, 12, 13);
  AddJointConstraint(2, 11, 5, 2, 3, 4);
  AddJointConstraint(5, 2, 8, 5, 6, 7);
  AddKneeConstraint(5, 2, 12, 9, GetConstraintLength(14));
  AddKneeConstraint(5, 2, 13, 10, GetConstraintLength(14));

  const rstl::vector< TUniqueId > ids =
      mActor->FindConnectedObjects_if(mgr, kSS_Modify, kSM_Follow, CAIWaypointPredicate());
  for (int i = 0; i < ids.size(); ++i) {
    if (const CScriptAIWaypoint* waypoint =
            TCastToConstPtr< CScriptAIWaypoint >(mgr.GetObjectById(ids[i]))) {
      mWaypointIds.push_back(ids[i]);
      mWaypointParticles.push_back(waypoint->GetLocatorIndex());
      mWaypointActive.push_back(waypoint->GetActive());
      if (mWaypointIds.capacity() - mWaypointIds.size() <= 0) {
        break;
      }
    }
  }
  SatisfyWorldConstraintsOnConstruction(mgr);
}

void CPirateRagDoll::Prime(CStateManager& mgr, const CTransform4f& xf, CModelData& modelData) {
  const CAABox& bounds = mActor->GetBaseBoundingBox();
  CVector3f max = bounds.GetMaxPoint();
  max.SetZ((bounds.GetMaxPoint().GetZ() - bounds.GetMinPoint().GetZ()) * 0.5f +
           bounds.GetMinPoint().GetZ());
  mActor->SetBoundingBox(CAABox(bounds.GetMinPoint(), max));
  mActor->RemoveMaterial(kMT_ProjectilePassthrough, mgr);
  CRagDoll::Prime(mgr, xf, modelData);
}

void CPirateRagDoll::CheckStatic(float dt) {
  mImpactCount = 0;
  mImpactVel = 0.f;
  float staticDistance = mStaticSpeedThreshold * dt;
  float threshold = staticDistance * staticDistance;
  mAverageVel = CVector3f::Zero();
  bool movingSlowly = true;
  for (int i = 0; i < mParticles.size(); ++i) {
    bool pinned = false;
    for (int j = 0; j < mWaypointParticles.size(); ++j) {
      if (mWaypointParticles[j] == i && mWaypointActive[j]) {
        pinned = true;
        break;
      }
    }
    if (pinned) {
      continue;
    }
    CVector3f delta = mParticles[i].GetPosition() - mParticles[i].GetPreviousPosition();
    mAverageVel += delta;
    if (delta.MagSquared() > threshold) {
      movingSlowly = false;
    }
    if (mParticles[i].IsImpactPending()) {
      ++mImpactCount;
      mImpactVel = rstl::max_val(mImpactVel, mParticles[i].GetImpactFrameVelocity());
    }
    mParticles[i].ClearImpactFrameVelocity();
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

void CPirateRagDoll::Update(CStateManager& mgr, float dt, float waterTop) {
  if (!IsOver() || WillContinueSmallMovements()) {
    if (mActorAttached) {
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
    oldCenter[kDZ] =
        rstl::min_val(mParticles[8].GetPosition().GetZ() - mParticles[8].GetRadius(),
                      mParticles[11].GetPosition().GetZ() - mParticles[11].GetRadius());
    oldCenter[kDZ] = rstl::min_val(oldCenter[kDZ],
                                   mParticles[0].GetPosition().GetZ() - mParticles[0].GetRadius());
    if (oldCenter.GetZ() < 0.5f + waterTop) {
      mTorsoImpulse *= 1000.f;
    }
    const CVector3f acceleration = mTorsoImpulse * 0.333f * (1.f / mActor->GetMass());
    mParticles[11].Acceleration() += acceleration;
    mParticles[8].Acceleration() += acceleration;
    mParticles[0].Acceleration() += acceleration;
    mTorsoImpulse = CVector3f::Zero();

    bool needsUpdate = !IsOver() || mHitByProjectile;
    if (!needsUpdate) {
      if (waterTop != mPrevWaterTop) {
        needsUpdate = true;
      }
      if (!needsUpdate && !(mTorsoImpulse == CVector3f::Zero())) {
        needsUpdate = true;
      }
      if (!needsUpdate) {
        for (int i = 0; i < mWaypointIds.size(); ++i) {
          const CActor* waypoint = static_cast< const CActor* >(mgr.GetObjectById(mWaypointIds[i]));
          if (waypoint != nullptr) {
            bool active = waypoint->GetActive();
            if (active != mWaypointActive[i] ||
                !(mParticles[mWaypointParticles[i]].GetPosition() == waypoint->GetTranslation())) {
              mWaypointActive[i] = active;
              needsUpdate = true;
            }
          } else if (mWaypointActive[i]) {
            mWaypointActive[i] = false;
            needsUpdate = true;
          }
        }
      }
    }
    if (IsOver() && needsUpdate) {
      mOver = false;
      mPrevMovingSlowly = false;
    }
    if (needsUpdate) {
      CRagDoll::Update(mgr, dt, waterTop);
    }

    int activeWaypoints = 0;
    for (int i = 0; i < mWaypointIds.size(); ++i) {
      const CActor* waypoint = static_cast< const CActor* >(mgr.GetObjectById(mWaypointIds[i]));
      if (waypoint != nullptr && waypoint->GetActive()) {
        ++activeWaypoints;
        mParticles[mWaypointParticles[i]].Position() = waypoint->GetTranslation();
      }
    }
    mStaticSpeedThreshold = activeWaypoints > 0 ? 0.1f : 0.5f;

    CVector3f newCenter = mParticles[8].GetPosition() * 0.25f +
                          mParticles[11].GetPosition() * 0.25f + mParticles[0].GetPosition() * 0.5f;
    newCenter[kDZ] =
        rstl::min_val(mParticles[8].GetPosition().GetZ() - mParticles[8].GetRadius(),
                      mParticles[11].GetPosition().GetZ() - mParticles[11].GetRadius());
    newCenter[kDZ] = rstl::min_val(newCenter[kDZ],
                                   mParticles[0].GetPosition().GetZ() - mParticles[0].GetRadius());
    const CVector3f velocity = (1.f / dt) * (newCenter - oldCenter);
    mActor->SetTransform(CTransform4f::Identity());
    mActor->SetTranslation(newCenter);
    mActor->SetVelocityWR(velocity);

    mElapsedTime += dt;
    if (activeWaypoints == 0 || mElapsedTime > 1.f) {
      mSfxTimer -= dt;
      UpdateImpactSfx(mgr);
    }
  } else {
    mActor->SetMomentumWR(CVector3f::Zero());
    mActor->Stop();
  }
  mPrevWaterTop = waterTop;
}

void CPirateRagDoll::PreRender(const CVector3f& pos, CModelData& modelData) {
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

    if (mActorAttached) {
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
    CalfAlign(pose, 8, 9, 10, right);
    jointRotation = BoneAlign(pose, layout, 11, 12, rootRotation);
    BoneAlign(pose, layout, 12, 13, rootRotation * jointRotation);
    CalfAlign(pose, 11, 12, 13, right);

    CQuaternion ankleRotation = CQuaternion::XRotation(CRelAngle::FromDegrees(-70.f));
    pose.Rotation(mParticles[10].GetBone().val()) = ankleRotation;
    pose.Rotation(mParticles[13].GetBone().val()) = ankleRotation;
    AlignShoulderPad(pose, 5, 6, mLeftShoulderPadId, up, 1.f);
    AlignShoulderPad(pose, 2, 3, mRightShoulderPadId, up, -1.3f);
    animData->Pose().BuildPose(layout, pose);
    animData->SetPoseBuilt(true);
  }
}

void CPirateRagDoll::UpdateImpactSfx(CStateManager& mgr) {
  if (mImpactVel > mMinImpactVelocity && mSfxTimer < 0.f) {
    const CVector3f delta = mActor->GetTranslation() - mLastSfxPos;
    if (mInitSfx || delta.MagSquared() > 0.1f) {
      float volume = rstl::min_val(mImpactVolumeScale * mImpactVel, mMaxImpactVolume);
      CSfxManager::AddEmitter(mThudSfx, mActor->GetTranslation(), CCast::ToUint8(volume),
                              mActor->GetCurrentAreaId().Value(), true, false,
                              CSfxManager::kMedPriority);
      mSfxTimer = mgr.Random()->Float() * mSfxInterval + mSfxInterval;
      mInitSfx = false;
      mLastSfxPos = mActor->GetTranslation();
    }
  }
}

void CPirateRagDoll::AlignShoulderPad(CJointData_LinearStorage& pose, int shoulder, int elbow,
                                      const CSegId& padId, const CVector3f& axis, float side) {
  if (padId == CSegId::Invalid()) {
    return;
  }
  const CVector3f direction =
      (mParticles[elbow].GetPosition() - mParticles[shoulder].GetPosition()).AsNormalized();
  float angle = 57.295776f * static_cast< float >(asin(CVector3f::Dot(axis, direction)));
  if (angle > -20.f) {
    float lift = angle + 20.f;
    CQuaternion rotation = CQuaternion::YRotation(CRelAngle::FromDegrees(side * lift));
    pose.Rotation(padId.val()) = rotation;
    CQuaternion offsetRotation =
        CQuaternion::YRotation(CRelAngle::FromDegrees(side * (lift - (80.f + 10.f * side))));
    const CVector3f offset = CVector3f::Up() * (0.18f * (lift / 110.f));
    const CVector3f translation = pose.Translation(padId.val());
    pose.Translation(padId.val()) = translation + offsetRotation.Transform(offset);
  }
}

static CRagDoll* CreatePirateRagDoll(CStateManager& mgr, CPatterned* actor, ushort soundId,
                                     uint flags, float gravity, float floatingGravity,
                                     const rstl::reserved_vector< float, 14 >& radii) {
  return rs_new CPirateRagDoll(mgr, actor, soundId, flags, gravity, floatingGravity, radii);
}

#ifndef MONOLITHIC
static void SetFuncPtrs() {
  static SPirateRagDoll_FuncPtrs funcPtrs;
  funcPtrs.mFactory = &CreatePirateRagDoll;
  SetSPirateRagDoll_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSPirateRagDoll_FuncPtrs(nullptr); }
#endif
