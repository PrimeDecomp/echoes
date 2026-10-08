#include "MetroidPrime/Enemies/CIngPuddle.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Enemies/CCollisionTracker.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrIngPuddle.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "REL/REL_Setup.h"

// Defined in the main DOL.
template <>
TStateMachineStateBase< CPatterned >::~TStateMachineStateBase();

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"StateOver",
     reinterpret_cast< CPatterned::StateMachine::TriggerFunc >(&CIngPuddle::StateOver)},
    {"IsDead", reinterpret_cast< CPatterned::StateMachine::TriggerFunc >(&CIngPuddle::IsDead)},
    {"HasPatrolPath",
     reinterpret_cast< CPatterned::StateMachine::TriggerFunc >(&CIngPuddle::HasPatrolPath)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Start", reinterpret_cast< CPatterned::StateMachine::StateFunc >(&CIngPuddle::Start)},
    {"Patrol", reinterpret_cast< CPatterned::StateMachine::StateFunc >(&CIngPuddle::Patrol)},
    {"Idle", reinterpret_cast< CPatterned::StateMachine::StateFunc >(&CIngPuddle::Idle)},
    {"Dead", reinterpret_cast< CPatterned::StateMachine::StateFunc >(&CIngPuddle::Dead)},
};

static EMaterialTypes skMaterial0 = kMT_NonSolidDamageable; // Guessed name
static EMaterialTypes skMaterial1 = kMT_Target;             // Guessed name
static EMaterialTypes skMaterial2 = kMT_RadarObject;        // Guessed name

CIngPuddle::CIngPuddle(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                       const CTransform4f& xf, const CVector3f& scale,
                       const CActorParameters& actorParams, const SLdrIngPuddleData& data)
: CPhysicsActor(uid, name, info, 0, xf, CModelData::None(),
                CMaterialList(skMaterial0, skMaterial1, skMaterial2),
                CAABox(CVector3f(-2.f, -2.f, 0.f), CVector3f(2.f, 2.f, 2.f)), SMoverData(1.f),
                actorParams, CPhysicsActor::skDefaultStepData)
, mProperties(data)
, mHealthInfo(LdrToHealthInfo(data.health))
, mDamageVulnerability(LdrToDamageVulnerability(data.vulnerability))
, mTouchBounds(CAABox::MakeNullBox())
, mScale(scale)
, mStateMachineToken(gpSimplePool->GetObj(SObjectTag('FSM2', data.stateMachine)))
, mPatrolState(kPS_Invalid)
, mBlobEffectId(kInvalidUniqueId)
, mMoveDirection(CVector3f::Zero())
, mCurrentWaypointId(kInvalidUniqueId)
, mNextWaypointId(kInvalidUniqueId)
, mLastWaypointPosition(CVector3f::Zero())
, mDeathEffect(data.puddleDeath != kInvalidAssetId
                   ? rstl::optional_object< TLockedToken< CGenDescription > >(
                         TLockedToken< CGenDescription >(
                             gpSimplePool->GetObj(SObjectTag('PART', data.puddleDeath))))
                   : rstl::optional_object< TLockedToken< CGenDescription > >())
, mNormalHitEffect(data.puddleHitNormalDamage != kInvalidAssetId
                       ? rstl::optional_object< TLockedToken< CGenDescription > >(
                             TLockedToken< CGenDescription >(gpSimplePool->GetObj(
                                 SObjectTag('PART', data.puddleHitNormalDamage))))
                       : rstl::optional_object< TLockedToken< CGenDescription > >())
, mHeavyHitEffect(data.puddleHitHeavyDamage != kInvalidAssetId
                      ? rstl::optional_object< TLockedToken< CGenDescription > >(
                            TLockedToken< CGenDescription >(gpSimplePool->GetObj(
                                SObjectTag('PART', data.puddleHitHeavyDamage))))
                      : rstl::optional_object< TLockedToken< CGenDescription > >())
, mSfxHandle()
, mIdling(false) {}

CIngPuddle::~CIngPuddle() {}

void CIngPuddle::PreThink(float dt, CStateManager& mgr) {
  if (GetActive()) {
    const CPlane plane = mSurfaceAlignment.GetSurface().GetPlane();
    if (CVector3f::Dot(mMoveDirection, plane.GetNormal()) < 0.95f) {
      const float distance = plane.GetHeight(GetTranslation());
      const CVector3f onSurface = GetTranslation() - (distance - 0.1f) * plane.GetNormal();
      SetTranslation(CVector3f::Lerp(GetTranslation(), onSurface, 0.6f * dt));
    }
    mMoveDirection = CVector3f::Zero();
  }
  CEntity::PreThink(dt, mgr);
}

void CIngPuddle::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    if (mStateMachine.HasState()) {
      mStateMachine.Update(mgr, reinterpret_cast< CPatterned& >(*this), dt);
      mSurfaceAlignment.Update(*this, mgr, dt);
      UpdateBlobEffect(mgr, dt);
      UpdateTouchBounds();
    } else {
      InitializeStateMachine(mgr);
    }
  }
}

void CIngPuddle::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const float wasActive = GetActive();
  const EScriptObjectMessage message = msg.GetMessage();
  CActor::AcceptScriptMsg(mgr, msg);
  switch (message) {
  case kSM_Create:
    SpawnBlobEffect(mgr, TLockedToken< CGenDescription >(
                             gpSimplePool->GetObj(SObjectTag('PART', mProperties.blobEffect))));
    mSurfaceAlignment.SetMode(CSurfaceAlignmentHelper::kM_NearbySurface);
    mSurfaceAlignment.AlignNearPosition(*this, mgr, GetTranslation(), 1.f);
    break;
  case kSM_Delete:
    mgr.DeleteObjectRequest(mBlobEffectId);
    if (mSfxHandle) {
      CSfxManager::RemoveEmitter(mSfxHandle);
      mSfxHandle.Clear();
    }
    break;
  case kSM_Deactivate:
    if (wasActive) {
      if (CCollisionTracker* effect =
              static_cast< CCollisionTracker* >(mgr.ObjectById(mBlobEffectId))) {
        effect->SetParticleEmissionRateScalar(0.f);
      }
      if (mSfxHandle) {
        CSfxManager::RemoveEmitter(mSfxHandle);
        mSfxHandle.Clear();
      }
    }
    break;
  case kSM_Damage:
    HandleDamage(mgr, msg.GetSenderId());
    break;
  }
}

const CDamageVulnerability* CIngPuddle::GetDamageVulnerability() const {
  return &mDamageVulnerability;
}

CVector3f CIngPuddle::GetAimPosition(const CStateManager& mgr, float dt) const {
  return CPhysicsActor::GetAimPosition(mgr, dt);
}

void CIngPuddle::Touch(CActor& actor, CStateManager& mgr) { CActor::Touch(actor, mgr); }

rstl::optional_object< CAABox > CIngPuddle::GetTouchBounds() const { return mTouchBounds; }

bool CIngPuddle::StateOver(CStateManager& mgr, const float& arg) {
  return mPatrolState == kPS_Finished;
}

bool CIngPuddle::IsDead(CStateManager& mgr, const float& arg) { return mHealthInfo.GetHP() <= 0.f; }

bool CIngPuddle::HasPatrolPath(CStateManager& mgr, const float& arg) {
  return CheckConnectedObject(mgr, kSS_Patrol, kSM_Follow) != kInvalidUniqueId;
}

void CIngPuddle::Start(CStateManager& mgr, int msg, float dt) {}

void CIngPuddle::Idle(CStateManager& mgr, int msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mIdling = true;
    break;
  case kStateMsg_Update:
    if (!mSfxHandle) {
      mSfxHandle = CSfxManager::AddEmitter(mProperties.sound_IngSpotIdle, GetTranslation(), 127,
                                           GetCurrentAreaId().Value(), true, true,
                                           CSfxManager::kMedPriority);
    }
    break;
  case kStateMsg_Deactivate:
    mIdling = false;
    if (mSfxHandle) {
      CSfxManager::RemoveEmitter(mSfxHandle);
      mSfxHandle.Clear();
    }
    break;
  }
}

void CIngPuddle::Dead(CStateManager& mgr, int msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    RemoveMaterial(kMT_Target, kMT_Orbit, mgr);
    if (mDeathEffect.valid()) {
      CEntity* explosion = rs_new CExplosion(
          *mDeathEffect, mgr.AllocateUniqueId(),
          CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true, kInvalidEditorId),
          rstl::string_l("IngPuddleDeathFx"), GetTransform(), 0, mScale, CColor::White(), -1);
      if (explosion != nullptr) {
        mgr.AddObject(explosion);
      }
    }
    CSfxManager::AddEmitter(mProperties.sound_IngSpotDeath, GetTranslation(), 127,
                            GetCurrentAreaId().Value(), true, false, CSfxManager::kMedPriority);
    mgr.DeleteObjectRequest(GetUniqueId());
    break;
  }
  }
}

void CIngPuddle::Patrol(CStateManager& mgr, int msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mNextWaypointId = mCurrentWaypointId == kInvalidUniqueId
                          ? CheckConnectedObject(mgr, kSS_Patrol, kSM_Follow)
                          : mCurrentWaypointId;
    mLastWaypointPosition = GetTranslation();
    mPatrolState = kPS_Patrolling;
    break;
  case kStateMsg_Update: {
    CScriptWaypoint* next = nullptr;
    CScriptWaypoint* waypoint = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(mNextWaypointId));
    if (waypoint != nullptr) {
      const CVector3f position = GetTranslation();
      const CVector3f toLast = mLastWaypointPosition - position;
      const CVector3f toWaypoint = waypoint->GetTranslation() - position;
      if (CVector3f::Dot(toLast, toWaypoint) > 0.f) {
        mCurrentWaypointId = mNextWaypointId;
        mNextWaypointId = waypoint->CheckConnectedObject(mgr, kSS_Arrived, kSM_Next);
        mLastWaypointPosition = position;
        mgr.SendScriptMsg(waypoint, GetUniqueId(), kSM_Arrived, GetUniqueId());
      }
      next =
          TCastToPtr< CScriptWaypoint >(const_cast< CEntity* >(mgr.GetObjectById(mNextWaypointId)));
      if (next != nullptr) {
        const CVector3f toNext = next->GetTranslation() - position;
        if (toNext.IsMagnitudeSafe()) {
          CScriptAIWaypoint* aiWaypoint = TCastToPtr< CScriptAIWaypoint >(next);
          const float speedScale = aiWaypoint != nullptr ? aiWaypoint->GetSpeed() : 1.f;
          MoveAlongSurface(toNext.AsNormalized(), mProperties.puddleSpeed * speedScale, dt);
        }
      }
    } else {
      mPatrolState = kPS_Finished;
    }
    if (!mSfxHandle) {
      mSfxHandle = CSfxManager::AddEmitter(mProperties.sound_IngSpotMove, GetTranslation(), 127,
                                           GetCurrentAreaId().Value(), true, true,
                                           CSfxManager::kMedPriority);
    } else {
      CSfxManager::UpdateEmitter(mSfxHandle, GetTranslation(), GetTransform().GetForward(), 127);
    }
    break;
  }
  case kStateMsg_Deactivate:
    if (mSfxHandle) {
      CSfxManager::RemoveEmitter(mSfxHandle);
      mSfxHandle.Clear();
    }
    break;
  }
}

const CGenericFSM2* CIngPuddle::GetStateMachine() const {
  if (mStateMachineToken->IsLoaded()) {
    TToken< CGenericFSM2 > machine(*mStateMachineToken);
    return *machine;
  }
  return nullptr;
}

void CIngPuddle::InitializeStateMachine(CStateManager& mgr) {
  const CGenericFSM2* machine = GetStateMachine();
  if (machine != nullptr) {
    mStateMachine.Setup(*machine);
    mStateMachine.SetTriggerFunctions(skTriggers, 3);
    mStateMachine.SetStateFunctions(skStates, 4);
    mStateMachine.SetState(mgr, reinterpret_cast< CPatterned& >(*this), rstl::string_l("Start"));
  }
}

void CIngPuddle::SpawnBlobEffect(CStateManager& mgr, const TLockedToken< CGenDescription >& desc) {
  const TUniqueId id = mgr.AllocateUniqueId();
  CCollisionTracker* effect =
      rs_new CCollisionTracker(desc, id, GetCurrentAreaId(), true, rstl::string_l("IngBlobEffect"),
                               CTransform4f::Translate(GetTranslation()), GetUniqueId(), 0,
                               CCollisionTracker::skDefaultExtents);
  if (effect != nullptr) {
    mBlobEffectId = id;
    effect->SetParticleEmissionRateScalar(0.f);
    mgr.AddObject(effect);
  }
}

void CIngPuddle::UpdateBlobEffect(CStateManager& mgr, float dt) {
  if (CCollisionTracker* effect =
          static_cast< CCollisionTracker* >(mgr.ObjectById(mBlobEffectId))) {
    if (mHealthInfo.GetHP() > 0.f) {
      const CVector3f normal = mSurfaceAlignment.GetSurface().GetNormal();
      const CVector3f position = GetTranslation();
      const CVector3f& up =
          CMath::AbsF(normal.GetZ() < 0.95f) != 0.f ? CVector3f::Up() : CVector3f::Right();
      effect->SetTransform(CTransform4f::LookAt(position, position + normal, up));
      effect->SetParticleEmissionRateScalar(1.f);
    } else {
      effect->SetParticleEmissionRateScalar(0.f);
    }
  }
}

void CIngPuddle::MoveAlongSurface(const CVector3f& direction, float speed, float dt) {
  const CVector3f position = GetTranslation();
  const CVector3f normal = mSurfaceAlignment.GetSurface().GetNormal();
  const float along = CVector3f::Dot(direction, normal);
  const CVector3f projected =
      along * along < 0.6f * direction.MagSquared() ? direction - along * normal : direction;
  if (projected.IsMagnitudeSafe()) {
    const CVector3f unit = projected.AsNormalized();
    SetTranslation(position + dt * (speed * unit));
    mMoveDirection = unit;
  } else {
    mMoveDirection = CVector3f::Zero();
  }
}

void CIngPuddle::UpdateTouchBounds() {
  const CVector3f position = GetTranslation();
  const CVector3f extent(2.f, 2.f, 2.f);
  mTouchBounds = CAABox(position - extent, position + extent);
}

void CIngPuddle::HandleDamage(CStateManager& mgr, TUniqueId weaponId) {
  CWeapon* weapon = TCastToPtr< CWeapon >(const_cast< CEntity* >(mgr.GetObjectById(weaponId)));
  if (weapon == nullptr) {
    return;
  }
  CExplosion* explosion = nullptr;
  if (weapon->GetCurrentDamageInfo().GetDamage(*GetDamageVulnerability()) >= 25.f) {
    if (mHeavyHitEffect.valid()) {
      explosion = rs_new CExplosion(
          *mHeavyHitEffect, mgr.AllocateUniqueId(),
          CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true, kInvalidEditorId),
          rstl::string_l("IngPuddleDamageFx"), GetTransform(), 0, mScale, CColor::White(), -1);
    }
    CSfxManager::AddEmitter(mProperties.sound_HitHeavyDamage, GetTranslation(), 127,
                            GetCurrentAreaId().Value(), true, false, CSfxManager::kMedPriority);
  } else {
    if (mNormalHitEffect.valid()) {
      explosion = rs_new CExplosion(
          *mNormalHitEffect, mgr.AllocateUniqueId(),
          CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true, kInvalidEditorId),
          rstl::string_l("IngPuddleDamageFx"), GetTransform(), 0, mScale, CColor::White(), -1);
    }
    CSfxManager::AddEmitter(mProperties.sound_HitNormalDamage, GetTranslation(), 127,
                            GetCurrentAreaId().Value(), true, false, CSfxManager::kMedPriority);
  }
  if (explosion != nullptr) {
    mgr.AddObject(explosion);
  }
}

CEntity* REL_LoadIngPuddle(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrIngPuddle sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrIngPuddle.inc"

  return rs_new CIngPuddle(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      sldrThis.editorProperties.transform.scale, LdrToActorParameters(sldrThis.actorInformation),
      sldrThis.ingPuddleProperties);
}

static void SetFuncPtrs() {
  static SIngPuddle_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &REL_LoadIngPuddle;
  SetSIngPuddle_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSIngPuddle_FuncPtrs(nullptr); }
