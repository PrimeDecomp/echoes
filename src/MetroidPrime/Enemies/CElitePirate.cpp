#include "MetroidPrime/Enemies/CElitePirate.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/BodyState/CBodyStateInfo.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CRumbleManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Enemies/CElitePirateGrenadeLauncher.hpp"
#include "MetroidPrime/PathFinding/CPathFindArea.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrElitePirate.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTeamAiMgr.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"
#include "REL/REL_Setup.h"
#include "rstl/math.hpp"

// Guessed name; material used by the melee and touch damage filters.
static EMaterialTypes sSolidMaterial = kMT_Solid;

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"Alerted", static_cast< CPatterned::StateMachine::TriggerFunc >(&CElitePirate::Alerted)},
    {"AngryAttackOver",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CElitePirate::AngryAttackOver)},
    {"AttackPatternOver",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CElitePirate::AttackPatternOver)},
    {"BreakProjectileAttack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CElitePirate::BreakProjectileAttack)},
    {"CanShockwave",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CElitePirate::CanShockwave)},
    {"ClearLineOfSight",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CElitePirate::ClearLineOfSight)},
    {"DonePursuing",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CElitePirate::DonePursuing)},
    {"DoneTurning",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CElitePirate::DoneTurning)},
    {"HasAttackPattern",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CElitePirate::HasAttackPattern)},
    {"InDetectionRange",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CElitePirate::InDetectionRange)},
    {"InPosition", static_cast< CPatterned::StateMachine::TriggerFunc >(&CElitePirate::InPosition)},
    {"NotReachedTarget",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CElitePirate::NotReachedTarget)},
    {"PickedSpreadShot",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CElitePirate::PickedSpreadShot)},
    {"PlayerInNoAttack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CElitePirate::PlayerInNoAttack)},
    {"PoweredDown",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CElitePirate::PoweredDown)},
    {"PoweredUp", static_cast< CPatterned::StateMachine::TriggerFunc >(&CElitePirate::PoweredUp)},
    {"ReadyToCharge",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CElitePirate::ReadyToCharge)},
    {"ReturnedToPatrol",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CElitePirate::ReturnedToPatrol)},
    {"ShieldKilled",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CElitePirate::ShieldKilled)},
    {"ShockwaveIsNext",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CElitePirate::ShockwaveIsNext)},
    {"ShotAt", static_cast< CPatterned::StateMachine::TriggerFunc >(&CElitePirate::ShotAt)},
    {"ShouldAlert",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CElitePirate::ShouldAlert)},
    {"ShouldFire", static_cast< CPatterned::StateMachine::TriggerFunc >(&CElitePirate::ShouldFire)},
    {"ShouldMeleeAttack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CElitePirate::ShouldMeleeAttack)},
    {"ShouldShockwave",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CElitePirate::ShouldShockwave)},
    {"ShouldTurn", static_cast< CPatterned::StateMachine::TriggerFunc >(&CElitePirate::ShouldTurn)},
    {"SpotPlayer", static_cast< CPatterned::StateMachine::TriggerFunc >(&CElitePirate::SpotPlayer)},
    {"StillAngry", static_cast< CPatterned::StateMachine::TriggerFunc >(&CElitePirate::StillAngry)},
    {"TargetNotOnMesh",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CElitePirate::TargetNotOnMesh)},
    {"TargetUnreachable",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CElitePirate::TargetUnreachable)},
    {"TooClose", static_cast< CPatterned::StateMachine::TriggerFunc >(&CElitePirate::TooClose)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Alert", static_cast< CPatterned::StateMachine::StateFunc >(&CElitePirate::Alert)},
    {"AngryAttackBegin",
     static_cast< CPatterned::StateMachine::StateFunc >(&CElitePirate::AngryAttackBegin)},
    {"AngryAttackEnd",
     static_cast< CPatterned::StateMachine::StateFunc >(&CElitePirate::AngryAttackEnd)},
    {"Dead", static_cast< CPatterned::StateMachine::StateFunc >(&CElitePirate::Dead)},
    {"FollowAttackPattern",
     static_cast< CPatterned::StateMachine::StateFunc >(&CElitePirate::FollowAttackPattern)},
    {"InvulnAlert", static_cast< CPatterned::StateMachine::StateFunc >(&CElitePirate::InvulnAlert)},
    {"MeleeAttack", static_cast< CPatterned::StateMachine::StateFunc >(&CElitePirate::MeleeAttack)},
    {"Patrol", static_cast< CPatterned::StateMachine::StateFunc >(&CElitePirate::Patrol)},
    {"PowerDown", static_cast< CPatterned::StateMachine::StateFunc >(&CElitePirate::PowerDown)},
    {"PowerUp", static_cast< CPatterned::StateMachine::StateFunc >(&CElitePirate::PowerUp)},
    {"ProjectileAttack",
     static_cast< CPatterned::StateMachine::StateFunc >(&CElitePirate::ProjectileAttack)},
    {"Pursue", static_cast< CPatterned::StateMachine::StateFunc >(&CElitePirate::Pursue)},
    {"Shielding", static_cast< CPatterned::StateMachine::StateFunc >(&CElitePirate::Shielding)},
    {"ShieldUp", static_cast< CPatterned::StateMachine::StateFunc >(&CElitePirate::ShieldUp)},
    {"Shockwave", static_cast< CPatterned::StateMachine::StateFunc >(&CElitePirate::Shockwave)},
    {"SpreadShot", static_cast< CPatterned::StateMachine::StateFunc >(&CElitePirate::SpreadShot)},
    {"Stunned", static_cast< CPatterned::StateMachine::StateFunc >(&CElitePirate::Stunned)},
    {"TargetPatrol",
     static_cast< CPatterned::StateMachine::StateFunc >(&CElitePirate::TargetPatrol)},
    {"Taunt", static_cast< CPatterned::StateMachine::StateFunc >(&CElitePirate::Taunt)},
    {"Turn", static_cast< CPatterned::StateMachine::StateFunc >(&CElitePirate::Turn)},
    {"Wait", static_cast< CPatterned::StateMachine::StateFunc >(&CElitePirate::Wait)},
};

static CPatterned::StateMachine::SCodeFunction skCodeFuncs[] = {
    {"SelectTarget",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CElitePirate::SelectTarget)},
    {"PickAttackType",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CElitePirate::PickAttackType)},
};

// Guessed names; materials used by the wall avoidance, rocket and collision setup tests.
static EMaterialTypes sWallIncludeSolidMaterial = kMT_Solid;
static EMaterialTypes sWallIncludeAIBlockMaterial = kMT_AIBlock;
static EMaterialTypes sWallExcludeFloorMaterial = kMT_Floor;
static EMaterialTypes sWallExcludePlayerMaterial = kMT_Player;
static EMaterialTypes sWallExcludeAIPassthroughMaterial = kMT_AIPassthrough;
static EMaterialTypes sWallExcludeCollisionActorMaterial = kMT_CollisionActor;
static EMaterialTypes sRocketSolidMaterial = kMT_Solid;
static EMaterialTypes sRocketCharacterMaterial = kMT_Character;
static EMaterialTypes sClearSightSolidMaterial = kMT_Solid;
static EMaterialTypes sClearSightFloorMaterial = kMT_Floor;
static EMaterialTypes sCollisionInclude = kMT_Solid;
static EMaterialTypes sCollisionExclude0 = kMT_ProjectilePassthrough;
static EMaterialTypes sCollisionExclude1 = kMT_CollisionActor;
static EMaterialTypes sCollisionExclude2 = kMT_AIPassthrough;
static EMaterialTypes sCollisionExclude3 = kMT_Player;
static EMaterialTypes sCollisionExclude4 = kMT_Platform;
static EMaterialTypes sAIJointMaterial = kMT_AIJoint;
static EMaterialTypes sCameraPassthroughMaterial = kMT_CameraPassthrough;

static const CElitePirate::SJointInfo skLeftArmJointList[] = {
    {"L_shoulder", "L_elbow", 1.f, 1.5f},
    {"L_elbow", "L_wrist", 1.1f, 1.3f},
    {"L_knee", "L_ankle", 0.9f, 1.2f},
};

static const CElitePirate::SJointInfo skRightArmJointList[] = {
    {"R_shoulder", "R_elbow", 1.f, 1.5f},
    {"R_elbow", "R_wrist", 1.1f, 1.3f},
    {"R_knee", "R_ankle", 0.9f, 1.2f},
};

static const CElitePirate::SSphereJointInfo skSphereJointList[] = {
    {"Head_1", 1.2f}, {"L_Palm_LCTR", 1.5f}, {"R_Palm_LCTR", 1.5f}, {"Spine_1", 1.8f},
    {"Collar", 1.5f}, {"L_ball", 0.8f},      {"R_ball", 0.8f},      {"Grenade_Collision_LCTR", 1.f},
};

// Guessed names; locator names looked up in the animation data.
static const char* const skpHeadLCTR = "Head_1";
static const char* const skpLauncherLCTR = "grenadeLauncher_LCTR";
static const char* const skpRightClawLCTR = "R_Palm_LCTR";
static const char* const skpLeftClawLCTR = "L_Palm_LCTR";
static const char* const skpGrenadeLauncherLCTR = "lockon_target_LCTR";
static const char* const skpRightBallLCTR = "R_ball";

static CVector3f skExtendedClawBounds(1.5f, 1.5f, 1.5f);
static CVector3f skLocalShieldBounds(4.f, 4.f, 2.f);

template < typename T >
void CElitePirate::DeliverCommand(EStateMsg msg, pas::EAnimationState state, const T& cmd) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mBodyController->CommandMgr().DeliverCmd(cmd);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*mBodyController, state)) {
      mBodyController->CommandMgr().DeliverCmd(cmd);
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

CElitePirate::CElitePirate(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                           const CTransform4f& xf, const CModelData& modelData,
                           const CPatternedInfo& patternedInfo, const CActorParameters& actorParams,
                           const CElitePirateData& data)
: CPatterned(kPAI_ElitePirate, uid, name, kFT_Zero, info, xf, modelData, patternedInfo, kMT_Ground,
             kCT_One, kBT_BiPedal, actorParams)
, x7c0_(-1)
, mVulnerability(patternedInfo.GetDamageVulnerability())
, mShieldCollisionMgr(nullptr)
, mLeftClawPos(CVector3f::Zero())
, mRightClawPos(CVector3f::Zero())
, mPathDestination(CVector3f::Zero())
, mPowerUpPos(CVector3f::Zero())
, mData(data)
, mBodyCollisionMgr(nullptr)
, mCollisionAabb(GetBoundingBox(), GetMaterialList())
, mEnergyAbsorbDesc(data.mEnergyAbsorbEffect != kInvalidAssetId
                        ? rstl::optional_object< TLockedToken< CGenDescription > >(
                              TLockedToken< CGenDescription >(gpSimplePool->GetObj(
                                  SObjectTag('PART', data.mEnergyAbsorbEffect))))
                        : rstl::optional_object< TLockedToken< CGenDescription > >())
, mHeadId(kInvalidUniqueId)
, mLauncherId(kInvalidUniqueId)
, mEnergyAttractorId(kInvalidUniqueId)
, mTargetId(kInvalidUniqueId)
, mInitialSpeed(mSpeed)
, mSteeringSpeed(1.f)
, mHp(0.f)
, mAttackTimer(0.f)
, mShotAtTimer(0.f)
, mElapsedTime(0.f)
, mUnreachableTime(0.f)
, mWallBackoffTime(0.f)
, mMarkedRegionIndex(0)
, mPathFindSearch(nullptr, 1 + (patternedInfo.IsAnEncounter() ? 0x200 : 0),
                  patternedInfo.GetPathfindingIndex(), 1.f, 1.f, 0, CPFRegion::kRP_Center)
, mTargetDestPos(CVector3f::Zero())
, mPositionHistory(5.f)
, mDamageOn(false)
, mShotAt(false)
, mAlert(false)
, mAlertPending(false)
, mReturnedToPatrol(false)
, mLauncherAlive(false)
, mAlertDone(false)
, mLastActionFallback(kEA_Invalid)
, mAttackState(kEA_Invalid)
, mFsm(gpSimplePool->GetObj(SObjectTag('FSM2', data.mStateMachine)))
, mAlertTauntType(pas::kTT_Six)
, mPoweredLocomotionType(pas::kLT_Relaxed)
, mPoweredUp(false)
, mMeleeSeverity(pas::kS_One)
, mNextMeleeSeverity(pas::kS_One)
, mLastMeleeTime(-1000.f)
, mAttackingRightClaw(false)
, mAttackingLeftClaw(false)
, mShockwaveSeverity(pas::kS_One)
, mLastShockwaveSeverity(pas::kS_Seven)
, mTurnDirection(CVector3f::Zero())
, mPlayerTargetPos(CVector3f::Zero())
, mProjectileInfo(data.mRocket, data.mRocketDamage)
, mRocketsFired(0)
, mRocketCount(0)
, mAttackType(kRM_Homing)
, mDefaultAttackType(kRM_Spread)
, mBreakProjectileAttack(false)
, mAttractorBlend(0.f)
, mAttractorPos(CVector3f::Zero())
, mTauntType(pas::kTT_Zero)
, mAngryCount(0)
, mShockwaveIsNext(false)
, mAngryAttackOver(false)
, mPrevShockwave(false)
, mHintHandled(false)
, mAbsorbFlash(0.f)
, mAbsorbingEnergy(false) {
  mProjectileInfo.Token().Lock();
  KnockBackController().EnableAllAnimReactions(false);
  KnockBackController().EnableBurnDeath(false);
  KnockBackController().EnableExplodeDeath(false);
  KnockBackController().EnableBurn(false);
  KnockBackController().EnableSlow(false);
  KnockBackController().EnableFreeze(false);
  KnockBackController().EnableAnimReaction(CKnockBackMgr::kAR_Flinch, true);
  SetupPathFindSearch();
  if (HasLocomotionAnimation(10) == true) {
    mPoweredLocomotionType = pas::kLT_Internal10;
  } else if (HasLocomotionAnimation(11) == true) {
    mPoweredLocomotionType = pas::kLT_Internal11;
  } else if (HasLocomotionAnimation(12) == true) {
    mPoweredLocomotionType = pas::kLT_Internal12;
  }
  if (mData.mShieldedModel != kInvalidAssetId) {
    mShield.mModel =
        rstl::optional_object< TLockedToken< CSkinnedModel > >(TLockedToken< CSkinnedModel >(
            rs_new CSkinnedModel(gpSimplePool->GetObj(SObjectTag('CMDL', mData.mShieldedModel)),
                                 gpSimplePool->GetObj(SObjectTag('CSKR', mData.mShieldedSkinRules)),
                                 GetAnimationData()->GetModelData()->GetLayoutInfo())));
  }
}

CElitePirate::~CElitePirate() {}

bool CElitePirate::HasLocomotionAnimation(int locomotionType) const {
  const CPASDatabase& pasDatabase = GetAnimationData()->GetPASDatabase();
  const CPASAnimParmData parms(pas::kAS_Locomotion, CPASAnimParm::FromEnum(0),
                               CPASAnimParm::FromEnum(locomotionType));
  return pasDatabase.FindBestAnimation(parms, -1).second == mData.mInitialAnimation;
}

CGenericFSM2* CElitePirate::GetFsm() {
  if (mFsm->IsLoaded()) {
    TToken< CGenericFSM2 > token(*mFsm);
    return *token;
  }
  return nullptr;
}

void CElitePirate::SetupStateMachineHelper(CStateManager& mgr) {
  InitializeStateMachine(mgr);
  mStateMachine->SetTriggerFunctions(skTriggers, 31);
  mStateMachine->SetStateFunctions(skStates, 21);
  mStateMachine->SetCodeFunctions(skCodeFuncs, 2);
}

void CElitePirate::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  if (!mStateMachine->HasState()) {
    SetupStateMachineHelper(mgr);
    return;
  }
  if (mShield.mPopPending == true) {
    PopShield(mgr);
    mShield.mPopPending = false;
  }
  CPatterned::Think(dt, mgr);
  CElitePirateGrenadeLauncher* launcher =
      static_cast< CElitePirateGrenadeLauncher* >(mgr.ObjectById(mLauncherId));
  if (launcher != nullptr) {
    launcher->SetAddColor(mColor);
    launcher->SetSlowedSpeed(mBodyController->GetTimeScale());
    launcher->SetFollowPlayer(!mBodyController->IsFrozen());
  }
  mBodyCollisionMgr->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
  mShieldCollisionMgr->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
  if (IsShieldActive() == true && mShield.mActive == true) {
    mSpeed = 2.f * mInitialSpeed;
  } else {
    mSpeed = mInitialSpeed;
  }
  UpdateAttackTimer(dt);
  UpdateBreadCrumbTrail();
  UpdateGrenadeLauncher(mgr, mLauncherId, rstl::string_l(skpLauncherLCTR));
  UpdateAttractorBlend(dt);
  UpdateShieldEffect(dt, mgr);
  mElapsedTime += dt;
  if (!mLauncherAlive && mIngPossessionBlend > 0.15f) {
    DeleteGrenadeLauncher(mgr);
    mLauncherId = mgr.AllocateUniqueId();
    CreateGrenadeLauncher(mgr, mLauncherId);
    CEntity* newLauncher = mgr.ObjectById(mLauncherId);
    if (newLauncher != nullptr) {
      newLauncher->SetActive(GetActive());
    }
  }
  UpdateFlash(dt);
  AvoidWall(dt, mgr);
}

void CElitePirate::AvoidWall(float dt, CStateManager& mgr) {
  if (mAttackState == kEA_FollowAttackPattern || mAttackState == kEA_PowerDown ||
      mAttackState == kEA_PowerUp) {
    return;
  }
  const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
      CMaterialList(sWallIncludeSolidMaterial, sWallIncludeAIBlockMaterial),
      CMaterialList(sWallExcludeFloorMaterial, sWallExcludePlayerMaterial,
                    sWallExcludeAIPassthroughMaterial, sWallExcludeCollisionActorMaterial));
  const CVector3f forwardOffset = 1.8f * GetTransform().GetForward();
  const CVector3f clawStart = mLeftClawPos + forwardOffset;
  const CVector3f clawDirection = (mRightClawPos + forwardOffset) - clawStart;
  if (clawDirection.CanBeNormalized() == true) {
    const CGameArea& area = mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId());
    const float clawLength = clawDirection.Magnitude();
    if (!CGameCollision::RayStaticLineOfSightTest(area, clawStart, clawDirection.AsNormalized(),
                                                  clawLength, filter)) {
      MoveInOneFrameOR(0.15f * (-1.f * CVector3f::Forward()), dt);
      mWallBackoffTime = mElapsedTime;
      return;
    }
  }
  if (IsInStopPursuitHint(mgr)) {
    return;
  }
  const CActor* launcher = static_cast< const CActor* >(mgr.GetObjectById(mLauncherId));
  if (launcher == nullptr) {
    return;
  }
  const CVector3f locatorPos = GetLctrTransform(rstl::string_l("Head_1")).GetTranslation();
  CVector3f forward(GetTransform().GetForward().GetX(), GetTransform().GetForward().GetY(), 0.f);
  if (forward.CanBeNormalized() == true) {
    forward.Normalize();
  }
  const CVector3f start = locatorPos + 1.5f * forward +
                          2.2f * CVector3f(forward.GetY(), -forward.GetX(), 0.f) +
                          -0.6f * CVector3f::Up();
  const CVector3f end = static_cast< const CElitePirateGrenadeLauncher* >(launcher)
                            ->GetTurretTransform()
                            .GetTranslation() +
                        2.f * forward + 0.1f * CVector3f::Up();
  const CVector3f toEnd = end - start;
  if (toEnd.CanBeNormalized() != true) {
    return;
  }
  CAABox bounds = CAABox::MakeMaxInvertedBox();
  bounds.AccumulateBounds(start);
  bounds.AccumulateBounds(end);
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  mgr.BuildNearList(nearList, bounds, filter, this);
  const float length = toEnd.Magnitude();
  if (!CGameCollision::RayDynamicLineOfSightTest(mgr, start, toEnd.AsNormalized(), length, filter,
                                                 nearList, this)) {
    MoveInOneFrameOR(0.15f * (-1.f * CVector3f::Forward()), dt);
    mWallBackoffTime = mElapsedTime;
  }
}

void CElitePirate::UpdateFlash(float dt) {
  if (IsShieldActive() == true) {
    mShield.mFlash = rstl::min_val(1.f, mShield.mFlash + dt / 0.8f);
  } else {
    mShield.mFlash = rstl::max_val(0.f, mShield.mFlash - 0.5f * dt);
  }
  if (mAbsorbingEnergy == true) {
    mAbsorbFlash = rstl::min_val(1.f, mAbsorbFlash + dt / 3.f);
  } else {
    mAbsorbFlash = rstl::max_val(0.f, mAbsorbFlash - 0.5f * dt);
  }
}

void CElitePirate::UpdateShieldEffect(float dt, CStateManager& mgr) {
  if (mShield.mEffect.get() != nullptr) {
    mShield.mEffect->SetGlobalTranslation(mLeftClawPos);
    mShield.mEffect->Update(dt);
  }
}

void CElitePirate::UpdateAttractorBlend(float dt) {
  if (IsShieldActive() == true) {
    mAttractorBlend += dt / 0.8f;
    if (mAttractorBlend > 1.f) {
      mAttractorBlend = 1.f;
    }
  } else {
    mAttractorBlend -= dt / 0.4f;
    if (mAttractorBlend < 0.f) {
      mAttractorBlend = 0.f;
    }
  }
}

void CElitePirate::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  bool passToParent = true;
  switch (msg.GetMessage()) {
  case kSM_Create: {
    mBodyController->Activate(mgr, pas::kAS_Invalid);
    SetupCollisionManager(mgr);
    mLauncherId = mgr.AllocateUniqueId();
    CreateGrenadeLauncher(mgr, mLauncherId);
    CEntity* launcher = mgr.ObjectById(mLauncherId);
    if (launcher != nullptr) {
      launcher->SetActive(GetActive());
    }
    const float maxSpeed = mBodyController->GetBodyStateInfo().GetMaxSpeed();
    if (maxSpeed > 0.f) {
      mSteeringSpeed =
          (0.99f * mBodyController->GetBodyStateInfo().GetLocomotionSpeed(pas::kLA_Walk)) /
          maxSpeed;
    }
    mBodyController->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
    mBodyController->CommandMgr().SetSteeringSpeedRange(mSteeringSpeed, mSteeringSpeed);
    break;
  }
  case kSM_Activate: {
    mBodyCollisionMgr->SetActive(mgr, true);
    CEntity* launcher = mgr.ObjectById(mLauncherId);
    if (launcher != nullptr) {
      launcher->SetActive(true);
    }
    break;
  }
  case kSM_Deactivate:
    mBodyCollisionMgr->SetActive(mgr, false);
    mShieldCollisionMgr->SetActive(mgr, false);
    break;
  case kSM_Delete:
    UnmarkPathRegion(mgr);
    mBodyCollisionMgr->Destroy(mgr);
    mShieldCollisionMgr->Destroy(mgr);
    mgr.DeleteObjectRequest(mLauncherId);
    mLauncherId = kInvalidUniqueId;
    break;
  case kSM_Alert:
    mAlert = true;
    mAlertPending = true;
    break;
  case kSM_AreaLoaded: {
    const TAreaId areaId = GetCurrentAreaId();
    mPathFindSearch.SetArea(mgr.GetWorld()->GetAreaAlways(areaId).GetPostConstructed()->mPathArea);
    break;
  }
  case kSM_HitObject: {
    if (!mAlive) {
      break;
    }
    const TUniqueId senderId = msg.GetSenderId();
    if (GetHealthInfo()->GetHP() > 0.f && mAttackState == kEA_Melee) {
      const CCollisionActor* actor =
          TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(senderId));
      if (actor != nullptr) {
        const TUniqueId touchedId = actor->GetLastTouchedObject();
        CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(touchedId));
        if (player != nullptr) {
          if (mAttackingRightClaw == true) {
            MeleeDamagePlayer(mgr, player->GetUniqueId());
          }
          if (mAttackingLeftClaw == true) {
            if (mgr.GetPlayer(0)->GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
              PushPlayer(mgr, 60.f, 15.f);
            } else {
              PushPlayer(mgr, 25.f, 15.f);
            }
          }
        }
      }
    }
    CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(senderId));
    if (actor != nullptr) {
      const TUniqueId touchedId = actor->GetLastTouchedObject();
      if (touchedId != GetUniqueId()) {
        CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(touchedId));
        if (player != nullptr && mCurDamageRemTime <= 0.f && mLastMeleeTime + 1.5f < mElapsedTime) {
          const CMaterialFilter filter =
              CMaterialFilter::MakeIncludeExclude(CMaterialList(sSolidMaterial), CMaterialList());
          mgr.ApplyDamage(GetUniqueId(), player->GetUniqueId(), GetUniqueId(), GetContactDamage(),
                          filter, CVector3f::Zero());
          mCurDamageRemTime = mDamageWaitTime;
        }
      }
    }
    break;
  }
  case kSM_ReflectedDamage:
  case kSM_ResistedDamage:
    SetShotAt(true);
    break;
  case kSM_Damage: {
    passToParent = false;
    if (IsShieldActive() == true) {
      CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(msg.GetSenderId()));
      if (actor == nullptr) {
        break;
      }
      const CGameProjectile* projectile =
          TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(actor->GetLastTouchedObject()));
      if (projectile == nullptr) {
        break;
      }
      const float damage = projectile->GetCurrentDamageInfo().GetDamage();
      const ushort weapon = projectile->GetCurrentDamageInfo().GetWeaponMode1();
      switch (mShield.mType) {
      case kST_Light:
        if (weapon == kWT_Dark || weapon == kWT_Annihilator) {
          mShield.mDamageAbsorbed += damage;
          mShield.mLastHitTime = mElapsedTime;
        }
        break;
      case kST_Dark:
        if (weapon == kWT_Light || weapon == kWT_Annihilator) {
          mShield.mDamageAbsorbed += damage;
          mShield.mLastHitTime = mElapsedTime;
        }
        break;
      }
    } else if (mAbsorbingEnergy != true) {
      CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(msg.GetSenderId()));
      if (actor != nullptr) {
        const CGameProjectile* projectile =
            TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(actor->GetLastTouchedObject()));
        if (projectile != nullptr) {
          const TUniqueId projectileId = actor->GetLastTouchedObject();
          CDamageInfo damage = projectile->GetCurrentDamageInfo();
          damage.SetRadius(0.f);
          mgr.ApplyDamage(
              projectileId, GetUniqueId(), projectileId, damage,
              CMaterialFilter::MakeIncludeExclude(CMaterialList(sSolidMaterial), CMaterialList()),
              CVector3f::Zero());
          SetShotAt(true);
        }
      }
      const bool lowHealth =
          100.f * (GetHealthInfo()->GetHP() / GetHealthInfo()->GetInitialHP()) <= 25.f;
      mKnockBackController.EnableFreeze(lowHealth);
      mKnockBackController.EnableSlow(lowHealth);
    }
    break;
  }
  }
  if (passToParent) {
    CPatterned::AcceptScriptMsg(mgr, msg);
  }
}

void CElitePirate::UpdateEnergyAbsorb(CStateManager& mgr, EStateMsg msg) {
  if (msg == kStateMsg_Activate) {
    SetupHealthInfo(mgr, true);
    mAbsorbingEnergy = true;
  } else if (msg == kStateMsg_Deactivate) {
    SetupHealthInfo(mgr, false);
    mAbsorbingEnergy = false;
  }
}

void CElitePirate::AddToRenderer(const CStateManager& mgr) const { CPatterned::AddToRenderer(mgr); }

void CElitePirate::PreRender(CStateManager& mgr) {
  CPatterned::PreRender(mgr);
  mLeftClawPos = GetLctrTransform(rstl::string_l(skpLeftClawLCTR)).GetTranslation();
  mRightClawPos = GetLctrTransform(rstl::string_l(skpRightClawLCTR)).GetTranslation();
  CElitePirateGrenadeLauncher* launcher =
      static_cast< CElitePirateGrenadeLauncher* >(mgr.ObjectById(mLauncherId));
  if (launcher != nullptr) {
    launcher->SetModelFlags(GetModelFlags());
  }
  SetModelFlags(GetModelFlags().UseShaderSet(0));
  UpdateShieldEffect(0.f, mgr);
}

void CElitePirate::Render(const CStateManager& mgr) const {
  if (1.f + mShield.mLastHitTime > mElapsedTime) {
    gpRender->SetAmbientColor(
        CColor::Lerp(CColor::White(), CColor::Black(), mElapsedTime - mShield.mLastHitTime));
  } else {
    gpRender->SetAmbientColor(CColor::Black());
  }
  CPatterned::Render(mgr);
  RenderShieldFlash();
  if (mAlive == true && !mShield.mEffect.null()) {
    mShield.mEffect->Render();
  }
}

void CElitePirate::RenderShieldFlash() const {
  if (mShield.mFlash == 0.f && mAbsorbFlash == 0.f) {
    return;
  }
  gpRender->SetModelMatrix(GetTransform() * CTransform4f::Scale(GetModelData()->GetScale()));
  CColor color = CColor::White();
  float alpha;
  if (mAbsorbFlash > 0.f) {
    alpha =
        255.f * (mAbsorbFlash *
                 (0.25f + 0.15f * (0.5f * (1.f + static_cast< float >(sin(8.f * mElapsedTime))))));
  } else {
    float rate;
    switch (mShield.mType) {
    case kST_Dark:
      color = CColor::Purple();
      rate = 16.f;
      break;
    case kST_Light:
      rate = 10.f;
      break;
    default:
      return;
    }
    const float pulse = 0.5f * (1.f + static_cast< float >(sin(mElapsedTime * rate)));
    float flash = mShield.mFlash;
    if (flash == 1.f) {
      flash = 0.4f * pulse + 0.6f;
    }
    alpha = 255.f * flash;
  }
  const CModelFlags flags(
      CModelFlags::kT_Blend, 0,
      CModelFlags::EFlags(CModelFlags::kF_DepthCompare | CModelFlags::kF_DepthUpdate),
      color.WithAlphaOf(alpha));
  GetAnimationData()->Render(***mShield.mModel, flags);
}

void CElitePirate::RenderIngSnatchingTransition(const CStateManager& mgr) const {
  CPatterned::RenderIngSnatchingTransition(mgr);
  const CPlane plane(GetTranslation(), CUnitVector3f(CVector3f::Up(), CUnitVector3f::kN_No));
  GetModelData()->SetupWorldSpacePortalPlane(GetTransform(), plane);
}

CVector3f CElitePirate::GetAimPosition(const CStateManager& mgr, float dt) const {
  const CVector3f aimPosition = CPatterned::GetAimPosition(mgr, dt);
  if (mAttractorBlend == 0.f) {
    return aimPosition;
  }
  if (const CCollisionActor* attractor =
          TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(mEnergyAttractorId))) {
    mAttractorPos = attractor->GetTranslation();
  }
  return CVector3f::Lerp(aimPosition, mAttractorPos, mAttractorBlend);
}

void CElitePirate::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                   EUserEventType type, float dt) {
  bool handled = false;
  switch (type) {
  case kUE_Projectile:
    switch (mAttackState) {
    case kEA_Shockwave:
      CreateShockWave(mgr, node);
      handled = true;
      break;
    case kEA_Projectile:
    case kEA_SpreadShot:
      FireRocket(mgr);
      handled = true;
      break;
    }
    break;
  case kUE_BeginAction:
    if (mAttackState == kEA_Melee) {
      mAttackingRightClaw = true;
      mAttackingLeftClaw = true;
      ExtendTouchBounds(mgr, mRightClawIds, skExtendedClawBounds);
      ExtendTouchBounds(mgr, mLeftClawIds, skExtendedClawBounds);
    }
    break;
  case kUE_EndAction:
    switch (mAttackState) {
    case kEA_Projectile:
      mBreakProjectileAttack = true;
      break;
    case kEA_Melee:
      mAttackingRightClaw = false;
      mAttackingLeftClaw = false;
      ExtendTouchBounds(mgr, mRightClawIds, CVector3f::Zero());
      ExtendTouchBounds(mgr, mLeftClawIds, CVector3f::Zero());
      break;
    }
    break;
  case kUE_DamageOn:
    mDamageOn = true;
    handled = true;
    break;
  case kUE_DamageOff:
    mDamageOn = false;
    handled = true;
    break;
  case kUE_ScreenShake:
    ApplyScreenShake(mgr, GetTranslation(),
                     FindConnectedObject(mgr, kSS_InternalState1, kSM_Attach));
    ProcessStompGround(mgr);
    handled = true;
    break;
  case kUE_ObjectDrop:
    ApplyScreenShake(mgr, GetTranslation(), FindConnectedObject(mgr, kSS_Footstep, kSM_Attach));
    handled = true;
    break;
  case kUE_BecomeShootThrough:
    for (uint i = 0; i < mBodyCollisionMgr->GetNumCollisionActors(); ++i) {
      const TUniqueId uid = mBodyCollisionMgr->GetCollisionDescFromIndex(i).GetCollisionActorId();
      if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(uid))) {
        actor->AddMaterial(kMT_ProjectilePassthrough, mgr);
      }
    }
    handled = true;
    break;
  case kUE_GenerateEnd:
    mPoweredUp = true;
    break;
  case kUE_RemoveCollision:
    mBodyCollisionMgr->SetActive(mgr, false);
    RemoveMaterial(kMT_Solid, mgr);
    break;
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

void CElitePirate::Dead(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    UnmarkPathRegion(mgr);
    RemoveMaterial(kMT_Target, kMT_Orbit, kMT_SeekerTarget, mgr);
    if (CEntity* launcher = mgr.ObjectById(mLauncherId)) {
      launcher->SetActive(false);
    }
    if (mShield.mSfx) {
      CSfxManager::RemoveEmitter(mShield.mSfx);
    }
    mgr.DeleteObjectRequest(mLauncherId);
    mLauncherId = kInvalidUniqueId;
    mKnockBackController.EnableKnockBackPhysics(false);
  }
  CPatterned::Dead(mgr, msg, dt);
}

void CElitePirate::FireRocket(CStateManager& mgr) {
  ++mRocketsFired;
  const CElitePirateGrenadeLauncher* launcher =
      static_cast< const CElitePirateGrenadeLauncher* >(mgr.GetObjectById(mLauncherId));
  if (launcher != nullptr && mProjectileInfo.Token().TryCache()) {
    CTransform4f xf = launcher->GetTurretTransform();
    TUniqueId homingTarget = kInvalidUniqueId;
    switch (mAttackType) {
    case kRM_Homing:
      homingTarget = mgr.GetPlayer(0)->GetUniqueId();
      break;
    case kRM_Spread: {
      CPlayer* player = mgr.GetPlayer(0);
      CVector3f direction = player->GetAimPosition(mgr, 0.f) - xf.GetTranslation();
      direction.SetZ(0.f);
      if (direction.CanBeNormalized() == true) {
        direction.Normalize();
        const float x = direction.GetX();
        const float y = direction.GetY();
        const bool raisedAim = FindHintType(mgr) == CScriptAIHint::kHT_GrenadeLauncherRaisedAim;
        const float scale = raisedAim ? 1.5f : 1.f;
        const CVector3f offsets[4] = {
            scale * CVector3f(3.2f * -y, 3.2f * x, 0.f),
            scale * CVector3f(1.2f * -y, 1.2f * x, 0.f),
            scale * CVector3f(1.2f * y, 1.2f * -x, 0.f),
            scale * CVector3f(3.2f * y, 3.2f * -x, 0.f),
        };
        CVector3f target = player->GetAimPosition(mgr, 0.f) + offsets[mRocketsFired - 1];
        if (raisedAim) {
          target.SetZ(target.GetZ() + mgr.Random()->Range(6.f, 12.f));
          xf = CTransform4f::LookAt(xf.GetTranslation(), target, CVector3f::Up());
          homingTarget = mgr.GetPlayer(0)->GetUniqueId();
          break;
        }
        xf = CTransform4f::LookAt(xf.GetTranslation(), target, CVector3f::Up());
      }
      homingTarget = kInvalidUniqueId;
      break;
    }
    }
    CEnergyProjectile* projectile = rs_new CEnergyProjectile(
        true, mProjectileInfo.Token(), kWT_AI, xf, kMT_Character, mProjectileInfo.GetDamage(),
        mgr.AllocateUniqueId(), GetCurrentAreaId(), GetUniqueId(), homingTarget, kPA_None, false,
        CVector3f::One(), CImpactVisorEffect::None(), false, true, false, 1.f, 4.f, 4.f);
    if (projectile != nullptr) {
      mgr.AddObject(projectile);
    }
  }
}

void CElitePirate::CreateShockWave(CStateManager& mgr, const CInt32POINode& node) {
  CTransform4f xf = GetTransform();
  const bool isRightFoot = node.GetLocatorName() == rstl::string_l(skpRightBallLCTR);
  const CVector3f& clawPos = isRightFoot ? mRightClawPos : mLeftClawPos;
  xf.SetTranslation(CVector3f(clawPos.GetX(), clawPos.GetY(), GetTranslation().GetZ()));
  CShockWave* shockWave =
      rs_new CShockWave(mgr.AllocateUniqueId(), rstl::string_l("Shock Wave"),
                        CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true), xf,
                        GetUniqueId(), GetShockWaveInfo(), 2.f, 0.4f);
  if (shockWave != nullptr) {
    mgr.AddObject(shockWave);
  }
}

void CElitePirate::MeleeDamagePlayer(CStateManager& mgr, TUniqueId uid) {
  mgr.ApplyDamage(
      GetUniqueId(), uid, GetUniqueId(), mData.mMeleeDamage,
      CMaterialFilter::MakeIncludeExclude(CMaterialList(sSolidMaterial), CMaterialList()),
      CVector3f::Zero());
  mAttackingRightClaw = false;
  mLastMeleeTime = mElapsedTime;
}

bool CElitePirate::SpotPlayer(CStateManager& mgr, const CTriggerData& data) const {
  if (mAlert) {
    return true;
  }
  return CPatterned::SpotPlayer(mgr, data);
}

bool CElitePirate::InDetectionRange(CStateManager& mgr, const CTriggerData& data) const {
  if (mAlert) {
    return true;
  }
  return CPatterned::InDetectionRange(mgr, data);
}

bool CElitePirate::HasAttackPattern(CStateManager& mgr, const CTriggerData& data) const {
  return GetConnectedObject(mgr, kSS_Attack, kSM_Follow) != kInvalidUniqueId;
}

bool CElitePirate::AttackPatternOver(CStateManager& mgr, const CTriggerData& data) const {
  return mWaypointNavigation.GetDestination() == kInvalidUniqueId;
}

void CElitePirate::FollowAttackPattern(CStateManager& mgr, EStateMsg msg, float dt) {
  SetAttackState(kEA_FollowAttackPattern, msg);
  UpdateEnergyAbsorb(mgr, msg);
  mWaypointNavigation.Patrol(mgr, msg, dt, *this);
  switch (msg) {
  case kStateMsg_Activate: {
    mAlert = false;
    const TUniqueId waypointId = GetConnectedObject(mgr, kSS_Attack, kSM_Follow);
    mWaypointNavigation.SetDestination(waypointId);
    if (const CScriptWaypoint* waypoint =
            TCastToConstPtr< CScriptWaypoint >(mgr.ObjectById(waypointId))) {
      const CVector3f forward = GetTransform().GetForward();
      const CVector3f toWaypoint = waypoint->GetTranslation() - GetTranslation();
      if (CVector3f::Dot(forward, toWaypoint) <= 0.f) {
        mWaypointNavigation.SetInPosition(true);
      }
    }
    break;
  }
  default:
    break;
  }
}

bool CElitePirate::NotReachedTarget(CStateManager& mgr, const CTriggerData& data) const {
  if (mWallBackoffTime > 0.5f * (mShield.mShieldUpTime + mElapsedTime)) {
    return true;
  }
  return 15.f + mShield.mShieldUpTime < mElapsedTime;
}

bool CElitePirate::ShieldKilled(CStateManager& mgr, const CTriggerData& data) const {
  return mShield.mDamageAbsorbed >
         (mShield.mType == kST_Light ? mData.mLightShieldLimit : mData.mDarkShieldLimit);
}

bool CElitePirate::ShouldTurn(CStateManager& mgr, const CTriggerData& data) const {
  const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
  if (target != nullptr) {
    const CVector2f delta = (target->GetTranslation() - GetTranslation()).ToVec2f();
    const CVector2f forward = GetTransform().GetForward().ToVec2f();
    return CVector2f::GetAngleDiff(forward, delta) > 31.5f * (M_PIF / 180.f);
  }
  return false;
}

bool CElitePirate::DoneTurning(CStateManager& mgr, const CTriggerData& data) const {
  return AnimOver(mgr, data);
}

bool CElitePirate::DonePursuing(CStateManager& mgr, const CTriggerData& data) const {
  if (mUnreachableTime > 0.5f) {
    return true;
  }
  const int hint = FindHintType(mgr);
  if (hint == CScriptAIHint::kHT_ElitePirateNoAttack) {
    return true;
  }
  if (hint == CScriptAIHint::kHT_GrenadeLauncherRaisedAim) {
    if (PathShagged(mgr, CTriggerData(0.f)) == true || mPathFindSearch.IsOver()) {
      return true;
    }
  }
  if (mWallBackoffTime > mPursueStartTime) {
    if (GetCurrentAreaId() != mgr.GetPlayer(0)->GetCurrentAreaId()) {
      return true;
    }
    if (hint != CScriptAIHint::kHT_GrenadeLauncherRaisedAim) {
      const CVector3f delta = mgr.GetPlayer(0)->GetAimPosition(mgr, 0.f) - GetTranslation();
      if (delta.MagSquared() < mData.mMaxShockwaveRange * mData.mMaxShockwaveRange) {
        return true;
      }
    }
    if (IsInStopPursuitHint(mgr) == true) {
      return true;
    }
    return ClearLineOfSight(mgr, CTriggerData(0.f));
  }
  const CVector3f delta = mgr.GetPlayer(0)->GetAimPosition(mgr, 0.f) - GetTranslation();
  const float maxRange = mData.mMaxMeleeRange;
  if (delta.MagSquared() < 4.f * (maxRange * maxRange)) {
    return true;
  }
  if (2.5f + mPursueStartTime > mElapsedTime) {
    if (ClearLineOfSight(mgr, CTriggerData(0.f)) == true || IsInStopPursuitHint(mgr) == true) {
      return true;
    }
  }
  return mPredictedLeashTime > 3.f;
}

bool CElitePirate::ShouldAlert(CStateManager& mgr, const CTriggerData& data) const {
  if (!mAlertDone) {
    if (mPowerUpPos == CVector3f::Zero() || (mPowerUpPos - GetTranslation()).MagSquared() > 25.f) {
      return true;
    }
  }
  return false;
}

bool CElitePirate::ShouldFire(CStateManager& mgr, const CTriggerData& data) const {
  return CanFireRocket(mgr, mLauncherId);
}

bool CElitePirate::CanFireRocket(CStateManager& mgr, TUniqueId launcherId) const {
  if (mAttackTimer <= 0.f && launcherId != kInvalidUniqueId) {
    const CActor* launcher = static_cast< const CActor* >(mgr.GetObjectById(launcherId));
    if (launcher != nullptr) {
      const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
      if (target != nullptr) {
        const CVector3f targetPos = target->GetAimPosition(mgr, 0.f);
        const float distanceSq = (targetPos - GetTranslation()).MagSquared();
        if (distanceSq > mData.mMinRocketRange * mData.mMinRocketRange &&
            distanceSq < mData.mMaxRocketRange * mData.mMaxRocketRange &&
            !ShouldTurn(mgr, CTriggerData(0.f))) {
          const CVector3f launchPos = GetGrenadeLaunchPos(*launcher);
          const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
              CMaterialList(sRocketSolidMaterial, sRocketCharacterMaterial), CMaterialList());
          if (mgr.RayCollideWorld(launchPos, targetPos, filter, this)) {
            return true;
          }
        }
      }
    }
  }
  return false;
}

bool CElitePirate::ShouldMeleeAttack(CStateManager& mgr, const CTriggerData& data) const {
  if (FindHintType(mgr) != -1) {
    return false;
  }
  const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
  if (target != nullptr) {
    const float dx = target->GetTranslation().GetX() - GetTranslation().GetX();
    const float dy = target->GetTranslation().GetY() - GetTranslation().GetY();
    const float dz = target->GetTranslation().GetZ() - GetTranslation().GetZ();
    if (dx * dx + dy * dy + dz * dz <= mData.mMaxMeleeRange * mData.mMaxMeleeRange) {
      const CVector2f delta = CVector2f(dx, dy);
      const CVector2f forward = GetTransform().GetForward().ToVec2f();
      if (CVector2f::GetAngleDiff(forward, delta) < 31.5f * (M_PIF / 180.f)) {
        return true;
      }
    }
  }
  return false;
}

bool CElitePirate::ShouldShockwave(CStateManager& mgr, const CTriggerData& data) const {
  if (mAttackTimer <= 0.f) {
    for (uint i = 0; i < mgr.GetNumPlayers(); ++i) {
      CPlayer* player = mgr.GetPlayer(i);
      if (GetCurrentAreaId() == player->GetCurrentAreaId()) {
        const CVector3f delta = player->GetAimPosition(mgr, 0.f) - GetTranslation();
        const float maxRange = mData.mMaxShockwaveRange;
        const float distanceSq = delta.MagSquared();
        if (distanceSq <= maxRange * maxRange) {
          const CPlayer::EPlayerMorphBallState morphState =
              player->GetSpawnedMorphballState() == CPlayer::kMS_Unmorphed
                  ? player->GetMorphballTransitionState()
                  : CPlayer::kMS_Unmorphed;
          if (morphState == CPlayer::kMS_Morphed) {
            return true;
          }
          const float minRange = mData.mMinShockwaveRange;
          if (distanceSq >= minRange * minRange) {
            return fabs(delta.GetZ()) < 3.f;
          }
        }
      }
    }
  }
  return false;
}

bool CElitePirate::ShotAt(CStateManager& mgr, const CTriggerData& data) const { return mShotAt; }

bool CElitePirate::InPosition(CStateManager& mgr, const CTriggerData& data) const {
  const CVector3f delta = mTargetDestPos - GetTranslation();
  return CVector3f::Dot(delta, delta) < 25.f;
}

bool CElitePirate::TooClose(CStateManager& mgr, const CTriggerData& data) const {
  const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
  if (target != nullptr) {
    const CVector3f delta = GetTranslation() - target->GetTranslation();
    return CVector3f::Dot(delta, delta) < 0.5f * (mData.mMaxMeleeRange * mData.mMaxMeleeRange);
  }
  return false;
}

bool CElitePirate::TargetNotOnMesh(CStateManager& mgr, const CTriggerData& data) const {
  if (FindHintType(mgr) == CScriptAIHint::kHT_ElitePirateNoAttack) {
    return false;
  }
  if (FindHintType(mgr) == CScriptAIHint::kHT_GrenadeLauncherRaisedAim) {
    return true;
  }
  return mPathFindSearch.NearlyOnPath(mgr.GetPlayer(0)->GetTranslation(), 4.f) !=
         CPathFindSearch::kR_Success;
}

bool CElitePirate::TargetUnreachable(CStateManager& mgr, const CTriggerData& data) const {
  const int hint = FindHintType(mgr);
  if (hint == CScriptAIHint::kHT_GrenadeLauncherRaisedAim ||
      hint == CScriptAIHint::kHT_ElitePirateNoAttack) {
    return true;
  }
  return mUnreachableTime > 1.f;
}

bool CElitePirate::PoweredDown(CStateManager& mgr, const CTriggerData& data) const {
  return mPoweredLocomotionType != pas::kLT_Relaxed;
}

bool CElitePirate::PoweredUp(CStateManager& mgr, const CTriggerData& data) const {
  return mPoweredUp;
}

bool CElitePirate::ReadyToCharge(CStateManager& mgr, const CTriggerData& data) const {
  if (FindHintType(mgr) == CScriptAIHint::kHT_ElitePirateNoAttack &&
      !ShieldKilled(mgr, CTriggerData(0.f))) {
    return false;
  }
  const float hp = GetHealthInfo()->GetHP();
  const float resistance = GetHealthInfo()->GetKnockBackResistance();
  return mShield.mShieldUpTime + (1.7f * (resistance / hp) + 0.3f) < mElapsedTime;
}

bool CElitePirate::Alerted(CStateManager& mgr, const CTriggerData& data) const {
  return mAlertPending;
}

bool CElitePirate::ReturnedToPatrol(CStateManager& mgr, const CTriggerData& data) const {
  return mReturnedToPatrol;
}

void CElitePirate::Wait(CStateManager& mgr, EStateMsg msg, float dt) {
  UpdateEnergyAbsorb(mgr, msg);
  if (msg == kStateMsg_Activate) {
    mBodyController->CommandMgr().ClearLocomotionCmds();
    mBodyController->SetLocomotionType(pas::kLT_Relaxed);
  }
}

void CElitePirate::PowerDown(CStateManager& mgr, EStateMsg msg, float dt) {
  SetAttackState(kEA_PowerDown, msg);
  UpdateEnergyAbsorb(mgr, msg);
  mBodyController->SetLocomotionType(mPoweredLocomotionType);
  mKnockBackController.EnableAnimReaction(CKnockBackMgr::kAR_Flinch, false);
}

void CElitePirate::PowerUp(CStateManager& mgr, EStateMsg msg, float dt) {
  SetAttackState(kEA_PowerUp, msg);
  UpdateEnergyAbsorb(mgr, msg);
  mBodyController->SetLocomotionType(pas::kLT_Relaxed);
  if (msg == kStateMsg_Activate) {
    mPowerUpPos = GetTranslation();
    SendScriptMsgs(kSS_Attack, mgr, kInvalidUniqueId, kSM_None);
  } else if (msg == kStateMsg_Deactivate) {
    mKnockBackController.EnableAnimReaction(CKnockBackMgr::kAR_Flinch, true);
    ActivateGrenadeLauncher(mgr, true);
  }
}

void CElitePirate::PursuePosition(CStateManager& mgr, EStateMsg msg, const CVector3f& position,
                                  float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    UpdatePathDestination(mgr, position, dt);
    mUnreachableTime = 0.f;
    break;
  case kStateMsg_Update: {
    CVector3f offset = position - mPathDestination;
    offset.SetZ(0.f);
    if (offset.MagSquared() > 16.f) {
      UpdatePathDestination(mgr, position, dt);
    }
    const rstl::reserved_vector< CVector3f, 16 >& waypoints = mPathFindSearch.GetWaypoints();
    if (waypoints.size() > 0) {
      CVector3f toEnd = GetTranslation() - waypoints[waypoints.size() - 1];
      toEnd.SetZ(0.f);
      if (toEnd.Magnitude() < 4.f && !IsPlayerOnPath(mgr)) {
        mUnreachableTime += dt;
        return;
      }
    }
    mUnreachableTime = 0.f;
    CVector3f move;
    if (!PathShagged(mgr, CTriggerData(0.f)) && !mPathFindSearch.IsOver()) {
      mPathFindNavigation.PathFind(mgr, msg, dt, *this);
      move = mBodyController->CommandMgr().GetMoveVector();
    } else {
      move = mSteeringBehaviors.Arrival(*this, position, 7.f);
    }
    mBodyController->CommandMgr().DeliverCmd(CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
    break;
  }
  }
}

void CElitePirate::Pursue(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mBodyController->SetLocomotionType(pas::kLT_Relaxed);
    mAlert = false;
    mPursueStartTime = mElapsedTime;
    mKnockBackController.EnableAnimReaction(CKnockBackMgr::kAR_Flinch, true);
    break;
  case kStateMsg_Update: {
    CVector3f position = mPathDestination;
    const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
    if (target != nullptr) {
      position = target->GetTranslation();
    }
    PursuePosition(mgr, msg, position, dt);
    break;
  }
  }
}

void CElitePirate::Taunt(CStateManager& mgr, EStateMsg msg, float dt) {
  DeliverCommand(msg, pas::kAS_Taunt, CBCTauntCmd(mTauntType));
  if (msg == kStateMsg_Activate) {
    mAlertDone = true;
    mBodyController->SetLocomotionType(pas::kLT_Relaxed);
  } else if (msg == kStateMsg_Deactivate) {
    const pas::ETauntType tauntCycle[4] = {pas::kTT_Zero, pas::kTT_Six, pas::kTT_One,
                                           pas::kTT_Zero};
    for (int i = 0; i < 3; ++i) {
      if (mTauntType == tauntCycle[i]) {
        mTauntType = tauntCycle[i + 1];
        break;
      }
    }
    mAngryCount = mgr.Random()->Range(1, 3);
    mAngryAttackOver = false;
    mHintHandled = false;
  }
}

void CElitePirate::Stunned(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    mBodyController->SetLocomotionType(pas::kLT_Relaxed);
    mShield.mFlash = 0.f;
  }
  DeliverCommand(msg, pas::kAS_Taunt, CBCTauntCmd(pas::kTT_Two));
}

void CElitePirate::TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mBodyController->SetLocomotionType(pas::kLT_Relaxed);
    if (HasPatrolPath(mgr, CTriggerData(0.f))) {
      SetDestPos(mWaypointNavigation.GetDestinationPosition());
    } else {
      SetDestPos(mLatestLeashPosition);
    }
    mTargetDestPos = mDestPos;
    if (GetSearchPath() != nullptr) {
      PathFind(mgr, msg, dt);
    }
    SetShotAt(false);
    mReturnedToPatrol = false;
    mCurPlayerLeashTime = 0.f;
    break;
  case kStateMsg_Update:
    if (!PathShagged(mgr, CTriggerData(0.f))) {
      if (mPathFindSearch.IsOver()) {
        mReturnedToPatrol = true;
      } else {
        PathFind(mgr, msg, dt);
      }
    } else {
      mBodyController->CommandMgr().DeliverCmd(CBCLocomotionCmd(
          mSteeringBehaviors.Arrival(*this, mTargetDestPos, 25.f), CVector3f::Zero(), 1.f));
      mReturnedToPatrol = true;
    }
    break;
  case kStateMsg_Deactivate:
    mAlert = false;
    break;
  }
}

void CElitePirate::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mBodyController->SetLocomotionType(pas::kLT_Relaxed);
    mHitByPlayerProjectile = false;
    break;
  default:
    break;
  }
  CPatterned::Patrol(mgr, msg, dt);
}

void CElitePirate::Turn(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
    if (target == nullptr) {
      return;
    }
    const CVector3f delta = target->GetTranslation() - GetTranslation();
    if (!delta.CanBeNormalized()) {
      return;
    }
    mTurnDirection = delta.AsNormalized();
  }
  if (ShouldAlert(mgr, CTriggerData(0.f))) {
    UpdateEnergyAbsorb(mgr, msg);
  }
  DeliverCommand(msg, pas::kAS_Turn,
                 CBCLocomotionCmd(CVector3f::Zero(), mTurnDirection.AsNormalized(), 1.f));
}

void CElitePirate::SelectTarget(CStateManager& mgr, float dt) {
  mTargetId = CScriptTeamAiMgr::ChoosePlayer(mgr, *this);
}

void CElitePirate::PickAttackType(CStateManager& mgr, float dt) {
  if (mgr.IsRandomAvailable()) {
    if (FindHintType(mgr) == CScriptAIHint::kHT_GrenadeLauncherRaisedAim) {
      mAttackType = kRM_Spread;
    } else if (mgr.Random()->Range(0.f, 1.f) < 0.8f) {
      if (mDefaultAttackType == kRM_Homing) {
        mAttackType = kRM_Spread;
      } else {
        mAttackType = kRM_Homing;
      }
    } else {
      mAttackType = mDefaultAttackType;
    }
  }
}

void CElitePirate::Alert(CStateManager& mgr, EStateMsg msg, float dt) {
  SetAttackState(kEA_Alert, msg);
  UpdateEnergyAbsorb(mgr, msg);
  DeliverCommand(msg, pas::kAS_Taunt, CBCTauntCmd(mAlertTauntType));
  if (msg == kStateMsg_Activate) {
    mAlertDone = true;
    mPathDestination = GetTranslation();
  } else if (msg == kStateMsg_Deactivate) {
    SetShotAt(false);
    ActivateGrenadeLauncher(mgr, true);
  }
}

void CElitePirate::InvulnAlert(CStateManager& mgr, EStateMsg msg, float dt) {
  UpdateEnergyAbsorb(mgr, msg);
  Alert(mgr, msg, dt);
}

CProjectileInfo* CElitePirate::ProjectileInfo() { return &mProjectileInfo; }

void CElitePirate::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    mRocketsFired = 0;
    mRocketCount = mgr.Random()->Range(mData.mMinRockets, mData.mMaxRockets);
    mBreakProjectileAttack = false;
    mBodyController->SetLocomotionType(pas::kLT_Relaxed);
  }
  SetAttackState(kEA_Projectile, msg);
  if (msg == kStateMsg_Deactivate) {
    UpdateAttackTimeLeft(mgr);
  }
  if (mRocketsFired >= mRocketCount) {
    mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
  } else {
    const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
    if (target != nullptr) {
      mPlayerTargetPos = target->GetTranslation();
      mBodyController->CommandMgr().SetTargetVector(target->GetTranslation() - GetTranslation());
    }
    DeliverCommand(msg, pas::kAS_LoopAttack, CBCLoopAttackCmd(pas::kLAT_Zero));
  }
}

bool CElitePirate::BreakProjectileAttack(CStateManager& mgr, const CTriggerData& data) const {
  return mBreakProjectileAttack;
}

bool CElitePirate::CanShockwave(CStateManager& mgr, const CTriggerData& data) const {
  return FindHintType(mgr) != CScriptAIHint::kHT_GrenadeLauncherRaisedAim;
}

bool CElitePirate::ClearLineOfSight(CStateManager& mgr, const CTriggerData& data) const {
  const CElitePirateGrenadeLauncher* launcher =
      static_cast< const CElitePirateGrenadeLauncher* >(mgr.GetObjectById(mLauncherId));
  if (launcher == nullptr) {
    return false;
  }
  const CVector3f launcherPos = launcher->GetTurretTransform().GetTranslation();
  const CVector3f playerPos = mgr.GetPlayer(0)->GetAimPosition(mgr, 0.f);
  const CVector3f delta = playerPos - launcherPos;
  if (delta.CanBeNormalized() != true) {
    return false;
  }
  const CVector3f flat(delta.GetX(), delta.GetY(), 0.f);
  if (flat.MagSquared() > 0.5f * (mData.mMaxShockwaveRange * mData.mMaxShockwaveRange)) {
    return false;
  }
  const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
      CMaterialList(sClearSightSolidMaterial), CMaterialList(sClearSightFloorMaterial));
  return CGameCollision::RayStaticLineOfSightTest(mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()),
                                                  launcherPos, delta.AsNormalized(),
                                                  delta.Magnitude(), filter);
}

bool CElitePirate::PickedSpreadShot(CStateManager& mgr, const CTriggerData& data) const {
  return mAttackType == kRM_Spread;
}

void CElitePirate::SpreadShot(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    mRocketsFired = 0;
    mRocketCount = 4;
    mBreakProjectileAttack = false;
    mBodyController->SetLocomotionType(pas::kLT_Relaxed);
  }
  SetAttackState(kEA_SpreadShot, msg);
  if (msg == kStateMsg_Deactivate) {
    UpdateAttackTimeLeft(mgr);
  }
  const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
  if (target != nullptr) {
    mPlayerTargetPos = target->GetTranslation();
    mBodyController->CommandMgr().SetTargetVector(target->GetTranslation() - GetTranslation());
  }
  DeliverCommand(msg, pas::kAS_ProjectileAttack,
                 CBCProjectileAttackCmd(pas::kS_Nine, mPlayerTargetPos, false));
}

void CElitePirate::Shockwave(CStateManager& mgr, EStateMsg msg, float dt) {
  SetAttackState(kEA_Shockwave, msg);
  if (msg == kStateMsg_Activate) {
    const float totalWeight =
        mData.mShockwaveWeight0 +
        (mData.mShockwaveWeight1 + (mData.mShockwaveWeight2 + mData.mShockwaveWeight5));
    const float roll = mgr.Random()->Range(0.f, totalWeight);
    if (roll < mData.mShockwaveWeight0) {
      mShockwaveSeverity = pas::kS_Zero;
    } else if (roll < mData.mShockwaveWeight0 + mData.mShockwaveWeight1) {
      mShockwaveSeverity = pas::kS_One;
    } else if (roll <
               mData.mShockwaveWeight1 + (mData.mShockwaveWeight0 + mData.mShockwaveWeight1)) {
      mShockwaveSeverity = pas::kS_Two;
    } else {
      mShockwaveSeverity = pas::kS_Five;
    }
  } else if (msg == kStateMsg_Deactivate) {
    UpdateAttackTimeLeft(mgr);
    mLastShockwaveSeverity = mShockwaveSeverity;
  }
  const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
  if (target == nullptr) {
    mAnimationState.SetState(CAnimationState::kAS_Over);
  } else {
    mBodyController->CommandMgr().SetTargetVector(target->GetTranslation() - GetTranslation());
    DeliverCommand(msg, pas::kAS_ProjectileAttack,
                   CBCProjectileAttackCmd(mShockwaveSeverity, target->GetTranslation(), false));
  }
}

void CElitePirate::MeleeAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  SetAttackState(kEA_Melee, msg);
  DeliverCommand(msg, pas::kAS_MeleeAttack, CBCMeleeAttackCmd(mMeleeSeverity));
  if (msg == kStateMsg_Activate) {
    mBodyController->SetLocomotionType(pas::kLT_Relaxed);
    mKnockBackController.EnableAnimReaction(CKnockBackMgr::kAR_Flinch, true);
    if (mgr.Random()->Float() < 0.7f) {
      mNextMeleeSeverity = mMeleeSeverity == pas::kS_One ? pas::kS_Seven : pas::kS_One;
    }
  } else if (msg == kStateMsg_Deactivate) {
    mAttackingRightClaw = false;
    mAttackingLeftClaw = false;
    mMeleeSeverity = mNextMeleeSeverity;
    mAngryCount = 0;
    ExtendTouchBounds(mgr, mRightClawIds, CVector3f::Zero());
    ExtendTouchBounds(mgr, mLeftClawIds, CVector3f::Zero());
  }
}

void CElitePirate::PushAttackHistory(rstl::reserved_vector< EAction, 3 >& history, EAction attack) {
  if (history.size() == 3) {
    history.erase(history.begin());
  }
  history.push_back(attack);
}

void CElitePirate::SetAttackState(EAction state, EStateMsg msg) {
  if (msg == kStateMsg_Deactivate) {
    PushAttackHistory(mAttackHistory, mAttackState);
    mAttackState = kEA_None;
  } else {
    mAttackState = state;
  }
}

void CElitePirate::ShieldUp(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    mBodyController->SetLocomotionType(pas::kLT_Crouch);
    mShieldCollisionMgr->SetActive(mgr, true);
    mKnockBackController.EnableAnimReaction(CKnockBackMgr::kAR_Flinch, false);
    mBodyController->CommandMgr().ClearLocomotionCmds();
    mShield.mShieldUpTime = mElapsedTime;
    switch (mShield.mType) {
    case kST_Dark:
      mShield.mType = kST_Light;
      break;
    case kST_Light:
      mShield.mType = kST_Dark;
      break;
    case kST_None:
      if (mgr.Random()->Range(0.f, 1.f) < 0.5f) {
        mShield.mType = kST_Light;
      } else {
        mShield.mType = kST_Dark;
      }
      break;
    }
    CAssetId shieldEffect;
    ushort shieldSound;
    if (mShield.mType == kST_Light) {
      shieldEffect = mData.mLightShield;
      shieldSound = mData.mLightShieldSound;
    } else {
      shieldEffect = mData.mDarkShield;
      shieldSound = mData.mDarkShieldSound;
    }
    mShield.mEffect = rstl::auto_ptr< CElementGen >(rs_new CElementGen(
        TToken< CGenDescription >(gpSimplePool->GetObj(SObjectTag('PART', shieldEffect)))));
    mShield.mEffect->SetParticleEmission(true);
    mShield.mDamageAbsorbed = 0.f;
    mShield.mSfx = CSfxManager::AddEmitter(shieldSound, GetTranslation(),
                                           GetCurrentAreaId().Value(), false, true);
    SetupHealthInfo(mgr, true);
  } else if (msg == kStateMsg_Update) {
    RotateToPoint(mgr.GetPlayer(0)->GetTranslation(), dt, 100.f * (M_PIF / 180.f));
  }
}

const char* CElitePirate::GetShieldModelName() const {
  return mShield.mType == kST_Dark ? "BodyShield_dark" : "BodyShield_light";
}

void CElitePirate::Shielding(CStateManager& mgr, EStateMsg msg, float dt) {
  if (!ShieldKilled(mgr, CTriggerData(0.f))) {
    CVector3f position = mPathDestination;
    const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
    if (target != nullptr) {
      position = target->GetTranslation();
    }
    PursuePosition(mgr, msg, position, dt);
  }
  switch (msg) {
  case kStateMsg_Activate:
    mShield.mActive = true;
    mAngryCount = 0;
    break;
  case kStateMsg_Update:
    if (mShotAt == true) {
      mShotAtTimer -= dt;
      if (mShotAtTimer <= 0.f) {
        mShotAt = false;
      }
    }
    CSfxManager::UpdateEmitter(mShield.mSfx, mLeftClawPos, CVector3f::Zero(), 127);
    break;
  case kStateMsg_Deactivate:
    mShieldCollisionMgr->SetActive(mgr, false);
    SetupHealthInfo(mgr, false);
    mShield.mActive = false;
    mShield.mEffect = rstl::auto_ptr< CElementGen >();
    mShield.mPopPending = true;
    CSfxManager::RemoveEmitter(mShield.mSfx);
    mShield.mSfx = CSfxHandle();
    break;
  }
}

void CElitePirate::PopShield(CStateManager& mgr) {
  const TLockedToken< CGenDescription >* effect = nullptr;
  if (mShield.mType == kST_Light) {
    if (!mShield.mLightPopEffect && mData.mLightShieldPop != kInvalidAssetId) {
      mShield.mLightPopEffect =
          rstl::optional_object< TLockedToken< CGenDescription > >(TLockedToken< CGenDescription >(
              gpSimplePool->GetObj(SObjectTag('PART', mData.mLightShieldPop))));
    }
    if (mShield.mLightPopEffect) {
      effect = &*mShield.mLightPopEffect;
    }
  } else {
    if (!mShield.mDarkPopEffect && mData.mDarkShieldPop != kInvalidAssetId) {
      mShield.mDarkPopEffect =
          rstl::optional_object< TLockedToken< CGenDescription > >(TLockedToken< CGenDescription >(
              gpSimplePool->GetObj(SObjectTag('PART', mData.mDarkShieldPop))));
    }
    if (mShield.mDarkPopEffect) {
      effect = &*mShield.mDarkPopEffect;
    }
  }
  if (effect != nullptr) {
    CTransform4f xf = GetTransform();
    xf.SetTranslation(mLeftClawPos);
    CExplosion* explosion = rs_new CExplosion(
        *effect, mgr.AllocateUniqueId(),
        CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true, kInvalidEditorId),
        rstl::string_l("IngSmasher shield pop"), xf, 0, CVector3f(1.f, 1.f, 1.f), CColor::White(),
        -1);
    mgr.AddObject(explosion);
  }
}

void CElitePirate::SetupCollisionManager(CStateManager& mgr) {
  rstl::vector< CJointCollisionDescription > joints;
  joints.reserve(14);
  AddCollisionList(skLeftArmJointList, 3, joints);
  AddCollisionList(skRightArmJointList, 3, joints);
  AddSphereCollisionList(skSphereJointList, 8, joints);
  mBodyCollisionMgr =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), joints, true);
  mBodyCollisionMgr->SetActive(mgr, GetActive());
  mRightClawIds.clear();
  mLeftClawIds.clear();
  const CSegId clawSegId = GetAnimationData()->GetLocatorSegId(rstl::string_l(skpLeftClawLCTR));
  const CJointCollisionDescription shield = CJointCollisionDescription::OBBCollision(
      clawSegId, skLocalShieldBounds, CVector3f::Zero(), rstl::string_l("Shield"), 5.f);
  joints.clear();
  joints.push_back(shield);
  mShieldCollisionMgr =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), joints, false);
  SetupCollisionActorInfo(mgr);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      CMaterialList(sCollisionInclude),
      CMaterialList(sCollisionExclude0, sCollisionExclude1, sCollisionExclude2, sCollisionExclude3,
                    sCollisionExclude4)));
  AddMaterial(kMT_ProjectilePassthrough, mgr);
  mEnergyAttractorId = mShieldCollisionMgr->GetCollisionDescFromIndex(0).GetCollisionActorId();
  if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(mEnergyAttractorId))) {
    actor->SetResponseType(kWCR_None);
  }
  SetupHealthInfo(mgr, false);
  mShieldCollisionMgr->AddMaterialList(mgr,
                                       CMaterialList(sAIJointMaterial, sCameraPassthroughMaterial));
}

void CElitePirate::SetupCollisionActorInfo(CStateManager& mgr) {
  for (uint i = 0; i < mBodyCollisionMgr->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& desc = mBodyCollisionMgr->GetCollisionDescFromIndex(i);
    const TUniqueId uid = desc.GetCollisionActorId();
    if (TCastToPtr< CCollisionActor >(mgr.ObjectById(uid)) != nullptr) {
      if (desc.GetName() == rstl::string_l(skpHeadLCTR)) {
        mHeadId = uid;
      } else if (IsArmClawCollider(desc.GetName(), skpRightClawLCTR, skRightArmJointList, 3)) {
        mRightClawIds.push_back(uid);
      } else if (IsArmClawCollider(desc.GetName(), skpLeftClawLCTR, skLeftArmJointList, 3)) {
        mLeftClawIds.push_back(uid);
      }
    }
  }
}

void CElitePirate::AddCollisionList(const SJointInfo* joints, int count,
                                    rstl::vector< CJointCollisionDescription >& list) {
  const CAnimData& animData = *GetAnimationData();
  for (int i = 0; i < count; ++i) {
    const CSegId from = animData.GetLocatorSegId(rstl::string_l(joints[i].mFrom));
    const CSegId to = animData.GetLocatorSegId(rstl::string_l(joints[i].mTo));
    if (from.val() != 0xff && to.val() != 0xff) {
      const CJointCollisionDescription desc = CJointCollisionDescription::SphereSubdivideCollision(
          from, to, joints[i].mRadius, joints[i].mSeparation,
          CJointCollisionDescription::kOT_BetweenJoints, rstl::string_l(joints[i].mFrom), 5.f);
      list.push_back(desc);
    }
  }
}

void CElitePirate::AddSphereCollisionList(const SSphereJointInfo* joints, int count,
                                          rstl::vector< CJointCollisionDescription >& list) {
  const CAnimData& animData = *GetAnimationData();
  for (int i = 0; i < count; ++i) {
    const CSegId id = animData.GetLocatorSegId(rstl::string_l(joints[i].mName));
    if (id.val() != 0xff) {
      const CJointCollisionDescription desc = CJointCollisionDescription::SphereCollision(
          id, CVector3f::Zero(), joints[i].mRadius, rstl::string_l(joints[i].mName), 5.f);
      list.push_back(desc);
    }
  }
}

bool CElitePirate::IsArmClawCollider(const rstl::string& name, const char* locator,
                                     const SJointInfo* joints, int count) const {
  if (name == rstl::string_l(locator)) {
    return true;
  }
  for (int i = 0; i < count; ++i) {
    if (name == rstl::string_l(joints[i].mFrom)) {
      return true;
    }
  }
  return false;
}

bool CElitePirate::IsArmClawCollider(TUniqueId uid,
                                     const rstl::reserved_vector< TUniqueId, 7 >& ids) const {
  for (AUTO(it, ids.begin()); it != ids.end(); ++it) {
    if (*it == uid) {
      return true;
    }
  }
  return false;
}

void CElitePirate::ExtendTouchBounds(CStateManager& mgr,
                                     const rstl::reserved_vector< TUniqueId, 7 >& ids,
                                     const CVector3f& bounds) const {
  for (AUTO(it, ids.begin()); it != ids.end(); ++it) {
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(*it))) {
      actor->SetExtendedTouchBounds(bounds);
    }
  }
}

void CElitePirate::SetupPathFindSearch() {
  const float radius = 1.5f * GetModelData()->GetScale().GetY();
  const float height = 6.f * GetModelData()->GetScale().GetZ();
  const CAABox box(CVector3f(-radius, -radius, 0.f), CVector3f(radius, radius, height));
  SetBoundingBox(box);
  mCollisionAabb.SetBox(box);
  mPathFindSearch.SetCharacterRadius(radius);
  mPathFindSearch.SetCharacterHeight(height);
}

bool CElitePirate::IsShieldActive() const {
  return mBodyController->GetLocomotionType() == pas::kLT_Crouch;
}

void CElitePirate::SetupHealthInfo(CStateManager& mgr, bool shielded) {
  const CSegId clawSegId = GetAnimationData()->GetLocatorSegId(rstl::string_l(skpLeftClawLCTR));
  const CHealthInfo* health = GetHealthInfo();
  mHp = health->GetHP();
  for (uint i = 0; i < mBodyCollisionMgr->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& desc = mBodyCollisionMgr->GetCollisionDescFromIndex(i);
    const TUniqueId uid = desc.GetCollisionActorId();
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(uid))) {
      *actor->HealthInfo() = *health;
      if (shielded == true) {
        actor->SetDamageVulnerability(CDamageVulnerability::ReflectVulnerabilty());
      } else {
        actor->SetDamageVulnerability(mVulnerability.MakeIgnoreRadius());
      }
      if (desc.GetPivotId() == clawSegId) {
        if (shielded == true) {
          actor->AddMaterial(kMT_ProjectilePassthrough, mgr);
        } else {
          actor->RemoveMaterial(kMT_ProjectilePassthrough, mgr);
        }
      }
    }
  }
  if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(mEnergyAttractorId))) {
    if (!shielded) {
      actor->SetDamageVulnerability(mVulnerability.MakeIgnoreRadius());
    } else {
      CDamageVulnerability vulnerability(CDamageVulnerability::ReflectVulnerabilty());
      const int weaponTypes[3] = {kWT_Dark, kWT_Light, kWT_Annihilator};
      for (int i = 0; i < 3; ++i) {
        const int weapon = weaponTypes[i];
        if (mShield.mType == kST_Dark && weapon == kWT_Dark) {
          continue;
        }
        if (mShield.mType == kST_Light && weapon == kWT_Light) {
          continue;
        }
        vulnerability.SetVulnerability(weapon, CWeaponTypeVulnerability::Normal());
        vulnerability.SetChargedVulnerability(weapon, CWeaponTypeVulnerability::Normal());
        vulnerability.SetComboVulnerability(weapon, CWeaponTypeVulnerability::Normal());
      }
      actor->SetDamageVulnerability(vulnerability);
    }
  }
}

void CElitePirate::UpdateAttackTimer(float dt) {
  if (mAttackTimer > 0.f) {
    mAttackTimer -= dt;
  }
}

void CElitePirate::CreateGrenadeLauncher(CStateManager& mgr, TUniqueId uid) {
  const CAnimationParameters& params =
      IsIngPossessed() ? mData.mIngLauncherAnimation : mData.mLauncherAnimation;
  if (params.GetACSFile() != kInvalidAssetId) {
    CModelData modelData(CAnimRes(params.GetACSFile(), params.GetCharacter(),
                                  GetModelData()->GetScale(), params.GetInitialAnimation(), true));
    CElitePirateGrenadeLauncher* launcher = rs_new CElitePirateGrenadeLauncher(
        uid, rstl::string_l("Rocket Launcher"),
        CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true), GetTransform(),
        modelData, mData.mLauncherActorParams, GetUniqueId(), 0.f);
    if (launcher != nullptr) {
      mgr.AddObject(launcher);
    }
    mLauncherAlive = IsIngPossessed();
  }
}

void CElitePirate::DeleteGrenadeLauncher(CStateManager& mgr) {
  mgr.DeleteObjectRequest(mLauncherId);
  mLauncherId = kInvalidUniqueId;
}

void CElitePirate::ActivateGrenadeLauncher(CStateManager& mgr, bool activate) {
  ActivateGrenadeLauncherById(mgr, activate, mLauncherId);
}

void CElitePirate::ActivateGrenadeLauncherById(CStateManager& mgr, bool activate,
                                               TUniqueId uid) const {
  if (uid != kInvalidUniqueId) {
    if (CEntity* entity = mgr.ObjectById(uid)) {
      const EScriptObjectMessage msg = activate ? kSM_Start : kSM_Stop;
      mgr.SendScriptMsg(entity, GetUniqueId(), msg);
    }
  }
}

void CElitePirate::UpdateGrenadeLauncher(CStateManager& mgr, TUniqueId& uid,
                                         const rstl::string& locator) const {
  if (uid != kInvalidUniqueId) {
    if (CActor* actor = static_cast< CActor* >(mgr.ObjectById(uid))) {
      actor->SetTransform(CTransform4f(GetLctrTransform(locator)));
    } else {
      uid = kInvalidUniqueId;
    }
  }
}

CVector3f CElitePirate::GetGrenadeLaunchPos(const CActor& actor) const {
  const CTransform4f locator = actor.GetLocatorTransform(rstl::string_l(skpGrenadeLauncherLCTR));
  return actor.GetTranslation() + actor.GetTransform().Rotate(locator.GetTranslation());
}

void CElitePirate::UpdateBreadCrumbTrail() {
  const CVector3f position = GetTranslation();
  if (mPathFindSearch.OnPath(position) == CPathFindSearch::kR_Success) {
    mPositionHistory.Clear();
  }
  mPositionHistory.AddValue(position);
}

void CElitePirate::UpdatePathDestination(CStateManager& mgr, const CVector3f& position, float dt) {
  CVector3f destination = position;
  mPathFindSearch.SetAvoidanceFilter(4);
  UnmarkPathRegion(mgr);
  mPathFindNavigation.SetDestination(destination);
  mPathFindNavigation.PathFind(mgr, kStateMsg_Activate, dt, *this);
  if (PathShagged(mgr, CTriggerData(0.f)) == true) {
    const float heightDelta = fabs(position.GetZ() - GetTranslation().GetZ());
    if (heightDelta > 2.f) {
      destination.SetZ(GetTranslation().GetZ());
      mPathFindNavigation.SetDestination(destination);
      mPathFindNavigation.PathFind(mgr, kStateMsg_Activate, dt, *this);
    }
    if (PathShagged(mgr, CTriggerData(0.f)) == true) {
      destination = 0.5f * (position + mPathDestination);
      mPathFindNavigation.SetDestination(destination);
      mPathFindNavigation.PathFind(mgr, kStateMsg_Activate, dt, *this);
    }
    if (PathShagged(mgr, CTriggerData(0.f)) == true) {
      destination = mPathDestination;
      mPathFindNavigation.SetDestination(destination);
      mPathFindNavigation.PathFind(mgr, kStateMsg_Activate, dt, *this);
    }
  }
  mPathDestination = destination;
  MarkPathRegion(mgr);
}

void CElitePirate::UpdateAttackTimeLeft(CStateManager& mgr) {
  if (mgr.IsRandomAvailable() && mgr.Random()->Float() > mData.mRepeatedAttackChance) {
    mAttackTimer = GetAverageAttackTime() + mgr.Random()->Float() * mAttackTimeVariation;
  }
}

void CElitePirate::ProcessStompGround(CStateManager& mgr) {
  const bool bigStomp = mAttackState == kEA_Shockwave;
  for (uint i = 0; i < mgr.GetNumPlayers(); ++i) {
    CPlayer* player = mgr.GetPlayer(i);
    const float distance = (GetTranslation() - player->GetTranslation()).Magnitude();
    float magnitude = bigStomp ? 1.f : 0.25f;
    magnitude *= GetModelData()->GetScale().Magnitude();
    magnitude -= 0.05f * distance;
    if (magnitude > 0.f && player->GetSurfaceRestraint() != CPlayer::kSR_Air &&
        player->GetFluidCount() == 0) {
      const CPlayer::EPlayerMorphBallState morphState =
          player->GetSpawnedMorphballState() == CPlayer::kMS_Unmorphed
              ? player->GetMorphballTransitionState()
              : CPlayer::kMS_Unmorphed;
      if (morphState != CPlayer::kMS_Morphed) {
        const CCameraManager* cameraManager = mgr.GetCameraManager(i);
        if (cameraManager->GetCurrentCameraId(true) ==
            cameraManager->GetFirstPersonCamera()->GetUniqueId()) {
          mgr.RumbleManager(mgr.MaskUIdNumPlayers(player->GetUniqueId()))
              ->Rumble(mgr, kRFX_CameraShake, 1.f, kRP_Two);
        }
      } else {
        const CVector3f impulse = (bigStomp ? 20.f : 10.f) * CVector3f::Up();
        player->Stop();
        player->ApplyImpulseWR(player->GetMass() * impulse, CAxisAngle::Identity());
        player->SetMoveState(NPlayer::kMS_ApplyJump, mgr);
      }
    }
  }
}

void CElitePirate::PushPlayer(CStateManager& mgr, float scale, float verticalSpeed) {
  CPlayer* player = mgr.GetPlayer(0);
  CVector3f direction = player->GetTranslation() - GetTranslation();
  direction.SetZ(0.f);
  direction.Normalize();
  direction *= scale;
  direction.SetZ(verticalSpeed);
  player->Stop();
  player->ApplyImpulseWR(player->GetMass() * direction, CAxisAngle::Identity());
  player->SetMoveState(NPlayer::kMS_ApplyJump, mgr);
}

void CElitePirate::SetShotAt(bool shotAt) { mShotAt = shotAt; }

CShockWaveInfo CElitePirate::GetShockWaveInfo() const {
  if (mShockwaveSeverity == 5) {
    return CShockWaveInfo(mData.mDoubleShockWave);
  }
  return CShockWaveInfo(mData.mSingleShockWave);
}

bool CElitePirate::IsPlayerOnPath(CStateManager& mgr) const {
  return !mPathFindSearch.OnPath(mgr.GetPlayer(0)->GetTranslation());
}

bool CElitePirate::CanBeUnPossessed(CStateManager& mgr) const { return false; }

void CElitePirate::AngryAttackBegin(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    mAngryAttackOver = false;
    switch (FindHintType(mgr)) {
    case CScriptAIHint::kHT_ElitePirateShockwave:
      mShockwaveIsNext = true;
      break;
    case CScriptAIHint::kHT_GrenadeLauncherRaisedAim:
      mShockwaveIsNext = false;
      break;
    default:
      if (!mHintHandled) {
        mShockwaveIsNext = mgr.Random()->Range(0.f, 1.f) < 0.7f;
      } else {
        mShockwaveIsNext = mPrevShockwave != true;
      }
      break;
    }
    mHintHandled = true;
    mPrevShockwave = mShockwaveIsNext;
  }
}

void CElitePirate::AngryAttackEnd(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    --mAngryCount;
    mAngryAttackOver = true;
  }
}

bool CElitePirate::AngryAttackOver(CStateManager& mgr, const CTriggerData& data) const {
  return mAngryAttackOver;
}

bool CElitePirate::StillAngry(CStateManager& mgr, const CTriggerData& data) const {
  return mAngryCount > 0;
}

bool CElitePirate::ShockwaveIsNext(CStateManager& mgr, const CTriggerData& data) const {
  return mShockwaveIsNext;
}

bool CElitePirate::IsInStopPursuitHint(CStateManager& mgr) const {
  const CObjectList& list = mgr.GetObjectListById(kOL_AiWaypoint);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    const CScriptAIHint* hint = TCastToConstPtr< CScriptAIHint >(list[i]);
    if (hint != nullptr && hint->GetHintType() == CScriptAIHint::kHT_ElitePirateStopPursuit &&
        hint->GetCurrentAreaId() == GetCurrentAreaId() && hint->GetActive() == true) {
      CVector3f offset = hint->GetTranslation() - GetTranslation();
      offset.SetZ(0.f);
      if (offset.Magnitude() < hint->GetRadius()) {
        return true;
      }
    }
  }
  return false;
}

int CElitePirate::FindHintType(CStateManager& mgr) const {
  const CObjectList& list = mgr.GetObjectListById(kOL_AiWaypoint);
  const CVector3f playerPos = mgr.GetPlayer(0)->GetTranslation();
  float minDistance = FLT_MAX;
  const CScriptAIHint* closest = nullptr;
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    const CScriptAIHint* hint = TCastToConstPtr< CScriptAIHint >(list[i]);
    if (hint != nullptr && hint->GetCurrentAreaId() == GetCurrentAreaId() &&
        hint->GetActive() == true &&
        (hint->GetHintType() == CScriptAIHint::kHT_ElitePirateShockwave ||
         hint->GetHintType() == CScriptAIHint::kHT_ElitePirateNoAttack ||
         hint->GetHintType() == CScriptAIHint::kHT_GrenadeLauncherRaisedAim)) {
      const CVector3f offset = hint->GetTranslation() - playerPos;
      const float distance = offset.Magnitude();
      if (distance < hint->GetRadius()) {
        if (hint->GetHintType() == CScriptAIHint::kHT_ElitePirateNoAttack) {
          return CScriptAIHint::kHT_ElitePirateNoAttack;
        }
        if (distance < minDistance) {
          minDistance = distance;
          closest = hint;
        }
      }
    }
  }
  if (closest != nullptr) {
    return closest->GetHintType();
  }
  return -1;
}

bool CElitePirate::PlayerInNoAttack(CStateManager& mgr, const CTriggerData& data) const {
  return FindHintType(mgr) == CScriptAIHint::kHT_ElitePirateNoAttack;
}

CPFArea* CElitePirate::GetPathArea(CStateManager& mgr) const {
  const TAreaId areaId = GetCurrentAreaId();
  return mgr.GetWorld()->GetAreaAlways(areaId).GetPostConstructed()->mPathArea;
}

void CElitePirate::MarkPathRegion(CStateManager& mgr) {
  const rstl::reserved_vector< CVector3f, 16 >& waypoints = mPathFindSearch.GetWaypoints();
  if (waypoints.size() != 0) {
    const CVector3f lastPoint = waypoints[waypoints.size() - 1];
    const CPFRegion* lastRegion = GetPathArea(mgr)->FindClosestRegion(
        lastPoint, GetSearchPath()->GetRegionFlags(), GetSearchPath()->GetCreatureMask(), 2.f);
    for (int i = waypoints.size() - 1; i > 0; --i) {
      const CVector3f midpoint = 0.5f * (waypoints[i] + waypoints[i - 1]);
      CPFRegion* region = GetPathArea(mgr)->FindClosestRegion(
          midpoint, GetSearchPath()->GetRegionFlags(), GetSearchPath()->GetCreatureMask(), 2.f);
      if (region && region != lastRegion && region->Data()->GetAvoidanceFlags() == 0) {
        region->Data()->SetAvoidanceFlags(region->Data()->GetAvoidanceFlags() | 4);
        mMarkedRegionIndex = region->GetIndex();
        break;
      }
    }
  }
}

void CElitePirate::UnmarkPathRegion(CStateManager& mgr) {
  if (mMarkedRegionIndex == -1 || GetPathArea(mgr) == nullptr) {
    return;
  }
  CPFRegion* region = GetPathArea(mgr)->GetRegionPtr(mMarkedRegionIndex);
  if (region != nullptr) {
    region->Data()->SetAvoidanceFlags(region->Data()->GetAvoidanceFlags() & ~4);
  }
  mMarkedRegionIndex = -1;
}

CEntity* LoadElitePirate(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrElitePirate sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrElitePirate.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  const CElitePirateData data(
      sldrThis.patterned.stateMachine2, sldrThis.patterned.animationInformation.initial_anim,
      LdrToDamageInfo(sldrThis.meleeDamage), sldrThis.maxMeleeRange, sldrThis.minShockwaveRange,
      sldrThis.maxShockwaveRange, sldrThis.minRocketRange, sldrThis.maxRocketRange,
      sldrThis.unknown_0x5236c2b6, sldrThis.unknown_0x01eaab17, sldrThis.tauntInterval,
      sldrThis.darkShield, static_cast< ushort >(sldrThis.darkShieldSound), sldrThis.darkShieldPop,
      sldrThis.lightShield, static_cast< ushort >(sldrThis.lightShieldSound),
      sldrThis.lightShieldPop, sldrThis.tauntVariance, sldrThis.unknown_0x28b39197,
      sldrThis.unknown_0xe27de71b, sldrThis.unknown_0x665e7ace, sldrThis.unknown_0xacd4d06d,
      sldrThis.repeatedAttackChance, sldrThis.energyAttractionForce, sldrThis.alwaysFF,
      static_cast< ushort >(sldrThis.alwaysFF_0x23f5e1ee),
      LdrToActorParameters(sldrThis.rocketLauncherActorInfo),
      LdrToAnimationParameters(sldrThis.rocketLauncherAnimInfo),
      LdrToAnimationParameters(sldrThis.unknown_0x7e6e0d38), sldrThis.rocket,
      LdrToDamageInfo(sldrThis.rocketDamage), sldrThis.unknown_0x624222f8,
      sldrThis.unknown_0x31e43a1c, sldrThis.visorElectricEffect,
      static_cast< ushort >(sldrThis.sound_VisorElectric), sldrThis.singleShockWaveInfo,
      sldrThis.doubleShockWaveInfo, sldrThis.shieldedModel, sldrThis.shieldedSkinRules);
  return rs_new CElitePirate(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                             LdrToEntityInfo(info, sldrThis.editorProperties),
                             LdrToTransform4f(sldrThis.editorProperties), *modelData,
                             LdrToPatternedInfo(sldrThis.patterned, &sldrThis.ingPossessionData),
                             LdrToActorParameters(sldrThis.actorInformation), data);
}

#ifndef MONOLITHIC
static void SetFuncPtrs() {
  static SElitePirate_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &LoadElitePirate;
  SetSElitePirate_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSElitePirate_FuncPtrs(nullptr); }
#endif
