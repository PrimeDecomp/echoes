#include "MetroidPrime/Enemies/CIngSpiderballGuardian.hpp"

#include "Kyoto/Animation/CCECharacterInfo.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Animation/CPOINode.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CEffectWaypointPredicate.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrIngSpiderballGuardian.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIWaypoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "REL/REL_Setup.h"
#include <float.h>

static EMaterialTypes skProximityDamageMaterial = kMT_Solid; // Guessed name

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"StateOver",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CIngSpiderballGuardian::StateOver)},
    {"Damaged",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CIngSpiderballGuardian::Damaged)},
    {"Stunned",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CIngSpiderballGuardian::Stunned)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Start", static_cast< CPatterned::StateMachine::StateFunc >(&CIngSpiderballGuardian::Start)},
    {"Patrol", static_cast< CPatterned::StateMachine::StateFunc >(&CIngSpiderballGuardian::Patrol)},
    {"DamageReaction",
     static_cast< CPatterned::StateMachine::StateFunc >(&CIngSpiderballGuardian::DamageReaction)},
    {"StunnedReaction",
     static_cast< CPatterned::StateMachine::StateFunc >(&CIngSpiderballGuardian::StunnedReaction)},
    {"Dead", static_cast< CPatterned::StateMachine::StateFunc >(&CIngSpiderballGuardian::Dead)},
};

static CPatterned::StateMachine::SCodeFunction skCodeFuncs[] = {
    {"EnableEnergyBar",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CIngSpiderballGuardian::EnableEnergyBar)},
    {"BeginNextPuzzleSection", static_cast< CPatterned::StateMachine::CodeFunc >(
                                   &CIngSpiderballGuardian::BeginNextPuzzleSection)},
};

static EMaterialTypes skCreateExcludeMaterial0 = kMT_Solid;    // Guessed name
static EMaterialTypes skCreateExcludeMaterial1 = kMT_Platform; // Guessed name
static EMaterialTypes skCreateExcludeMaterial2 = kMT_Player;   // Guessed name
static EMaterialTypes skWaypointRayMaterial = kMT_Platform;    // Guessed name

// Guessed names; effects shown for the patrolling, damaged and stunned states.
static const char* const skEffectNames[] = {
    "DamageSphereBlue",
    "DamageSphere",
    "DamageSphereGreen",
    "DamageSphere",
};

// Guessed name; hits needed in each puzzle phase before the guardian charges.
static const int skHitsToCharge[] = {1, 1, 1, 1, 2, 3};

CIngSpiderballGuardian::CIngSpiderballGuardian(TUniqueId uid, const rstl::string& name,
                                               const CEntityInfo& info, const CTransform4f& xf,
                                               const CModelData& modelData,
                                               const CActorParameters& actorParams,
                                               const CPatternedInfo& patternedInfo,
                                               const SLdrIngSpiderballGuardianData& data)
: CPatterned(kPAI_IngSpiderballGuardian, uid, name, kFT_Zero, info, xf, modelData, patternedInfo,
             kMT_Flyer, kCT_One, kBT_Flyer, actorParams)
, mData(data)
, mProximityDamage(LdrToDamageInfo(data.proximityDamage))
, x944_(kInvalidUniqueId)
, mRollSfx()
, mState(kGS_Patrol)
, mSoundState(kGS_Patrol)
, mHitCount(0)
, mPhase(0)
, mPrevWaypointId(kInvalidUniqueId)
, mTargetWaypointId(kInvalidUniqueId)
, mPrevWaypointPos(GetTranslation())
, mAnimSpeed(1.f)
, mRadius(GetModelData()->GetScale().GetZ())
, mDefaultSpeed(mSpeed)
, mWaypointSpeed(1.f)
, mCurrentSpeed(0.f)
, mTargetSpeed(data.ingSpiderballGuardianStruct.minPatrolSpeed)
, mStunHealth(data.ingSpiderballGuardianStruct.stunnedHitPoints)
, mStunTimer(0.f)
, mRecoveryDelay(0.f)
, mChargeTimer(0.f)
, mEffectsActive(false)
, mProximityDamageEnabled(true)
, mRolling(true)
, mStunned(false)
, mCanBeDamaged(true)
, mPlayStunSound(false)
, mReturnToPatrolRequested(false) {
  const CPASDatabase& pasDatabase = GetAnimationData()->GetPASDatabase();
  const CPASAnimParmData parms(pas::kAS_Locomotion, CPASAnimParm::FromEnum(0),
                               CPASAnimParm::FromEnum(1));
  const rstl::pair< float, int > anim = pasDatabase.FindBestAnimation(parms, -1);
  if (anim.second != -1) {
    mAnimSpeed = GetAverageAnimVelocity(anim.second);
  }
  mKnockBackController.EnableAllAnimReactions(false);
  mKnockBackController.EnableKnockBackPhysics(false);
}

CIngSpiderballGuardian::~CIngSpiderballGuardian() {}

void CIngSpiderballGuardian::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const TUniqueId senderId = msg.GetSenderId();
  const EScriptObjectMessage message = msg.GetMessage();
  CPatterned::AcceptScriptMsg(mgr, msg);
  switch (message) {
  case kSM_Create:
    BodyController()->Activate(mgr, pas::kAS_Invalid);
    SetMaterialFilter(CMaterialFilter::MakeExclude(CMaterialList(
        skCreateExcludeMaterial0, skCreateExcludeMaterial1, skCreateExcludeMaterial2)));
    break;
  case kSM_Delete:
    if (mgr.GetBossId() == GetUniqueId()) {
      mgr.SetBossParams(kInvalidUniqueId, 0.f, 0);
    }
    if (mRollSfx) {
      CSfxManager::RemoveEmitter(mRollSfx);
      mRollSfx.Clear();
    }
    break;
  case kSM_Deactivate:
    if (mRollSfx) {
      CSfxManager::RemoveEmitter(mRollSfx);
      mRollSfx.Clear();
    }
    break;
  case kSM_Damage:
    mHitByPlayerProjectile = true;
    break;
  case kSM_ResistedDamage:
    if (mCanBeDamaged) {
      if (const CWeapon* weapon = TCastToConstPtr< CWeapon >(mgr.GetObjectById(senderId))) {
        ApplyStunDamage(
            mgr, weapon->GetCurrentDamageInfo().GetVulnerableDamage(*GetDamageVulnerability()));
        mDamageColor = skHitsWithoutDamageColor;
        mDamageCooldownTimer = 2.f;
      }
    }
    break;
  case kSM_InternalMessage00:
    mCanBeDamaged = false;
    break;
  case kSM_InternalMessage01:
    mCanBeDamaged = true;
    break;
  case kSM_InternalMessage02:
    mProximityDamageEnabled = false;
    break;
  case kSM_InternalMessage03:
    mProximityDamageEnabled = true;
    break;
  case kSM_InternalMessage04:
    if (++mHitCount >= skHitsToCharge[mPhase]) {
      SetGuardianState(mgr, kGS_Charging);
    }
    if (mState == kGS_Stunned || mState == kGS_Charging) {
      mRecoveryDelay = 1.5f;
    }
    break;
  case kSM_InternalMessage05:
    mReturnToPatrolRequested = true;
    break;
  }
}

void CIngSpiderballGuardian::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    CPatterned::Think(dt, mgr);
    ApplyProximityDamage(mgr);
    UpdateRollSound();
    UpdateStateTimers(mgr, dt);
  }
}

void CIngSpiderballGuardian::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                             EUserEventType type, float dt) {
  bool handled = false;
  switch (type) {
  case kUE_SoundPlay:
    if (mState == kGS_Patrol || mState == kGS_Damaged) {
      const CPlayer* player = mgr.GetPlayer(0);
      const CVector3f position = GetTranslation();
      const CVector3f toPlayer = player->GetTranslation() - position;
      if (toPlayer.MagSquared() < mData.unknown_0x32133b39 * mData.unknown_0x32133b39) {
        if (mgr.Random()->Range(0.f, 1.f) <= node.GetWeight()) {
          PlayCustomSound(position, GetTransform().GetUp(), mData.audioPlaybackParms, false);
        }
      }
    }
    handled = true;
    break;
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

void CIngSpiderballGuardian::Render(const CStateManager& mgr) const { CPatterned::Render(mgr); }

void CIngSpiderballGuardian::PreRender(CStateManager& mgr) { CPatterned::PreRender(mgr); }

void CIngSpiderballGuardian::AddToRenderer(const CStateManager& mgr) const {
  CPatterned::AddToRenderer(mgr);
}

const CDamageVulnerability* CIngSpiderballGuardian::GetDamageVulnerability() const {
  return CPatterned::GetDamageVulnerability();
}

CVector3f CIngSpiderballGuardian::GetAimPosition(const CStateManager& mgr, float dt) const {
  return CPatterned::GetAimPosition(mgr, dt);
}

void CIngSpiderballGuardian::Death(CStateManager& mgr, const CVector3f& direction,
                                   EScriptObjectState state) {
  BodyController()->CommandMgr().DeliverCmd(CBCKnockDownCmd(direction, pas::kS_One));
  if (mStateMachine->HasState()) {
    mStateMachine->SetState(mgr, *this, rstl::string_l("Dead"));
  }
}

void CIngSpiderballGuardian::SetupStateMachine(CStateManager& mgr) {
  StateMachine* stateMachine = mStateMachine.get();
  CPatterned::SetupStateMachine(mgr);
  stateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
  stateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
  stateMachine->SetCodeFunctions(skCodeFuncs, ARRAY_SIZE(skCodeFuncs));
}

bool CIngSpiderballGuardian::StateOver(CStateManager& mgr, const CTriggerData& data) const {
  return mAnimationState.IsOver();
}

bool CIngSpiderballGuardian::Damaged(CStateManager& mgr, const CTriggerData& data) const {
  return mHitByPlayerProjectile;
}

bool CIngSpiderballGuardian::Stunned(CStateManager& mgr, const CTriggerData& data) const {
  return mStunned;
}

void CIngSpiderballGuardian::Start(CStateManager& mgr, EStateMsg msg, float dt) {}

void CIngSpiderballGuardian::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    if (mTargetWaypointId == kInvalidUniqueId) {
      mTargetWaypointId = mPrevWaypointId == kInvalidUniqueId
                              ? CheckConnectedObject(mgr, kSS_Patrol, kSM_Follow)
                              : mPrevWaypointId;
    }
    mPrevWaypointPos = GetTranslation();
    UpdateEffects(mgr, true);
    mDisabledAnimationDeltas = 1;
    mCurrentSpeed = 0.f;
    break;
  case kStateMsg_Update:
    if (!mHitByPlayerProjectile && !mStunned) {
      SelectNextWaypoint(mgr);
      MoveAlongWaypoints(mgr, dt);
    }
    break;
  }
}

void CIngSpiderballGuardian::DamageReaction(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    UpdateEffects(mgr, false);
    mHitByPlayerProjectile = false;
    mSpeed = mDefaultSpeed;
    mDisabledAnimationDeltas = 1;
    mDamageColor = skDamageColor;
    mDamageCooldownTimer = 4.f;
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_KnockBack)) {
      BodyController()->CommandMgr().DeliverCmd(
          CBCKnockBackCmd(GetTransform().GetForward(), pas::kS_One));
    } else if (BodyController()->GetCurrentStateId() == pas::kAS_KnockBack) {
      mRolling = false;
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mRolling = true;
    if (const CScriptWaypoint* waypoint =
            TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(mTargetWaypointId))) {
      const CVector3f position = GetTranslation();
      const CVector3f waypointUp = waypoint->GetTransform().GetUp();
      const CVector3f toTarget = waypoint->GetTranslation() + mRadius * waypointUp - position;
      if (toTarget.IsMagnitudeSafe()) {
        SetTransform(CTransform4f::LookAt(position, position + toTarget, waypointUp));
      }
    }
    break;
  }
}

void CIngSpiderballGuardian::StunnedReaction(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    UpdateEffects(mgr, false);
    mSpeed = mDefaultSpeed;
    mDisabledAnimationDeltas = 1;
    mDamageColor = skHitsWithoutDamageColor;
    mDamageCooldownTimer = 2.f;
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_KnockBack)) {
      BodyController()->CommandMgr().DeliverCmd(
          CBCKnockBackCmd(GetTransform().GetForward(), pas::kS_Zero));
    } else if (BodyController()->GetCurrentStateId() == pas::kAS_KnockBack) {
      mRolling = false;
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mStunned = false;
    mRolling = true;
    if (const CScriptWaypoint* waypoint =
            TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(mTargetWaypointId))) {
      const CVector3f position = GetTranslation();
      const CVector3f waypointUp = waypoint->GetTransform().GetUp();
      const CVector3f toTarget = waypoint->GetTranslation() + mRadius * waypointUp - position;
      if (toTarget.IsMagnitudeSafe()) {
        SetTransform(CTransform4f::LookAt(position, position + toTarget, waypointUp));
      }
    }
    if (mPlayStunSound) {
      PlayCustomSound(GetTranslation(), GetTransform().GetUp(), mData.sound_EnterStunned, false);
      mPlayStunSound = false;
    }
    break;
  }
}

void CIngSpiderballGuardian::Dead(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    UpdateEffects(mgr, false);
    mSpeed = mDefaultSpeed;
    mRolling = false;
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mDisabledAnimationDeltas = 0;
    mgr.SetBossParams(kInvalidUniqueId, 0.f, 0);
    break;
  case kStateMsg_Update:
    CPatterned::Dead(mgr, msg, dt);
    break;
  }
}

void CIngSpiderballGuardian::EnableEnergyBar(CStateManager& mgr, float dt) {
  mgr.SetBossParams(GetUniqueId(), GetHealthInfo()->GetInitialHP(),
                    gpStringTable->GetStringIndex("BossSpiderBallGuardian"));
}

void CIngSpiderballGuardian::BeginNextPuzzleSection(CStateManager& mgr, float dt) {
  ++mPhase;
  SetGuardianState(mgr, kGS_Patrol);
}

void CIngSpiderballGuardian::SelectNextWaypoint(CStateManager& mgr) {
  const TUniqueId previousId = mPrevWaypointId;
  CScriptWaypoint* waypoint = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(mTargetWaypointId));
  if (waypoint != nullptr) {
    const CVector3f position = GetTranslation();
    const CVector3f targetPoint =
        waypoint->GetTranslation() + mRadius * waypoint->GetTransform().GetUp();
    const CVector3f travelled = targetPoint - mPrevWaypointPos;
    const CVector3f remaining = targetPoint - position;
    if (CVector3f::Dot(travelled, remaining) <= 0.f) {
      mPrevWaypointId = mTargetWaypointId;
      mTargetWaypointId = kInvalidUniqueId;
      mPrevWaypointPos = position;
      waypoint->SendScriptMsgs(kSS_Arrived, mgr, GetUniqueId(), kSM_None);

      const rstl::vector< TUniqueId > connections =
          waypoint->FindConnectedObjects_if(mgr, kSS_Arrived, kSM_Next, CEffectWaypointPredicate());

      int turnPreference = 0;
      if (const CScriptAIWaypoint* aiWaypoint = TCastToPtr< CScriptAIWaypoint >(waypoint)) {
        turnPreference = aiWaypoint->GetTurnPreference();
        mWaypointSpeed = aiWaypoint->GetSpeed();
        mRolling = !(aiWaypoint->GetFlags() & 2);
      } else {
        mWaypointSpeed = 1.f;
        mRolling = true;
      }

      const CMaterialFilter filter =
          CMaterialFilter::MakeInclude(CMaterialList(skWaypointRayMaterial));
      float bestScore = FLT_MAX;
      bool found = false;
      const CVector3f forward = GetTransform().GetForward();
      const CVector3f right = GetTransform().GetRight();
      for (rstl::vector< TUniqueId >::const_iterator it = connections.begin();
           it != connections.end(); ++it) {
        const CScriptWaypoint* candidate =
            TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(*it));
        if (candidate == nullptr || !candidate->GetActive()) {
          continue;
        }

        const CVector3f candidatePoint =
            candidate->GetTranslation() + mRadius * candidate->GetTransform().GetUp();
        const CVector3f toCandidate = candidatePoint - position;
        float score = 0.f;
        if (turnPreference == 1) {
          score = CVector3f::GetAngleDiff(-forward, toCandidate);
          if (CVector3f::Dot(right, toCandidate) > 0.f) {
            score = M_2PIF - score;
          }
        } else if (turnPreference == 2) {
          score = CVector3f::GetAngleDiff(-forward, toCandidate);
          if (CVector3f::Dot(right, toCandidate) < 0.f) {
            score = M_2PIF - score;
          }
        }
        if (*it == previousId) {
          score += M_2PIF;
        }
        if (!found || score < bestScore) {
          if (mgr.RayCollideWorld(position, candidatePoint, filter, this)) {
            bestScore = score;
            found = true;
            mTargetWaypointId = *it;
          } else if (mReturnToPatrolRequested && mRecoveryDelay <= 0.f) {
            if (mState == kGS_Stunned || mState == kGS_Charging) {
              mTargetWaypointId = *it;
              SetGuardianState(mgr, kGS_Patrol);
              break;
            }
          } else if (!found) {
            mTargetWaypointId = *it;
          }
        }
      }
    }
  } else {
    mTargetWaypointId = previousId;
    mPrevWaypointId = mTargetWaypointId;
  }
}

void CIngSpiderballGuardian::MoveAlongWaypoints(CStateManager& mgr, float dt) {
  const CScriptWaypoint* waypoint =
      TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(mTargetWaypointId));
  if (waypoint == nullptr) {
    return;
  }

  const SLdrIngSpiderballGuardianStruct& props = GetPhaseProperties();
  const CVector3f position = GetTranslation();
  const float maxTurn =
      dt *
      CRelAngle::FromDegrees(props.angularSpeed * mCurrentSpeed / props.maxPatrolSpeed).AsRadians();
  const CVector3f waypointUp = waypoint->GetTransform().GetUp();
  const CVector3f currentUp = GetTransform().GetUp();
  CVector3f newUp = waypointUp;
  if (CVector3f::GetAngleDiff(waypointUp, currentUp) > maxTurn) {
    newUp = CVector3f::Slerp(currentUp, waypointUp, CRelAngle::FromRadians(maxTurn));
  }

  const CVector3f toTarget = waypoint->GetTranslation() + mRadius * waypointUp - position;
  CVector3f direction =
      toTarget.IsMagnitudeSafe() ? toTarget.AsNormalized() : GetTransform().GetForward();
  mCurrentSpeed = CMath::Min(mTargetSpeed, mCurrentSpeed + props.linearAcceleration * dt);
  if (mAnimSpeed > FLT_EPSILON) {
    mSpeed = mCurrentSpeed * mWaypointSpeed / mAnimSpeed;
  }

  const CVector3f displacement = dt * (mCurrentSpeed * mWaypointSpeed) * direction;
  CVector3f moveDirection = direction;
  const CVector3f forward = GetTransform().GetForward();
  if (CVector3f::GetAngleDiff(moveDirection, forward) > maxTurn) {
    const CVector3f right = GetTransform().GetRight();
    if (CMath::IsEpsilon(CVector3f::Dot(right, moveDirection), 0.f, 0.00001f)) {
      moveDirection = right;
    }
    direction = CVector3f::Slerp(forward, moveDirection, CRelAngle::FromRadians(maxTurn));
  }

  const CVector3f newPosition = position + displacement;
  SetTransform(CTransform4f::LookAt(newPosition, newPosition + direction, newUp));
}

void CIngSpiderballGuardian::ApplyProximityDamage(CStateManager& mgr) {
  if (mEffectsActive && mProximityDamageEnabled && mCurDamageRemTime <= 0.f) {
    CPlayer* player = mgr.GetPlayer(0);
    const CVector3f toPlayer = player->GetAimPosition(mgr, 0.f) - GetTranslation();
    if (toPlayer.MagSquared() <= mData.damageRadius * mData.damageRadius) {
      mgr.ApplyDamage(GetUniqueId(), player->GetUniqueId(), GetUniqueId(), mProximityDamage,
                      CMaterialFilter::MakeIncludeExclude(CMaterialList(skProximityDamageMaterial),
                                                          CMaterialList()),
                      CVector3f::Zero());
      mCurDamageRemTime = mDamageWaitTime;
    }
  }
}

void CIngSpiderballGuardian::UpdateEffects(CStateManager& mgr, bool active) {
  mEffectsActive = active;
  CAnimData* animData = AnimationData();
  switch (mState) {
  case kGS_Patrol:
    animData->SetEffectState(rstl::string_l(skEffectNames[0]), active, mgr);
    animData->SetEffectState(rstl::string_l(skEffectNames[1]), false, mgr);
    animData->SetEffectState(rstl::string_l(skEffectNames[2]), false, mgr);
    break;
  case kGS_Stunned:
    animData->SetEffectState(rstl::string_l(skEffectNames[0]), false, mgr);
    animData->SetEffectState(rstl::string_l(skEffectNames[1]), false, mgr);
    animData->SetEffectState(rstl::string_l(skEffectNames[2]), active, mgr);
    break;
  case kGS_Damaged:
  case kGS_Charging:
    animData->SetEffectState(rstl::string_l(skEffectNames[0]), false, mgr);
    animData->SetEffectState(rstl::string_l(skEffectNames[1]), active, mgr);
    animData->SetEffectState(rstl::string_l(skEffectNames[2]), false, mgr);
    break;
  default:
    animData->SetEffectState(rstl::string_l(skEffectNames[0]), false, mgr);
    animData->SetEffectState(rstl::string_l(skEffectNames[1]), false, mgr);
    animData->SetEffectState(rstl::string_l(skEffectNames[2]), false, mgr);
    break;
  }
}

void CIngSpiderballGuardian::UpdateStateTimers(CStateManager& mgr, float dt) {
  switch (mState) {
  case kGS_Stunned:
    mStunTimer -= dt;
    mRecoveryDelay -= dt;
    if (mStunTimer <= 0.f && mRecoveryDelay <= 0.f && mCanBeDamaged) {
      SetGuardianState(mgr, kGS_Patrol);
    }
    break;
  case kGS_Charging:
    mChargeTimer -= dt;
    mRecoveryDelay -= dt;
    if (mChargeTimer <= 0.f && mRecoveryDelay <= 0.f && mCanBeDamaged) {
      SetGuardianState(mgr, kGS_Patrol);
    }
    break;
  }
}

void CIngSpiderballGuardian::UpdateRollSound() {
  if (mRolling) {
    if (mSoundState != mState) {
      mSoundState = mState;
      CSfxManager::RemoveEmitter(mRollSfx);
      mRollSfx.Clear();
    }
    if (!mRollSfx) {
      switch (mSoundState) {
      case kGS_Stunned:
        mRollSfx = PlayCustomSound(GetTranslation(), GetTransform().GetUp(),
                                   mData.sound_SpiderballSlowRolling, true);
        break;
      case kGS_Charging:
        mRollSfx = PlayCustomSound(GetTranslation(), GetTransform().GetUp(),
                                   mData.sound_SpiderballFastRolling, true);
        break;
      case kGS_Patrol:
      case kGS_Damaged:
      default:
        mRollSfx = PlayCustomSound(GetTranslation(), GetTransform().GetUp(),
                                   mData.sound_SpiderballRolling, true);
        break;
      }
    } else {
      CSfxManager::UpdateEmitter(mRollSfx, GetTranslation(), GetTransform().GetUp(), 127);
    }
  } else if (mRollSfx) {
    CSfxManager::RemoveEmitter(mRollSfx);
    mRollSfx.Clear();
  }
}

void CIngSpiderballGuardian::SetGuardianState(CStateManager& mgr, EGuardianState state) {
  const SLdrIngSpiderballGuardianStruct& props = GetPhaseProperties();
  if (mState == kGS_Stunned && state != kGS_Charging && state != mState) {
    SendScriptMsgs(kSS_InternalState01, mgr, GetUniqueId(), kSM_None);
  }

  switch (state) {
  case kGS_Patrol:
    if (mState != kGS_Patrol) {
      SendScriptMsgs(kSS_InternalState02, mgr, GetUniqueId(), kSM_None);
    }
    mTargetSpeed = props.minPatrolSpeed;
    mStunHealth = props.stunnedHitPoints;
    mStunTimer = 0.f;
    mRecoveryDelay = 0.f;
    mHitCount = 0;
    mReturnToPatrolRequested = false;
    break;
  case kGS_Damaged:
    mTargetSpeed = (props.stunnedHitPoints - mStunHealth) / props.stunnedHitPoints *
                       (props.maxPatrolSpeed - props.minPatrolSpeed) +
                   props.minPatrolSpeed;
    break;
  case kGS_Stunned:
    if (mState != kGS_Stunned) {
      SendScriptMsgs(kSS_InternalState00, mgr, GetUniqueId(), kSM_None);
      mPlayStunSound = true;
    }
    mStunTimer = props.stunnedTime;
    mRecoveryDelay = 0.f;
    mTargetSpeed = props.stunnedSpeed;
    break;
  case kGS_Charging:
    mTargetSpeed = props.maxPatrolSpeed;
    mChargeTimer = props.maxChargeTime;
    if (mState != kGS_Charging) {
      PlayCustomSound(GetTranslation(), GetTransform().GetUp(), mData.audioPlaybackParms_0x44c1f241,
                      false);
      SendScriptMsgs(kSS_InternalState03, mgr, GetUniqueId(), kSM_None);
    }
    break;
  }

  mState = state;
  if (mEffectsActive) {
    UpdateEffects(mgr, true);
  }
}

void CIngSpiderballGuardian::ApplyStunDamage(CStateManager& mgr, float damage) {
  switch (mState) {
  case kGS_Patrol:
  case kGS_Damaged:
    mStunHealth -= damage;
    if (mStunHealth <= 0.f) {
      SetGuardianState(mgr, kGS_Stunned);
    } else {
      SetGuardianState(mgr, kGS_Damaged);
    }
    mStunned |= damage > 0.f;
    break;
  case kGS_Stunned:
    SetGuardianState(mgr, kGS_Stunned);
    mStunned |= damage > 0.f;
    break;
  case kGS_Charging:
    break;
  }
}

const SLdrIngSpiderballGuardianStruct& CIngSpiderballGuardian::GetPhaseProperties() const {
  switch (mPhase) {
  case 5:
    return mData.ingSpiderballGuardianStruct_0xc463268c;
  case 4:
    return mData.ingSpiderballGuardianStruct_0xfc58adff;
  case 3:
    return mData.ingSpiderballGuardianStruct_0x5d612911;
  case 2:
    return mData.ingSpiderballGuardianStruct_0x8c2fbb19;
  case 1:
    return mData.ingSpiderballGuardianStruct_0x2d163ff7;
  case 0:
    return mData.ingSpiderballGuardianStruct;
  default:
    return mData.ingSpiderballGuardianStruct;
  }
}

CEntity* LoadIngSpiderballGuardian(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrIngSpiderballGuardian sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrIngSpiderballGuardian.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  return rs_new CIngSpiderballGuardian(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      *modelData, LdrToActorParameters(sldrThis.actorInformation),
      LdrToPatternedInfo(sldrThis.patterned, nullptr), sldrThis.ingSpiderballGuardianProperties);
}

#ifndef MONOLITHIC
static void SetFuncPtrs() {
  static SIngSpiderballGuardian_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &LoadIngSpiderballGuardian;
  SetSIngSpiderballGuardian_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSIngSpiderballGuardian_FuncPtrs(nullptr); }
#endif
