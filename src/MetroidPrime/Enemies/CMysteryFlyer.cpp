#include "MetroidPrime/Enemies/CMysteryFlyer.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrMysteryFlyer.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTeamAiMgr.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "REL/REL_Setup.h"

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"StateOver", static_cast< CPatterned::StateMachine::TriggerFunc >(&CMysteryFlyer::StateOver)},
    {"NearPatrol",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMysteryFlyer::NearPatrol)},
    {"InDetectionRange",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMysteryFlyer::InDetectionRange)},
    {"AttackPathOver",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMysteryFlyer::AttackPathOver)},
    {"ShouldAttack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMysteryFlyer::ShouldAttack)},
    {"Leash", static_cast< CPatterned::StateMachine::TriggerFunc >(&CMysteryFlyer::Leash)},
    {"ShouldApproach",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMysteryFlyer::ShouldApproach)},
    {"PathOver", static_cast< CPatterned::StateMachine::TriggerFunc >(&CMysteryFlyer::PathOver)},
    {"Stuck", static_cast< CPatterned::StateMachine::TriggerFunc >(&CMysteryFlyer::Stuck)},
    {"LineOfSight",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMysteryFlyer::LineOfSight)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Start", static_cast< CPatterned::StateMachine::StateFunc >(&CMysteryFlyer::Start)},
    {"Generate", static_cast< CPatterned::StateMachine::StateFunc >(&CMysteryFlyer::Generate)},
    {"GoToPatrol", static_cast< CPatterned::StateMachine::StateFunc >(&CMysteryFlyer::GoToPatrol)},
    {"Patrol", static_cast< CPatterned::StateMachine::StateFunc >(&CMysteryFlyer::Patrol)},
    {"FollowAttackPath",
     static_cast< CPatterned::StateMachine::StateFunc >(&CMysteryFlyer::FollowAttackPath)},
    {"Hover", static_cast< CPatterned::StateMachine::StateFunc >(&CMysteryFlyer::Hover)},
    {"Approach", static_cast< CPatterned::StateMachine::StateFunc >(&CMysteryFlyer::Approach)},
    {"Attack", static_cast< CPatterned::StateMachine::StateFunc >(&CMysteryFlyer::Attack)},
    {"Dodge", static_cast< CPatterned::StateMachine::StateFunc >(&CMysteryFlyer::Dodge)},
    {"Dead", static_cast< CPatterned::StateMachine::StateFunc >(&CMysteryFlyer::Dead)},
};

static CPatterned::StateMachine::SCodeFunction skCodeFuncs[] = {
    {"ResetAttack", static_cast< CPatterned::StateMachine::CodeFunc >(&CMysteryFlyer::ResetAttack)},
    {"CalcLineOfSight",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CMysteryFlyer::CalcLineOfSight)},
};

CMysteryFlyer::CMysteryFlyer(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                             const CTransform4f& xf, const CModelData& modelData,
                             const CActorParameters& actorParams,
                             const CPatternedInfo& patternedInfo, const SLdrMysteryFlyerData& data)
: CPatterned(kPAI_MysteryFlyer, uid, name, kFT_Zero, info, xf, modelData, patternedInfo, kMT_Flyer,
             kCT_One, kBT_AiMovedFlyer, actorParams)
, mData(data)
, mGenerateAnimId(patternedInfo.GetAnimationParameters().GetInitialAnimation())
, mPatrolWaypointId(kInvalidUniqueId)
, mShotProjectileInfo(data.shotProjectile, LdrToDamageInfo(data.shotDamage))
, mMoving(false)
, mHasLineOfSight(false)
, mAlerted(false)
, mHasAttackPath(false)
, mPathFindSearch(nullptr, 3, patternedInfo.GetPathfindingIndex(), 0.f, 0.f, 0,
                  CPFRegion::kRP_Center)
, mAttackInterval(1.f)
, mAttackTimer(0.f)
, mMoveDirection(CVector3f::Forward())
, mCurrentSpeed(0.f)
, mTeamAiMgrId(kInvalidUniqueId)
, mHoverHeight(data.hoverHeight)
, mStuckReferencePos(CVector3f::Zero())
, mStuckTimer(0.f)
, mLineOfSightTimer(10.f) {
  mShotProjectileInfo.Token().Lock();
}

CMysteryFlyer::~CMysteryFlyer() {}

void CMysteryFlyer::SetupStateMachine(CStateManager& mgr) {
  StateMachine* stateMachine = mStateMachine.get();
  CPatterned::SetupStateMachine(mgr);
  stateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
  stateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
  stateMachine->SetCodeFunctions(skCodeFuncs, ARRAY_SIZE(skCodeFuncs));
}

void CMysteryFlyer::Think(float dt, CStateManager& mgr) {
  CPatterned::Think(dt, mgr);
  if (GetActive()) {
    UpdateMovement(mgr, dt);
    mLineOfSightTimer += dt;
  }
}

void CMysteryFlyer::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CPatterned::AcceptScriptMsg(mgr, msg);
  switch (message) {
  case kSM_Create:
    BodyController()->Activate(mgr, pas::kAS_Invalid);
    BodyController()->SetPlayDeathAnims(false);
    mHoverHeight += mgr.Random()->Float();
    break;
  case kSM_AreaLoaded:
    mPathFindSearch.SetArea(
        mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()).GetPostConstructed()->mPathArea);
    JoinTeam(mgr);
    mTeamAiMgrId = CScriptTeamAiMgr::GetAssociatedTeamId(*this, mgr);
    break;
  case kSM_Damage:
    if (BodyController()->HasBodyState(pas::kAS_KnockBack)) {
      if (BodyController()->GetCurrentStateId() != pas::kAS_KnockBack) {
        BodyController()->CommandMgr().DeliverCmd(
            CBCKnockBackCmd(GetTransform().GetForward(), pas::kS_Zero));
      }
    }
    // fallthrough
  case kSM_Alert:
    mAlerted = true;
    break;
  }
}

void CMysteryFlyer::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                    EUserEventType type, float dt) {
  if (!(type == kUE_FadeIn ? true : false)) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

CProjectileInfo* CMysteryFlyer::ProjectileInfo() { return &mShotProjectileInfo; }

void CMysteryFlyer::Start(CStateManager& mgr, EStateMsg msg, float dt) {}

void CMysteryFlyer::Generate(CStateManager& mgr, EStateMsg msg, float dt) {
  if (mData.needsToGenerate) {
    switch (msg) {
    case kStateMsg_Activate:
      mAnimationState.SetState(CAnimationState::kAS_Ready);
      break;
    case kStateMsg_Update:
      if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Generate)) {
        BodyController()->CommandMgr().DeliverCmd(
            CBCGenerateCmd(pas::kGType_Zero, mGenerateAnimId));
      }
      break;
    case kStateMsg_Deactivate:
      mAnimationState.SetState(CAnimationState::kAS_NotReady);
      break;
    }
  }
}

void CMysteryFlyer::GoToPatrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    QuitTeam(mgr);
    mPatrolWaypointId = FindConnectedObject(mgr, kSS_Patrol, kSM_Follow);
    if (mPatrolWaypointId != kInvalidUniqueId) {
      if (const CScriptWaypoint* waypoint =
              TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(mPatrolWaypointId))) {
        CVector3f offset = waypoint->GetTranslation() - GetTranslation();
        if (offset.CanBeNormalized()) {
          offset = 3.f * offset.Normalize();
        }
        mPathFindNavigation.SetDestination(waypoint->GetTranslation() - offset);
        mWaypointNavigation.SetLastDestination(
            waypoint->FindConnectedObject(mgr, kSS_Arrived, kSM_Next));
      } else {
        mPatrolWaypointId = kInvalidUniqueId;
      }
    }
    mMoving = true;
    break;
  }
  if (mPatrolWaypointId != kInvalidUniqueId) {
    mPathFindNavigation.PathFind(mgr, msg, dt, *this);
  }
  switch (msg) {
  case kStateMsg_Update:
    UpdateSteering(mgr, dt);
    break;
  case kStateMsg_Deactivate:
    mMoving = false;
    break;
  }
}

void CMysteryFlyer::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mWaypointNavigation.SetLastDestination(kInvalidUniqueId);
    mMoving = true;
    break;
  }
  CPatterned::Patrol(mgr, msg, dt);
  switch (msg) {
  case kStateMsg_Update:
    UpdateSteering(mgr, dt);
    break;
  case kStateMsg_Deactivate:
    mMoving = false;
    break;
  }
}

void CMysteryFlyer::FollowAttackPath(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    const TUniqueId attackPath = FindConnectedObject(mgr, kSS_Attack, kSM_Follow);
    mHasAttackPath = false;
    if (attackPath != kInvalidUniqueId) {
      mHasAttackPath = true;
      mWaypointNavigation.SetLastDestination(attackPath);
      mMoving = true;
    }
    break;
  }
  }
  if (mHasAttackPath) {
    CPatterned::Patrol(mgr, msg, dt);
  }
  switch (msg) {
  case kStateMsg_Update:
    UpdateSteering(mgr, dt);
    break;
  case kStateMsg_Deactivate:
    mMoving = false;
    break;
  }
}

void CMysteryFlyer::Hover(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    JoinTeam(mgr);
    break;
  case kStateMsg_Update:
    FaceTarget(mgr, dt);
    mAttackTimer += dt;
    break;
  }
}

void CMysteryFlyer::Approach(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    const CTransform4f& playerXf = mgr.GetPlayer(0)->GetTransform();
    mPathFindNavigation.SetDestination(
        playerXf.GetTranslation() +
        (mMaxAttackRange * playerXf.GetForward() + mHoverHeight * CVector3f::Up()));
    mMoving = true;
    break;
  }
  case kStateMsg_Update:
    mAttackTimer += dt;
    FaceTarget(mgr, dt);
    break;
  case kStateMsg_Deactivate:
    mMoving = false;
    break;
  }
  mPathFindNavigation.PathFind(mgr, msg, dt, *this);
}

void CMysteryFlyer::Attack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    CScriptTeamAiMgr::StartAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamAiMgrId,
                                  GetUniqueId());
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_ProjectileAttack)) {
      BodyController()->CommandMgr().DeliverCmd(
          CBCProjectileAttackCmd(pas::kS_Zero, mgr.GetPlayer(0)->GetAimPosition(mgr, 0.f), false));
    }
    FaceTarget(mgr, dt);
    break;
  case kStateMsg_Deactivate:
    CScriptTeamAiMgr::EndAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamAiMgrId, GetUniqueId(),
                                true);
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CMysteryFlyer::Dodge(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Step)) {
      BodyController()->CommandMgr().DeliverCmd(CBCStepCmd(
          static_cast< pas::EStepDirection >(mgr.Random()->Next() % 10), pas::kStep_Normal));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CMysteryFlyer::Dead(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    MassiveDeath(mgr);
    break;
  }
}

void CMysteryFlyer::ResetAttack(CStateManager& mgr, float dt) {
  const float attackTimeVariation = mAttackTimeVariation;
  const float averageAttackTime = GetAverageAttackTime();
  mAttackInterval = averageAttackTime + (mgr.Random()->Float() - 0.5f) * attackTimeVariation;
  mAttackTimer = 0.f;
}

void CMysteryFlyer::CalcLineOfSight(CStateManager& mgr, float dt) { UpdateLineOfSight(mgr); }

bool CMysteryFlyer::StateOver(CStateManager& mgr, const CTriggerData& data) const {
  return mAnimationState.IsOver();
}

bool CMysteryFlyer::NearPatrol(CStateManager& mgr, const CTriggerData& data) const {
  bool nearPatrol = true;
  if (mPatrolWaypointId != kInvalidUniqueId) {
    nearPatrol = mPathFindSearch.IsOver();
  }
  return nearPatrol;
}

bool CMysteryFlyer::InDetectionRange(CStateManager& mgr, const CTriggerData& data) const {
  return mAlerted || CPatterned::InDetectionRange(mgr, data);
}

bool CMysteryFlyer::AttackPathOver(CStateManager& mgr, const CTriggerData& data) const {
  return !mHasAttackPath || mWaypointNavigation.GetDestination() == kInvalidUniqueId;
}

bool CMysteryFlyer::ShouldAttack(CStateManager& mgr, const CTriggerData& data) const {
  bool shouldAttack = false;
  if (mAttackTimer > mAttackInterval) {
    const float distance = (mgr.GetPlayer(0)->GetTranslation() - GetTranslation()).Magnitude();
    shouldAttack = distance > mMinAttackRange && distance < mMaxAttackRange;
    if (shouldAttack && mTeamAiMgrId != kInvalidUniqueId) {
      shouldAttack = CScriptTeamAiMgr::CanStartAttack(CScriptTeamAiMgr::kAT_Projectile, mgr,
                                                      mTeamAiMgrId, GetUniqueId());
    }
  }
  return shouldAttack;
}

bool CMysteryFlyer::Leash(CStateManager& mgr, const CTriggerData& data) const {
  return CPatterned::Leash(mgr, data);
}

bool CMysteryFlyer::ShouldApproach(CStateManager& mgr, const CTriggerData& data) const {
  const CVector3f toPlayer = mgr.GetPlayer(0)->GetTranslation() - GetTranslation();
  const float maxAttackRange = mMaxAttackRange;
  return toPlayer.Magnitude() > maxAttackRange;
}

bool CMysteryFlyer::PathOver(CStateManager& mgr, const CTriggerData& data) const {
  return mPathFindSearch.GetResult() || mPathFindSearch.IsOver();
}

bool CMysteryFlyer::Stuck(CStateManager& mgr, const CTriggerData& data) const {
  return mStuckTimer > 0.5f;
}

bool CMysteryFlyer::LineOfSight(CStateManager& mgr, const CTriggerData& data) const {
  return mHasLineOfSight;
}

void CMysteryFlyer::JoinTeam(CStateManager& mgr) {
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

void CMysteryFlyer::QuitTeam(CStateManager& mgr) {
  if (mTeamAiMgrId != kInvalidUniqueId) {
    if (CScriptTeamAiMgr* team = TCastToPtr< CScriptTeamAiMgr >(mgr.ObjectById(mTeamAiMgrId))) {
      if (team->IsPartOfTeam(GetUniqueId())) {
        team->QuitTeam(GetUniqueId());
        mTeamAiMgrId = kInvalidUniqueId;
      }
    }
  }
}

void CMysteryFlyer::FaceTarget(CStateManager& mgr, float dt) {
  CVector3f toPlayer = mgr.GetPlayer(0)->GetTranslation() - GetTranslation();
  toPlayer.SetZ(0.f);
  BodyController()->FaceDirection(toPlayer, dt);
}

bool CMysteryFlyer::UpdateLineOfSight(CStateManager& mgr) {
  if (mLineOfSightTimer > 0.7f) {
    mLineOfSightTimer = 0.f;
    mHasLineOfSight =
        mgr.RayCollideWorld(GetTranslation(), mgr.GetPlayer(0)->GetAimPosition(mgr, 0.f),
                            CMaterialFilter::MakeExclude(CMaterialList(kMT_Player)), this);
  }
  return mHasLineOfSight;
}

CVector3f CMysteryFlyer::CalculateSeparation(CStateManager& mgr) {
  CVector3f total = CVector3f::Zero();
  float count = 0.f;
  const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    const CPatterned* other = TCastToConstPtr< CPatterned >(list[i]);
    if (other != nullptr && other != this && other->GetCurrentAreaId() == GetCurrentAreaId()) {
      const CVector3f separation =
          mSteeringBehaviors.Separation(*this, other->GetTranslation(), mData.separationDistance);
      if (separation.IsMagnitudeSafe()) {
        total += separation.AsNormalized();
        count += 1.f;
      }
    }
  }

  const CVector3f toPlayer = mgr.GetPlayer(0)->GetTranslation() - GetTranslation();
  const float maxAttackRange = mMaxAttackRange;
  if (toPlayer.Magnitude() < maxAttackRange) {
    const CVector3f separation =
        mSteeringBehaviors.Separation(*this, mgr.GetPlayer(0)->GetTranslation(), mMinAttackRange);
    if (separation.IsMagnitudeSafe()) {
      total += separation.AsNormalized();
      count += 1.f;
    }
    const float heightError = mHoverHeight + toPlayer.GetZ();
    const float absHeightError = CMath::AbsF(heightError);
    if (absHeightError > 1.f) {
      total.SetZ(total.GetZ() + heightError / absHeightError);
      count += 1.f;
    }
  }

  if (count > 0.f) {
    total *= 1.f / count;
  }
  return total;
}

void CMysteryFlyer::UpdateMovement(CStateManager& mgr, float dt) {
  CVector3f desired = BodyController()->GetCommandMgr().GetMoveVector();
  desired = desired + CalculateSeparation(mgr);

  const float acceleration = dt * mData.hoverSpeed;
  if (mMoving) {
    mCurrentSpeed = CMath::Min(mData.hoverSpeed, mCurrentSpeed + acceleration);
  } else {
    mCurrentSpeed = rstl::max_val(0.f, mCurrentSpeed - acceleration);
    if (mCurrentSpeed == 0.f) {
      mMoveDirection = GetTransform().GetForward();
    }
  }

  if (mCurrentSpeed > 0.f && desired.IsMagnitudeSafe()) {
    mMoveDirection.Normalize();
    mMoveDirection = CVector3f::Slerp(mMoveDirection, desired.AsNormalized(),
                                      CRelAngle::FromDegrees(360.f * dt));
    const float distance = mCurrentSpeed * dt;
    MoveToInOneFrameWR(GetTranslation() + mMoveDirection * distance, dt);
    if (mMoving &&
        (GetTranslation() - mStuckReferencePos).Magnitude() < mCurrentSpeed * (0.2f * dt)) {
      mStuckTimer += dt;
    } else {
      mStuckTimer = 0.f;
      mStuckReferencePos = GetTranslation();
    }
  } else {
    mStuckTimer = 0.f;
    mStuckReferencePos = GetTranslation();
  }
}

void CMysteryFlyer::UpdateSteering(CStateManager& mgr, float dt) {
  CVector3f direction = GetTransform().GetForward();
  const TUniqueId destinationId = mWaypointNavigation.GetDestination();
  if (const CEntity* destination = mgr.GetObjectById(destinationId)) {
    const CActor* actor = static_cast< const CActor* >(destination);
    direction = actor->GetTranslation() - GetTranslation();
    direction.SetZ(0.f);
  }
  BodyController()->FaceDirection(direction, dt);
}

CEntity* REL_LoadMysteryFlyer(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrMysteryFlyer sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrMysteryFlyer.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  return rs_new CMysteryFlyer(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      *modelData, LdrToActorParameters(sldrThis.actorInformation),
      LdrToPatternedInfo(sldrThis.patterned, nullptr), sldrThis.mysteryFlyerProperties);
}

static void SetFuncPtrs() {
  static SMysteryFlyer_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &REL_LoadMysteryFlyer;
  SetSMysteryFlyer_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSMysteryFlyer_FuncPtrs(nullptr); }
