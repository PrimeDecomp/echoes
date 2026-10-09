#include "MetroidPrime/Enemies/COctapedeSegment.hpp"

#include "Collision/CCollisionInfo.hpp"
#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Kyoto/Animation/CPOINode.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrOctapedeSegment.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptEffect.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "REL/REL_Setup.h"
#include <stdio.h>

static EMaterialTypes skExplosionDamageMaterial = kMT_Solid;

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Patrol", static_cast< CPatterned::StateMachine::StateFunc >(&COctapedeSegment::Patrol)},
    {"Dead", static_cast< CPatterned::StateMachine::StateFunc >(&COctapedeSegment::Dead)},
};

static EMaterialTypes skIncludeMaterialA = kMT_Solid;
static EMaterialTypes skIncludeMaterialB = kMT_AIBlock;
static EMaterialTypes skExcludeMaterialA = kMT_Character;
static EMaterialTypes skExcludeMaterialB = kMT_CollisionActor;
static EMaterialTypes skExcludeMaterialC = kMT_AIPassthrough;

static EMaterialTypes skBounceMaterialA = kMT_Solid;
static EMaterialTypes skBounceMaterialB = kMT_Ceiling;
static EMaterialTypes skBounceMaterialC = kMT_Wall;
static EMaterialTypes skBounceMaterialD = kMT_Floor;
static EMaterialTypes skBounceMaterialE = kMT_Character;
static EMaterialTypes skBounceMaterialF = kMT_AIBlock;
static EMaterialTypes skFloorMaterial = kMT_Floor;

static TUniqueId FindHeadSegment(CStateManager& mgr, TUniqueId segmentId);

COctapedeSegment::COctapedeSegment(
    TUniqueId uid, const rstl::string& name, CEntityInfo& info, const CTransform4f& xf,
    const CModelData& modelData, const CPatternedInfo& patternedInfo,
    const CActorParameters& actorParams, float stickyReach, float floorTurnSpeed,
    float waypointApproachDistance, float visibleDistance, float projectileBoundsMultiplier,
    float collisionLookAhead, float animSpeedScalar, float maxAudibleDistance, bool initiallyPaused,
    float segmentSpacing, CAssetId betweenSegmentsEffect, float minBreakApartSpeed,
    float maxBreakApartSpeed, float minBreakApartAngle, float maxBreakApartAngle,
    float minBreakApartSpinSpeed, float maxBreakApartSpinSpeed, float minRunAroundTime,
    float maxRunAroundTime, float minTurnInterval, float maxTurnInterval, int minBounces,
    int maxBounces, float bounciness, const CDamageInfo& explosionDamage, ushort walkSound,
    ushort idleSound, ushort separateSound, ushort bounceSound, ushort explodeSound,
    float soundFalloff)
: CWallCrawler(kPAI_OctapedeSegment, uid, name, kFT_Zero, info, xf, modelData, patternedInfo,
               kMT_Ground, kCT_Zero, kBT_WallWalker, actorParams,
               patternedInfo.GetHalfExtent() * modelData.GetScale().GetX(), stickyReach,
               floorTurnSpeed, waypointApproachDistance, visibleDistance, kT_OctapedeSegment,
               initiallyPaused, projectileBoundsMultiplier, 0.167f, 0.6f, 1.5f, 0.6f, 1.5f)
, mCurrentWaypointId(kInvalidUniqueId)
, mAnimSpeedScalar(animSpeedScalar)
, mMaxAudibleDistance(maxAudibleDistance)
, mCollisionLookAhead(collisionLookAhead)
, mSegmentState(kSegS_None)
, mFollowedSegmentId(kInvalidUniqueId)
, mHeadSegmentId(kInvalidUniqueId)
, mSegmentSpacing(segmentSpacing)
, mElectric(betweenSegmentsEffect != kInvalidAssetId
                ? rstl::auto_ptr< CParticleElectric >(
                      rs_new CParticleElectric(TCachedToken< CElectricDescription >(
                          gpSimplePool->GetObj(SObjectTag('ELSC', betweenSegmentsEffect)), true)))
                : rstl::auto_ptr< CParticleElectric >())
, mMinBreakApartSpeed(minBreakApartSpeed)
, mMaxBreakApartSpeed(maxBreakApartSpeed)
, mMinBreakApartAngle(minBreakApartAngle)
, mMaxBreakApartAngle(maxBreakApartAngle)
, mMinRunAroundTime(minRunAroundTime)
, mMaxRunAroundTime(maxRunAroundTime)
, mMinTurnInterval(minTurnInterval)
, mMaxTurnInterval(maxTurnInterval)
, mBounceDirection(CVector3f::Forward())
, mBreakApartSpeed(0.f)
, mMinBreakApartSpinSpeed(minBreakApartSpinSpeed)
, mMaxBreakApartSpinSpeed(maxBreakApartSpinSpeed)
, mRunAroundTimer(0.f)
, mMinBounces(minBounces)
, mMaxBounces(maxBounces)
, mBounciness(bounciness)
, mBouncesRemaining(0)
, mTurnTimer(0.f)
, mRunDirection(CVector3f::Forward())
, mExplosionDamage(explosionDamage)
, mWalkSound(walkSound)
, mIdleSound(idleSound)
, mSeparateSound(separateSound)
, mBounceSound(bounceSound)
, mExplodeSound(explodeSound)
, mSoundFalloff(soundFalloff)
, mSharedFreezeDuration(0.f)
, mSharedBurnDuration(0.f)
, mSharedBurnDamage(0.f)
, mBrokenApart(false)
, mBounceLatch(false) {
  mAlignToFloor = true;
  SetDrawShadow(false);
  mSpeed = mAnimSpeedScalar;
  mKnockBackController.EnableKnockBackPhysics(false);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      CMaterialList(skIncludeMaterialA, skIncludeMaterialB),
      CMaterialList(skExcludeMaterialA, skExcludeMaterialB, skExcludeMaterialC)));
}

COctapedeSegment::~COctapedeSegment() {}

void COctapedeSegment::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    mAlignToFloor = true;
    SetMomentumWR(CVector3f::Zero());
    mHasAlignSurface = false;
    SetMovable(false);
    TUniqueId connected = kInvalidUniqueId;
    const TUniqueId* waypoint;
    if (mFollowedSegmentId != kInvalidUniqueId) {
      waypoint = &mFollowedSegmentId;
    } else if (mCurrentWaypointId != kInvalidUniqueId) {
      waypoint = &mCurrentWaypointId;
    } else {
      connected = GetConnectedObject(mgr, kSS_Patrol, kSM_Follow);
      waypoint = &connected;
    }
    if (*waypoint != kInvalidUniqueId && mFollowedSegmentId == kInvalidUniqueId) {
      mDestObj = *waypoint;
    }
    mSegmentState = kSegS_Attached;
    break;
  }
  case kStateMsg_Update: {
    if (mFollowedSegmentId == kInvalidUniqueId) {
      UpdateWPDestination(mgr);
    } else if (const COctapedeSegment* front =
                   TCastToConstPtr< COctapedeSegment >(mgr.GetObjectById(mFollowedSegmentId))) {
      const CVector3f frontPosition = front->GetTranslation();
      SetDestPos(frontPosition);
    }

    const float desiredSpacing = 2.f * mColSphere.GetSphere().GetRadius() + mSegmentSpacing;
    if (mFollowedSegmentId == kInvalidUniqueId) {
      float speedScale = 1.f;
      TUniqueId nearestId = kInvalidUniqueId;
      float nearestDist = 10000000.f;
      const uint count = mChildSegmentIds.size();
      const CVector3f position = GetTranslation();
      for (uint i = 0; i < count; ++i) {
        const TUniqueId childId = mChildSegmentIds[i];
        const CActor* child = static_cast< const CActor* >(mgr.GetObjectById(childId));
        if (child) {
          const CVector3f delta = position - child->GetTranslation();
          const float distSq = delta.MagSquared();
          if (distSq < nearestDist * nearestDist) {
            nearestDist = distSq;
            nearestId = childId;
          }
        }
      }
      if (nearestId != kInvalidUniqueId) {
        if (const CActor* nearest = static_cast< const CActor* >(mgr.GetObjectById(nearestId))) {
          const float distance = (position - nearest->GetTranslation()).Magnitude();
          if (distance > desiredSpacing) {
            speedScale = desiredSpacing / distance;
          }
        }
      }
      mSpeed = mAnimSpeedScalar * speedScale;
    }

    const CVector3f up = GetTransform().GetUp();
    CVector3f toDest = mDestPos - GetTranslation();
    toDest.Normalize();
    BodyController()->CommandMgr().DeliverCmd(
        CBCLocomotionCmd(ProjectVectorToPlane(toDest, up), CVector3f::Zero(), 0.f));
    const CVector3f seek = ProjectVectorToPlane(mSteeringBehaviors.Seek(*this, mDestPos), up);
    BodyController()->CommandMgr().DeliverCmd(
        CBCLocomotionCmd(ProjectVectorToPlane(seek, up), CVector3f::Zero(), 1.f));
    BodyController()->CommandMgr().DeliverCmd(
        CBCLocomotionCmd(GetTransform().GetForward(), CVector3f::Zero(), 0.f));
    if (mFollowedSegmentId != kInvalidUniqueId) {
      const CVector3f separation =
          ProjectVectorToPlane(mSteeringBehaviors.Separation(*this, mDestPos, desiredSpacing), up);
      if (separation.MagSquared() > 0.f) {
        BodyController()->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(separation, CVector3f::Zero(), 1.f));
      }
    }
    break;
  }
  case kStateMsg_Deactivate:
    if (mFollowedSegmentId == kInvalidUniqueId) {
      mCurrentWaypointId = mDestObj;
    }
    break;
  }
}

void COctapedeSegment::SetupStateMachine(CStateManager& mgr) {
  StateMachine* stateMachine = mStateMachine.get();
  CPatterned::SetupStateMachine(mgr);
  stateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
}

void COctapedeSegment::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_AreaLoaded: {
    mChildSegmentIds.reserve(16);
    for (rstl::vector< SConnection >::const_iterator it = GetConnectionList().begin();
         it != GetConnectionList().end(); ++it) {
      const EScriptObjectState state = it->state;
      const EScriptObjectMessage connMsg = it->msg;
      const TUniqueId targetId = mgr.GetIdForScript(it->objId);
      if (state == kSS_Approach) {
        if (connMsg == kSM_Action) {
          if (TCastToConstPtr< COctapedeSegment >(mgr.GetObjectById(targetId))) {
            mChildSegmentIds.push_back_unsafe(targetId);
          }
        } else if (TCastToConstPtr< COctapedeSegment >(mgr.GetObjectById(targetId))) {
          mFollowedSegmentId = targetId;
        }
      }
    }
    mHeadSegmentId = FindHeadSegment(mgr, mFollowedSegmentId);
    if (mHeadSegmentId != kInvalidUniqueId) {
      const COctapedeSegment* head =
          TCastToConstPtr< COctapedeSegment >(mgr.GetObjectById(mHeadSegmentId));
      if (mHeadSegmentId != GetUniqueId() && mChildSegmentIds.size() > 0) {
        mChildSegmentIds.clear();
      }
    }
    break;
  }
  case kSM_Create:
    if (!BodyController()->GetIsActive()) {
      BodyController()->SetLocomotionType(pas::kLT_Relaxed);
      BodyController()->Activate(mgr, pas::kAS_Invalid);
    }
    AddMaterial(kMT_Character, mgr);
    break;
  case kSM_Damage:
  case kSM_ResistedDamage:
    if (mHeadSegmentId != kInvalidUniqueId) {
      if (COctapedeSegment* head = TCastToPtr< COctapedeSegment >(mgr.ObjectById(mHeadSegmentId))) {
        const bool onFire = BodyController()->IsOnFire();
        const bool headFrozen = head->BodyController()->IsFrozen();
        const bool headOnFire = head->BodyController()->IsOnFire();
        if (BodyController()->IsFrozen() && !headFrozen) {
          head->Freeze(mgr, CVector3f::Zero(), CVector3f::Forward(), mSharedFreezeDuration, -1.f);
        } else if (onFire && !headOnFire) {
          head->Burn(mgr, mSharedBurnDuration, mSharedBurnDamage);
        }
      }
    }
    break;
  case kSM_Increment:
  case kSM_Decrement:
  case kSM_AIUpdateDisabled:
    break;
  case kSM_Landed:
    if (mSegmentState == kSegS_Bouncing && mBouncesRemaining > 0) {
      return;
    }
    SendScriptMsgs(kSS_MaxReached, mgr, kInvalidUniqueId, kSM_None);
    break;
  }
  CWallCrawler::AcceptScriptMsg(mgr, msg);
}

void COctapedeSegment::PreThink(float dt, CStateManager& mgr) { CWallCrawler::PreThink(dt, mgr); }

void COctapedeSegment::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  if (mFollowedSegmentId == kInvalidUniqueId && !mBrokenApart && !FindLoopedSound(mWalkSound)) {
    const uint sfx = mWalkSound;
    ProcessSoundEvent(sfx | 0xA0000000, 1.f, 0, mSoundFalloff, mMaxAudibleDistance, CSegId(0), 0, 0,
                      0.f, 20, 127, GetDistanceToCamera(mgr), GetTranslation(),
                      mgr.GetNextAreaId().Value(), mgr, true);
  }

  static float skLookAheadTime = 0.02f;
  ++mThinkCounter;
  mPlayerObstructed = false;
  const CGameArea& area = mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId());
  if (area.GetOcclusionState() != CGameArea::kOS_Visible) {
    mPlayerObstructed = true;
  }
  if (mBouncesRemaining <= 0) {
    if (mPlayerObstructed) {
      SetMovable(false);
      return;
    }
    SetMovable(!mAlignToFloor);
  } else {
    SetMovable(true);
  }

  CWallCrawler::Think(dt, mgr);
  if (!mDisableMove && close_enough(BodyController()->GetPercentageFrozen(), 0.f) &&
      mSegmentState != kSegS_Bouncing && mAlignToFloor) {
    AlignToFloor(mgr, mColSphere.GetSphere().GetRadius(),
                 GetTranslation() + GetVelocityWR() * skLookAheadTime, dt);
  }

  if (mHeadSegmentId != kInvalidUniqueId) {
    if (COctapedeSegment* head = TCastToPtr< COctapedeSegment >(mgr.ObjectById(mHeadSegmentId))) {
      const CHealthInfo* health = GetHealthInfo();
      const CHealthInfo* headHealth = head->GetHealthInfo();
      if (headHealth->GetHP() > health->GetHP()) {
        *head->HealthInfo() = *health;
      } else if (headHealth->GetHP() < health->GetHP()) {
        *HealthInfo() = *head->GetHealthInfo();
      }
      if (head->GetHealthInfo()->GetHP() <= 0.f && mSegmentState != kSegS_Bouncing &&
          !mBrokenApart) {
        if (BodyController()->IsFrozen()) {
          MassiveFrozenDeath(mgr);
        } else {
          Death(mgr, CVector3f::Zero(), kSS_DeathRattle);
        }
      }
    } else if (BodyController()->IsFrozen()) {
      MassiveFrozenDeath(mgr);
    } else if (mSegmentState != kSegS_Bouncing && !mBrokenApart) {
      Death(mgr, CVector3f::Zero(), kSS_DeathRattle);
    }
    RemoveMaterial(kMT_Target, mgr);
    RemoveMaterial(kMT_Orbit, mgr);
  } else if (mSegmentState != kSegS_Bouncing && mSegmentState != kSegS_Landed) {
    AddMaterial(kMT_Target, mgr);
    AddMaterial(kMT_Orbit, mgr);
    if (GetHealthInfo()->GetHP() <= 0.f && !mBrokenApart) {
      if (BodyController()->IsFrozen()) {
        MassiveFrozenDeath(mgr);
      } else {
        Death(mgr, CVector3f::Zero(), kSS_DeathRattle);
      }
    }
  } else {
    if (mSegmentState == kSegS_Bouncing && BodyController()->IsFrozen()) {
      MassiveFrozenDeath(mgr);
    }
    RemoveMaterial(kMT_Target, mgr);
    RemoveMaterial(kMT_Orbit, mgr);
  }

  if (mFollowedSegmentId != kInvalidUniqueId && mElectric.get() && mAlive) {
    if (const CActor* front = TCastToConstPtr< CActor >(mgr.GetObjectById(mFollowedSegmentId))) {
      mElectric->SetOverrideIPos(GetTranslation());
      mElectric->SetOverrideFPos(front->GetTranslation());
      mElectric->Update(dt);
    }
  }
}

void COctapedeSegment::Render(const CStateManager& mgr) const {
  if (mElectric.get() && mSegmentState == kSegS_Attached) {
    mElectric->Render();
  }
  CWallCrawler::Render(mgr);
}

void COctapedeSegment::PreRender(CStateManager& mgr) { CPatterned::PreRender(mgr); }

void COctapedeSegment::AddToRenderer(const CStateManager& mgr) const {
  CPatterned::AddToRenderer(mgr);
}

// Finds the head of the chain that the given segment follows.
static TUniqueId FindHeadSegment(CStateManager& mgr, TUniqueId segmentId) {
  const CEntity* segment = mgr.GetObjectById(segmentId);
  if (segment) {
    const rstl::vector< SConnection >& connections = segment->GetConnectionList();
    for (rstl::vector< SConnection >::const_iterator it = connections.begin();
         it != connections.end(); ++it) {
      const EScriptObjectState state = it->state;
      const EScriptObjectMessage msg = it->msg;
      const TUniqueId targetId = mgr.GetIdForScript(it->objId);
      if (state == kSS_Approach && msg != kSM_Action) {
        if (TCastToConstPtr< COctapedeSegment >(mgr.GetObjectById(targetId))) {
          return FindHeadSegment(mgr, targetId);
        }
      }
    }
  }
  return segmentId;
}

void COctapedeSegment::Death(CStateManager& mgr, const CVector3f& direction,
                             EScriptObjectState state) {
  if (BodyController()->IsFrozen()) {
    MassiveFrozenDeath(mgr);
    return;
  }

  AnimationData()->SetEffectState(rstl::string_l("electric_ball"), true, mgr);
  if (COctapedeSegment* head = TCastToPtr< COctapedeSegment >(mgr.ObjectById(mHeadSegmentId))) {
    *head->HealthInfo() = *HealthInfo();
  }
  mAlignToFloor = false;
  const CVector3f up = GetTransform().GetUp();
  SetTranslation(GetTranslation() + (up * mColSphere.GetSphere().GetRadius()) * 0.5f);
  MoveCollisionPrimitive(CVector3f::Zero());
  BodyController()->SetLocomotionType(pas::kLT_Lurk);
  mSegmentState = kSegS_Bouncing;
  RemoveMaterial(kMT_Target, mgr);
  RemoveMaterial(kMT_Orbit, mgr);
  SetMovable(true);
  Stop();
  mgr.DeliverScriptMsg(CScriptMsg(kInvalidUniqueId, GetUniqueId(), kSM_Falling));
  mgr.DeliverScriptMsg(CScriptMsg(kInvalidUniqueId, GetUniqueId(), kSM_Falling));

  mBreakApartSpeed =
      mMinBreakApartSpeed + (mMaxBreakApartSpeed - mMinBreakApartSpeed) * mgr.Random()->Float();
  const float yawDegrees = 360.f * mgr.Random()->Float();
  const float minAngle = mMinBreakApartAngle;
  mBounceDirection = (CMatrix3f::RotateZ(CRelAngle::FromDegrees(yawDegrees)) *
                      CMatrix3f::RotateX(CRelAngle::FromDegrees(-(
                          (mMaxBreakApartAngle - minAngle) * mgr.Random()->Float() + minAngle)))) *
                     mAlignSurface.GetNormal();
  const CVector3f velocity = mBounceDirection * mBreakApartSpeed;
  SetConstantForceWR(GetMass() * velocity);
  SetVelocityWR(velocity);

  const CVector3f spinAxis = CVector3f::Cross(mBounceDirection, GetTransform().GetUp());
  const float minSpin = mMinBreakApartSpinSpeed;
  const float spin = (mMaxBreakApartSpinSpeed - minSpin) * mgr.Random()->Float() + minSpin;
  SetAngularVelocityWR(CAxisAngle::FromVector(spinAxis * spin));
  mDisabledAnimationDeltas = kADF_Rotation;
  const float minRun = mMinRunAroundTime;
  mRunAroundTimer = (mMaxRunAroundTime - minRun) * mgr.Random()->Float() + minRun;
  const int bounceRange = mMaxBounces - mMinBounces;
  if (bounceRange > 0) {
    mBouncesRemaining = mMinBounces + mgr.Random()->Next() % bounceRange;
  } else {
    mBouncesRemaining = mMinBounces;
  }
  if (mStateMachine->HasState()) {
    mStateMachine->SetState(mgr, *this, rstl::string_l("Dead"));
  }
  mBrokenApart = true;
  mDisableMove = true;
  mPlayerObstructed = true;
  StopLoopedSounds();
  if (mFollowedSegmentId == kInvalidUniqueId) {
    const ushort sfx = mSeparateSound;
    ProcessSoundEvent(sfx, 1.f, 0, mSoundFalloff, mMaxAudibleDistance, CSegId(0), 0, 0, 0.f, 20,
                      127, GetDistanceToCamera(mgr), GetTranslation(), mgr.GetNextAreaId().Value(),
                      mgr, true);
  }
  SendScriptMsgs(kSS_InternalState00, mgr, kInvalidUniqueId, kSM_None);
}

void COctapedeSegment::Dead(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    const float turnRange = mMaxTurnInterval - mMinTurnInterval;
    mTurnTimer = turnRange * mgr.Random()->Float() + mMinTurnInterval;
    const CRelAngle angle = CRelAngle::FromDegrees(360.f * mgr.Random()->Float());
    const CQuaternion rotation =
        CQuaternion::AxisAngle(CUnitVector3f(GetTransform().GetUp()), angle);
    mRunDirection = rotation.BuildTransform() * GetTransform().GetForward();
    break;
  }
  case kStateMsg_Update:
    if (mBouncesRemaining <= 0 && mBrokenApart) {
      mSegmentState = kSegS_Landed;
      mRunAroundTimer -= dt;
      mTurnTimer -= dt;
      if (mRunAroundTimer <= 0.f) {
        Explode(mgr);
      } else {
        if (mTurnTimer <= 0.f) {
          const float turnRange = mMaxTurnInterval - mMinTurnInterval;
          mTurnTimer = turnRange * mgr.Random()->Float() + mMinTurnInterval;
          const CRelAngle angle = CRelAngle::FromDegrees(360.f * mgr.Random()->Float());
          const CQuaternion rotation =
              CQuaternion::AxisAngle(CUnitVector3f(GetTransform().GetUp()), angle);
          mRunDirection = rotation.BuildTransform() * GetTransform().GetForward();
        }
        BodyController()->CommandMgr().DeliverCmd(CBCLocomotionCmd(
            ProjectVectorToPlane(mRunDirection, GetTransform().GetUp()), CVector3f::Zero(), 1.f));
      }
    } else if (mStateMachine->GetTime() > 15.f) {
      Explode(mgr);
    }
    break;
  }
}

void COctapedeSegment::Bounce(CStateManager& mgr, const CVector3f& normal, bool counts) {
  if (mBouncesRemaining > 0 && !mBounceLatch) {
    mBounceLatch = true;
    mgr.DeliverScriptMsg(CScriptMsg(kInvalidUniqueId, GetUniqueId(), kSM_Falling));

    const CVector3f velocity = GetVelocityWR();
    const float speed = velocity.Magnitude();
    CVector3f direction = GetTransform().GetForward();
    if (velocity.CanBeNormalized()) {
      const CVector3f velocityDir = velocity.AsNormalized();
      const float scale = 2.f * CVector3f::Dot(normal, velocityDir);
      direction = velocityDir - normal * scale;
      SetVelocityWR((direction * speed) * mBounciness);
    } else {
      SetVelocityWR(mBounciness * normal);
    }
    mBounceDirection = normal;
    const CVector3f axis = CVector3f::Cross(direction, GetTransform().GetUp());
    const float bounciness = mBounciness;
    const float angle = GetAngularVelocityWR().GetAngle();
    SetAngularVelocityWR(CAxisAngle::FromVector((axis * angle) * bounciness));
    if (counts) {
      --mBouncesRemaining;
    }
    const ushort sfx = mBounceSound;
    ProcessSoundEvent(sfx, 1.f, 0, mSoundFalloff, mMaxAudibleDistance, CSegId(0), 0, 0, 0.f, 20,
                      127, GetDistanceToCamera(mgr), GetTranslation(), mgr.GetNextAreaId().Value(),
                      mgr, true);
    if (mBouncesRemaining <= 0) {
      mgr.DeliverScriptMsg(CScriptMsg(kInvalidUniqueId, GetUniqueId(), kSM_Landed));
      Stop();
      SetVelocityWR(CVector3f::Zero());
      SetAngularVelocityWR(CAxisAngle::Identity());
      mDisableMove = false;
      mAlignToFloor = true;
      SetMomentumWR(CVector3f::Zero());
      mHasAlignSurface = false;
      mDisabledAnimationDeltas = 0;
      mPlayerObstructed = false;
    }
    mgr.DeliverScriptMsg(CScriptMsg(kInvalidUniqueId, GetUniqueId(), kSM_Falling));
  } else {
    mBounceLatch = false;
  }
}

void COctapedeSegment::CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                                    CStateManager& mgr) {
  static CMaterialList skBounceMask(skBounceMaterialA, skBounceMaterialB, skBounceMaterialC,
                                    skBounceMaterialD, skBounceMaterialE, skBounceMaterialF);
  if (mBrokenApart) {
    const CEntity* entity = mgr.GetObjectById(id);
    const COctapedeSegment* segment = TCastToConstPtr< COctapedeSegment >(entity);
    const CPlayer* player = TCastToConstPtr< CPlayer >(entity);
    const CActor* actor = TCastToConstPtr< CActor >(entity);
    if (segment || player) {
      Explode(mgr);
    } else if (actor) {
      const CVector3f direction = (GetTranslation() - actor->GetTranslation()).AsNormalized();
      Bounce(mgr, direction, false);
    } else if (list.GetCount() > 0) {
      for (int i = 0; i < list.GetCount(); ++i) {
        const CCollisionInfo& info = list[i];
        if (info.GetMaterialLeft().SharesMaterials(skBounceMask) && mBouncesRemaining > 0) {
          const bool onFloor =
              info.GetMaterialLeft().SharesMaterials(CMaterialList(skFloorMaterial));
          const CVector3f normal = CVector3f::Dot(GetVelocityWR(), info.GetNormalLeft()) > 0.f
                                       ? info.GetNormalRight()
                                       : info.GetNormalLeft();
          Bounce(mgr, normal, onFloor);
        }
      }
    } else {
      Bounce(mgr, GetTransform().GetUp(), true);
    }
  }
  CPatterned::CollidedWith(id, list, mgr);
}

void COctapedeSegment::Explode(CStateManager& mgr) {
  const TUniqueId effectId = FindConnectedObject(mgr, kSS_DGNR, kSM_Activate);
  if (effectId != kInvalidUniqueId) {
    if (CScriptEffect* effect = TCastToPtr< CScriptEffect >(mgr.ObjectById(effectId))) {
      effect->SetTransform(GetTransform());
    }
  }
  const ushort sfx = mExplodeSound;
  ProcessSoundEvent(sfx, 1.f, 0, mSoundFalloff, mMaxAudibleDistance, CSegId(0), 0, 0, 0.f, 20, 127,
                    GetDistanceToCamera(mgr), GetTranslation(), mgr.GetNextAreaId().Value(), mgr,
                    true);
  SendScriptMsgs(kSS_DGNR, mgr, kInvalidUniqueId, kSM_None);
  mgr.ApplyDamageToWorld(GetUniqueId(), *this, GetTranslation(), mExplosionDamage,
                         CMaterialFilter::MakeIncludeExclude(
                             CMaterialList(skExplosionDamageMaterial), CMaterialList()));
  mgr.DeleteObjectRequest(GetUniqueId());
}

void COctapedeSegment::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                       EUserEventType type, float dt) {
  switch (type) {
  case kUE_SoundPlay:
    if (mFollowedSegmentId == kInvalidUniqueId) {
      const float roll = mgr.Random()->Float();
      if (roll < node.GetWeight()) {
        const ushort sfx = mIdleSound;
        ProcessSoundEvent(sfx, 1.f, 0, mSoundFalloff, mMaxAudibleDistance, CSegId(0), 0, 0, 0.f, 20,
                          127, GetDistanceToCamera(mgr), GetTranslation(),
                          mgr.GetNextAreaId().Value(), mgr, true);
      }
    }
    break;
  default:
    break;
  }
  CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}

void COctapedeSegment::ThinkAboutMove(float dt) {
  if (mSegmentState != kSegS_Bouncing) {
    CPatterned::ThinkAboutMove(dt);
  }
}

void COctapedeSegment::Freeze(CStateManager& mgr, const CVector3f& position,
                              CUnitVector3f direction, float duration, float intoFreezeDuration) {
  if (!BodyController()->IsFrozen()) {
    mSharedFreezeDuration = duration;
    CPatterned::Freeze(mgr, position, direction, duration, intoFreezeDuration);
    const uint count = mChildSegmentIds.size();
    for (uint i = 0; i < count; ++i) {
      const TUniqueId childId = mChildSegmentIds[i];
      if (CPatterned* child = static_cast< CPatterned* >(mgr.ObjectById(childId))) {
        child->Freeze(mgr, position, direction, duration, intoFreezeDuration);
      }
    }
  }
}

void COctapedeSegment::Burn(CStateManager& mgr, float duration, float damage) {
  if (mKnockBackController.IsBurnEnabled()) {
    CPatterned::Burn(mgr, duration, damage);
    mSharedBurnDuration = duration;
    mSharedBurnDamage = damage;
    const uint count = mChildSegmentIds.size();
    for (uint i = 0; i < count; ++i) {
      const TUniqueId childId = mChildSegmentIds[i];
      if (CPatterned* child = static_cast< CPatterned* >(mgr.ObjectById(childId))) {
        child->Burn(mgr, duration, damage);
      }
    }
  }
}

CEntity* REL_LoadOctapedeSegment(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrOctapedeSegment sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrOctapedeSegment.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  return rs_new COctapedeSegment(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      *modelData, LdrToPatternedInfo(sldrThis.patterned, nullptr),
      LdrToActorParameters(sldrThis.actorInformation), sldrThis.stickyReach,
      sldrThis.floorTurnSpeed, sldrThis.waypointApproachDistance, sldrThis.visibleDistance,
      sldrThis.projectileBoundsMultiplier, sldrThis.collisionLookAhead, sldrThis.animSpeedScalar,
      sldrThis.maxAudibleDistance, sldrThis.initiallyPaused, sldrThis.unknown_0x4fb8747e,
      sldrThis.betweenSegmentsEffect, sldrThis.minBreakApartSpeed, sldrThis.maxBreakApartSpeed,
      sldrThis.minBreakApartAngle, sldrThis.maxBreakApartAngle, sldrThis.minBreakApartSpinSpeed,
      sldrThis.maxBreakApartSpinSpeed, sldrThis.minRunAroundTime, sldrThis.maxRunAroundTime,
      sldrThis.unknown_0x2caddcbe, sldrThis.unknown_0x4d320455, sldrThis.minBounces,
      sldrThis.maxBounces, sldrThis.bounciness, LdrToDamageInfo(sldrThis.explosionDamage),
      sldrThis.walkSound, sldrThis.idleSound, sldrThis.seperateSound, sldrThis.bounceSound,
      sldrThis.explodeSound, sldrThis.unknown_0x0c4763d7);
}

static void SetFuncPtrs() {
  static SOctapedeSegment_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &REL_LoadOctapedeSegment;
  SetSOctapedeSegment_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSOctapedeSegment_FuncPtrs(nullptr); }
