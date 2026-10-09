#include "MetroidPrime/Enemies/CIngSnatchingSwarm.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrIngSnatchingSwarm.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "REL/REL_Setup.h"

#include <limits.h>

// Defined in the main DOL.
template <>
TStateMachineStateBase< CPatterned >::~TStateMachineStateBase();

// Guessed name: a possible target together with the number of swarms already chasing it.
struct SSnatchCandidate {
  SSnatchCandidate(TUniqueId id, int swarmCount) : mId(id), mSwarmCount(swarmCount) {}

  TUniqueId mId;
  int mSwarmCount;
};

static EMaterialTypes skImpactSolid = kMT_Solid; // Guessed name

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"StateOver",
     reinterpret_cast< CPatterned::StateMachine::TriggerFunc >(&CIngSnatchingSwarm::StateOver)},
    {"IsDead",
     reinterpret_cast< CPatterned::StateMachine::TriggerFunc >(&CIngSnatchingSwarm::IsDead)},
    {"LifetimeOver",
     reinterpret_cast< CPatterned::StateMachine::TriggerFunc >(&CIngSnatchingSwarm::LifetimeOver)},
    {"ShouldLoiter",
     reinterpret_cast< CPatterned::StateMachine::TriggerFunc >(&CIngSnatchingSwarm::ShouldLoiter)},
    {"HasTarget",
     reinterpret_cast< CPatterned::StateMachine::TriggerFunc >(&CIngSnatchingSwarm::HasTarget)},
    {"InSnatchingRange", reinterpret_cast< CPatterned::StateMachine::TriggerFunc >(
                             &CIngSnatchingSwarm::InSnatchingRange)},
    {"HitPlayer",
     reinterpret_cast< CPatterned::StateMachine::TriggerFunc >(&CIngSnatchingSwarm::HitPlayer)},
    {"HitTarget",
     reinterpret_cast< CPatterned::StateMachine::TriggerFunc >(&CIngSnatchingSwarm::HitTarget)},
    {"HitWorld",
     reinterpret_cast< CPatterned::StateMachine::TriggerFunc >(&CIngSnatchingSwarm::HitWorld)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Start", reinterpret_cast< CPatterned::StateMachine::StateFunc >(&CIngSnatchingSwarm::Start)},
    {"Dead", reinterpret_cast< CPatterned::StateMachine::StateFunc >(&CIngSnatchingSwarm::Dead)},
    {"ExitPortal",
     reinterpret_cast< CPatterned::StateMachine::StateFunc >(&CIngSnatchingSwarm::ExitPortal)},
    {"FollowArcPath",
     reinterpret_cast< CPatterned::StateMachine::StateFunc >(&CIngSnatchingSwarm::FollowArcPath)},
    {"DiveToTarget",
     reinterpret_cast< CPatterned::StateMachine::StateFunc >(&CIngSnatchingSwarm::DiveToTarget)},
    {"SnatchTarget",
     reinterpret_cast< CPatterned::StateMachine::StateFunc >(&CIngSnatchingSwarm::SnatchTarget)},
};

static CPatterned::StateMachine::SCodeFunction skCodes[] = {
    {"SetPortalSwarmPath", reinterpret_cast< CPatterned::StateMachine::CodeFunc >(
                               &CIngSnatchingSwarm::SetPortalSwarmPath)},
    {"SelectTarget",
     reinterpret_cast< CPatterned::StateMachine::CodeFunc >(&CIngSnatchingSwarm::SelectTarget)},
    {"SetTargetDest",
     reinterpret_cast< CPatterned::StateMachine::CodeFunc >(&CIngSnatchingSwarm::SetTargetDest)},
    {"SetPlayerDest",
     reinterpret_cast< CPatterned::StateMachine::CodeFunc >(&CIngSnatchingSwarm::SetPlayerDest)},
    {"DamagePlayer",
     reinterpret_cast< CPatterned::StateMachine::CodeFunc >(&CIngSnatchingSwarm::DamagePlayer)},
    {"Explode",
     reinterpret_cast< CPatterned::StateMachine::CodeFunc >(&CIngSnatchingSwarm::Explode)},
};

static EMaterialTypes skMaterial0 = kMT_NonSolidDamageable;               // Guessed name
static EMaterialTypes skMaterial1 = kMT_Target;                           // Guessed name
static EMaterialTypes skMaterial2 = kMT_RadarObject;                      // Guessed name
static EMaterialTypes skCollisionSolid = kMT_Solid;                       // Guessed name
static EMaterialTypes skCollisionPassthrough = kMT_ProjectilePassthrough; // Guessed name

SIngSnatchingSwarmData::SIngSnatchingSwarmData(
    CAssetId stateMachine, CAssetId swarmParticle, CAssetId secondaryParticle,
    float loiterGeneratorRate, float trailDelayScale, float lifetime, float maxLinearSpeed,
    float maxLinearAcceleration, float maxTurnSpeed, bool useSteering, bool ignorePlayer,
    float unknown0xe6b57a25, float exitPortalDistance, float loiterTime, float loiterTimeVariation,
    float arcJitter, float beginSnatchingRange, CAssetId explosionEffect,
    const CDamageInfo& impactDamage, ushort impactSound, ushort idleSound, ushort moveSound,
    float health, const CDamageVulnerability& vulnerability)
: mStateMachine(stateMachine)
, mSwarmParticle(swarmParticle)
, mSecondaryParticle(secondaryParticle)
, mTrailDelayScale(trailDelayScale)
, mLoiterGeneratorRate(loiterGeneratorRate)
, mLifetime(lifetime)
, mMaxLinearSpeed(maxLinearSpeed)
, mMaxLinearAcceleration(maxLinearAcceleration)
, mMaxTurnSpeed(0.017453292f * maxTurnSpeed)
, mUnknown0xe6b57a25(unknown0xe6b57a25)
, mExitPortalDistance(exitPortalDistance)
, mLoiterTime(loiterTime)
, mLoiterTimeVariation(loiterTimeVariation)
, mArcJitter(arcJitter)
, mBeginSnatchingRangeSq(beginSnatchingRange * beginSnatchingRange)
, mExplosionEffect(explosionEffect)
, mImpactDamage(impactDamage)
, mImpactSound(impactSound)
, mIdleSound(idleSound)
, mMoveSound(moveSound)
, mHealth(health)
, mVulnerability(vulnerability) {
  mUseSteeringForMovement = useSteering;
  mIgnorePlayer = ignorePlayer;
}

CIngSnatchingSwarm::CIngSnatchingSwarm(TUniqueId uid, const rstl::string& name,
                                       const CEntityInfo& info, const CTransform4f& xf,
                                       const CVector3f& scale, const SIngSnatchingSwarmData& data)
: CActor(uid, name, info, 0, xf, CModelData::None(),
         CMaterialList(skMaterial0, skMaterial1, skMaterial2), CActorParameters::None(),
         kInvalidUniqueId)
, mData(data)
, mHealthInfo(data.mHealth, 10.f)
, mPathFindSearch(nullptr, 3, 0, 1.f, 1.f, 0, CPFRegion::kRP_Center)
, mDeltaTime(0.f)
, mAge(0.f)
, mTouchBounds(CAABox::MakeNullBox())
, mExplosionEffect(data.mExplosionEffect != kInvalidAssetId
                       ? rstl::optional_object< TLockedToken< CGenDescription > >(
                             TLockedToken< CGenDescription >(
                                 gpSimplePool->GetObj(SObjectTag('PART', data.mExplosionEffect))))
                       : rstl::optional_object< TLockedToken< CGenDescription > >())
, mHitNormal(CVector3f::Zero())
, mHitPoint(CVector3f::Zero())
, mScale(scale)
, mSpeed(0.f)
, mVelocity(CVector3f::Zero())
, mPreviousPosition(CVector3f::Zero())
, mStateMachineToken(gpSimplePool->GetObj(SObjectTag('FSM2', data.mStateMachine)))
, mPathState(kPS_Idle)
, mSwarmToken(gpSimplePool->GetObj(
      SObjectTag(gpResourceFactory->GetResourceTypeById(data.mSwarmParticle), data.mSwarmParticle)))
, mSwarmParticle(CreateParticle(scale, xf.GetTranslation(), mSwarmToken))
, mSecondaryToken(gpSimplePool->GetObj(SObjectTag(
      gpResourceFactory->GetResourceTypeById(data.mSecondaryParticle), data.mSecondaryParticle)))
, mSecondaryParticle(CreateParticle(scale, xf.GetTranslation(), mSecondaryToken))
, mStartPosition(xf.GetTranslation())
, mStartForward(xf.GetForward())
, mHitId(kInvalidUniqueId)
, mTargetId(kInvalidUniqueId)
, mUnknownVector(CVector3f::Zero())
, mLoiterEndTime(0.f)
, mArcLength(0.f)
, mArcSpeedScale(1.f)
, mHistoryIndex(0)
, mSfxHandle()
, mAlive(true)
, mStarted(false)
, mCheckCollision(false)
, mHitWorld(false)
, mLoiterEnded(false)
, mVisible(false) {}

CIngSnatchingSwarm::~CIngSnatchingSwarm() {}

void CIngSnatchingSwarm::PreThink(float dt, CStateManager& mgr) {
  mDeltaTime = dt;
  CEntity::PreThink(dt, mgr);
}

void CIngSnatchingSwarm::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    if (mStateMachine.HasState()) {
      mStateMachine.Update(mgr, reinterpret_cast< CPatterned& >(*this), dt);
      UpdateMovement(dt);
      CheckWorldCollision(mgr);
      UpdateParticles(mgr, dt);
      UpdateSfxEmitter();
      UpdateTouchBounds();
      mAge += dt;
    } else {
      InitializeStateMachine(mgr);
    }
  }
}

void CIngSnatchingSwarm::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CActor::AcceptScriptMsg(mgr, msg);
  switch (message) {
  case kSM_Create:
  case kSM_Activate:
    mStartPosition = GetTransform().GetTranslation();
    mStartForward = GetTransform().GetForward();
    break;
  case kSM_Delete:
    RemoveSfxEmitter();
    break;
  case kSM_AreaLoaded:
    mPathFindSearch.SetArea(
        mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()).GetPostConstructed()->mPathArea);
    if (mData.mIgnorePlayer == true) {
      CMaterialFilter filter = GetMaterialFilter();
      filter.ExcludeList().Add(kMT_Player);
      SetMaterialFilter(filter);
    }
    break;
  case kSM_Damage:
    mHitPoint = GetTranslation();
    mHitNormal = GetTransform().GetForward();
    break;
  }
}

void CIngSnatchingSwarm::Render(const CStateManager& mgr) const {
  if (mVisible) {
    CActor::Render(mgr);
    mSecondaryParticle->Render();
    mSwarmParticle->Render();
  }
}

void CIngSnatchingSwarm::PreRender(CStateManager& mgr) {
  if (mVisible) {
    CActor::PreRender(mgr);
  }
}

void CIngSnatchingSwarm::AddToRenderer(const CStateManager& mgr) const {
  if (mVisible) {
    gpRender->AddParticleGen(*mSwarmParticle);
    gpRender->AddParticleGen(*mSecondaryParticle);
  }
}

const CDamageVulnerability* CIngSnatchingSwarm::GetDamageVulnerability() const {
  return &mData.mVulnerability;
}

CVector3f CIngSnatchingSwarm::GetAimPosition(const CStateManager& mgr, float dt) const {
  return GetTranslation() + (mSpeed * dt) * GetTransform().GetForward();
}

void CIngSnatchingSwarm::Touch(CActor& actor, CStateManager& mgr) {
  if (CPhysicsActor* physicsActor = TCastToPtr< CPhysicsActor >(actor)) {
    if (mData.mIgnorePlayer == true && TCastToPtr< CPlayer >(physicsActor) != nullptr) {
      return;
    }
    const CAABox bounds = physicsActor->GetBoundingBox();
    const CVector3f position = GetTranslation();
    const CVector3f extent(1.f, 1.f, 1.f);
    if (bounds.DoBoundsOverlap(CAABox(position - extent, position + extent))) {
      CCollisionActor* collisionActor = TCastToPtr< CCollisionActor >(actor);
      if (collisionActor == nullptr) {
        mHitId = actor.GetUniqueId();
      } else {
        mHitId = collisionActor->GetOwnerId();
      }
    }
  }
}

rstl::optional_object< CAABox > CIngSnatchingSwarm::GetTouchBounds() const { return mTouchBounds; }

bool CIngSnatchingSwarm::StateOver(CStateManager& mgr, const float& arg) {
  return mPathState == kPS_Finished;
}

bool CIngSnatchingSwarm::IsDead(CStateManager& mgr, const float& arg) {
  return mAlive && mHealthInfo.GetHP() <= 0.f;
}

bool CIngSnatchingSwarm::LifetimeOver(CStateManager& mgr, const float& arg) {
  return mAge >= mLoiterEndTime + mData.mLifetime;
}

bool CIngSnatchingSwarm::ShouldLoiter(CStateManager& mgr, const float& arg) {
  return mAge < mLoiterEndTime;
}

bool CIngSnatchingSwarm::HasTarget(CStateManager& mgr, const float& arg) {
  const CEntity* target = mgr.GetObjectById(mTargetId);
  if (target != nullptr) {
    if (const CPatterned* patterned = TCastToConstPtr< CPatterned >(target)) {
      return patterned->GetAlive();
    }
    return true;
  }
  return false;
}

bool CIngSnatchingSwarm::InSnatchingRange(CStateManager& mgr, const float& arg) {
  const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
  if (target != nullptr) {
    const CVector3f delta = target->GetTranslation() - GetTranslation();
    return delta.MagSquared() <= mData.mBeginSnatchingRangeSq;
  }
  return false;
}

bool CIngSnatchingSwarm::HitPlayer(CStateManager& mgr, const float& arg) {
  return TCastToConstPtr< CPlayer >(mgr.GetObjectById(mHitId)) != nullptr;
}

bool CIngSnatchingSwarm::HitTarget(CStateManager& mgr, const float& arg) {
  return mHitId == mTargetId;
}

bool CIngSnatchingSwarm::HitWorld(CStateManager& mgr, const float& arg) { return mHitWorld; }

void CIngSnatchingSwarm::Start(CStateManager& mgr, int msg, float dt) {
  if (msg == kStateMsg_Activate) {
    mStarted = true;
  }
}

void CIngSnatchingSwarm::Dead(CStateManager& mgr, int msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAlive = false;
    mStarted = false;
    RemoveSfxEmitter();
    RemoveMaterial(kMT_Target, mgr);
    mSwarmParticle->SetGeneratorRate(0.f);
    mSecondaryParticle->SetGeneratorRate(0.f);
    break;
  case kStateMsg_Update:
    mSwarmParticle->SetGeneratorRate(0.f);
    mSecondaryParticle->SetGeneratorRate(0.f);
    if (mSwarmParticle->GetParticleCount() == 0 && mSecondaryParticle->GetParticleCount() == 0) {
      mgr.DeleteObjectRequest(GetUniqueId());
      SendScriptMsgs(kSS_Dead, mgr, kInvalidUniqueId, kSM_None);
    }
    break;
  }
}

void CIngSnatchingSwarm::ExitPortal(CStateManager& mgr, int msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    mPathState = kPS_Following;
    const CVector3f from = GetTranslation();
    const CVector3f base = mStartPosition + mData.mExitPortalDistance * mStartForward;
    mArcPoints.clear();
    mArcPoints.push_back(from);
    mArcPoints.push_back(from + CVector3f(mgr.Random()->Range(-1.f, 1.f),
                                          mgr.Random()->Range(-1.f, 1.f),
                                          mgr.Random()->Range(-1.f, 1.f)));
    mArcPoints.push_back(base + CVector3f(mgr.Random()->Range(-1.f, 1.f),
                                          mgr.Random()->Range(-1.f, 1.f),
                                          mgr.Random()->Range(-1.f, 1.f)));
    mArcPoints.push_back(base);
    mArcLength = CalculateArcLength();
    mArcSpeedScale = 1.f;
    const float loiterTime = mData.mLoiterTime;
    mLoiterEndTime = loiterTime + mgr.Random()->Range(0.f, mData.mLoiterTimeVariation);
    break;
  }
  case kStateMsg_Update:
    if (!FollowArc(mStateMachine.GetTime())) {
      mPathState = kPS_Finished;
    }
    break;
  }
}

void CIngSnatchingSwarm::FollowArcPath(CStateManager& mgr, int msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mPathState = kPS_Following;
    break;
  case kStateMsg_Update:
    if (!FollowArc(mStateMachine.GetTime())) {
      mPathState = kPS_Finished;
    }
    break;
  }
}

void CIngSnatchingSwarm::DiveToTarget(CStateManager& mgr, int msg, float dt) {
  switch (msg) {
  case kStateMsg_Update: {
    const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
    if (target != nullptr) {
      const CVector3f aimPosition = target->GetAimPosition(mgr, 0.f);
      MoveTowards(aimPosition, mData.mMaxLinearSpeed, dt);
    }
    break;
  }
  }
}

void CIngSnatchingSwarm::SnatchTarget(CStateManager& mgr, int msg, float dt) {
  switch (msg) {
  case kStateMsg_Update:
    if (CPatterned* target = TCastToPtr< CPatterned >(mgr.ObjectById(mTargetId))) {
      const float step = dt * mData.mMaxLinearSpeed;
      const CVector3f aimPosition = target->GetAimPosition(mgr, 0.f);
      const CVector3f delta = aimPosition - GetTranslation();
      if (delta.MagSquared() < step * step) {
        if (!target->IsIngPossessed()) {
          const CVector3f hitDirection = target->GetTransform().GetForward();
          target->BodyController()->CommandMgr().DeliverCmd(
              CBCKnockBackCmd(hitDirection, pas::kS_One, target->GetIngPossessionAnimation()));
          target->SendScriptMsgs(kSS_IngSnatch, mgr, kInvalidUniqueId, kSM_None);
        }
        target->SetIngPossessed(true, mgr);
        if (target->GetAttackTarget() == GetUniqueId()) {
          target->SetAttackTarget(mgr, kInvalidUniqueId);
        }
        mTargetId = kInvalidUniqueId;
      } else {
        MoveTowards(aimPosition, mData.mMaxLinearSpeed, dt);
      }
    }
    break;
  }
}

void CIngSnatchingSwarm::SelectTarget(CStateManager& mgr, float dt) {
  rstl::reserved_vector< SSnatchCandidate, 1024 > candidates;
  const CObjectList& list = mgr.GetObjectListById(kOL_Actor);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    const CPatterned* patterned = TCastToConstPtr< CPatterned >(list[i]);
    if (patterned != nullptr && patterned->GetActive() && patterned->GetAlive() &&
        patterned->GetCurrentAreaId() == GetCurrentAreaId() && patterned->CanBeIngPossessed(mgr)) {
      candidates.push_back(
          SSnatchCandidate(patterned->GetUniqueId(), patterned->IsIngPossessed() ? 1 : 0));
    }
  }
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    const CIngSnatchingSwarm* swarm = TCastToConstPtr< CIngSnatchingSwarm >(list[i]);
    if (swarm != nullptr && swarm != this) {
      const int count = candidates.size();
      for (int j = 0; j < count; ++j) {
        SSnatchCandidate& candidate = candidates[j];
        if (candidate.mId == swarm->mTargetId) {
          ++candidate.mSwarmCount;
          break;
        }
      }
    }
  }
  const CVector3f position = GetTranslation();
  int bestCount = INT_MAX;
  float bestDistance = 3.4028235e38f;
  for (int i = 0; i < candidates.size(); ++i) {
    const CActor* entity = static_cast< const CActor* >(mgr.GetObjectById(candidates[i].mId));
    if (entity != nullptr) {
      const CVector3f delta = entity->GetTranslation() - position;
      const float distance = delta.MagSquared();
      if (candidates[i].mSwarmCount < bestCount) {
        bestDistance = distance;
        bestCount = candidates[i].mSwarmCount;
        mTargetId = candidates[i].mId;
      } else if (distance < bestDistance && bestCount == candidates[i].mSwarmCount) {
        bestDistance = distance;
        mTargetId = candidates[i].mId;
      }
    }
  }
}

void CIngSnatchingSwarm::SetPortalSwarmPath(CStateManager& mgr, float dt) {
  mArcPoints.clear();
  mArcPoints.push_back(GetTranslation());
  const CVector3f base = mStartPosition + mData.mExitPortalDistance * mStartForward;
  for (int i = 1; i < 4; ++i) {
    const float jitter = mData.mArcJitter;
    mArcPoints.push_back(base + CVector3f(mgr.Random()->Range(-jitter, jitter),
                                          mgr.Random()->Range(-jitter, jitter),
                                          mgr.Random()->Range(-jitter, jitter)));
  }
  mArcLength = CalculateArcLength();
  mArcSpeedScale = 1.f;
}

void CIngSnatchingSwarm::SetPlayerDest(CStateManager& mgr, float dt) {
  mTargetId = mgr.GetPlayer(0)->GetUniqueId();
  mCheckCollision = true;
  mArcPoints.clear();
  const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
  if (target != nullptr) {
    const CVector3f destination = target->GetAimPosition(mgr, 0.f);
    const CVector3f from = GetTranslation();
    CVector3f next = from;
    if (mData.mUseSteeringForMovement ||
        mPathFindSearch.Search(from, destination) != CPathFindSearch::kR_Success) {
      const CVector3f direction = destination - from;
      if (direction.IsMagnitudeSafe()) {
        next = from + 5.f * direction.AsNormalized();
      }
    } else {
      mPathFindSearch.GetSplinePointWithLookahead(next, from, 0.5f * mData.mMaxLinearSpeed);
    }
    const CVector3f quarter = 0.25f * (next - from);
    mArcPoints.push_back(from);
    mArcPoints.push_back(from + quarter + RandomArcOffset(mgr));
    mArcPoints.push_back(next - quarter + RandomArcOffset(mgr));
    mArcPoints.push_back(next);
  }
  mArcLength = CalculateArcLength();
  mArcSpeedScale = 1.f - mgr.Random()->Range(0.f, 0.5f);
}

void CIngSnatchingSwarm::SetTargetDest(CStateManager& mgr, float dt) {
  mCheckCollision = true;
  mArcPoints.clear();
  CPatterned* target = TCastToPtr< CPatterned >(mgr.ObjectById(mTargetId));
  if (target != nullptr) {
    const CVector3f destination = target->GetAimPosition(mgr, 0.f);
    const CVector3f from = GetTranslation();
    CVector3f next = from;
    if (mData.mUseSteeringForMovement ||
        mPathFindSearch.Search(from, destination) != CPathFindSearch::kR_Success) {
      const CVector3f direction = destination - from;
      if (direction.IsMagnitudeSafe()) {
        next = from + 5.f * direction.AsNormalized();
      }
    } else {
      mPathFindSearch.GetSplinePointWithLookahead(next, from, 5.f);
    }
    const CVector3f quarter = 0.25f * (next - from);
    mArcPoints.push_back(from);
    mArcPoints.push_back(from + quarter + RandomArcOffset(mgr));
    mArcPoints.push_back(next - quarter + RandomArcOffset(mgr));
    mArcPoints.push_back(next);
    if (!target->IsIngPossessed()) {
      target->SetAttackTarget(mgr, GetUniqueId());
      mgr.DeliverScriptMsg(CScriptMsg(GetUniqueId(), target->GetUniqueId(), kSM_Alert));
    }
  }
  mArcLength = CalculateArcLength();
  mArcSpeedScale = 1.f - mgr.Random()->Range(0.f, 0.5f);
}

void CIngSnatchingSwarm::DamagePlayer(CStateManager& mgr, float dt) {
  mgr.ApplyDamage(
      GetUniqueId(), mHitId, GetUniqueId(), mData.mImpactDamage,
      CMaterialFilter::MakeIncludeExclude(CMaterialList(skImpactSolid), CMaterialList()),
      CVector3f::Zero());
}

void CIngSnatchingSwarm::Explode(CStateManager& mgr, float dt) {
  if (mExplosionEffect.valid()) {
    const CVector3f position = GetTranslation();
    const CVector3f direction =
        (mHitNormal.GetX() != 0.f || mHitNormal.GetY() != 0.f || mHitNormal.GetZ() != 0.f)
            ? mHitNormal
            : GetTransform().GetForward();
    const CTransform4f xf = CTransform4f::LookAt(position, position + direction, CVector3f::Up());
    CEntity* explosion = rs_new CExplosion(
        *mExplosionEffect, mgr.AllocateUniqueId(),
        CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true, kInvalidEditorId),
        rstl::string_l("IngSwarmExplode"), xf, 0, mScale, CColor::White(), -1);
    if (explosion != nullptr) {
      mgr.AddObject(explosion);
      CSfxManager::AddEmitter(mData.mImpactSound, GetTranslation(), 127, GetCurrentAreaId().Value(),
                              true, false, CSfxManager::kMedPriority);
    }
  }
}

const CGenericFSM2* CIngSnatchingSwarm::GetStateMachine() const {
  if (mStateMachineToken->IsLoaded()) {
    TToken< CGenericFSM2 > machine(*mStateMachineToken);
    return *machine;
  }
  return nullptr;
}

void CIngSnatchingSwarm::InitializeStateMachine(CStateManager& mgr) {
  const CGenericFSM2* machine = GetStateMachine();
  if (machine != nullptr) {
    mStateMachine.Setup(*machine);
    mStateMachine.SetTriggerFunctions(skTriggers, 9);
    mStateMachine.SetStateFunctions(skStates, 6);
    mStateMachine.SetCodeFunctions(skCodes, 6);
    mStateMachine.SetState(mgr, reinterpret_cast< CPatterned& >(*this), rstl::string_l("Start"));
  }
}

float CIngSnatchingSwarm::CalculateArcLength() const {
  if (mArcPoints.size() == 4) {
    const CVector3f quarter =
        CMath::GetBezierPoint(mArcPoints[0], mArcPoints[1], mArcPoints[2], mArcPoints[3], 0.25f);
    const CVector3f half =
        CMath::GetBezierPoint(mArcPoints[0], mArcPoints[1], mArcPoints[2], mArcPoints[3], 0.5f);
    const CVector3f threeQuarters =
        CMath::GetBezierPoint(mArcPoints[0], mArcPoints[1], mArcPoints[2], mArcPoints[3], 0.75f);
    return (half - quarter).Magnitude() + (quarter - mArcPoints[0]).Magnitude() +
           (threeQuarters - half).Magnitude() + (mArcPoints[3] - threeQuarters).Magnitude();
  }
  return 0.f;
}

bool CIngSnatchingSwarm::FollowArc(float time) {
  const float speed = mArcSpeedScale * mData.mMaxLinearSpeed;
  if (mArcPoints.size() == 4 && speed > 0.f) {
    const float duration = mArcLength / speed;
    if (duration > 0.f) {
      const float t = time / duration;
      const float clamped = t < 1.f ? t : 1.f;
      const CVector3f point = CMath::GetBezierPoint(mArcPoints[0], mArcPoints[1], mArcPoints[2],
                                                    mArcPoints[3], clamped);
      const CVector3f delta = point - GetTranslation();
      const float length = delta.Magnitude();
      if (length > 1.1920929e-7f) {
        mVelocity = (speed / length) * delta;
      }
      return t < 1.f;
    }
  }
  return false;
}

void CIngSnatchingSwarm::MoveTowards(const CVector3f& target, float speed, float dt) {
  mSpeed = 0.f;
  const CVector3f position = GetTranslation();
  const CVector3f offset = target - position;
  mPreviousPosition = position;
  const float distance = offset.Magnitude();
  const float step = speed * dt;
  if (distance > step) {
    SetTranslation(position + (step / distance) * offset);
  } else {
    SetTranslation(target);
  }
}

CVector3f CIngSnatchingSwarm::RandomArcOffset(CStateManager& mgr) const {
  const CVector3f right = GetTransform().GetRight();
  const CVector3f up = GetTransform().GetUp();
  const float upAmount = mgr.Random()->Range(0.f, 4.f);
  const float sideAmount = mgr.Random()->Range(-2.f, 2.f);
  return sideAmount * right + upAmount * up;
}

void CIngSnatchingSwarm::UpdateParticles(CStateManager& mgr, float dt) {
  if (!gpMain->IsMaxSpeed()) {
    const CVector3f position = GetTranslation();
    if (mPositionHistory.size() == 0) {
      rstl::reserved_vector< CVector3f, 60 > history(60, position);
      mPositionHistory = history;
    }
    mPositionHistory[mHistoryIndex] = position;
    ++mHistoryIndex;
    mHistoryIndex %= 60;
    mSwarmParticle->SetTranslation(position);
    if (!mLoiterEnded && mAlive) {
      const float zero = 0.f;
      if (!ShouldLoiter(mgr, zero)) {
        mSwarmParticle->SetGeneratorRate(1.f);
        mLoiterEnded = true;
      } else {
        mSwarmParticle->SetGeneratorRate(mData.mLoiterGeneratorRate);
      }
    }
    mSwarmParticle->Update(dt);
    const int offset = static_cast< int >(60.f * (dt / (1.f / 60.f)) * mData.mTrailDelayScale);
    const int index = (mHistoryIndex - offset + 60) % 60;
    mSecondaryParticle->SetTranslation(mPositionHistory[index]);
    mSecondaryParticle->Update(dt);
    rstl::optional_object< CAABox > bounds = mSwarmParticle->GetBounds();
    const rstl::optional_object< CAABox > secondaryBounds = mSecondaryParticle->GetBounds();
    if (!bounds.valid()) {
      bounds = secondaryBounds;
    } else if (secondaryBounds.valid()) {
      bounds->Include(*secondaryBounds);
    }
    if (bounds.valid()) {
      SetOtherBounds(*bounds);
      SetRenderBounds(*bounds);
    }
  }
  if (mAge >= 0.5f) {
    mVisible = true;
  }
}

void CIngSnatchingSwarm::UpdateMovement(float dt) {
  const float maxSpeed = mData.mMaxLinearSpeed;
  const float magnitude = mVelocity.Magnitude();
  float target = maxSpeed < magnitude ? maxSpeed : magnitude;
  const float acceleration = dt * mData.mMaxLinearAcceleration;
  if (mSpeed <= target) {
    mSpeed = CMath::Min(target, mSpeed + acceleration);
  } else {
    mSpeed = CMath::Max(mSpeed - acceleration, target);
  }
  const CVector3f position = GetTranslation();
  mPreviousPosition = position;
  if (target > 0.f) {
    const CVector3f forward = GetTransform().GetForward();
    const float angle = CVector3f::GetAngleDiff(forward, mVelocity);
    const float maxTurn = dt * mData.mMaxTurnSpeed;
    CVector3f direction = mVelocity.AsNormalized();
    if (angle > maxTurn) {
      direction = CVector3f::Slerp(forward, direction, CRelAngle::FromRadians(maxTurn));
    }
    SetTransform(CTransform4f::LookAt(position, position + direction, CVector3f::Up()));
  }
  SetTranslation(position + (mSpeed * dt) * GetTransform().GetForward());
  mVelocity = CVector3f::Zero();
}

void CIngSnatchingSwarm::UpdateTouchBounds() {
  const CVector3f position = GetTranslation();
  const CVector3f extent(1.f, 1.f, 1.f);
  mTouchBounds = CAABox(position - extent, position + extent);
}

void CIngSnatchingSwarm::CheckWorldCollision(CStateManager& mgr) {
  if (mCheckCollision) {
    const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
        CMaterialList(skCollisionSolid), CMaterialList(skCollisionPassthrough));
    CVector3f direction = GetTranslation() - mPreviousPosition;
    if (direction.IsMagnitudeSafe()) {
      const float length = direction.Magnitude();
      direction *= 1.f / length;
      const CRayCastResult result =
          CGameCollision::RayStaticIntersection(mgr, mPreviousPosition, direction, length, filter);
      if (result.IsValid()) {
        mHitPoint = mPreviousPosition + result.GetTime() * direction;
        mHitNormal = result.GetPlane().GetNormal();
        mHitWorld = true;
      }
    }
  }
}

void CIngSnatchingSwarm::UpdateSfxEmitter() {
  if (mStarted) {
    if (!mSfxHandle) {
      mSfxHandle = CSfxManager::AddEmitter(mData.mMoveSound, GetTranslation(), 127,
                                           GetCurrentAreaId().Value(), true, true,
                                           CSfxManager::kMedPriority);
    } else {
      CSfxManager::UpdateEmitter(mSfxHandle, GetTranslation(), GetTransform().GetForward(), 127);
    }
  }
}

void CIngSnatchingSwarm::RemoveSfxEmitter() {
  if (mSfxHandle) {
    CSfxManager::RemoveEmitter(mSfxHandle);
    mSfxHandle.Clear();
  }
}

CParticleGen* CIngSnatchingSwarm::CreateParticle(const CVector3f& scale,
                                                 const CVector3f& translation,
                                                 const CToken& token) {
  const CObjectReference* ref = token.GetRef();
  return CElementGen::ConstructChildParticleSystem(
      token, ref->GetTag().GetType(), 0, CElementGen::kOSF_One, false, true, translation,
      CTransform4f::Identity(), CVector3f::Zero(), CTransform4f::Identity(), scale, CColor::White(),
      CVector3f::One());
}

CEntity* LoadIngSnatchingSwarm(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrIngSnatchingSwarm sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrIngSnatchingSwarm.inc"

  const SIngSnatchingSwarmData data(
      sldrThis.stateMachine, sldrThis.swarmParticleSystem, sldrThis.secondarySwarmParticleSystem,
      sldrThis.unknown_0x7cae2ed5, sldrThis.unknown_0xf65e7ec5, sldrThis.lifetime,
      sldrThis.maxLinearSpeed, sldrThis.maxLinearAcceleration, sldrThis.maxTurnSpeed,
      sldrThis.useSteeringForMovement, sldrThis.ignorePlayer, sldrThis.unknown_0xe6b57a25,
      sldrThis.exitPortalDistance, sldrThis.unknown_0x2de5a19a, sldrThis.unknown_0x4e79f717,
      sldrThis.unknown_0xe8e0b5a6, sldrThis.beginSnatchingRange, sldrThis.pART,
      LdrToDamageInfo(sldrThis.impactDamage), sldrThis.sound_Impact, sldrThis.sound_Idle,
      sldrThis.sound_Move, sldrThis.health, LdrToDamageVulnerability(sldrThis.swarmVulnerability));

  return rs_new CIngSnatchingSwarm(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                                   LdrToEntityInfo(info, sldrThis.editorProperties),
                                   LdrToTransform4f(sldrThis.editorProperties),
                                   sldrThis.editorProperties.transform.scale, data);
}

#ifndef MONOLITHIC
static void SetFuncPtrs() {
  static SIngSnatchingSwarm_FuncPtrs funcPtrs;
  funcPtrs.mLoadIngSnatchingSwarm = &LoadIngSnatchingSwarm;
  SetSIngSnatchingSwarm_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSIngSnatchingSwarm_FuncPtrs(nullptr); }
#endif
