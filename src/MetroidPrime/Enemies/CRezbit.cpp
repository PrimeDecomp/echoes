#include "MetroidPrime/Enemies/CRezbit.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CRezbitEffect.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrRezbit.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTeamAiMgr.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CBeamInfo.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "MetroidPrime/Weapons/CPlasmaProjectile.hpp"
#include "MetroidPrime/Weapons/CWeaponAssetInfo.hpp"
#include "REL/REL_Setup.h"
#include "rstl/algorithm.hpp"

static const char* const skDeRezzedEffect = "DeRezzed";

static const pas::EStepDirection skStrafeDirections[] = {pas::kSD_Left, pas::kSD_Right, pas::kSD_Up,
                                                         pas::kSD_Down};

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"StateOver", static_cast< CPatterned::StateMachine::TriggerFunc >(&CRezbit::StateOver)},
    {"IsDeRezzed", static_cast< CPatterned::StateMachine::TriggerFunc >(&CRezbit::IsDeRezzed)},
    {"IsAlert", static_cast< CPatterned::StateMachine::TriggerFunc >(&CRezbit::IsAlert)},
    {"HasAttackPath",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CRezbit::HasAttackPath)},
    {"AttackPathOver",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CRezbit::AttackPathOver)},
    {"ShouldBecomeRezzed",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CRezbit::ShouldBecomeRezzed)},
    {"ShouldBecomeDeRezzed",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CRezbit::ShouldBecomeDeRezzed)},
    {"HasLineOfSight",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CRezbit::HasLineOfSight)},
    {"ShouldStrafe", static_cast< CPatterned::StateMachine::TriggerFunc >(&CRezbit::ShouldStrafe)},
    {"FoundStrafeDirection",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CRezbit::FoundStrafeDirection)},
    {"ShouldLaserAttack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CRezbit::ShouldLaserAttack)},
    {"ShouldEnergyBoltAttack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CRezbit::ShouldEnergyBoltAttack)},
    {"ShouldVirusAttack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CRezbit::ShouldVirusAttack)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Start", static_cast< CPatterned::StateMachine::StateFunc >(&CRezbit::Start)},
    {"Idle", static_cast< CPatterned::StateMachine::StateFunc >(&CRezbit::Idle)},
    {"Alert", static_cast< CPatterned::StateMachine::StateFunc >(&CRezbit::Alert)},
    {"Patrol", static_cast< CPatterned::StateMachine::StateFunc >(&CRezbit::Patrol)},
    {"FollowAttackPath",
     static_cast< CPatterned::StateMachine::StateFunc >(&CRezbit::FollowAttackPath)},
    {"PathFind", static_cast< CPatterned::StateMachine::StateFunc >(&CRezbit::PathFind)},
    {"Strafe", static_cast< CPatterned::StateMachine::StateFunc >(&CRezbit::Strafe)},
    {"BecomeRezzed", static_cast< CPatterned::StateMachine::StateFunc >(&CRezbit::BecomeRezzed)},
    {"BecomeDeRezzed",
     static_cast< CPatterned::StateMachine::StateFunc >(&CRezbit::BecomeDeRezzed)},
    {"CuttingLaserAttack",
     static_cast< CPatterned::StateMachine::StateFunc >(&CRezbit::CuttingLaserAttack)},
    {"EnergyBoltAttack",
     static_cast< CPatterned::StateMachine::StateFunc >(&CRezbit::EnergyBoltAttack)},
    {"VirusAttack", static_cast< CPatterned::StateMachine::StateFunc >(&CRezbit::VirusAttack)},
    {"Dead", static_cast< CPatterned::StateMachine::StateFunc >(&CRezbit::Dead)},
};

static CPatterned::StateMachine::SCodeFunction skCodeFuncs[] = {
    {"SelectTarget", static_cast< CPatterned::StateMachine::CodeFunc >(&CRezbit::SelectTarget)},
    {"SelectStrafeDirection",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CRezbit::SelectStrafeDirection)},
    {"SetTargetDest", static_cast< CPatterned::StateMachine::CodeFunc >(&CRezbit::SetTargetDest)},
    {"ResetAttackTimes",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CRezbit::ResetAttackTimes)},
    {"RaiseShields", static_cast< CPatterned::StateMachine::CodeFunc >(&CRezbit::RaiseShields)},
    {"SetNormalRezState",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CRezbit::SetNormalRezState)},
    {"SetDeRezzedState",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CRezbit::SetDeRezzedState)},
};

static CVector3f skShieldHalfExtents(3.5f, 0.5f, 5.f);

CRezbit::CRezbit(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                 const CTransform4f& xf, const CModelData& modelData,
                 const CActorParameters& actorParams, const CPatternedInfo& patternedInfo,
                 const SLdrRezbitData& data)
: CPatterned(kPAI_Rezbit, uid, name, kFT_Zero, info, xf, modelData, patternedInfo, kMT_Flyer,
             kCT_One, kBT_Flyer, actorParams)
, mData(data)
, mPathFindSearch(nullptr, 3, patternedInfo.GetPathfindingIndex(), 1.f, 1.f, 0,
                  CPFRegion::kRP_Center)
, mRezState(0)
, mLineOfSight(GetUniqueId(), CSegId::Invalid(), 0.1f, 0.05f)
, mShieldActorId(kInvalidUniqueId)
, mShieldBottomLeftSeg(CSegId::Invalid())
, mShieldTopRightSeg(CSegId::Invalid())
, mShieldExplodeEffect(data.shieldExplodeEffect == kInvalidAssetId
                           ? rstl::optional_object< TLockedToken< CGenDescription > >()
                           : rstl::optional_object< TLockedToken< CGenDescription > >(
                                 TLockedToken< CGenDescription >(gpSimplePool->GetObj(
                                     SObjectTag('PART', data.shieldExplodeEffect)))))
, mShieldSfx()
, mDeflectSfx()
, mDeflectSfxTime(0.f)
, mDerezTimer(0.f)
, mShieldTimer(0.f)
, mDerezHealth(GetHealthInfo()->GetHP() - data.unknown_0x4a6c4b40)
, mAlertDelay(0.f)
, x0c40_(4)
, mStrafeTimer(data.unknown_0x70e597d4)
, mStrafeDistance(10.f)
, mStrafeIndex(-1)
, mTeamAiMgrId(kInvalidUniqueId)
, mTargetId(kInvalidUniqueId)
, mAttackKind(-1)
, mWobbleTransform(CTransform4f::Identity())
, mAttackTimer(data.normalRezAttackTime)
, mAttackSelection(-1)
, mCuttingLaserInfo(data.cuttingLaserBeamInfo.weaponSystem,
                    LdrToDamageInfo(data.cuttingLaserDamage))
, mLaserId(kInvalidUniqueId)
, mLaserSfx()
, mLaserStart(CVector3f::Zero())
, mLaserEnd(CVector3f::Zero())
, mLaserDuration(0.f)
, mLaserElapsed(0.f)
, mEnergyBoltInfo(data.energyBoltProjectile, LdrToDamageInfo(data.energyBoltDamage))
, mBoltDuration(0.f)
, mBoltTimer(0.f)
, mBoltCount(-1)
, mLastBoltDirection(CVector3f::Zero())
, mVirusTimer(0.f)
, mVirusDamage(LdrToDamageInfo(data.virusDamage))
, mAlert(false)
, mHasPathDestination(false)
, mShieldBroken(false) {
  mEnergyBoltInfo.Token().Lock();
  BuildDerezModel(mData.derezModel, mData.derezSkinRules);
  const CAnimData* animData = GetModelData()->GetAnimationData();
  mShieldBottomLeftSeg = animData->GetLocatorSegId(rstl::string_l("BottomLeft_SDK"));
  mShieldTopRightSeg = animData->GetLocatorSegId(rstl::string_l("TopRight_SDK"));
  mLineOfSight.SetSegment(CSegId(1));
  mStrafeDirections[0] = -GetTransform().GetRight();
  mStrafeDirections[1] = GetTransform().GetRight();
  mStrafeDirections[2] = CVector3f::Up();
  mStrafeDirections[3] = CVector3f::Down();
  mStrafeDistance =
      GetAnimationDistance(CPASAnimParmData(pas::kAS_Step, CPASAnimParm::FromEnum(pas::kSD_Right),
                                            CPASAnimParm::FromEnum(pas::kStep_BreakDodge))) *
      GetModelData()->GetScale().GetX();
  mPathFindSearch.SetCharacterRadius(3.f);
  mPathFindSearch.SetCharacterHeight(7.f);
}

CRezbit::~CRezbit() {}

void CRezbit::SetupStateMachine(CStateManager& mgr) {
  StateMachine* stateMachine = mStateMachine.get();
  CPatterned::SetupStateMachine(mgr);
  stateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
  stateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
  stateMachine->SetCodeFunctions(skCodeFuncs, ARRAY_SIZE(skCodeFuncs));
}

bool CRezbit::Listen(CStateManager& mgr, const CVector3f& position, EListenNoiseType type) {
  bool heard = false;
  if (mAlive) {
    if (type == kLNT_PlayerFire) {
      if (!mAlert) {
        const CVector3f offset = position - GetTranslation();
        const float distanceSq = offset.MagSquared();
        if (distanceSq < mData.hearingRadius * mData.hearingRadius) {
          mAlert = true;
          mAlertDelay = 0.04f * CMath::SqrtF(distanceSq);
          heard = true;
        }
      }
    }
  }
  return heard;
}

void CRezbit::Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) {
  if (mStateMachine->HasState()) {
    mStateMachine->SetState(mgr, *this, rstl::string_l("Dead"));
  }
  if (state != kSS_InvalidState) {
    SendScriptMsgs(state, mgr, kInvalidUniqueId, kSM_None);
  }
}

CVector3f CRezbit::GetAimPosition(const CStateManager& mgr, float dt) const {
  return CPatterned::GetAimPosition(mgr, dt);
}

const CDamageVulnerability* CRezbit::GetDamageVulnerability() const {
  return CPatterned::GetDamageVulnerability();
}

CProjectileInfo* CRezbit::ProjectileInfo() { return &mEnergyBoltInfo; }

void CRezbit::AddToRenderer(const CStateManager& mgr) const { CPatterned::AddToRenderer(mgr); }

void CRezbit::PreRender(CStateManager& mgr) { CPatterned::PreRender(mgr); }

void CRezbit::Render(const CStateManager& mgr) const {
  const CTransform4f savedTransform = GetTransform();
  CTransform4f& transform = const_cast< CTransform4f& >(GetTransform());
  transform = transform * mWobbleTransform;
  CPatterned::Render(mgr);
  transform = savedTransform;

  if (mDeflectedMissiles.size() != 0 && mData.missileDeflectRadius > 0.f) {
    const CVector3f center = GetAimPosition(mgr, 0.f);
    for (rstl::reserved_vector< TUniqueId, 5 >::const_iterator it = mDeflectedMissiles.begin();
         it != mDeflectedMissiles.end(); ++it) {
      const CGameProjectile* projectile =
          TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(*it));
      if (projectile) {
        const float distance = (center - projectile->GetTranslation()).Magnitude();
        const float ratio = distance / mData.missileDeflectRadius;
        mgr.DrawSpaceWarp(projectile->GetTranslation(), 1.f - CMath::Clamp(0.f, ratio, 1.f));
      }
    }
  }
}

void CRezbit::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                              float dt) {
  bool handled = false;
  switch (type) {
  case kUE_EffectOn: {
    mLaserDuration = BodyController()->GetAnimTimeRemaining();
    const int animId = BodyController()->GetCurrentAnimId();
    const CCharAnimTime start =
        GetModelData()->GetAnimationData()->GetTimeOfUserEventForAnimation(animId, kUE_EffectOn);
    if (start != CCharAnimTime::Infinity()) {
      const CCharAnimTime stop =
          GetModelData()->GetAnimationData()->GetTimeOfUserEventForAnimation(animId, kUE_EffectOff);
      if (stop != CCharAnimTime::Infinity()) {
        mLaserDuration = rstl::max_val(0.f, stop.GetSeconds() - start.GetSeconds());
      }
    }
    if (SetupLaserTargets(mgr)) {
      SpawnLaser(mgr, dt);
    }
    handled = true;
    break;
  }
  case kUE_EffectOff:
    DeleteLaser(mgr);
    handled = true;
    break;
  case kUE_BeginAction:
    StartVirusAttack(mgr, node.GetLocatorName());
    handled = true;
    break;
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

void CRezbit::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    CPatterned::Think(dt, mgr);
    mLineOfSight.Update(dt, mgr);
    UpdateShield(mgr, dt);
    UpdateWobble(mgr);
    UpdateTimers(dt);
    if (mRezState == 1 && !BodyController()->IsFrozen()) {
      DeflectMissiles(mgr);
    }
  }
}

void CRezbit::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const TUniqueId sender = msg.GetSenderId();
  const EScriptObjectMessage message = msg.GetMessage();
  const bool wasActive = GetActive();
  CPatterned::AcceptScriptMsg(mgr, msg);
  switch (message) {
  case kSM_Create:
    BodyController()->Activate(mgr, pas::kAS_Invalid);
    CreateShieldCollisionActor(mgr);
    break;
  case kSM_Delete:
    mgr.DeleteObjectRequest(mShieldActorId);
    QuitTeam(mgr);
    if (mShieldSfx) {
      CSfxManager::RemoveEmitter(mShieldSfx);
    }
    if (mLaserSfx) {
      CSfxManager::RemoveEmitter(mLaserSfx);
    }
    break;
  case kSM_Activate:
    if (!wasActive) {
      if (CEntity* shield = mgr.ObjectById(mShieldActorId)) {
        if (BodyController()->BodyStateInfo().GetCurrentAdditiveStateId() ==
            pas::kAS_AdditiveLoopReaction) {
          shield->SetActive(true);
        }
      }
    }
    break;
  case kSM_Deactivate:
    if (wasActive) {
      if (CEntity* shield = mgr.ObjectById(mShieldActorId)) {
        shield->SetActive(false);
      }
      QuitTeam(mgr);
    }
    break;
  case kSM_Alert:
    mAlert = true;
    break;
  case kSM_Damage:
    mAlert = true;
    HandleShieldHit(mgr, sender);
    break;
  case kSM_ResistedDamage:
    mAlert = true;
    break;
  case kSM_AreaLoaded:
    mPathFindSearch.SetArea(
        mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()).GetPostConstructed()->mPathArea);
    break;
  }
}

void CRezbit::Start(CStateManager& mgr, EStateMsg msg, float dt) {}

void CRezbit::Idle(CStateManager& mgr, EStateMsg msg, float dt) {}

void CRezbit::Alert(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mAlert = true;
    LowerShields(mgr);
    mShieldTimer = FLT_MAX;
    mgr.InformListeners(GetTranslation(), kLNT_PlayerFire);
    break;
  case kStateMsg_Update:
    if (StateMachineState().GetTime() > mAlertDelay) {
      if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Taunt)) {
        BodyController()->CommandMgr().DeliverCmd(CBCTauntCmd(pas::kTT_Four));
      } else if (BodyController()->GetCurrentStateId() == pas::kAS_Taunt) {
        BodyController()->SetLocomotionType(pas::kLT_Lurk);
      }
    }
    if (const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId))) {
      const CVector3f toTarget = target->GetTranslation() - GetTranslation();
      if (toTarget.IsMagnitudeSafe()) {
        BodyController()->FaceDirection(toTarget.AsNormalized(), dt);
      }
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    BodyController()->SetLocomotionType(pas::kLT_Lurk);
    break;
  }
}

void CRezbit::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  mWaypointNavigation.Patrol(mgr, msg, dt, *this);
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    break;
  case kStateMsg_Update:
    UpdatePatrolMovement();
    break;
  }
}

void CRezbit::FollowAttackPath(CStateManager& mgr, EStateMsg msg, float dt) {
  mWaypointNavigation.Patrol(mgr, msg, dt, *this);
  switch (msg) {
  case kStateMsg_Activate:
    mWaypointNavigation.SetLastDestination(GetConnectedObject(mgr, kSS_Attack, kSM_Follow));
    mAttackKind = 4;
    break;
  case kStateMsg_Deactivate:
    mAttackKind = -1;
    break;
  }
}

void CRezbit::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  if (mHasPathDestination) {
    mPathFindNavigation.PathFind(mgr, msg, dt, *this);
    UpdateSeparation(mgr);
  }
  if (msg == kStateMsg_Update && !mHasPathDestination) {
    if (const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId))) {
      const CVector3f toTarget = target->GetTranslation() - GetTranslation();
      if (toTarget.IsMagnitudeSafe()) {
        BodyController()->FaceDirection(toTarget.AsNormalized(), dt);
      }
    }
  }
}

void CRezbit::Strafe(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    if (mStrafeIndex != -1) {
      mAttackKind = 0;
      mAnimationState.SetState(CAnimationState::kAS_Ready);
    } else {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    }
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Step)) {
      const pas::EStepDirection direction = skStrafeDirections[mStrafeIndex];
      BodyController()->CommandMgr().DeliverCmd(
          CBCStepCmd(direction, mRezState == 0 ? pas::kStep_Normal : pas::kStep_BreakDodge));
    } else if (BodyController()->GetCurrentStateId() == pas::kAS_Step) {
      if (const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId))) {
        BodyController()->CommandMgr().SetTargetVector(target->GetTranslation() - GetTranslation());
      }
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mAttackKind = -1;
    mStrafeIndex = -1;
    mStrafeTimer = mRezState == 0 ? mData.unknown_0x70e597d4 : mData.strafeDerezInterval;
    break;
  }
}

void CRezbit::BecomeRezzed(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(mRezState == 1 ? CAnimationState::kAS_Ready
                                            : CAnimationState::kAS_Over);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Generate)) {
      CBCGenerateCmd cmd(pas::kGType_Zero, -1);
      cmd.SetInterruptKnockBack(true);
      BodyController()->CommandMgr().DeliverCmd(cmd);
    } else if (BodyController()->GetCurrentStateId() == pas::kAS_Generate) {
      BodyController()->SetLocomotionType(pas::kLT_Lurk);
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    BodyController()->SetLocomotionType(pas::kLT_Lurk);
    SetNormalRezState(mgr, dt);
    break;
  }
}

void CRezbit::BecomeDeRezzed(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(mRezState == 0 ? CAnimationState::kAS_Ready
                                            : CAnimationState::kAS_Over);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Generate)) {
      CBCGenerateCmd cmd(pas::kGType_Zero, -1);
      cmd.SetInterruptKnockBack(true);
      BodyController()->CommandMgr().DeliverCmd(cmd);
    } else if (BodyController()->GetCurrentStateId() == pas::kAS_Generate) {
      BodyController()->SetLocomotionType(pas::kLT_Combat);
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    BodyController()->SetLocomotionType(pas::kLT_Combat);
    SetDeRezzedState(mgr, dt);
    break;
  }
}

void CRezbit::CuttingLaserAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mLaserElapsed = 0.f;
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mAttackKind = 1;
    CScriptTeamAiMgr::StartAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamAiMgrId,
                                  GetUniqueId());
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_ProjectileAttack)) {
      BodyController()->CommandMgr().DeliverCmd(CBCProjectileAttackCmd(
          pas::kS_One, GetTranslation() + GetTransform().GetForward(), false));
    } else if (mLaserId != kInvalidUniqueId && mLaserDuration > 0.f) {
      UpdateLaser(mgr, dt);
    } else if (const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId))) {
      BodyController()->CommandMgr().SetTargetVector(target->GetTranslation() - GetTranslation());
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    DeleteLaser(mgr);
    mAttackKind = -1;
    CScriptTeamAiMgr::EndAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamAiMgrId, GetUniqueId(),
                                false);
    break;
  }
}

void CRezbit::EnergyBoltAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mAttackKind = 2;
    mBoltCount = 0;
    mBoltTimer = mData.energyBoltBurstTime;
    mLastBoltDirection = GetTransform().GetForward();
    mBoltDuration =
        mData.energyBoltAttackDuration + mgr.Random()->Range(0.f, mData.energyBoltAttackVariance);
    CScriptTeamAiMgr::StartAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamAiMgrId,
                                  GetUniqueId());
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_LoopAttack)) {
      BodyController()->CommandMgr().DeliverCmd(CBCLoopAttackCmd(pas::kLAT_Zero, false));
    } else if (BodyController()->GetCurrentStateId() == pas::kAS_LoopAttack) {
      if (StateMachineState().GetTime() < mBoltDuration) {
        if (const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId))) {
          BodyController()->CommandMgr().SetTargetVector(target->GetTranslation() -
                                                         GetTranslation());
        }
        mBoltTimer -= dt;
        if (mBoltTimer <= 0.f) {
          FireEnergyBolt(mgr, dt);
          mBoltTimer = mData.energyBoltBurstTime;
        }
      } else {
        BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
      }
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
    mAttackKind = -1;
    CScriptTeamAiMgr::EndAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamAiMgrId, GetUniqueId(),
                                false);
    break;
  }
}

void CRezbit::VirusAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mAttackKind = 3;
    CScriptTeamAiMgr::StartAttack(CScriptTeamAiMgr::kAT_Melee, mgr, mTeamAiMgrId, GetUniqueId());
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_ProjectileAttack)) {
      BodyController()->CommandMgr().DeliverCmd(CBCProjectileAttackCmd(
          pas::kS_Two, GetTranslation() + GetTransform().GetForward(), false));
    } else if (const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId))) {
      BodyController()->CommandMgr().SetTargetVector(target->GetTranslation() - GetTranslation());
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mAttackKind = -1;
    mDerezTimer = 0.f;
    CScriptTeamAiMgr::EndAttack(CScriptTeamAiMgr::kAT_Melee, mgr, mTeamAiMgrId, GetUniqueId(),
                                false);
    break;
  }
}

void CRezbit::Dead(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    if (mShieldSfx) {
      CSfxManager::RemoveEmitter(mShieldSfx);
    }
    if (mLaserSfx) {
      CSfxManager::RemoveEmitter(mLaserSfx);
    }
    DeathDelete(mgr);
    break;
  }
}

void CRezbit::SelectTarget(CStateManager& mgr, float dt) {
  mTargetId = mgr.GetPlayer(0)->GetUniqueId();
  mLineOfSight.SetTarget(mTargetId);
}

void CRezbit::SelectStrafeDirection(CStateManager& mgr, float dt) {
  mStrafeIndex = -1;
  const float chance = mRezState == 0 ? mData.unknown_0x94980a67 : mData.strafeDerezChance;
  if (mgr.Random()->Range(0.f, 100.f) <= chance) {
    mStrafeIndex = FindStrafeDirection(mgr);
  } else {
    mStrafeTimer = mRezState == 0 ? mData.unknown_0x70e597d4 : mData.strafeDerezInterval;
  }
}

void CRezbit::SetTargetDest(CStateManager& mgr, float dt) {
  const CVector3f position = GetTranslation();
  CVector3f destination = position;
  mHasPathDestination = false;
  if (const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId))) {
    const CVector3f targetPos = target->GetAimPosition(mgr, 0.f);
    const CVector3f offset = targetPos - position;
    float maxRangeSq;
    if (mRezState == 0) {
      maxRangeSq = rstl::min_val(mData.energyBoltMaxAttackDist * mData.energyBoltMaxAttackDist,
                                 mData.cuttingLaserMaxAttackDist * mData.cuttingLaserMaxAttackDist);
    } else {
      maxRangeSq = mData.virusMaxAttackDist * mData.virusMaxAttackDist;
    }
    if (offset.MagSquared() > maxRangeSq || !mLineOfSight.HasLineOfSight()) {
      destination = targetPos;
      mHasPathDestination = true;
    }
  }
  mPathFindNavigation.SetDestination(destination);
  mPathFindNavigation.SetFaceTarget(mTargetId);
  mPathFindNavigation.SetUseLocomotionFacing(true);
  JoinTeam(mgr);
}

void CRezbit::ResetAttackTimes(CStateManager& mgr, float dt) {
  mAttackTimer = mData.normalRezAttackTime;
  mVirusTimer = mData.virusAttackTime;
  const float roll = mgr.Random()->Range(0.f, 100.f);
  if (roll <= mData.cuttingLaserChance) {
    mAttackSelection = 1;
  } else if (roll <= mData.cuttingLaserChance + mData.energyBoltChance) {
    mAttackSelection = 2;
  } else {
    mAttackSelection = -1;
  }
}

void CRezbit::RaiseShields(CStateManager& mgr, float dt) {
  if (mShieldBroken) {
    return;
  }
  if (BodyController()->BodyStateInfo().GetCurrentAdditiveStateId() !=
      pas::kAS_AdditiveLoopReaction) {
    BodyController()->CommandMgr().DeliverCmd(CBCAdditiveLoopReactionCmd(0, 1.f));
    mShieldTimer = mData.shieldUpTime;
  }
  if (CEntity* shield = mgr.ObjectById(mShieldActorId)) {
    shield->SetActive(true);
    if (!mShieldSfx) {
      mShieldSfx = PlayCustomSound(GetTranslation(), GetTransform().GetForward(),
                                   mData.sound_ShieldOn, true);
    }
  }
}

void CRezbit::SetNormalRezState(CStateManager& mgr, float dt) {
  if (mRezState != 0) {
    mRezState = 0;
    CAnimData* animData = AnimationData();
    animData->SetSkinnedModel(mNormalModel);
    animData->SetEffectState(rstl::string_l(skDeRezzedEffect), false, mgr);
    mDerezHealth = GetHealthInfo()->GetHP() - mData.unknown_0x4a6c4b40;
    SetVisorOrbitableFlags(CVisorParameters::kVOF_All, true);
  }
}

void CRezbit::SetDeRezzedState(CStateManager& mgr, float dt) {
  if (mRezState != 1) {
    mRezState = 1;
    CAnimData* animData = AnimationData();
    if (mDerezModel.valid()) {
      animData->SetSkinnedModel(*mDerezModel);
      animData->SetEffectState(rstl::string_l(skDeRezzedEffect), true, mgr);
    }
    mDerezTimer = mData.derezTime;
    SetVisorOrbitableFlags(CVisorParameters::kVOF_All, false);
    SetVisorOrbitableFlags(CVisorParameters::kVOF_Dark, true);
  }
}

bool CRezbit::StateOver(CStateManager& mgr, const CTriggerData& data) const {
  return mAnimationState.IsOver();
}

bool CRezbit::IsDeRezzed(CStateManager& mgr, const CTriggerData& data) const {
  return mRezState == 1;
}

bool CRezbit::IsAlert(CStateManager& mgr, const CTriggerData& data) const { return mAlert; }

bool CRezbit::HasAttackPath(CStateManager& mgr, const CTriggerData& data) const {
  return GetConnectedObject(mgr, kSS_Attack, kSM_Follow) != kInvalidUniqueId;
}

bool CRezbit::AttackPathOver(CStateManager& mgr, const CTriggerData& data) const {
  return mWaypointNavigation.GetDestination() == kInvalidUniqueId;
}

bool CRezbit::ShouldBecomeRezzed(CStateManager& mgr, const CTriggerData& data) const {
  return mAttackKind == -1 && mRezState != 0 && mDerezTimer <= 0.f;
}

bool CRezbit::ShouldBecomeDeRezzed(CStateManager& mgr, const CTriggerData& data) const {
  if (mAttackKind == -1 && GetHealthInfo()->GetHP() <= mDerezHealth && mAlert && mRezState != 1) {
    return GetBodyController()->GetBodyStateInfo().GetCurrentAdditiveStateId() !=
           pas::kAS_AdditiveLoopReaction;
  }
  return false;
}

bool CRezbit::HasLineOfSight(CStateManager& mgr, const CTriggerData& data) const {
  return mLineOfSight.HasLineOfSight();
}

bool CRezbit::ShouldStrafe(CStateManager& mgr, const CTriggerData& data) const {
  if (!mHasPathDestination && mStrafeTimer <= 0.f && mLineOfSight.GetClearTime() > 2.f) {
    const CEntity* shield = mgr.GetObjectById(mShieldActorId);
    return !shield || !shield->GetActive();
  }
  return false;
}

bool CRezbit::FoundStrafeDirection(CStateManager& mgr, const CTriggerData& data) const {
  return mStrafeIndex != -1;
}

bool CRezbit::ShouldLaserAttack(CStateManager& mgr, const CTriggerData& data) const {
  if (mAttackTimer <= 0.f && mAttackSelection == 1 && mLaserId == kInvalidUniqueId &&
      !mgr.GetCameraManager(0)->IsInCinematicCamera()) {
    if (mTeamAiMgrId == kInvalidUniqueId ||
        CScriptTeamAiMgr::CanStartAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamAiMgrId,
                                         GetUniqueId())) {
      const CEntity* shield = mgr.GetObjectById(mShieldActorId);
      if (!shield || !shield->GetActive()) {
        if (const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId))) {
          const float dy = target->GetTranslation().GetY() - GetTranslation().GetY();
          const float dx = target->GetTranslation().GetX() - GetTranslation().GetX();
          const float dz = 0.f;
          const float distanceSq = dx * dx + dy * dy + dz * dz;
          return distanceSq >= mData.cuttingLaserMinAttackDist * mData.cuttingLaserMinAttackDist &&
                 distanceSq <= mData.cuttingLaserMaxAttackDist * mData.cuttingLaserMaxAttackDist;
        }
      }
    }
  }
  return false;
}

bool CRezbit::ShouldEnergyBoltAttack(CStateManager& mgr, const CTriggerData& data) const {
  if (mAttackTimer <= 0.f && mAttackSelection == 2 &&
      !mgr.GetCameraManager(0)->IsInCinematicCamera()) {
    if (mTeamAiMgrId == kInvalidUniqueId ||
        CScriptTeamAiMgr::CanStartAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamAiMgrId,
                                         GetUniqueId())) {
      const CEntity* shield = mgr.GetObjectById(mShieldActorId);
      if (!shield || !shield->GetActive()) {
        if (const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId))) {
          const float dy = target->GetTranslation().GetY() - GetTranslation().GetY();
          const float dx = target->GetTranslation().GetX() - GetTranslation().GetX();
          const float dz = 0.f;
          const float distanceSq = dx * dx + dy * dy + dz * dz;
          return distanceSq >= mData.energyBoltMinAttackDist * mData.energyBoltMinAttackDist &&
                 distanceSq <= mData.energyBoltMaxAttackDist * mData.energyBoltMaxAttackDist;
        }
      }
    }
  }
  return false;
}

bool CRezbit::ShouldVirusAttack(CStateManager& mgr, const CTriggerData& data) const {
  if (mVirusTimer <= 0.f && !mgr.GetCameraManager(0)->IsInCinematicCamera()) {
    if (mTeamAiMgrId == kInvalidUniqueId ||
        CScriptTeamAiMgr::CanStartAttack(CScriptTeamAiMgr::kAT_Melee, mgr, mTeamAiMgrId,
                                         GetUniqueId())) {
      if (const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(mTargetId))) {
        if (player->GetRezbitState() == CPlayer::kRS_None &&
            player->GetSpawnedMorphballState() != CPlayer::kMS_Morphed) {
          const float dy = player->GetTranslation().GetY() - GetTranslation().GetY();
          const float dx = player->GetTranslation().GetX() - GetTranslation().GetX();
          const float dz = 0.f;
          const float distanceSq = dx * dx + dy * dy + dz * dz;
          return distanceSq >= mData.virusMinAttackDist * mData.virusMinAttackDist &&
                 distanceSq <= mData.virusMaxAttackDist * mData.virusMaxAttackDist;
        }
      }
    }
  }
  return false;
}

void CRezbit::QuitTeam(CStateManager& mgr) {
  if (mTeamAiMgrId != kInvalidUniqueId) {
    if (CScriptTeamAiMgr* team = TCastToPtr< CScriptTeamAiMgr >(mgr.ObjectById(mTeamAiMgrId))) {
      if (team->IsPartOfTeam(GetUniqueId())) {
        team->QuitTeam(GetUniqueId());
        mTeamAiMgrId = kInvalidUniqueId;
      }
    }
  }
}

void CRezbit::JoinTeam(CStateManager& mgr) {
  if (mTeamAiMgrId == kInvalidUniqueId) {
    mTeamAiMgrId = CScriptTeamAiMgr::GetAssociatedTeamId(*this, mgr);
  }
  if (mTeamAiMgrId != kInvalidUniqueId) {
    if (CScriptTeamAiMgr* team = TCastToPtr< CScriptTeamAiMgr >(mgr.ObjectById(mTeamAiMgrId))) {
      team->JoinTeam(*this, CTeamAiRole::kTAR_Projectile, CTeamAiRole::kTAR_Unknown,
                     CTeamAiRole::kTAR_Invalid);
    }
  }
}

void CRezbit::CheckVerticalClearance(CStateManager& mgr,
                                     rstl::reserved_vector< bool, 4 >& available) {
  const CVector3f position = GetTranslation();
  const CEntity* target = mgr.GetObjectById(mTargetId);
  const float heightAboveTarget =
      position.GetZ() - static_cast< const CActor* >(target)->GetTranslation().GetZ();
  if (mStrafeDistance + heightAboveTarget > 16.f) {
    available[2] = false;
  }
  if (heightAboveTarget - mStrafeDistance < 3.f) {
    available[3] = false;
  }
  if (available[2] || available[3]) {
    const CRayCastResult result = mgr.RayStaticIntersection(
        position, CVector3f::Down(), 16.f, CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid)));
    if (result.IsValid()) {
      const float groundDistance = result.GetTime();
      if (groundDistance + mStrafeDistance > 16.f) {
        available[2] = false;
      }
      if (groundDistance - mStrafeDistance < 3.f) {
        available[3] = false;
      }
    } else {
      available[2] = false;
    }
  }
}

void CRezbit::CheckNeighbors(CStateManager& mgr, rstl::reserved_vector< bool, 4 >& available) {
  const float radiusSq = mStrafeDistance * mStrafeDistance;
  const CVector3f position = GetTranslation();
  const CVector3f right = GetTransform().GetRight();
  const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    const CActor* other = static_cast< const CActor* >(list[i]);
    if (other && other != this && other->GetCurrentAreaId() == GetCurrentAreaId()) {
      const CVector3f toOther = other->GetTranslation() - position;
      if (toOther.MagSquared() < radiusSq) {
        if (CVector3f::Dot(toOther, right) >= 0.f) {
          if (available[1] && CVector3f::GetAngleDiff(right, toOther) < 1.0471976f) {
            available[1] = false;
          }
        } else if (available[0] && CVector3f::GetAngleDiff(-right, toOther) < 1.0471976f) {
          available[1] = false;
        }
        if (toOther.GetZ() >= 0.f) {
          if (available[2] && CVector3f::GetAngleDiff(CVector3f::Up(), toOther) < 1.0471976f) {
            available[2] = false;
          }
        } else if (available[3] &&
                   CVector3f::GetAngleDiff(CVector3f::Down(), toOther) < 1.0471976f) {
          available[3] = false;
        }
      }
    }
  }
}

int CRezbit::FindStrafeDirection(CStateManager& mgr) {
  rstl::reserved_vector< bool, 4 > available(4, true);
  CheckVerticalClearance(mgr, available);
  CheckNeighbors(mgr, available);
  rstl::reserved_vector< long, 4 > valid;
  const CVector3f* direction = mStrafeDirections;
  for (int i = 0; i < available.size(); ++direction, ++i) {
    if (available[i]) {
      available[i] = CanStrafe(mgr, mStrafeDistance, *direction);
    }
    if (available[i]) {
      valid.push_back(i);
    }
  }
  if (valid.size() != 0) {
    return valid[mgr.Random()->Range(0, valid.size() - 1)];
  }
  return -1;
}

bool CRezbit::CanStrafe(CStateManager& mgr, float distance, const CVector3f& direction) {
  const CVector3f center = GetBoundingBox().GetCenterPoint();
  const CVector3f end = center + direction * distance;
  if (mgr.RayCollideWorld(center, end,
                          CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid),
                                                              CMaterialList(kMT_CollisionActor)),
                          this) &&
      !mPathFindSearch.OnPath(end)) {
    return true;
  }
  return false;
}

void CRezbit::UpdateSeparation(CStateManager& mgr) {
  const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    const CPatterned* other = TCastToConstPtr< CPatterned >(list[i]);
    if (other && other != this && other->GetCurrentAreaId() == GetCurrentAreaId()) {
      const CVector3f separation = mSteeringBehaviors.Separation(
          *this, other->GetTranslation(), 20.f * GetModelData()->GetScale().GetX());
      if (separation.IsMagnitudeSafe()) {
        BodyController()->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(separation.AsNormalized(), CVector3f::Zero(), 0.5f));
      }
    }
  }
}

void CRezbit::UpdateDeflectSfx(float dt) {
  if (CSfxManager::IsPlaying(mDeflectSfx)) {
    mDeflectSfxTime += dt;
    CSfxManager::UpdateEmitter(mDeflectSfx, GetTranslation(), GetTransform().GetForward(),
                               mData.sound_DeflectMissile.maxVolume);
  }
}

void CRezbit::DeflectMissiles(CStateManager& mgr) {
  const CVector3f center = GetAimPosition(mgr, 0.f);
  const CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Projectile));
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  mgr.BuildNearList(nearList,
                    CAABox(center - mData.missileDeflectRadius * CVector3f::One(),
                           center + mData.missileDeflectRadius * CVector3f::One()),
                    filter, this);

  const rstl::reserved_vector< TUniqueId, 5 > previousMissiles = mDeflectedMissiles;
  mDeflectedMissiles.clear();
  if (nearList.size() == 0) {
    return;
  }

  const float radiusSq = mData.missileDeflectRadius * mData.missileDeflectRadius;
  for (rstl::reserved_vector< TUniqueId, 1024 >::const_iterator it = nearList.begin();
       it != nearList.end(); ++it) {
    CGameProjectile* projectile = TCastToPtr< CGameProjectile >(mgr.ObjectById(*it));
    if (!projectile ||
        !(projectile->GetType() == kWT_Missile ||
          (projectile->GetType() == kWT_Power && projectile->HasAttrib(CWeapon::kPA_ComboShot)))) {
      continue;
    }
    const CVector3f delta = projectile->GetTranslation() - center;
    if (delta.MagSquared() < radiusSq) {
      mDeflectedMissiles.push_back(*it);
      projectile->SetMinHomingDistance(mData.missileDeflectRadius);
      CProjectileWeapon& weapon = projectile->Projectile();
      const CVector3f dir = delta + (projectile->GetTranslation() - projectile->GetPreviousPos());
      const CVector3f axis = CVector3f::Cross(dir, delta);
      if (axis.CanBeNormalized()) {
        const CQuaternion rotation = CQuaternion::AxisAngle(
            CUnitVector3f(axis), CRelAngle::FromDegrees(0.017453292f * mData.missileDeflectRate));
        weapon.SetWorldSpaceOrientation(rotation.BuildTransform4f() *
                                        weapon.GetTransform().GetRotation());
      }
    }
  }

  for (rstl::reserved_vector< TUniqueId, 5 >::const_iterator it = mDeflectedMissiles.begin();
       it != mDeflectedMissiles.end(); ++it) {
    if (rstl::find(previousMissiles.begin(), previousMissiles.end(), *it) !=
        previousMissiles.end()) {
      continue;
    }
    bool playSound = true;
    if (CSfxManager::IsPlaying(mDeflectSfx)) {
      if (mDeflectSfxTime > 0.5f) {
        CSfxManager::SfxStop(mDeflectSfx);
      } else {
        playSound = false;
      }
    }
    if (playSound) {
      mDeflectSfx = PlayCustomSound(GetTranslation(), GetTransform().GetForward(),
                                    mData.sound_DeflectMissile, false);
      mDeflectSfxTime = 0.f;
    }
    break;
  }
}

void CRezbit::StartVirusAttack(CStateManager& mgr, const rstl::string& locator) {
  if (CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(mTargetId))) {
    if (player->GetSpawnedMorphballState() != CPlayer::kMS_Morphed &&
        !player->GetCameraManager()->IsInCinematicCamera()) {
      const CVector3f start = GetLctrTransform(locator).GetTranslation();
      const CVector3f end = player->GetTranslation();
      if (mgr.RayCollideWorld(start, end,
                              CMaterialFilter::MakeInclude(
                                  CMaterialList(kMT_Solid, kMT_Character, kMT_CollisionActor,
                                                kMT_Player, kMT_ProjectilePassthrough)),
                              this)) {
        player->StartRezbitState(
            mgr, CRezbitEffectOptions(mData.virusMorphballFx, mData.sound_VirusHUD,
                                      mData.sound_HUDReboot, 3.f, 1.5f, 2.5f, true));
        mgr.ApplyDamage(GetUniqueId(), mTargetId, GetUniqueId(), mVirusDamage,
                        CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid)), CVector3f::Zero());
      }
    }
  }
}

void CRezbit::FireEnergyBolt(CStateManager& mgr, float dt) {
  const CEntity* target = mgr.GetObjectById(mTargetId);
  const CVector3f gunPos = GetLctrTransform(CSegId(1)).GetTranslation();
  CVector3f aimPos = static_cast< const CActor* >(target)->GetAimPosition(mgr, 0.f);
  if (const CPlayer* player = TCastToConstPtr< CPlayer >(target)) {
    aimPos = ProjectileInfo()->PredictInterceptPos(gunPos, aimPos, *player, false, dt);
  }
  CVector3f dir = aimPos - gunPos;
  if (dir.IsMagnitudeSafe()) {
    if (mBoltCount > 0 && mLastBoltDirection.IsMagnitudeSafe() &&
        CVector3f::GetAngleDiff(dir, mLastBoltDirection) > 0.17453292f) {
      const CVector3f limited =
          CVector3f::Slerp(mLastBoltDirection.AsNormalized(), dir.AsNormalized(),
                           CRelAngle::FromRadians(0.17453292f));
      aimPos = gunPos + dir.Magnitude() * limited;
    }
    const CTransform4f xf = CTransform4f::LookAt(gunPos, aimPos, CVector3f::Up());
    LaunchProjectile(xf, mgr, 6, 0, false, CImpactVisorEffect::None(), CVector3f(1.f, 1.f, 1.f));
    PlayCustomSound(GetTranslation(), GetTransform().GetForward(), mData.sound_EnergyBolt, false);
    ++mBoltCount;
    mLastBoltDirection = aimPos - gunPos;
  }
}

void CRezbit::DeleteLaser(CStateManager& mgr) {
  if (CPlasmaProjectile* laser = static_cast< CPlasmaProjectile* >(mgr.ObjectById(mLaserId))) {
    laser->ResetBeam(mgr, false);
    CSfxManager::RemoveEmitter(mLaserSfx);
    mgr.DeleteObjectRequest(mLaserId);
    mLaserId = kInvalidUniqueId;
    mLaserSfx = CSfxHandle();
  }
}

void CRezbit::UpdateLaser(CStateManager& mgr, float dt) {
  if (CPlasmaProjectile* laser = static_cast< CPlasmaProjectile* >(mgr.ObjectById(mLaserId))) {
    const float t = mLaserElapsed / mLaserDuration;
    const CVector3f position = (1.f - t) * mLaserStart + t * mLaserEnd;
    mLaserElapsed += dt;
    laser->UpdateFx(CTransform4f::LookAt(GetTranslation(), position, CVector3f::Up()), dt, mgr);
    CSfxManager::UpdateEmitter(mLaserSfx, laser->GetCurrentPos(), CVector3f::Up(), 127);
  }
}

void CRezbit::SpawnLaser(CStateManager& mgr, float dt) {
  const CBeamInfo beamInfo = TLdrToBeamInfo(mData.cuttingLaserBeamInfo, 0x291);
  const TUniqueId uid = mgr.AllocateUniqueId();
  CDamageInfo damage = mCuttingLaserInfo.GetDamage();
  damage.SetDamage(dt * damage.GetDamage());
  damage.SetNoImmunity(true);
  CPlasmaProjectile* laser = rs_new CPlasmaProjectile(
      mCuttingLaserInfo.Token(), rstl::string_l("RezBitCuttingLaser"), kWT_Light, beamInfo,
      CTransform4f::Identity(), kMT_ProjectilePassthrough, damage, uid, GetCurrentAreaId(),
      GetUniqueId(), CWeaponAssetInfo(), false,
      CWeapon::kPA_KeepInCinematic | CWeapon::kPA_BigStrike);
  if (laser) {
    laser->Fire(CTransform4f::LookAt(GetTranslation(), mLaserStart, CVector3f::Up()), mgr, false);
    laser->SetDamageDuration(1.f);
    mgr.AddObject(*laser);
    laser->SetNextDrawNode(GetUniqueId());
    mLaserSfx = PlayCustomSound(GetTranslation(), CVector3f::Up(), mData.sound_CuttingLaser, true);
    mLaserId = uid;
  }
}

bool CRezbit::SetupLaserTargets(CStateManager& mgr) {
  if (const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId))) {
    const CVector3f startAim = target->GetAimPosition(mgr, 0.f);
    const CVector3f targetPos = target->GetTranslation();
    const CVector3f endAim = target->GetAimPosition(mgr, 0.5f * mLaserDuration);
    const CVector3f center = 0.5f * (((endAim + targetPos) - startAim) + endAim);
    CVector3f perpendicular(center.GetY() - GetTranslation().GetY(),
                            -(center.GetX() - GetTranslation().GetX()), 0.f);
    if (perpendicular.IsMagnitudeSafe()) {
      const CVector3f direction = perpendicular.AsNormalized();
      mLaserStart = center + 15.f * direction;
      mLaserEnd = center - 15.f * direction;
      return true;
    }
  }
  return false;
}

void CRezbit::UpdatePatrolMovement() {
  CVector3f move = BodyController()->CommandMgr().GetMoveVector();
  BodyController()->CommandMgr().ClearLocomotionCmds();
  const pas::EStepDirection direction = FindBestStepDirection(move);
  if (move.IsMagnitudeSafe() && direction != pas::kSD_Forward) {
    CVector3f face = CVector3f::Zero();
    switch (direction) {
    case pas::kSD_Backward:
      face = -move;
      break;
    case pas::kSD_Left:
      face = CVector3f(move.GetY(), -move.GetX(), move.GetZ());
      break;
    case pas::kSD_Right:
      face = CVector3f(-move.GetY(), move.GetX(), move.GetZ());
      break;
    }
    BodyController()->CommandMgr().DeliverCmd(CBCLocomotionCmd(move, face.AsNormalized(), 1.f));
  } else {
    BodyController()->CommandMgr().DeliverCmd(CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
  }
}

void CRezbit::BreakShield(CStateManager& mgr) {
  if (!mShieldBroken) {
    mShieldBroken = true;
    if (BodyController()->BodyStateInfo().GetCurrentAdditiveStateId() ==
        pas::kAS_AdditiveLoopReaction) {
      BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_Unknown33));
    }
    if (CActor* shield = static_cast< CActor* >(mgr.ObjectById(mShieldActorId))) {
      if (mShieldExplodeEffect.valid()) {
        const CTransform4f explosionXf(GetTransform().BuildMatrix3f(), shield->GetTranslation());
        CExplosion* explosion = rs_new CExplosion(
            *mShieldExplodeEffect, mgr.AllocateUniqueId(),
            CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true, kInvalidEditorId),
            rstl::string_l("RezbitShieldExplosionFx"), explosionXf, 0, GetModelData()->GetScale(),
            CColor::White(), -1);
        if (explosion) {
          mgr.AddObject(explosion);
        }
      }
      PlayCustomSound(GetTranslation(), GetTransform().GetForward(), mData.sound_ShieldExplode,
                      false);
      if (mShieldSfx) {
        CSfxManager::RemoveEmitter(mShieldSfx);
        mShieldSfx = CSfxHandle();
      }
      shield->SetActive(false);
    }
  }
}

void CRezbit::LowerShields(CStateManager& mgr) {
  if (mShieldBroken) {
    return;
  }
  if (BodyController()->BodyStateInfo().GetCurrentAdditiveStateId() ==
      pas::kAS_AdditiveLoopReaction) {
    BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_AdditiveIdle));
  }
  mShieldTimer = mData.shieldDownTime + mgr.Random()->Range(0.f, mData.shieldDownTimeVariance);
  if (CEntity* shield = mgr.ObjectById(mShieldActorId)) {
    shield->SetActive(false);
  }
  if (mShieldSfx) {
    CSfxManager::RemoveEmitter(mShieldSfx);
    mShieldSfx = CSfxHandle();
    PlayCustomSound(GetTranslation(), GetTransform().GetForward(), mData.sound_ShieldOff, false);
  }
}

void CRezbit::UpdateWobble(CStateManager& mgr) {
  const float a = mgr.Random()->Range(-1.f, 1.f);
  const float b = mgr.Random()->Range(-1.f, 1.f);
  const float xz = 2.f * a * rstl::max_val(0.f, mDamageCooldownTimer);
  const float yz = 2.f * b * rstl::max_val(0.f, mDamageCooldownTimer);
  mWobbleTransform = CTransform4f::Shear(0.f, xz, 0.f, yz, 0.f, 0.f);
}

void CRezbit::UpdateShield(CStateManager& mgr, float dt) {
  if (!mShieldBroken) {
    if (mRezState == 0 && mAlert) {
      if (mAttackKind > 0 && mAttackKind < 5) {
        if (BodyController()->BodyStateInfo().GetCurrentAdditiveStateId() ==
            pas::kAS_AdditiveLoopReaction) {
          LowerShields(mgr);
        }
      } else {
        mShieldTimer -= dt;
        if (mShieldTimer <= 0.f) {
          if (BodyController()->BodyStateInfo().GetCurrentAdditiveStateId() ==
              pas::kAS_AdditiveLoopReaction) {
            LowerShields(mgr);
          } else {
            RaiseShields(mgr, dt);
          }
        }
      }
    } else if (BodyController()->BodyStateInfo().GetCurrentAdditiveStateId() ==
               pas::kAS_AdditiveLoopReaction) {
      LowerShields(mgr);
    }
    if (CCollisionActor* shield = static_cast< CCollisionActor* >(mgr.ObjectById(mShieldActorId))) {
      shield->SetTransform(GetTransform());
      const CTransform4f bottomLeft = GetLctrTransform(mShieldBottomLeftSeg);
      const CTransform4f topRight = GetLctrTransform(mShieldTopRightSeg);
      shield->SetTranslation(0.5f * (bottomLeft.GetTranslation() + topRight.GetTranslation()));
      if (mShieldSfx) {
        CSfxManager::UpdateEmitter(mShieldSfx, GetTranslation(), GetTransform().GetForward(), 0x7f);
      }
    }
  } else if (BodyController()->BodyStateInfo().GetCurrentAdditiveStateId() ==
             pas::kAS_AdditiveLoopReaction) {
    BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_Unknown33));
  }
}

void CRezbit::UpdateTimers(float dt) {
  if (mHitByPlayerProjectile) {
    mAlert = true;
    mHitByPlayerProjectile = false;
  }
  mAttackTimer -= dt;
  mVirusTimer -= dt;
  mStrafeTimer -= dt;
  mDerezTimer -= dt;
}

void CRezbit::CheckShieldBroken(CStateManager& mgr, TUniqueId id) {
  if (id == mShieldActorId) {
    if (CActor* shield = static_cast< CActor* >(mgr.ObjectById(mShieldActorId))) {
      if (shield->GetHealthInfo()->GetHP() <= 0.f) {
        BreakShield(mgr);
      }
    }
  }
}

void CRezbit::HandleShieldHit(CStateManager& mgr, const TUniqueId& id) {
  if (id == mShieldActorId) {
    CheckShieldBroken(mgr, id);
  } else {
    if (mRezState == 1 && mAttackKind == 3) {
      const CWeapon* weapon = TCastToConstPtr< CWeapon >(mgr.GetObjectById(id));
      if (weapon && weapon->HasAttrib(CWeapon::kPA_Charged)) {
        mDerezTimer = 0.f;
        BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_AbortScripted));
      }
    }
    PlayCustomSound(GetTranslation(), GetTransform().GetForward(), mData.sound_Flinch, false);
  }
}

void CRezbit::CreateShieldCollisionActor(CStateManager& mgr) {
  const TUniqueId uid = mgr.AllocateUniqueId();
  const CVector3f halfExtents = GetModelData()->GetScale() * skShieldHalfExtents;
  CCollisionActor* shield =
      rs_new CCollisionActor(uid, GetCurrentAreaId(), GetUniqueId(), halfExtents,
                             CVector3f(0.f, -0.5f * halfExtents.GetY(), 0.f), false, 1000.f);
  if (shield) {
    mShieldActorId = uid;
    shield->SetDamageVulnerability(LdrToDamageVulnerability(mData.shieldVulnerability));
    shield->SetResponseType(kWCR_Unknown15);
    *shield->HealthInfo() = CHealthInfo(mData.shieldHitPoints, 10.f);
    mgr.AddObject(*shield);
  }
}

void CRezbit::BuildDerezModel(CAssetId model, CAssetId skinRules) {
  if (model != kInvalidAssetId && skinRules != kInvalidAssetId) {
    mDerezModel = rstl::optional_object< TLockedToken< CSkinnedModel > >(
        rs_new CSkinnedModel(gpSimplePool->GetObj(SObjectTag('CMDL', model)),
                             gpSimplePool->GetObj(SObjectTag('CSKR', skinRules)),
                             GetAnimationData()->GetModelData()->GetLayoutInfo()));
    (*mDerezModel)->SetLayoutInfo(GetAnimationData()->GetModelData()->GetLayoutInfo());
  }
}

CEntity* REL_LoadRezbit(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrRezbit sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrRezbit.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  return rs_new CRezbit(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                        LdrToEntityInfo(info, sldrThis.editorProperties),
                        LdrToTransform4f(sldrThis.editorProperties), *modelData,
                        LdrToActorParameters(sldrThis.actorInformation),
                        LdrToPatternedInfo(sldrThis.patterned, nullptr), sldrThis.rezbitProperties);
}

static void SetFuncPtrs() {
  static SRezbit_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &REL_LoadRezbit;
  SetSRezbit_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSRezbit_FuncPtrs(nullptr); }
