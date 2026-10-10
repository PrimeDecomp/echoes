#include "MetroidPrime/Enemies/CSplinter.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CSafeZoneManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Enemies/CIngSnatchingSwarm.hpp"
#include "MetroidPrime/Enemies/CSplinterAcidSac.hpp"
#include "MetroidPrime/Enemies/CTeamAiRole.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSplinter.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAiJumpPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTeamAiMgr.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Weapons/CImpactVisorEffect.hpp"
#include "REL/REL_Setup.h"
#include "WorldFormat/CMetroidAreaCollider.hpp"

static const char* const skWebAttachLocator = "Web_attach_LCTR";
static const char* const skAcidSackLocator = "AcidSack_LCTR_SDK";
static const char* const skEyeLocator = "Eye_LCTR";
static const char* const skEyesEffect = "Eyes";

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"AlertMessageReceived",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::AlertMessageReceived)},
    {"AttackPointIsLegal",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::AttackPointIsLegal)},
    {"CableShot", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::CableShot)},
    {"CanPounce", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::CanPounce)},
    {"CanSeePlayer",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::CanSeePlayer)},
    {"DropCompleted",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::DropCompleted)},
    {"FacingPlayer",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::FacingPlayer)},
    {"FacingPlayerExactly",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::FacingPlayerExactly)},
    {"FleesFromPlayer",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::FleesFromPlayer)},
    {"GrabbedIt", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::GrabbedIt)},
    {"HasAnyLandPoints",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::HasAnyLandPoints)},
    {"HasValidJumpTarget",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::HasValidJumpTarget)},
    {"HoldingSac", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::HoldingSac)},
    {"InAttackRange",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::InAttackRange)},
    {"InCocoon", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::InCocoon)},
    {"IngSwarmIncoming",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::IngSwarmIncoming)},
    {"IsDarkSplinter",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::IsDarkSplinter)},
    {"IsHeckler", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::IsHeckler)},
    {"InHeckleRange",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::InHeckleRange)},
    {"IsMegaDark", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::IsMegaDark)},
    {"IsMegaLight", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::IsMegaLight)},
    {"IsMelee", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::IsMelee)},
    {"IsOnCable", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::IsOnCable)},
    {"IsOnPad", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::IsOnPad)},
    {"IsWorkerSplinter",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::IsWorkerSplinter)},
    {"Landed", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::Landed)},
    {"MBallMoved", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::MBallMoved)},
    {"MorphballWaiting",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::MorphballWaiting)},
    {"NeedsToSlideOff",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::NeedsToSlideOff)},
    {"PlayerIsInSafeZone",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::PlayerIsInSafeZone)},
    {"PounceTimeOut",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::PounceTimeOut)},
    {"ReachedMBall",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::ReachedMBall)},
    {"ReachedSac", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::ReachedSac)},
    {"ReachedTop", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::ReachedTop)},
    {"SacMissing", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::SacMissing)},
    {"SacPresent", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::SacPresent)},
    {"ShotFromOtherArea",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::ShotFromOtherArea)},
    {"ShouldDrop", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::ShouldDrop)},
    {"ShouldEmergeFromCocoon",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::ShouldEmergeFromCocoon)},
    {"ShouldTaunt", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::ShouldTaunt)},
    {"ShouldUnHide",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::ShouldUnHide)},
    {"SkipAlert", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::SkipAlert)},
    {"Stuck", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::Stuck)},
    {"StuckOnTeammate",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::StuckOnTeammate)},
    {"TeamAttacked",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::TeamAttacked)},
    {"TooMuchTurning",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSplinter::TooMuchTurning)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Alert", static_cast< CPatterned::StateMachine::StateFunc >(&CSplinter::Alert)},
    {"Dead", static_cast< CPatterned::StateMachine::StateFunc >(&CSplinter::Dead)},
    {"Descend", static_cast< CPatterned::StateMachine::StateFunc >(&CSplinter::Descend)},
    {"Drop", static_cast< CPatterned::StateMachine::StateFunc >(&CSplinter::Drop)},
    {"DropAndExplode",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSplinter::DropAndExplode)},
    {"FallFromCocoon",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSplinter::FallFromCocoon)},
    {"FastTurn", static_cast< CPatterned::StateMachine::StateFunc >(&CSplinter::FastTurn)},
    {"GoToPad", static_cast< CPatterned::StateMachine::StateFunc >(&CSplinter::GoToPad)},
    {"GrabMBall", static_cast< CPatterned::StateMachine::StateFunc >(&CSplinter::GrabMBall)},
    {"GrabSac", static_cast< CPatterned::StateMachine::StateFunc >(&CSplinter::GrabSac)},
    {"Jump", static_cast< CPatterned::StateMachine::StateFunc >(&CSplinter::Jump)},
    {"JumpToPoint", static_cast< CPatterned::StateMachine::StateFunc >(&CSplinter::JumpToPoint)},
    {"MegaSplinterSpit",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSplinter::MegaSplinterSpit)},
    {"Null", static_cast< CPatterned::StateMachine::StateFunc >(&CSplinter::Null)},
    {"PathFind", static_cast< CPatterned::StateMachine::StateFunc >(&CSplinter::PathFind)},
    {"Patrol", static_cast< CPatterned::StateMachine::StateFunc >(&CSplinter::Patrol)},
    {"Pounce", static_cast< CPatterned::StateMachine::StateFunc >(&CSplinter::Pounce)},
    {"Retreat", static_cast< CPatterned::StateMachine::StateFunc >(&CSplinter::Retreat)},
    {"Rise", static_cast< CPatterned::StateMachine::StateFunc >(&CSplinter::Rise)},
    {"RunAndHide", static_cast< CPatterned::StateMachine::StateFunc >(&CSplinter::RunAndHide)},
    {"Scream", static_cast< CPatterned::StateMachine::StateFunc >(&CSplinter::Scream)},
    {"Sidestep", static_cast< CPatterned::StateMachine::StateFunc >(&CSplinter::Sidestep)},
    {"SlideOff", static_cast< CPatterned::StateMachine::StateFunc >(&CSplinter::SlideOff)},
    {"Taunt", static_cast< CPatterned::StateMachine::StateFunc >(&CSplinter::Taunt)},
    {"TurnToPlayer", static_cast< CPatterned::StateMachine::StateFunc >(&CSplinter::TurnToPlayer)},
    {"WaitInCocoon", static_cast< CPatterned::StateMachine::StateFunc >(&CSplinter::WaitInCocoon)},
    {"WaitOnCable", static_cast< CPatterned::StateMachine::StateFunc >(&CSplinter::WaitOnCable)},
};

CSplinter::SSurfaceAlignment::SSurfaceAlignment()
: CSurfaceAlignmentHelper(CMaterialFilter::MakeInclude(CMaterialList(kMT_Ceiling, kMT_Wall)))
, mLastSurfacePosition(0.f, 0.f, -100000.f) {}

CSplinter::SStateMachine2::SStateMachine2(CAssetId stateMachine) {
  if (stateMachine != kInvalidAssetId) {
    mToken = gpSimplePool->GetObj(SObjectTag('FSM2', stateMachine));
  }
}

CSplinter::SParticleData::SParticleData(CAssetId particle, const CDamageInfo& damage)
: mEffect(particle != kInvalidAssetId ? rstl::optional_object< TLockedToken< CGenDescription > >(
                                            TLockedToken< CGenDescription >(
                                                gpSimplePool->GetObj(SObjectTag('PART', particle))))
                                      : rstl::optional_object< TLockedToken< CGenDescription > >())
, mDamage(damage) {}

CSplinter::SJumpData::SJumpData() {
  mStart = CVector3f::Zero();
  mTarget = CVector3f::Zero();
  Reset();
}

void CSplinter::SJumpData::Reset() {
  mHeight = 0.f;
  mLanded = mIsHighJump = mUnknown2 = mHasValidTarget = mHasTarget = mJumping = mVelocitySet =
      mHasLineOfSight = mUnknown8 = false;
  mStart = CVector3f::Zero();
  mTarget = mStart;
  mPhase = 0;
}

CSplinter::SSacData::SSacData(int a, int b, int c, int d, bool e, bool f)
: mGrabbedId(kInvalidUniqueId)
, mDescendStartZ(0.f)
, mMorphBallPos(0.f, 0.f, -1000.f)
, mMorphBallIdleTime(0.f)
, mUnused(CVector3f::Zero())
, mSacOnCable(e)
, mHoldingSac(false)
, mCarriesSac(f)
, mSacAncs(a)
, mSacCharacter(b)
, mSacAnim(c)
, mUnknown34(d)
, mSacId(kInvalidUniqueId)
, mSacOffset(CVector3f::Zero())
, mDropHeight(50.f) {}

CSplinter::SSacData::~SSacData() {}

CSplinter::SSpitData::SSpitData(float f1, CAssetId projectile, float f2, const CDamageInfo& damage,
                                CAssetId visorEffect)
: mPosition(CVector3f::Zero())
, mProjectileInfo(projectile, damage)
, mVisorEffect(visorEffect != kInvalidAssetId
                   ? rstl::optional_object< TLockedToken< CGenDescription > >(
                         TLockedToken< CGenDescription >(
                             gpSimplePool->GetObj(SObjectTag('PART', visorEffect))))
                   : rstl::optional_object< TLockedToken< CGenDescription > >())
, mOrbitPosition(CVector3f::Zero())
, mUnknown50(f1)
, mUnknown54(f2)
, mUnknown58(-1000.f)
, mUnknown5c(-1000.f) {
  mSpitting = false;
  mOrbitValid = false;
  mProjectileInfo.Token().Lock();
}

CSplinter::CSplinter(TUniqueId uid, const rstl::string& name, CEntityInfo& info,
                     const CTransform4f& xf, const CModelData& modelData,
                     const CPatternedInfo& patternedInfo, CAssetId stateMachine2, float f1,
                     float f2, float f3, float f4, float f5, float f6, float f7,
                     const CDamageInfo& attackDamage, bool isWorker, bool skipAlert, int i1, int i2,
                     bool inCocoon, bool b2, bool b3, bool b5, int sacAncs, int sacCharacter,
                     int sacAnim, int sacAnim2, CAssetId particle, const CDamageInfo& damageInfo,
                     bool isMegaSplinter, CAssetId spitProjectile, const CDamageInfo& spitDamage,
                     CAssetId spitVisorEffect, const CActorParameters& actorParams)
: CPatterned(kPAI_Splinter, uid, name, kFT_Zero, info, xf, modelData, patternedInfo, kMT_Ground,
             kCT_One, kBT_BiPedal, actorParams)
, mTeamAiMgrId(kInvalidUniqueId)
, mPathFindSearch(nullptr, (patternedInfo.IsAnEncounter() ? 0x200 : 0) + 1,
                  patternedInfo.GetPathfindingIndex(), 1.f, 1.f, 0, CPFRegion::kRP_Center)
, mTime(0.f)
, mUnknown8b4(0.f)
, mUnhideTime(0.f)
, mUnknownId(kInvalidUniqueId)
, mStuckDeadline(0.f)
, mSeparationForce(CVector3f::Zero())
, mUnknown8d0(0.f)
, mUnknown8d8(0.f)
, mUnknown8dc(-1000.f)
, mUnknown8e0(0)
, mWebAttachLocator(0xff)
, mUnknown8e4(CVector3f::Zero())
, mEyeDirection(CVector3f::Zero())
, mUnknown92c(0)
, mUnknown930(1.f)
, mUnknown934_0(false)
, mInCocoon(inCocoon)
, mUnknown934_2(false)
, mUnknown934_3(false)
, mAlertMessageReceived(false)
, mIsWorker(isWorker)
, mSkipAlert(skipAlert)
, mUnknown934_7(b5)
, mIngEffectActive(false)
, mPathDestination(CVector3f::Zero())
, mUnknown944(100.f)
, mAlignment()
, mUnknown9b0(0)
, mEvasion(-1)
, mPrevEvasion(-1)
, mUnknown9e0(f1)
, mUnknown9e4(f2)
, mUnknown9e8(f3)
, mUnknown9ec(f4)
, mUnknown9f0(i1)
, mUnknown9f4(i2)
, mAttackDamage(attackDamage)
, mEvadeChecks()
, mUnknowna88(kInvalidUniqueId)
, mUnknowna8a(false)
, mSac(sacAncs, sacCharacter, sacAnim, sacAnim2, b2, b3)
, mUnknownad8(0.f)
, mUnknownadc(1)
, mUnknownae0(0.f)
, mStateMachine2(stateMachine2)
, mParticle(particle, damageInfo)
, mUnknownb1c(f5)
, mUnknownb20(0)
, mUnknownb24(0.f)
, mSpit()
, mUnknownb94(-1000.f)
, mUnknownb98(kInvalidUniqueId) {
  if (BodyController()->GetTurnSpeed() > 120.f) {
    BodyController()->SetTurnSpeed(120.f);
    mTurnSpeed = 120.f;
  }
  mWebAttachLocator = AnimationData()->GetLocatorSegId(rstl::string_l("Web_attach_LCTR"));
  if (isMegaSplinter == true) {
    mSpit = SSpitData(f6, spitProjectile, f7, spitDamage, spitVisorEffect);
    KnockBackController().EnableKnockBackPhysics(false);
  }
}

void CSplinter::QuitTeam(CStateManager& mgr) {
  if (mTeamAiMgrId != kInvalidUniqueId) {
    if (CScriptTeamAiMgr* team = TCastToPtr< CScriptTeamAiMgr >(mgr.ObjectById(mTeamAiMgrId))) {
      if (team->IsPartOfTeam(GetUniqueId())) {
        team->QuitTeam(GetUniqueId());
      }
    }
  }
}

void CSplinter::JoinTeam(CStateManager& mgr) {
  if (mTeamAiMgrId != kInvalidUniqueId) {
    if (CScriptTeamAiMgr* team = TCastToPtr< CScriptTeamAiMgr >(mgr.ObjectById(mTeamAiMgrId))) {
      team->JoinTeam(*this, CTeamAiRole::kTAR_Melee, CTeamAiRole::kTAR_Unknown,
                     CTeamAiRole::kTAR_Invalid);
      team->SetPlayerForwardProjectionDistance(5.f);
    }
  }
}

void CSplinter::SetupStateMachineHelper(CStateManager& mgr) {
  InitializeStateMachine(mgr);
  mStateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
  mStateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
}

void CSplinter::SetupStateMachine(CStateManager& mgr) {
  CPatterned::SetupStateMachine(mgr);
  if (mStateMachine2.mToken.valid() != true) {
    StateMachine* stateMachine = mStateMachine.get();
    stateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
    stateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
  }
}

bool CSplinter::IsListening() const { return true; }

float CSplinter::GetGravityConstant() const {
  if (mInCocoon == true) {
    return 0.f;
  }
  return kDefaultGravityAccel;
}

CProjectileInfo* CSplinter::ProjectileInfo() { return &mSpit->mProjectileInfo; }

CDamageInfo CSplinter::GetContactDamage() const {
  if (mJump.mJumping == true) {
    CDamageInfo damage = CPatterned::GetContactDamage();
    damage.SetDamage(0.f);
    damage.SetRadiusDamage(0.f);
    damage.SetRadius(0.f);
    damage.SetKnockBackPower(0.f);
    return damage;
  }
  return CPatterned::GetContactDamage();
}

CVector3f CSplinter::GetOrbitPosition(const CStateManager& mgr) const {
  if (IsMega() == true && mSpit->mOrbitValid == true) {
    return mSpit->mOrbitPosition;
  }
  return CPatterned::GetOrbitPosition(mgr);
}

const CActor* CSplinter::GetGrabbed(const CStateManager& mgr) const {
  if (mSac.mGrabbedId == kInvalidUniqueId) {
    return nullptr;
  }
  return TCastToConstPtr< CActor >(mgr.GetObjectById(mSac.mGrabbedId));
}

CActor* CSplinter::FindGrabbed(CStateManager& mgr) const {
  if (mSac.mGrabbedId == kInvalidUniqueId) {
    return nullptr;
  }
  return TCastToPtr< CActor >(mgr.ObjectById(mSac.mGrabbedId));
}

bool CSplinter::IsGrabbingPlayer(CStateManager& mgr) const {
  return mSac.mGrabbedId != kInvalidUniqueId && mSac.mGrabbedId == mgr.GetPlayer(0)->GetUniqueId();
}

void CSplinter::DetachMorphBall(CStateManager& mgr) {
  CPlayer* player = mgr.GetPlayer(0);
  if (player->GetAttachedActorId() == GetUniqueId()) {
    player->DetachActorFromPlayer();
  }
  player->EnableLeaveMorphBall(true);
  player->AddMaterial(kMT_Solid, mgr);
}

void CSplinter::AttachMorphBall(CStateManager& mgr) {
  CPlayer* player = mgr.GetPlayer(0);
  player->AttachActorToPlayer(GetUniqueId(), true);
  player->Stop();
  player->RemoveMaterial(kMT_Solid, mgr);
  player->EnableLeaveMorphBall(false);
  player->GetMorphBall()->DisableHalfPipeStatus();
}

void CSplinter::PlayStepAnim(CStateManager& mgr, int direction, bool loop) {
  if (mgr.IsRandomAvailable()) {
    BodyController()->CommandMgr().ClearLocomotionCmds();
    CPASAnimParmData parms(pas::kAS_Step, CPASAnimParm::FromEnum(direction),
                           CPASAnimParm::FromEnum(3));
    if (loop == true) {
      BodyController()->LoopBestAnimation(parms, *mgr.Random());
    } else {
      BodyController()->PlayBestAnimation(parms, *mgr.Random());
    }
  }
}

void CSplinter::Retreat(CStateManager& mgr, EStateMsg msg, float dt) {
  mWaypointNavigation.Patrol(mgr, msg, dt, *this);
  if (msg == kStateMsg_Activate) {
    TUniqueId destination = GetConnectedObject(mgr, kSS_Retreat, kSM_Follow);
    mWaypointNavigation.SetDestination(destination);
  }
}

void CSplinter::GrabMBall(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    PlayStepAnim(mgr, 1, false);
    AttachMorphBall(mgr);
  }
}

bool CSplinter::MorphballWaiting(CStateManager& mgr, const CTriggerData& data) const {
  return false;
}

bool CSplinter::ReachedMBall(CStateManager& mgr, const CTriggerData& data) const {
  return mgr.GetPlayer(0)->GetTranslation().GetZ() + 1.f < GetTranslation().GetZ();
}

bool CSplinter::ReachedTop(CStateManager& mgr, const CTriggerData& data) const {
  return GetTranslation().GetZ() > mSac.mDescendStartZ - 0.1f;
}

bool CSplinter::HoldingSac(CStateManager& mgr, const CTriggerData& data) const {
  return mSac.mHoldingSac;
}

bool CSplinter::IsMegaLight(CStateManager& mgr, const CTriggerData& data) const {
  return !IsIngPossessed() && IsMega() == true;
}

bool CSplinter::IsMegaDark(CStateManager& mgr, const CTriggerData& data) const {
  return IsIngPossessed() == true && IsMega() == true;
}

bool CSplinter::AlertMessageReceived(CStateManager& mgr, const CTriggerData& data) const {
  return mAlertMessageReceived;
}

bool CSplinter::IsWorkerSplinter(CStateManager& mgr, const CTriggerData& data) const {
  if (IsIngPossessed() == true) {
    return false;
  }
  return mIsWorker;
}

bool CSplinter::SkipAlert(CStateManager& mgr, const CTriggerData& data) const { return mSkipAlert; }

bool CSplinter::InCocoon(CStateManager& mgr, const CTriggerData& data) const { return mInCocoon; }

bool CSplinter::Landed(CStateManager& mgr, const CTriggerData& data) const { return mJump.mLanded; }

bool CSplinter::HasValidJumpTarget(CStateManager& mgr, const CTriggerData& data) const {
  return mJump.mHasValidTarget;
}

void CSplinter::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                                float dt) {
  switch (type) {
  case kUE_Projectile:
    if (IsMega() == true) {
      CTransform4f xf = GetTransform();
      CVector3f position = mSpit->mPosition;
      xf.SetTranslation(position);
      CVector3f up = xf.GetUp();
      CVector3f aimPosition = mgr.GetPlayer(0)->GetAimPosition(mgr, 0.f);
      xf = CTransform4f::LookAt(position, aimPosition, up);
      LaunchProjectile(xf, mgr, 1, 0, false,
                       CImpactVisorEffect::ParticleEffect(
                           mSpit->mVisorEffect, CSfxManager::kInternalInvalidSfxId, false),
                       CVector3f(1.f, 1.f, 1.f));
    }
    break;
  case kUE_Landing:
    return;
  case kUE_Unknown37:
    if (IsMega() == true) {
      mSpit->mSpitting = false;
    }
    return;
  case kUE_Unknown38:
    if (IsMega() == true) {
      mSpit->mOrbitPosition = GetOrbitPosition(mgr);
      mSpit->mOrbitValid = true;
    }
    return;
  case kUE_Unknown39:
    if (IsMega() == true) {
      mSpit->mOrbitValid = false;
      RemoveMaterial(kMT_Target, kMT_Orbit, mgr);
    }
    return;
  default:
    break;
  }
  CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}

CSplinterAcidSac* CSplinter::GetSac(CStateManager& mgr) const {
  if (mSac.mSacId == kInvalidUniqueId) {
    return nullptr;
  }
  return static_cast< CSplinterAcidSac* >(mgr.ObjectById(mSac.mSacId));
}

bool CSplinter::HasUnpoppedSac(CStateManager& mgr) const {
  CSplinterAcidSac* sac = GetSac(mgr);
  if (sac == nullptr) {
    return false;
  }
  return !sac->IsPopped();
}

void CSplinter::PopSac(CStateManager& mgr) {
  if (GetSac(mgr) != nullptr) {
    GetSac(mgr)->Pop();
  }
}

bool CSplinter::IsSacMissing(const CStateManager& mgr) const {
  if (mSac.mCarriesSac == true) {
    return false;
  }
  const CActor* grabbed = GetGrabbed(mgr);
  if (grabbed == nullptr || !grabbed->GetActive()) {
    return true;
  }
  return false;
}

bool CSplinter::SacMissing(CStateManager& mgr, const CTriggerData& data) const {
  return IsSacMissing(mgr);
}

bool CSplinter::CableShot(CStateManager& mgr, const CTriggerData& data) const {
  return !HasUnpoppedSac(mgr);
}

bool CSplinter::DropCompleted(CStateManager& mgr, const CTriggerData& data) const {
  if (GetAlive() == false) {
    return true;
  }
  return GetBodyController()->IsAnimationOver();
}

bool CSplinter::GrabbedIt(CStateManager& mgr, const CTriggerData& data) const {
  return GetBodyController()->IsAnimationOver() == true;
}

bool CSplinter::ShouldDrop(CStateManager& mgr, const CTriggerData& data) const {
  if (!HasUnpoppedSac(mgr)) {
    return true;
  }
  if (mSac.mCarriesSac == false) {
    return false;
  }
  return GetTranslation().GetZ() < mSac.mDropHeight;
}

bool CSplinter::ReachedSac(CStateManager& mgr, const CTriggerData& data) const {
  if (mSac.mCarriesSac == true) {
    return false;
  }
  const CActor* grabbed = GetGrabbed(mgr);
  if (grabbed == nullptr) {
    return true;
  }
  return GetTranslation().GetZ() < grabbed->GetTranslation().GetZ() + 1.f;
}

bool CSplinter::SacPresent(CStateManager& mgr, const CTriggerData& data) const {
  if (!GetActive()) {
    return false;
  }
  if (mSac.mCarriesSac == true) {
    return true;
  }
  return mSac.mGrabbedId != kInvalidUniqueId;
}

static rstl::string skRelFileName = rstl::string_l("Splinter.rel");

CSplinterAcidSac::CSplinterAcidSac(TUniqueId uid, TAreaId area, const rstl::string& name,
                                   const CModelData& modelData, int initialAnimation,
                                   int popAnimation, const CDamageVulnerability& vulnerability)
: CActor(uid, name, CEntityInfo(area, rstl::vector< SConnection >(), false, kInvalidEditorId), 0,
         CTransform4f::Identity(), modelData, CMaterialList(), CActorParameters::None(),
         kInvalidUniqueId)
, mPopAnimation(popAnimation)
, mInitialAnimation(initialAnimation)
, mPopped(false)
, mPopTimer(0.f)
, mDamageVulnerability(vulnerability)
, mHealthInfo(1.f, 0.f)
, mRelToken(skRelFileName, 0) {
  SetDrawShadow(false);
  AnimationData()->SetAnimation(CAnimPlaybackParms(initialAnimation, -1, 1.f, true), false);
}

bool CSplinterAcidSac::IsPopped() const { return mPopped; }

CHealthInfo* CSplinterAcidSac::HealthInfo() { return &mHealthInfo; }

const CDamageVulnerability* CSplinterAcidSac::GetDamageVulnerability() const {
  return &mDamageVulnerability;
}

void CSplinterAcidSac::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Damage:
    Pop();
    break;
  }
  CActor::AcceptScriptMsg(mgr, msg);
}

void CSplinterAcidSac::Pop() {
  if (mPopped != true) {
    AnimationData()->SetAnimation(CAnimPlaybackParms(mPopAnimation, -1, 1.f, true), false);
    mPopped = true;
  }
}

rstl::optional_object< CAABox > CSplinterAcidSac::GetTouchBounds() const {
  const CVector3f& position = GetTranslation();
  rstl::optional_object< CAABox > bounds(
      CAABox(CVector3f(position.GetX() - 0.4f, position.GetY() - 0.4f, position.GetZ() - 0.f),
             CVector3f(position.GetX() + 0.4f, position.GetY() + 0.4f, position.GetZ() + 0.f)));
  bounds->AccumulateBounds(
      CVector3f(position.GetX() + 0.4f, position.GetY() + 0.4f, position.GetZ() + 30.f));
  return bounds;
}

void CSplinterAcidSac::PreRender(CStateManager& mgr) {
  CActor::PreRender(mgr);
  if (mPopTimer > 0.f) {
    CColor black = CColor::Black();
    black.SetAlpha(0.f);
    SetModelColor(CColor::Lerp(CColor::White(), black, mPopTimer));
  } else {
    SetModelFlags(CModelFlags(CModelFlags::kT_Two, CColor::White()));
  }
}

void CSplinterAcidSac::Render(const CStateManager& mgr) const { CActor::Render(mgr); }

void CSplinterAcidSac::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    AddMaterial(kMT_Solid, mgr);
    CActor::Think(dt, mgr);
    UpdateAnimation(dt, mgr, true);
    if (mPopped == true) {
      mPopTimer += dt;
      if (mPopTimer > 1.f) {
        mgr.DeleteObjectRequest(GetUniqueId());
      }
    }
  }
}

void CSplinter::KnockBack(CStateManager& mgr, const CKnockBackInfo& info) {
  if (mJump.mJumping == true && mJump.mUnknown8 == true && !IsMega()) {
    const CPASDatabase& pasDatabase = BodyController()->GetPASDatabase();
    CPASAnimParmData parms(pas::kAS_Unknown26);
    rstl::pair< float, int > anim = pasDatabase.FindBestAnimation(parms, *mgr.Random(), -1);
    if (anim.first > 0.f) {
      AnimationData()->AddAdditiveAnimation(anim.second, 1.f, false, false);
    }
  } else {
    CPatterned::KnockBack(mgr, info);
  }
}

bool CSplinter::MBallMoved(CStateManager& mgr, const CTriggerData& data) const {
  CPlayer* player = mgr.GetPlayer(0);
  if (!player->GetActive() || player->GetCurrentAreaId() != GetCurrentAreaId() ||
      (player->GetSpawnedMorphballState() == CPlayer::kMS_Unmorphed
           ? player->GetMorphballTransitionState()
           : CPlayer::kMS_Unmorphed) != CPlayer::kMS_Morphed) {
    return true;
  }
  return (player->GetTranslation() - mSac.mMorphBallPos).Magnitude() > 0.1f;
}

TUniqueId CSplinter::FindConnectedId(const CStateManager& mgr) const {
  for (rstl::vector< SConnection >::const_iterator it = GetConnectionList().begin();
       it != GetConnectionList().end(); ++it) {
    if (it->state == kSS_Connect) {
      return mgr.GetIdForScript(it->objId);
    }
  }
  return kInvalidUniqueId;
}

void CSplinter::CreateSac(CStateManager& mgr) {
  if (mSac.mSacAncs != kInvalidAssetId) {
    if (mSac.mSacId == kInvalidUniqueId) {
      mSac.mSacId = mgr.AllocateUniqueId();
      CSplinterAcidSac* sac = rs_new CSplinterAcidSac(
          mSac.mSacId, GetCurrentAreaId(), rstl::string_l("Worker Splinter Cable"),
          CModelData(CAnimRes(mSac.mSacAncs, mSac.mSacCharacter, CVector3f(1.f, 1.f, 1.f),
                              mSac.mSacAnim, true)),
          mSac.mSacAnim, mSac.mUnknown34, *GetDamageVulnerability());
      mgr.AddObject(sac);
    }
  }
}

bool CSplinter::IsOnCable(CStateManager& mgr, const CTriggerData& data) const {
  return mSac.mSacOnCable;
}

bool CSplinter::IngSwarmIncoming(CStateManager& mgr, const CTriggerData& data) const {
  bool incoming = false;
  if (GetAlive() == true && mUnknowna88 != kInvalidUniqueId) {
    incoming = true;
  }
  return incoming;
}

bool CSplinter::IsDarkSplinter(CStateManager& mgr, const CTriggerData& data) const {
  return IsIngPossessed();
}

bool CSplinter::StuckOnTeammate(CStateManager& mgr, const CTriggerData& data) const {
  return mStuckDeadline > mTime;
}

bool CSplinter::ShouldTaunt(CStateManager& mgr, const CTriggerData& data) const {
  return mUnknown9b0 > 0;
}

void CSplinter::SetCableMovement(CStateManager& mgr, bool onCable) {
  if (onCable == true) {
    mVerticalMovement = true;
    mOnGround = false;
    RemoveMaterial(kMT_GroundCollider, mgr);
  } else {
    mVerticalMovement = false;
    mOnGround = false;
    AddMaterial(kMT_GroundCollider, mgr);
  }
}

void CSplinter::KillSelf(CStateManager& mgr) {
  CDamageInfo damage(CWeaponMode(kWT_Power), 1000.f, 1.f, 0.f, false, false);
  mgr.ApplyDamage(GetUniqueId(), GetUniqueId(), GetUniqueId(), damage,
                  CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
                  CVector3f::Zero());
  ExplodeSac(mgr);
}

void CSplinter::ExplodeSac(CStateManager& mgr) {
  if (mSac.mHoldingSac == true && mSac.mGrabbedId != kInvalidUniqueId &&
      mParticle.mEffect.valid() == true) {
    CActor* grabbed = FindGrabbed(mgr);
    if (grabbed != nullptr) {
      CTransform4f xf(grabbed->GetTransform());
      xf.SetTranslation(mSac.mSacOffset);
      CExplosion* explosion =
          rs_new CExplosion(*mParticle.mEffect, mgr.AllocateUniqueId(),
                            CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true),
                            rstl::string_l("Acid Sac Explosion"), xf, 0,
                            1.f * GetModelData()->GetScale(), CColor::White(), -1);
      mgr.AddObject(explosion);
      mgr.ApplyDamageToWorld(
          GetUniqueId(), *this, xf.GetTranslation(), mParticle.mDamage,
          CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()));
    }
    mSac.mHoldingSac = false;
  }
}

void CSplinter::Rise(CStateManager& mgr, EStateMsg msg, float dt) {
  if (HasUnpoppedSac(mgr)) {
    if (msg == kStateMsg_Activate) {
      if (!IsSacMissing(mgr)) {
        PlayStepAnim(mgr, 4, true);
      } else {
        PlayStepAnim(mgr, 3, true);
      }
    }
    if (IsGrabbingPlayer(mgr) != true && mSac.mHoldingSac == true && FindGrabbed(mgr) == nullptr) {
      KillSelf(mgr);
    } else if (msg == kStateMsg_Deactivate) {
      if (mSac.mGrabbedId != kInvalidUniqueId) {
        if (IsGrabbingPlayer(mgr) == true) {
          DetachMorphBall(mgr);
        } else {
          CActor* grabbed = FindGrabbed(mgr);
          if (grabbed != nullptr) {
            grabbed->SendActive(mgr, false);
          }
        }
        if (!ReachedTop(mgr, CTriggerData(0))) {
          ExplodeSac(mgr);
        }
        mSac.mGrabbedId = kInvalidUniqueId;
      }
      SendScriptMsgs(kSS_DeGenerate, mgr, kInvalidUniqueId, kSM_None);
      mgr.DeleteObjectRequest(GetUniqueId());
    }
  }
}

void CSplinter::DropAndExplode(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    SetCableMovement(mgr, false);
    PopSac(mgr);
    mOnGround = false;
    PlayStepAnim(mgr, 4, true);
  }
  if (mOnGround == true) {
    KillSelf(mgr);
  }
}

void CSplinter::Drop(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    PlayStepAnim(mgr, 2, false);
    SetCableMovement(mgr, false);
    PopSac(mgr);
  }
}

void CSplinter::GrabSac(CStateManager& mgr, EStateMsg msg, float dt) {
  if (HasUnpoppedSac(mgr)) {
    if (msg == kStateMsg_Activate) {
      PlayStepAnim(mgr, 1, false);
    } else if (msg == kStateMsg_Deactivate) {
      CActor* grabbed = FindGrabbed(mgr);
      if (grabbed != nullptr) {
        grabbed->SendActive(mgr, false);
        mSac.mHoldingSac = true;
      }
    }
  }
}

void CSplinter::Descend(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    PlayStepAnim(mgr, 5, true);
    if (mSac.mDescendStartZ == 0.f) {
      mSac.mDescendStartZ = GetTranslation().GetZ();
    }
    static CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
        CMaterialList(kMT_Solid),
        CMaterialList(kMT_Character, kMT_Player, kMT_ProjectilePassthrough));
    CRayCastResult result = CGameCollision::RayStaticIntersection(
        mgr, GetTranslation() + CVector3f(0.f, 0.f, 0.5f), CVector3f::Down(), 50.f, filter);
    if (result.IsValid() == true) {
      mSac.mDropHeight = 5.f + result.GetPoint().GetZ();
    }
  }
}

const CScriptTeamAiMgr* CSplinter::GetTeamAiMgr(const CStateManager& mgr) const {
  return TCastToConstPtr< CScriptTeamAiMgr >(mgr.GetObjectById(mTeamAiMgrId));
}

CScriptTeamAiMgr* CSplinter::FindTeamAiMgr(CStateManager& mgr) const {
  return TCastToPtr< CScriptTeamAiMgr >(mgr.ObjectById(mTeamAiMgrId));
}

bool CSplinter::TeamAttacked(CStateManager& mgr, const CTriggerData& data) const {
  if (GetTeamAiMgr(mgr) != nullptr) {
    return GetTeamAiMgr(mgr)->GetWasHit();
  }
  return false;
}

bool CSplinter::IsHeckler(CStateManager& mgr, const CTriggerData& data) const {
  if (GetTeamAiMgr(mgr) == nullptr) {
    return false;
  }
  const CTeamAiRole* role = GetTeamAiMgr(mgr)->GetRole(GetUniqueId());
  if (role == nullptr) {
    return false;
  }
  if (role->GetTeamAiRole() == CTeamAiRole::kTAR_Unknown) {
    mUnknown8e0 = 0;
    return true;
  }
  return false;
}

bool CSplinter::IsMelee(CStateManager& mgr, const CTriggerData& data) const {
  if (GetTeamAiMgr(mgr) == nullptr) {
    return true;
  }
  const CTeamAiRole* role = GetTeamAiMgr(mgr)->GetRole(GetUniqueId());
  if (role == nullptr) {
    return false;
  }
  return role->GetTeamAiRole() == CTeamAiRole::kTAR_Melee;
}

bool CSplinter::NeedsToSlideOff(CStateManager& mgr, const CTriggerData& data) const {
  if (IsMega() == true) {
    return false;
  }
  return mUnknownadc == false;
}

bool CSplinter::HasRetreatPath(CStateManager& mgr) const {
  if (IsIngPossessed() == true) {
    return false;
  }
  return GetConnectedObject(mgr, kSS_Retreat, kSM_Follow) != kInvalidUniqueId;
}

bool CSplinter::FleesFromPlayer(CStateManager& mgr, const CTriggerData& data) const {
  return HasRetreatPath(mgr);
}

float CSplinter::GetHeckleRange() const { return 10.f + mMinAttackRange; }

bool CSplinter::InHeckleRange(CStateManager& mgr, const CTriggerData& data) const {
  float minRange = mMinAttackRange;
  float distSq = (mgr.GetPlayer(0)->GetTranslation() - GetTranslation()).MagSquared();
  float maxRange = GetHeckleRange();
  return distSq > minRange * minRange && distSq < maxRange * maxRange;
}

bool CSplinter::Stuck(CStateManager& mgr, const CTriggerData& data) const {
  if (mgr.GetPlayer(0)->GetCurrentAreaId() != GetCurrentAreaId()) {
    return false;
  }
  return mPredictedLeashTime > 1.f;
}

bool CSplinter::PlayerIsInSafeZone(CStateManager& mgr, const CTriggerData& data) const {
  if (mgr.GetIsDarkWorld()) {
    return mgr.GetSafeZoneManager()->IsObjectInHurtfulSafeZone(*mgr.GetPlayer(0), mgr);
  }
  return false;
}

bool CSplinter::TooMuchTurning(CStateManager& mgr, const CTriggerData& data) const {
  bool result;
  if (IsMegaLight(mgr, CTriggerData(0)) == true) {
    return false;
  }
  result = mUnknown8d8 != 0.f && mTime > 4.f + mUnknown8d8;
  if (result == true) {
    mUnknown9b0 = 0;
  }
  return result;
}

CVector3f CSplinter::GetPlayerTargetPosition(CStateManager& mgr) const {
  CPlayer* player = mgr.GetPlayer(0);
  float x, y, targetZ;
  if (IsMega() == true) {
    CVector3f aim = player->GetAimPosition(mgr, 0.25f);
    x = aim.GetX();
    y = aim.GetY();
    targetZ = aim.GetZ();
  } else {
    x = player->GetTranslation().GetX();
    y = player->GetTranslation().GetY();
    targetZ = player->GetTranslation().GetZ();
  }
  if (targetZ > 5.f + GetTranslation().GetZ()) {
    return CVector3f(x, y, 5.f + GetTranslation().GetZ());
  }
  if (targetZ < GetTranslation().GetZ()) {
    return CVector3f(x, y, GetTranslation().GetZ());
  }
  return CVector3f(x, y, targetZ);
}

bool CSplinter::IsFacing(const CVector3f& point, float angle) const {
  CVector3f toPoint = point - GetTranslation();
  CVector3f facing = mEyeDirection;
  facing.SetZ(0.f);
  toPoint.SetZ(0.f);
  if (!toPoint.CanBeNormalized() || !facing.CanBeNormalized()) {
    return false;
  }
  toPoint.Normalize();
  facing.Normalize();
  return CVector3f::GetAngleDiff(facing, toPoint) < angle;
}

bool CSplinter::FacingPlayerExactly(CStateManager& mgr, const CTriggerData& data) const {
  return IsFacing(GetPlayerTargetPosition(mgr), 4.5f * (M_PIF / 180.f));
}

bool CSplinter::FacingPlayer(CStateManager& mgr, const CTriggerData& data) const {
  if (IsMega() == true && IsIngPossessed() == true) {
    return IsFacing(mgr.GetPlayer(0)->GetTranslation(), 0.4712389f);
  }
  return IsFacing(GetPlayerTargetPosition(mgr), 9.f * (M_PIF / 180.f));
}

void CSplinter::RotateToPlayer(CStateManager& mgr, float dt, float turnSpeed, float aimOffset) {
  RotateToPoint(CVector3f(mgr.GetPlayer(0)->GetAimPosition(mgr, aimOffset)), dt, turnSpeed);
}

void CSplinter::FaceTargetAtPlayer(CStateManager& mgr) {
  BodyController()->CommandMgr().SetTargetVector(mgr.GetPlayer(0)->GetTranslation() -
                                                 GetTranslation());
}

bool CSplinter::IsNearPath(const CVector3f& position, float tolerance) const {
  return mPathFindSearch.NearlyOnPath(position, tolerance) == CPathFindSearch::kR_Success;
}

CScriptAIHint* CSplinter::FindPadHint(CStateManager& mgr) const {
  CObjectList& list = mgr.ObjectListById(kOL_AiWaypoint);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    CScriptAIHint* hint = TCastToPtr< CScriptAIHint >(list[i]);
    if (hint != nullptr && hint->GetHintType() == CScriptAIHint::kHT_SplinterPad) {
      return hint;
    }
  }
  return nullptr;
}

bool CSplinter::IsOnPad(CStateManager& mgr, const CTriggerData& data) const {
  if (IsIngPossessed() == true) {
    return true;
  }
  const CScriptAIHint* hint = FindPadHint(mgr);
  if (hint == nullptr) {
    return true;
  }
  CVector3f delta = hint->GetTranslation() - GetTranslation();
  delta.SetZ(0.f);
  float radius = hint->GetRadius();
  if (data.GetFloat() > 0.f) {
    radius *= data.GetFloat();
  }
  return delta.Magnitude() < radius;
}

void CSplinter::StartPathTo(CStateManager& mgr, const CVector3f& position, float dt) {
  mPathDestination = position;
  mPathFindNavigation.SetDestination(mPathDestination);
  mPathFindNavigation.PathFind(mgr, kStateMsg_Activate, dt, *this);
}

void CSplinter::GoToTarget(CStateManager& mgr, EStateMsg msg, const CActor* target, float dt) {
  CVector3f position = target->GetTranslation() + CVector3f(0.f, 0.f, 0.5f);
  switch (msg) {
  case kStateMsg_Activate:
    StartPathTo(mgr, position, dt);
    break;
  case kStateMsg_Update: {
    CVector3f move = CVector3f::Zero();
    if (!PathShagged(mgr, CTriggerData(0.f)) && !mPathFindSearch.IsOver()) {
      mPathFindNavigation.PathFind(mgr, msg, dt, *this);
      move = BodyController()->GetCommandMgr().GetMoveVector();
    } else if (PathShagged(mgr, CTriggerData(0.f))) {
      StartPathTo(mgr, position, dt);
    } else {
      move = mSteeringBehaviors.Arrival(*this, position, 5.f);
    }
    BodyController()->CommandMgr().DeliverCmd(CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
    break;
  }
  }
}

void CSplinter::GoToPad(CStateManager& mgr, EStateMsg msg, float dt) {
  GoToTarget(mgr, msg, FindPadHint(mgr), dt);
  if (msg == kStateMsg_Activate) {
    mSpeed = 1.4f;
  } else if (msg == kStateMsg_Deactivate) {
    mSpeed = mUnknown930;
  }
}

void CSplinter::WaitOnCable(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate && mSac.mSacId != kInvalidUniqueId) {
    CSplinterAcidSac* sac = GetSac(mgr);
    if (sac == nullptr) {
      mSac.mSacId = kInvalidUniqueId;
    } else {
      sac->SetActive(true);
    }
  }
  CPlayer* player = mgr.GetPlayer(0);
  if (msg == kStateMsg_Activate || (player->GetSpawnedMorphballState() == CPlayer::kMS_Unmorphed
                                        ? player->GetMorphballTransitionState()
                                        : CPlayer::kMS_Unmorphed) != CPlayer::kMS_Morphed) {
    mSac.mMorphBallPos = CVector3f(0.f, 0.f, -1000.f);
    mSac.mMorphBallIdleTime = 0.f;
    return;
  }
  if (mSac.mMorphBallIdleTime == 0.f) {
    CVector3f delta = GetTranslation() - player->GetTranslation();
    delta.SetZ(0.f);
    if (delta.Magnitude() > 3.f) {
      return;
    }
    mSac.mMorphBallPos = player->GetTranslation();
  }
  CVector3f moved = player->GetTranslation() - mSac.mMorphBallPos;
  if (moved.Magnitude() > 0.1f) {
    mSac.mMorphBallIdleTime = 0.f;
    mSac.mMorphBallPos = CVector3f(0.f, 0.f, -1000.f);
  } else {
    mSac.mMorphBallIdleTime += dt;
  }
}

void CSplinter::RecoverCollision(CStateManager& mgr) {
  CAABox bounds = GetMotionVolume(0.f);
  CAreaCollisionCache cache(bounds);
  CGameCollision::BuildAreaCollisionCache(mgr, cache);
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  mgr.BuildColliderList(nearList, *this, bounds);
  CGameCollision::CollisionFailsafe(mgr, cache, *this, *GetCollisionPrimitive(), nearList, 0.f, 0,
                                    0.f);
}

template < typename T >
void CSplinter::DeliverCommand(EStateMsg msg, pas::EAnimationState state, const T& cmd) {
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

void CSplinter::SlideOff(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    RecoverCollision(mgr);
  }
  DeliverCommand(msg, pas::kAS_Step, CBCStepCmd(pas::kSD_Backward, pas::kStep_Normal));
  FaceTargetAtPlayer(mgr);
  mUnknownadc = false;
}

void CSplinter::Scream(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    mgr.InformListeners(GetTranslation(), kLNT_Scream);
  }
}

void CSplinter::Null(CStateManager& mgr, EStateMsg msg, float dt) { mJump = SJumpData(); }

void CSplinter::WaitInCocoon(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    RemoveMaterial(kMT_Character, kMT_Solid, mgr);
    BodyController()->SetLocomotionType(pas::kLT_Internal6);
  }
  SetMomentumWR(CVector3f(0.f, 0.f, 0.f));
  SetVelocityWR(CVector3f::Zero());
  if (mUnknown934_2 == true && mUnhideTime == 0.f && mgr.IsRandomAvailable() == true) {
    SetUnhideTime(mgr);
  }
}

void CSplinter::SetUnhideTime(CStateManager& mgr) {
  mUnhideTime = (mUnknown9ec - mUnknown9e8) * mgr.Random()->Float() + (mTime + mUnknown9e8);
}

void CSplinter::MegaSplinterSpit(CStateManager& mgr, EStateMsg msg, float dt) {
  RotateToPlayer(mgr, dt, 120.f * (M_PIF / 180.f), 0.2f);
  DeliverCommand(msg, pas::kAS_MeleeAttack, CBCMeleeAttackCmd(pas::kS_One));
}

void CSplinter::AddEvadeCheck(const CVector3f& start, const CVector3f& end, bool clear) {
  while (mEvadeChecks.size() >= 4) {
    mEvadeChecks.erase(mEvadeChecks.begin());
  }
  SEvadeCheck check;
  check.mStart = start;
  check.mEnd = end;
  check.mClear = clear;
  mEvadeChecks.push_back(check);
}

bool CSplinter::CanEvade(CStateManager& mgr, EEvadeDirection direction) {
  float forwardY = GetTransform().Get11();
  float forwardX = GetTransform().Get01();
  CVector3f offset = CVector3f::Zero();
  switch (direction) {
  case kED_Left:
    offset = CVector3f(forwardY, -forwardX, 0.f);
    break;
  case kED_Right:
    offset = CVector3f(-forwardY, forwardX, 0.f);
    break;
  case kED_Back:
    offset = CVector3f(forwardX, forwardY, 0.f) * -1.f;
    break;
  case kED_Forward:
    offset = CVector3f(forwardX, forwardY, 0.f);
    break;
  }
  offset.Normalize();
  CVector3f scale = GetModelData()->GetScale();
  float distance = 4.f * (0.5f * (scale.GetX() + scale.GetY()));
  offset = offset * distance;
  offset += GetTranslation();
  if (GetTeamAiMgr(mgr) != nullptr) {
    CVector3f center = GetTeamAiMgr(mgr)->GetCenter(mgr);
    float targetX = offset.GetX() - center.GetX();
    float targetY = offset.GetY() - center.GetY();
    float targetZ = offset.GetZ() - center.GetZ();
    float targetDistSq = targetX * targetX + targetY * targetY + targetZ * targetZ;
    if (targetDistSq > 256.f) {
      float x = GetTranslation().GetX() - center.GetX();
      float y = GetTranslation().GetY() - center.GetY();
      float z = GetTranslation().GetZ() - center.GetZ();
      if (targetDistSq > x * x + y * y + z * z) {
        return false;
      }
    }
  }
  if (IsIngPossessed() == true && mgr.GetIsDarkWorld() == true &&
      mgr.GetSafeZoneManager()->PointIsInHurtfulSafeZone(mgr, offset) == true) {
    return false;
  }
  for (float t = 0.f; t < 1.f; t += 0.2f) {
    CVector3f point = t * GetTranslation() + (1.f - t) * offset;
    if (!IsNearPath(point + CVector3f(0.f, 0.f, 0.5f), 0.f)) {
      AddEvadeCheck(GetTranslation(), point, false);
      return false;
    }
    AddEvadeCheck(GetTranslation(), point, true);
  }
  if (GetTeamAiMgr(mgr) != nullptr) {
    float radius = 0.5f * distance;
    if (GetTeamAiMgr(mgr)->AnyMembersInCircle(mgr, offset, radius, GetUniqueId()) == true) {
      return false;
    }
    CVector3f middle = 0.5f * (offset + GetTranslation());
    if (GetTeamAiMgr(mgr)->AnyMembersInCircle(mgr, middle, radius, GetUniqueId()) == true) {
      return false;
    }
  }
  return true;
}

int CSplinter::ChooseEvasion(CStateManager& mgr, bool preferStep) {
  static const int skCloseOrder[] = {5, 0, 2, 3, 4, 1, 6};
  static const int skStepOrder[] = {0, 2, 5, 6, 1, 3, 4};
  while (mEvadeChecks.size() != 0) {
    mEvadeChecks.erase(mEvadeChecks.begin());
  }
  int random = mgr.Random()->Next();
  const CVector3f& playerPosition = mgr.GetPlayer(0)->GetTranslation();
  float dx = GetTranslation().GetX() - playerPosition.GetX();
  float dy = GetTranslation().GetY() - playerPosition.GetY();
  float dz = GetTranslation().GetZ() - playerPosition.GetZ();
  bool close = dx * dx + dy * dy + dz * dz < 25.f;
  for (int i = 0; i < 5; ++i) {
    int evasion;
    if (close == true) {
      evasion = skCloseOrder[i];
    } else if (preferStep == true) {
      evasion = skStepOrder[i];
    } else {
      evasion = (random + i) % 7;
    }
    if (evasion != mPrevEvasion && (evasion != 0 || CanEvade(mgr, kED_Left)) &&
        (evasion != 2 || CanEvade(mgr, kED_Right)) && (evasion != 5 || CanEvade(mgr, kED_Back)) &&
        (evasion != 6 || CanEvade(mgr, kED_Forward))) {
      return evasion;
    }
  }
  return random % 7;
}

void CSplinter::DoEvasion(CStateManager& mgr, EStateMsg msg) {
  switch (mEvasion) {
  case 0:
    DeliverCommand(msg, pas::kAS_Step, CBCStepCmd(pas::kSD_Left, pas::kStep_Normal));
    break;
  case 2:
    DeliverCommand(msg, pas::kAS_Step, CBCStepCmd(pas::kSD_Right, pas::kStep_Normal));
    break;
  case 5:
    DeliverCommand(msg, pas::kAS_Step, CBCStepCmd(pas::kSD_Backward, pas::kStep_Normal));
    break;
  case 6:
    DeliverCommand(msg, pas::kAS_Step, CBCStepCmd(pas::kSD_Forward, pas::kStep_Normal));
    break;
  case 1:
    DeliverCommand(msg, pas::kAS_Taunt, CBCTauntCmd(pas::kTT_One));
    break;
  case 3:
    DeliverCommand(msg, pas::kAS_Taunt, CBCTauntCmd(pas::kTT_Zero));
    break;
  case 4:
    DeliverCommand(msg, pas::kAS_Taunt, CBCTauntCmd(pas::kTT_Three));
    break;
  }
  FaceTargetAtPlayer(mgr);
}

void CSplinter::Sidestep(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    mPrevEvasion = mEvasion;
    mUnknown9b0 -= 1;
    mEvasion = ChooseEvasion(mgr, true);
  }
  DoEvasion(mgr, msg);
  mStuckDeadline = 0.f;
}

void CSplinter::Taunt(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    mPrevEvasion = mEvasion;
    mUnknown9b0 -= 1;
    mEvasion = ChooseEvasion(mgr, false);
  }
  DoEvasion(mgr, msg);
}

void CSplinter::MassiveDeath(CStateManager& mgr) {
  CPatterned::MassiveDeath(mgr);
  ExplodeSac(mgr);
}

void CSplinter::Dead(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::Dead(mgr, msg, dt);
  switch (msg) {
  case kStateMsg_Activate:
    SetCableMovement(mgr, false);
    QuitTeam(mgr);
    RemoveMaterial(kMT_Target, kMT_Orbit, mgr);
    ExplodeSac(mgr);
    break;
  }
}

bool CSplinter::PounceTimeOut(CStateManager& mgr, const CTriggerData& data) const {
  if (mUnknowna88 != kInvalidUniqueId) {
    return false;
  }
  if (mUnknownb20 == 1 || GetHealthInfo()->GetHP() < mUnknownb1c) {
    return false;
  }
  return mTime > 2.5f + mSpit->mUnknown58;
}

bool CSplinter::ShouldEmergeFromCocoon(CStateManager& mgr, const CTriggerData& data) const {
  float unhideTime = mUnhideTime;
  if (0.f == unhideTime) {
    return false;
  }
  return unhideTime < mTime;
}

bool CSplinter::ShouldUnHide(CStateManager& mgr, const CTriggerData& data) const {
  if (mUnknownb98 == kInvalidUniqueId) {
    return true;
  }
  return mgr.GetPlayer(0)->GetCurrentAreaId() == GetCurrentAreaId();
}

bool CSplinter::HasAnyLandPoints(CStateManager& mgr, const CTriggerData& data) const {
  if (!mAlive) {
    return false;
  }
  for (rstl::vector< SConnection >::const_iterator it = GetConnectionList().begin();
       it != GetConnectionList().end(); ++it) {
    if (it->state == kSS_Attack && it->msg == kSM_Next) {
      return true;
    }
  }
  return false;
}

bool CSplinter::InAttackRange(CStateManager& mgr, const CTriggerData& data) const {
  float maxAttackRange = mMaxAttackRange;
  float f = (mgr.GetPlayer(0)->GetTranslation() - GetTranslation()).MagSquared();
  return f > 0.f && f < maxAttackRange * maxAttackRange;
}

void CSplinter::FastTurn(CStateManager& mgr, EStateMsg msg, float dt) {
  CPlayer* player = mgr.GetPlayer(0);
  if (player->GetSidewaysDashing() == true) {
    mSpit->mUnknown5c = mTime;
  }
  if (IsFacing(GetPlayerTargetPosition(mgr), 9.f * (M_PIF / 180.f)) == true) {
    BodyController()->CommandMgr().DeliverCmd(
        CBCLocomotionCmd(CVector3f::Zero(), CVector3f::Zero(), 1.f));
  } else {
    CVector3f delta = player->GetAimPosition(mgr, 0.4f) - GetTranslation();
    delta.SetZ(0.f);
    if (delta.IsMagnitudeSafe()) {
      BodyController()->CommandMgr().DeliverCmd(
          CBCLocomotionCmd(CVector3f::Zero(), delta.AsNormalized(), 1.f));
    }
  }
}

void CSplinter::TurnToPlayer(CStateManager& mgr, EStateMsg msg, float dt) {
  if (IsMega() == true && IsIngPossessed() == true) {
    FastTurn(mgr, msg, dt);
  } else {
    if (FacingPlayer(mgr, CTriggerData(0.f)) == true) {
      BodyController()->CommandMgr().DeliverCmd(
          CBCLocomotionCmd(CVector3f::Zero(), CVector3f::Zero(), 1.f));
    } else {
      CVector3f delta = mgr.GetPlayer(0)->GetTranslation() - GetTranslation();
      delta.SetZ(0.f);
      if (delta.IsMagnitudeSafe()) {
        BodyController()->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), delta.AsNormalized(), 1.f));
      }
    }
  }
  if (0.f == mUnknown8d8) {
    mUnknown8d8 = mTime;
  }
}

bool CSplinter::IsAttackBlocked(CStateManager& mgr) const {
  CObjectList& list = mgr.ObjectListById(kOL_AiWaypoint);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    CScriptAIHint* hint = TCastToPtr< CScriptAIHint >(list[i]);
    if (hint != nullptr && hint->GetCurrentAreaId() == GetCurrentAreaId() &&
        hint->GetHintType() == CScriptAIHint::kHT_SplinterAttackBlock) {
      const CVector3f delta = hint->GetTranslation() - GetTranslation();
      float radius = hint->GetRadius();
      if (delta.Magnitude() < radius &&
          IsFacing(hint->GetTranslation(), 50.f * (M_PIF / 180.f)) == true) {
        return true;
      }
    }
  }
  return false;
}

CScriptAIHint* CSplinter::FindHideHint(CStateManager& mgr) const {
  CObjectList& list = mgr.ObjectListById(kOL_AiWaypoint);
  CScriptAIHint* result = nullptr;
  float bestDistance = 3.4028235e38f;
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    CScriptAIHint* hint = TCastToPtr< CScriptAIHint >(list[i]);
    if (hint != nullptr && hint->GetHintType() == CScriptAIHint::kHT_SplinterHide &&
        hint->GetCurrentAreaId() == GetCurrentAreaId() && hint->GetActive()) {
      if (hint->GetInUse(kInvalidUniqueId) != true || !(bestDistance < 100000.f)) {
        float distance = (GetTranslation() - hint->GetTranslation()).Magnitude();
        if (hint->GetInUse(kInvalidUniqueId) == true) {
          distance += 100000.f;
        }
        if (distance < bestDistance) {
          CVector3f offset(0.f, 0.f, 0.5f);
          if (mPathFindSearch.PathExists(GetTranslation() + offset,
                                         hint->GetTranslation() + offset) ==
              CPathFindSearch::kR_Success) {
            bestDistance = distance;
            result = hint;
          }
        }
      }
    }
  }
  return result;
}

bool CSplinter::HasClearPathTo(CStateManager& mgr, const CVector3f& point) const {
  static CMaterialFilter filter =
      CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid, kMT_AIBlock));
  CVector3f start = GetTranslation() + CVector3f(0.f, 0.f, 0.5f);
  CVector3f delta = point + CVector3f::Up() - start;
  float distance = delta.Magnitude();
  return CGameCollision::RayStaticLineOfSightTest(mgr, start, (1.f / distance) * delta, distance,
                                                  filter);
}

bool CSplinter::AttackPointIsLegal(CStateManager& mgr, const CTriggerData& data) const {
  if (mgr.GetPlayer(0)->GetCurrentAreaId() != GetCurrentAreaId()) {
    return false;
  }
  CVector3f target = GetPlayerTargetPosition(mgr);
  if (mUnknown8e0 > 5 && IsAttackBlocked(mgr) == false) {
    mUnknown8e0 = 0;
    return true;
  }
  if (IsMega() == false) {
    if (IsNearPath(GetTranslation() + CVector3f(0.f, 0.f, 1.f), 2.f) == true) {
      for (float t = 0.f; t < 1.f; t += 0.2f) {
        float s = 1.f - t;
        CVector3f point = t * GetTranslation() + s * target;
        if (IsNearPath(point + CVector3f(0.f, 0.f, 1.f), 0.f) == false) {
          ++mUnknown8e0;
          return false;
        }
      }
    }
    if (IsAttackBlocked(mgr) == true) {
      mUnknown8e0 = 0;
      return false;
    }
  }
  if (!(mUnknown8e4 == CVector3f::Zero())) {
    for (float t = 0.f; t <= 1.f; t += 0.1f) {
      float s = 1.f - t;
      CVector3f point = t * GetTranslation() + s * target;
      CVector3f diff = point - CVector3f(mUnknown8e4.GetX(), mUnknown8e4.GetY(), point.GetZ());
      if (diff.MagSquared() < 4.f) {
        ++mUnknown8e0;
        return false;
      }
    }
  }
  if (HasClearPathTo(mgr, target) == false) {
    ++mUnknown8e0;
    return false;
  }
  return true;
}

bool CSplinter::Listen(CStateManager& mgr, const CVector3f& position, EListenNoiseType type) {
  switch (type) {
  case kLNT_Scream:
    mUnknown934_2 = true;
    break;
  case kLNT_PlayerFire:
    if (mgr.GetPlayer(0)->GetCurrentAreaId() != GetCurrentAreaId()) {
      mUnknownb94 = mTime;
    }
    break;
  }
  return false;
}

bool CSplinter::ShotFromOtherArea(CStateManager& mgr, const CTriggerData& data) const {
  if (mgr.GetPlayer(0)->GetCurrentAreaId() == GetCurrentAreaId()) {
    return false;
  }
  if (mPredictedLeashTime > 2.f || 2.f + mUnknownb94 > mTime) {
    CScriptAIHint* hint = FindHideHint(mgr);
    if (hint != nullptr) {
      hint->SetInUse(true);
      mUnknownb98 = hint->GetUniqueId();
      return true;
    }
  }
  return false;
}

void CSplinter::CheckFooting(CStateManager& mgr) {
  if (!(0.5f + mUnknownad8 > mTime)) {
    mUnknownad8 = mTime;
    static CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
        CMaterialList(kMT_Solid),
        CMaterialList(kMT_Character, kMT_Player, kMT_ProjectilePassthrough));
    CVector3f direction = -1.f * GetTransform().GetUp();
    CVector3f start = GetTranslation() + CVector3f(0.f, 0.f, 1.f);
    CRayCastResult result =
        CGameCollision::RayStaticIntersection(mgr, start, direction.AsNormalized(), 6.f, filter);
    if (result.IsValid() == true) {
      CAABox bounds = GetBoundingBox();
      if (result.GetPoint().GetZ() <
          GetTranslation().GetZ() -
              0.5f * (bounds.GetMaxPoint().GetZ() - bounds.GetMinPoint().GetZ())) {
        mUnknownadc = false;
      } else {
        mUnknownadc = true;
      }
    } else {
      mUnknownadc = false;
    }
  }
}

void CSplinter::RunAndHide(CStateManager& mgr, EStateMsg msg, float dt) {
  CScriptAIHint* hint = TCastToPtr< CScriptAIHint >(mgr.ObjectById(mUnknownb98));
  switch (msg) {
  case kStateMsg_Activate:
    mUnknown930 = mSpeed;
    mSpeed = 1.5f;
    BodyController()->SetTurnSpeed(3.f * BodyController()->GetTurnSpeed());
    break;
  case kStateMsg_Update:
    if (hint == nullptr) {
      mUnknownb98 = kInvalidUniqueId;
    } else {
      GoToTarget(mgr, msg, hint, dt);
      CheckFooting(mgr);
    }
    break;
  case kStateMsg_Deactivate:
    mSpeed = mUnknown930;
    if (hint == nullptr) {
      mUnknownb98 = kInvalidUniqueId;
    } else {
      hint->SetInUse(false);
      mUnknownb98 = hint->GetUniqueId();
    }
    BodyController()->SetTurnSpeed(BodyController()->GetTurnSpeed() / 3.f);
    break;
  }
}

float CSplinter::GetPatrolDelay(CStateManager& mgr) const {
  float delay = mUnknown9e0;
  if (mgr.IsRandomAvailable() == true) {
    delay += (mUnknown9e4 - mUnknown9e0) * mgr.Random()->Float();
  }
  if (InDetectionRange(mgr, CTriggerData(0.f)) == true) {
    delay *= 0.5f;
  }
  return delay;
}

void CSplinter::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    if (IsMega() == false) {
      mAlignment.SetMode(CSurfaceAlignmentHelper::kM_NearbySurface);
    }
    mUnknown934_3 = InDetectionRange(mgr, CTriggerData(0.f));
    mUnknown8b4 = mTime + GetPatrolDelay(mgr);
    break;
  case kStateMsg_Update:
    if (0.f != mUnknown9e0 && 0.f != mUnknown9e4) {
      bool taunt = false;
      if (mUnknown8b4 < mTime) {
        taunt = true;
      } else {
        bool inRange = InDetectionRange(mgr, CTriggerData(0.f));
        if (inRange != mUnknown934_3) {
          taunt = inRange;
          mUnknown934_3 = inRange;
        }
      }
      if (taunt == true) {
        BodyController()->CommandMgr().DeliverCmd(CBCTauntCmd(pas::kTT_Five));
        mUnknown8b4 = mTime + GetPatrolDelay(mgr);
      }
    }
    break;
  }
  CPatterned::Patrol(mgr, msg, dt);
  if (msg == kStateMsg_Deactivate) {
    AddMaterial(kMT_GroundCollider, mgr);
  }
}

void CSplinter::Alert(CStateManager& mgr, EStateMsg msg, float dt) {
  if (mSkipAlert != true) {
    if (msg == kStateMsg_Activate) {
      BodyController()->CommandMgr().ClearLocomotionCmds();
      if (HasRetreatPath(mgr) == false) {
        mUnknownae0 = mgr.Random()->Range(0.f, 1.f);
      }
    } else if (msg == kStateMsg_Update) {
      if (mUnknownae0 > 0.f) {
        mUnknownae0 -= dt;
        if (mUnknownae0 <= 0.f) {
          DeliverCommand(kStateMsg_Activate, pas::kAS_Taunt, CBCTauntCmd(pas::kTT_Four));
          return;
        }
      }
    }
    if (!(mUnknownae0 > 0.f)) {
      DeliverCommand(msg, pas::kAS_Taunt, CBCTauntCmd(pas::kTT_Four));
    }
  }
}

CVector3f CSplinter::GetAttackPosition(CStateManager& mgr) const {
  const CVector3f& playerPosition = mgr.GetPlayer(0)->GetTranslation();
  float px = playerPosition.GetX();
  float py = playerPosition.GetY();
  float pz = playerPosition.GetZ();
  CVector3f delta = GetTranslation() - playerPosition;
  float distance = delta.Magnitude();
  if (IsMelee(mgr, CTriggerData(0.f)) == true) {
    distance = 0.f;
  } else if (distance > GetHeckleRange()) {
    distance = GetHeckleRange();
  } else if (distance < mMinAttackRange) {
    distance = mMinAttackRange;
  }
  delta.Normalize();
  delta *= distance;
  return CVector3f(px + delta.GetX(), py + delta.GetY(), pz + delta.GetZ());
}

void CSplinter::UpdateAlignment(CStateManager& mgr, float dt) {
  if (mJump.mJumping != true) {
    if (mAlignment.GetMode() == CSurfaceAlignmentHelper::kM_NearbySurface) {
      if ((GetTranslation() - mAlignment.mLastSurfacePosition).MagSquared() < 0.25f) {
        mAlignment.OrientToStoredSurface(*this, dt);
        return;
      }
      mAlignment.mLastSurfacePosition = GetTranslation();
    }
    mAlignment.Update(*this, mgr, dt);
  }
}

void CSplinter::UpdateSwarmGrab(CStateManager& mgr) {
  if (mUnknowna88 != kInvalidUniqueId) {
    if (TCastToPtr< CIngSnatchingSwarm >(mgr.ObjectById(mUnknowna88)) != nullptr) {
      mUnknowna88 = kInvalidUniqueId;
    }
  }
  if (mUnknowna8a == false && IsIngPossessed() == true) {
    mUnknowna8a = true;
  }
}

void CSplinter::UpdateEyes(CStateManager& mgr) {
  if (mIngEffectActive != IsIngPossessed()) {
    mIngEffectActive = IsIngPossessed();
    AnimationData()->SetEffectState(rstl::string_l(skEyesEffect), mIngEffectActive, mgr);
  }
}

void CSplinter::UpdateHealthMessage(CStateManager& mgr, float dt) {
  if (mUnknownb1c > 0.f && !mUnknownb20) {
    if (GetHealthInfo()->GetHP() < mUnknownb1c) {
      mUnknownb24 += dt;
      if (mUnknownb24 > 1.f) {
        SendScriptMsgs(kSS_AILogicState1, mgr, kSM_None);
        mUnknownb20 = 1;
      }
    }
  }
}

void CSplinter::UpdateAlignmentMode(EStateMsg msg) {
  switch (msg) {
  case kStateMsg_Activate:
    mAlignment.SetMode(CSurfaceAlignmentHelper::kM_WorldUp);
    break;
  case kStateMsg_Deactivate:
    mAlignment.SetMode(CSurfaceAlignmentHelper::kM_NearbySurface);
    break;
  }
}

bool CSplinter::IsNearWall(CStateManager& mgr) const {
  if (IsMega() == true) {
    return false;
  }
  CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Solid, kMT_Wall, kMT_AIBlock), CMaterialList(kMT_Floor, kMT_AIPassthrough));
  CVector3f forward = GetTransform().GetForward();
  forward.SetZ(0.f);
  if (forward.CanBeNormalized() == true) {
    forward.Normalize();
  }
  float x = forward.GetX();
  float y = forward.GetY();
  CVector3f start =
      GetLctrTransform(rstl::string_l("Eye_LCTR")).GetTranslation() + CVector3f(0.f, 0.f, -0.2f);
  CVector3f ahead = forward * 2.f;
  CVector3f raised = CVector3f::Up() * 0.3f;
  CVector3f left = start + CVector3f(0.6f * y, 0.6f * -x, 0.f) + ahead + raised - start;
  CVector3f right = start + CVector3f(0.6f * -y, 0.6f * x, 0.f) + ahead + raised - start;
  const CGameArea& area = mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId());
  float leftLength = left.Magnitude();
  if (CGameCollision::RayStaticLineOfSightTest(area, start, left.AsNormalized(), leftLength,
                                               filter)) {
    const CGameArea& rightArea = mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId());
    float rightLength = right.Magnitude();
    if (!CGameCollision::RayStaticLineOfSightTest(rightArea, start, right.AsNormalized(),
                                                  rightLength, filter)) {
      return true;
    }
  } else {
    return true;
  }
  return false;
}

float CSplinter::ScoreJump(const CVector3f& jumpPoint, const CVector3f& waypoint,
                           const CVector3f& playerPosition, bool nearWall, float weight) const {
  CVector3f jumpDelta(jumpPoint - GetTranslation());
  float jumpDistance = jumpDelta.Magnitude();
  if (jumpDistance > 6.f) {
    return -1.f;
  }
  if (nearWall == false) {
    CVector3f waypointDelta = waypoint - GetTranslation();
    if (waypointDelta.Magnitude() < 6.f) {
      return -1.f;
    }
  }
  CVector3f playerDelta = playerPosition - waypoint;
  float playerDistance = playerDelta.Magnitude();
  if (nearWall == false) {
    CVector3f landingDelta = jumpPoint - waypoint;
    if (landingDelta.Magnitude() < playerDistance) {
      return -1.f;
    }
  }
  CVector3f gapDelta = waypoint - jumpPoint;
  return gapDelta.Magnitude() * weight + (jumpDistance + playerDistance);
}

rstl::pair< CScriptAiJumpPoint*, CScriptWaypoint* >
CSplinter::FindJumpPoints(CStateManager& mgr) const {
  bool nearWall = IsNearWall(mgr);
  CVector3f playerPosition = mgr.GetPlayer(0)->GetTranslation();
  float playerDistance = (playerPosition - GetTranslation()).Magnitude();
  CScriptAiJumpPoint* bestJumpPoint = nullptr;
  CScriptWaypoint* bestWaypoint = nullptr;
  float bestScore = 10000.f * playerDistance;
  CObjectList& list = mgr.ObjectListById(kOL_AiWaypoint);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    CScriptAiJumpPoint* jumpPoint = TCastToPtr< CScriptAiJumpPoint >(list[i]);
    if (jumpPoint != nullptr && jumpPoint->GetActive()) {
      if (jumpPoint->GetInUse(kInvalidUniqueId) != true &&
          jumpPoint->GetCurrentAreaId() == GetCurrentAreaId()) {
        TUniqueId waypointId = jumpPoint->CheckConnectedObject(mgr, kSS_Arrived, kSM_Next);
        if (waypointId != kInvalidUniqueId) {
          CScriptWaypoint* waypoint = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(waypointId));
          if (waypoint != nullptr) {
            float score = ScoreJump(jumpPoint->GetTranslation(), waypoint->GetTranslation(),
                                    playerPosition, nearWall, 0.3f);
            if (!(score < 0.f) && (nearWall != false || !(score > playerDistance)) &&
                score < bestScore) {
              bestScore = score;
              bestJumpPoint = jumpPoint;
              bestWaypoint = waypoint;
            }
          }
        }
      }
    }
  }
  return rstl::pair< CScriptAiJumpPoint*, CScriptWaypoint* >(bestJumpPoint, bestWaypoint);
}

bool CSplinter::FindJump(CStateManager& mgr) {
  mJump.Reset();
  mJump.mStart = GetTranslation();
  rstl::pair< CScriptAiJumpPoint*, CScriptWaypoint* > points = FindJumpPoints(mgr);
  if (points.first != nullptr) {
    mJump.mTarget = points.first->GetTranslation();
    mJump.mHeight = points.first->GetJumpApex();
    mJump.mStart = points.second->GetTranslation();
    mJump.mHasValidTarget = true;
    return true;
  }
  return false;
}

void CSplinter::ApplyJumpVelocity() {
  CVector3f velocity = CVector3f::Zero();
  float dz = mJump.mStart.GetZ() - GetTranslation().GetZ();
  float dx = mJump.mStart.GetX() - GetTranslation().GetX();
  float dy = mJump.mStart.GetY() - GetTranslation().GetY();
  float gravity = kDefaultGravityAccel;
  float height;
  if (dz < 0.f) {
    height = -dz;
  } else {
    height = dz;
  }
  velocity.SetZ(CMath::SqrtF(2.f * gravity * height));
  float time = velocity.GetZ() / gravity;
  if (dz < 0.f) {
    velocity.SetZ(-1.f * velocity.GetZ());
  }
  float halfTime = 0.5f * time;
  velocity.SetX(dx / halfTime);
  velocity.SetY(dy / halfTime);
  SetVelocityWR(velocity);
  mJump.mIsHighJump = true;
}

void CSplinter::UpdatePitchBend() {
  const CVector3f& scale = GetModelData()->GetScale();
  float t = (scale.GetZ() + (scale.GetX() + scale.GetY())) / 3.f - 0.5f;
  if (t < 0.f) {
    t = 0.f;
  } else if (t > 1.f) {
    t = 1.f;
  }
  float inverse = 1.f - t;
  float low;
  float high;
  if (!IsIngPossessed() && mIsWorker == true) {
    low = 12287.f;
    high = 16383.f;
  } else {
    low = 0.f;
    high = 8191.f;
  }
  SetSoundEventPitchBend(
      static_cast< int >(CMath::Clamp(0.f, inverse * (high - low) + low, 16383.f)));
}

void CSplinter::InitPathArea(CStateManager& mgr) {
  mPathFindSearch.SetArea(
      mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()).GetPostConstructed()->mPathArea);
}

void CSplinter::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  mUnknown8d8 = 0.f;
  switch (msg) {
  case kStateMsg_Activate:
    mJump.Reset();
    mPathDestination = CVector3f::Zero();
    mUnknown944 = 100.f;
    if (mTeamAiMgrId == kInvalidUniqueId) {
      mTeamAiMgrId = CScriptTeamAiMgr::GetAssociatedTeamId(*this, mgr);
      JoinTeam(mgr);
    }
    if (IsMega() == false) {
      mAlignment.SetMode(CSurfaceAlignmentHelper::kM_NearbySurface);
    }
    StartPathTo(mgr, GetAttackPosition(mgr), dt);
    CheckFooting(mgr);
    break;
  case kStateMsg_Update:
    if (mStuckDeadline != 0.f) {
      if (mStuckDeadline < mTime) {
        mStuckDeadline = 0.f;
      } else {
        BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_NextState));
        break;
      }
    }
    mUnknown944 += dt;
    if (mUnknown944 > 0.4f) {
      mUnknown944 = 0.f;
      if (FindJump(mgr) == true) {
        break;
      }
    }
    {
      CVector3f target = GetAttackPosition(mgr);
      if ((target - mPathDestination).MagSquared() > 16.f) {
        StartPathTo(mgr, target, dt);
      }
      CVector3f move;
      if (!PathShagged(mgr, CTriggerData(0.f)) && !mPathFindSearch.IsOver()) {
        mPathFindNavigation.PathFind(mgr, msg, dt, *this);
        move = BodyController()->GetCommandMgr().GetMoveVector();
      } else {
        move = mSteeringBehaviors.Arrival(*this, target, 5.f);
      }
      BodyController()->CommandMgr().DeliverCmd(CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
      CheckFooting(mgr);
    }
    break;
  }
}

bool CSplinter::CanPounce(CStateManager& mgr, const CTriggerData& data) const {
  if (mUnknowna88 != kInvalidUniqueId) {
    return false;
  }
  if (mUnknownb20 == 1 || GetHealthInfo()->GetHP() < mUnknownb1c) {
    return false;
  }
  return mTime > mSpit->mUnknown58;
}

bool CSplinter::CanSeePlayer(CStateManager& mgr, const CTriggerData& data) const {
  if (IsWorkerSplinter(mgr, CTriggerData(0.f)) == true ||
      !InDetectionRange(mgr, CTriggerData(0.f)) || !SpotPlayer(mgr, CTriggerData(0.f))) {
    return false;
  }
  if (0.4f + mUnknown8dc > mTime) {
    return false;
  }
  mUnknown8dc = mTime - mgr.Random()->Range(0.f, 0.08f);
  return PlayerSpot(mgr, CTriggerData(0.f));
}

void CSplinter::JumpToPoint(CStateManager& mgr, EStateMsg msg, float dt) {
  UpdateAlignmentMode(msg);
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mJump.mJumping = true;
    mJump.mPhase = 2;
    mJump.mHasLineOfSight = IsNearWall(mgr);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Jump)) {
      pas::EJumpState state = mJump.mHasLineOfSight == true ? pas::kJS_IntoJump : pas::kJS_Loop;
      pas::EJumpType type = mJump.mHasLineOfSight == true ? pas::kJT_One : pas::kJT_Normal;
      BodyController()->CommandMgr().DeliverCmd(
          CBCJumpCmd(mJump.mStart, type, state, 2, CBCJumpCmd::kFF_AmbushJump));
    }
    if (mAnimationState.GetState() == CAnimationState::kAS_Over) {
      mJump.Reset();
      mJump.mLanded = true;
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mJump.Reset();
    mJump.mJumping = false;
    break;
  }
}

void CSplinter::Pounce(CStateManager& mgr, EStateMsg msg, float dt) {
  UpdateAlignmentMode(msg);
  mUnknown8e0 = 0;
  switch (msg) {
  case kStateMsg_Activate:
    mUnknown930 = mSpeed;
    mSpit->mSpitting = true;
    if (mgr.Random()->Range(0.f, 1.f) < 0.7f) {
      mSpit->mUnknown60 = mgr.Random()->Range(0.41f, 0.47f);
    } else {
      mSpit->mUnknown60 = 0.f;
    }
    mSpeed = 1.f;
    break;
  case kStateMsg_Update:
    if (mgr.GetPlayer(0)->GetSidewaysDashing() == true) {
      mSpit->mUnknown5c = mTime;
    }
    if (mSpit->mSpitting == true) {
      RotateToPlayer(mgr, dt, 500.f * (M_PIF / 180.f), mSpit->mUnknown60);
    }
    break;
  case kStateMsg_Deactivate:
    mSpeed = mUnknown930;
    mSpit->mUnknown58 = mTime;
    if (mgr.IsRandomAvailable() == true) {
      mSpit->mUnknown58 += mgr.Random()->Range(mSpit->mUnknown50, mSpit->mUnknown54);
    }
    mSpit->mOrbitValid = false;
    AddMaterial(kMT_Target, kMT_Orbit, mgr);
    break;
  }
  DeliverCommand(msg, pas::kAS_MeleeAttack, CBCMeleeAttackCmd(pas::kS_Eleven));
}

void CSplinter::Jump(CStateManager& mgr, EStateMsg msg, float dt) {
  UpdateAlignmentMode(msg);
  mUnknown8e0 = 0;
  switch (msg) {
  case kStateMsg_Activate:
    mJump = SJumpData();
    mJump.mStart = GetPlayerTargetPosition(mgr);
    mJump.mTarget = GetTranslation();
    mJump.mJumping = true;
    mJump.mPhase = 0;
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mPrevEvasion = -1;
    mUnknown9b0 = 0;
    mEvasion = -1;
    mUnknown9b0 = mUnknown9f0 + mgr.Random()->Next() % (mUnknown9f4 + 1 - mUnknown9f0);
    if (FindTeamAiMgr(mgr) != nullptr) {
      FindTeamAiMgr(mgr)->StartMeleeAttack(GetUniqueId());
    }
    BodyController()->CommandMgr().SetTargetVector(mJump.mStart - mJump.mTarget);
    mJump.mHasLineOfSight = IsNearWall(mgr);
    if (mJump.mHasLineOfSight == true) {
      mJump.mHeight = 3.f;
    }
    break;
  case kStateMsg_Update:
    BodyController()->CommandMgr().SetTargetVector(mJump.mStart - mJump.mTarget);
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Jump)) {
      BodyController()->CommandMgr().DeliverCmd(CBCJumpCmd(
          mJump.mStart, mJump.mHasLineOfSight == true ? pas::kJT_One : pas::kJT_Normal,
          pas::kJS_IntoJump, mJump.mHasTarget == true ? 1 : 0, CBCJumpCmd::kFF_IntoJump));
    }
    if (mAnimationState.GetState() == CAnimationState::kAS_Over) {
      mJump.Reset();
      mJump.mLanded = true;
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    if (FindTeamAiMgr(mgr) != nullptr) {
      FindTeamAiMgr(mgr)->EndMeleeAttack(GetUniqueId());
    }
    mJump.mJumping = false;
    mUnknown8d8 = 0.f;
    break;
  }
}

void CSplinter::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  if (!mStateMachine->HasState() && mStateMachine->GetType() == 1) {
    SetupStateMachineHelper(mgr);
    return;
  }
  if (mJump.mPhase == 1) {
    CVector3f toStart = mJump.mStart - mJump.mTarget;
    CVector3f toSelf = GetTranslation() - mJump.mTarget;
    float halfDistance = 0.5f * toStart.Magnitude();
    if (toSelf.Magnitude() > halfDistance) {
      AddMaterial(kMT_Character, kMT_Solid, kMT_GroundCollider, mgr);
    }
  }
  if (!BodyController()->GetIsActive()) {
    BodyController()->Activate(mgr, pas::kAS_Invalid);
  }
  UpdateHealthMessage(mgr, dt);
  CPatterned::Think(dt, mgr);
  if (!mAlive) {
    return;
  }
  UpdateEyes(mgr);
  mTime += dt;
  if (mUnknown8d0 != 0.f) {
    if (0.2f + mUnknown8d0 < mTime) {
      mSeparationForce = CVector3f::Zero();
      mUnknown8d0 = 0.f;
    } else {
      ApplyForceWR(mSeparationForce, CAxisAngle::Identity());
    }
  }
  UpdateAlignment(mgr, dt);
  UpdateSwarmGrab(mgr);
  if (IsMega() == true) {
    if (IsIngPossessed() == true && !IsBeingSnatched()) {
      mSpeed = 1.3f * mUnknown930;
    }
    if (mgr.GetBossId() != GetUniqueId() && mIngPossessionBlend > 0.7f) {
      mgr.SetBossParams(GetUniqueId(), mIngPossessionData.ingPossessedHealth.health,
                        gpStringTable->GetStringIndex("BossBigSplinterDark"));
    }
  } else if (IsIngPossessed() == true) {
    mKnockBackController.SetPhysicsImpulseMagnitude(0.4f);
  }
}

void CSplinter::FallFromCocoon(CStateManager& mgr, EStateMsg msg, float dt) {
  mUnknown8d8 = 0.f;
  if (HasAnyLandPoints(mgr, CTriggerData(0.f)) == false) {
    AddMaterial(kMT_GroundCollider, mgr);
    mJump.mJumping = false;
    return;
  }
  UpdateAlignmentMode(msg);
  switch (msg) {
  case kStateMsg_Activate: {
    SetActive(true);
    mJump.mJumping = true;
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    rstl::reserved_vector< const CActor*, 10 > landPoints;
    for (rstl::vector< SConnection >::const_iterator it = GetConnectionList().begin();
         it != GetConnectionList().end(); ++it) {
      if (it->state == kSS_Attack && it->msg == kSM_Next) {
        landPoints.push_back(
            static_cast< const CActor* >(mgr.GetObjectById(mgr.GetIdForScript(it->objId))));
        if (landPoints.size() >= 10) {
          break;
        }
      }
    }
    if (landPoints.size() != 0) {
      const CActor* landPoint = landPoints[mgr.Random()->Next() % landPoints.size()];
      mJump = SJumpData();
      mJump.mTarget = GetTranslation();
      mJump.mStart = landPoint->GetTranslation();
      mJump.mPhase = 1;
      ApplyJumpVelocity();
      mAnimationState.SetState(CAnimationState::kAS_Ready);
      mInCocoon = false;
    }
    break;
  }
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Jump)) {
      BodyController()->CommandMgr().DeliverCmd(
          CBCJumpCmd(mJump.mStart, pas::kJT_Normal, pas::kJS_Loop, 2, CBCJumpCmd::kFF_AmbushJump));
    }
    if (mAnimationState.GetState() == CAnimationState::kAS_Over) {
      mJump = SJumpData();
      mJump.mLanded = true;
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    AddMaterial(kMT_GroundCollider, mgr);
    mJump.mJumping = false;
    if (IsMega() == true) {
      mSpit->mUnknown58 = mTime;
    }
    break;
  }
}

void CSplinter::CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                             CStateManager& mgr) {
  if (mAlive) {
    const CEntity* entity = mgr.GetObjectById(id);
    if (entity != nullptr) {
      if (TCastToConstPtr< CPlayer >(entity)) {
        if (mJump.mJumping == true && !mJump.mUnknown2) {
          mJump.mUnknown2 = true;
          mgr.ApplyDamage(
              GetUniqueId(), mgr.GetPlayer(0)->GetUniqueId(), GetUniqueId(), mAttackDamage,
              CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
              CVector3f::Zero());
        }
      } else if (FindTeamAiMgr(mgr) != nullptr) {
        if (FindTeamAiMgr(mgr)->IsPartOfTeam(id) == true) {
          mStuckDeadline = 0.7f + mTime;
          CAABox box = GetCollisionPrimitive()->CalculateAABox(GetTransform());
          float minY = box.GetMinPoint().GetY();
          float minZ = box.GetMinPoint().GetZ();
          float maxY = box.GetMaxPoint().GetY();
          const CPhysicsActor* other = TCastToConstPtr< CPhysicsActor >(entity);
          CAABox otherBox = other->GetCollisionPrimitive()->CalculateAABox(other->GetTransform());
          CVector3f otherCenter = otherBox.GetCenterPoint();
          if (otherCenter.GetZ() < minZ) {
            CVector2f separation = mSteeringBehaviors.Separation2D(
                *this, CVector2f(other->GetTranslation().GetX(), other->GetTranslation().GetY()),
                2.f * (maxY - minY));
            mSeparationForce = CVector3f(separation.GetX(), separation.GetY(), 0.f);
            mUnknown8d0 = mTime;
          }
        }
      }
    }
  }
  static CMaterialList skWallMaterials(kMT_Ceiling, kMT_Wall);
  for (int i = 0; i < list.GetCount(); ++i) {
    if (list[i].GetMaterialLeft().SharesMaterials(skWallMaterials) &&
        BodyController()->GetCurrentStateId() == pas::kAS_Jump) {
      mUnknown8e4 = GetTranslation();
      mJump.mVelocitySet = true;
      SetVelocityWR(CVector3f::Down());
    }
  }
  CPatterned::CollidedWith(id, list, mgr);
}

void CSplinter::ApplyLaunchVelocity() {
  if (mJump.mIsHighJump == true) {
    return;
  }
  CVector3f velocity = CVector3f::Zero();
  float dx = mJump.mStart.GetX() - GetTranslation().GetX();
  float dy = mJump.mStart.GetY() - GetTranslation().GetY();
  float gravity = GetGravityConstant();
  float z = GetTranslation().GetZ();
  float apex = mJump.mHeight + rstl::max_val(z, mJump.mStart.GetZ());
  float time = CMath::SqrtF(2.f * gravity * (apex - z));
  velocity.SetZ(time);
  time /= gravity;
  time += CMath::SqrtF(2.f * (apex - mJump.mStart.GetZ()) / gravity);
  float scale = 1.f / time;
  velocity.SetX(scale * dx);
  velocity.SetY(scale * dy);
  SetVelocityWR(velocity);
  mJump.mIsHighJump = true;
}

void CSplinter::ApplyAmbushVelocity(bool playerMorphed) {
  if (mJump.mIsHighJump == true) {
    return;
  }
  CVector3f velocity = CVector3f::Zero();
  float dx = mJump.mStart.GetX() - GetTranslation().GetX();
  float dy = mJump.mStart.GetY() - GetTranslation().GetY();
  float gravity = GetGravityConstant();
  float height = mJump.mHeight + static_cast< float >(playerMorphed == true);
  if (height == 0.f) {
    height = 0.8f;
  }
  float apex = height + rstl::max_val(GetTranslation().GetZ(), mJump.mStart.GetZ());
  velocity.SetZ(CMath::SqrtF(2.f * gravity * (apex - GetTranslation().GetZ())));
  float time =
      velocity.GetZ() / gravity + CMath::SqrtF(2.f * (apex - mJump.mStart.GetZ()) / gravity);
  if (time < 1.f) {
    time = 1.f;
  }
  float scale = 1.f / time;
  velocity.SetX(scale * dx);
  velocity.SetY(scale * dy);
  velocity.SetX(velocity.GetX() * 2.5f);
  velocity.SetY(velocity.GetY() * 2.5f);
  if (IsMega() == true && IsIngPossessed() == true) {
    velocity.SetX(velocity.GetX() * 1.4f);
    velocity.SetY(velocity.GetY() * 1.4f);
  }
  velocity.SetZ(CMath::Clamp(7.5f, velocity.GetZ() * 0.5f, 15.f));
  SetVelocityWR(velocity);
  mJump.mIsHighJump = true;
}

void CSplinter::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Create:
    if (mInCocoon == true) {
      mKnockBackController.EnableAllAnimReactions(false);
    }
    mUnknown9b0 = 0;
    mEvasion = -1;
    mUnknown9b0 = mUnknown9f0 + mgr.Random()->Next() % (mUnknown9f4 + 1 - mUnknown9f0);
    // fallthrough
  case kSM_Activate:
    if (mSac.mSacOnCable == true) {
      SetCableMovement(mgr, true);
      CreateSac(mgr);
      mSac.mGrabbedId = FindConnectedId(mgr);
    }
    break;
  case kSM_Alert:
    if (CIngSnatchingSwarm* swarm =
            TCastToPtr< CIngSnatchingSwarm >(mgr.ObjectById(msg.GetSenderId()))) {
      mUnknowna88 = swarm->GetUniqueId();
    } else {
      mAlertMessageReceived = true;
      if (mUnhideTime == 0.f) {
        mUnhideTime = 0.001f;
      }
    }
    break;
  case kSM_Deactivate:
    PopSac(mgr);
    QuitTeam(mgr);
    break;
  case kSM_Delete:
    if (mSac.mSacId != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(mSac.mSacId);
      mSac.mSacId = kInvalidUniqueId;
    }
    QuitTeam(mgr);
    break;
  case kSM_Damage:
    CPatterned::AcceptScriptMsg(mgr, msg);
    if (static_cast< bool >(mHitByPlayerProjectile) == true && FindTeamAiMgr(mgr) != nullptr) {
      FindTeamAiMgr(mgr)->NotifyWasHit();
    }
    return;
  case kSM_Launching:
    CPatterned::AcceptScriptMsg(mgr, CScriptMsg(msg.GetSenderId(), GetUniqueId(), kSM_OffGround));
    mJump.mUnknown8 = true;
    {
      float mass = GetMass();
      SetMomentumWR(CVector3f(0.f, 0.f, -GetGravityConstant() * mass));
    }
    switch (mJump.mPhase) {
    case 2:
      ApplyLaunchVelocity();
      break;
    default:
      ApplyAmbushVelocity(mgr.GetPlayer(0)->GetMorphballTransitionState() == CPlayer::kMS_Morphed);
      break;
    }
    break;
  case kSM_OffGround:
    if (mJump.mPhase == 0 && !mJump.mVelocitySet) {
      CPlayer* player = mgr.GetPlayer(0);
      if (player->GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
        mJump.mHasTarget = false;
      } else {
        float playerX = player->GetTranslation().GetX();
        float playerY = player->GetTranslation().GetY();
        float playerZ = player->GetTranslation().GetZ();
        CVector3f toStart = mJump.mStart - mJump.mTarget;
        CVector3f toPlayer = mgr.GetPlayer(0)->GetTranslation() - mJump.mTarget;
        float farDistance = 2.f * toStart.Magnitude();
        if (toPlayer.Magnitude() > farDistance) {
          mJump.mHasTarget = false;
        } else {
          CVector3f beyond(playerX - (mJump.mStart.GetX() + toStart.GetX()),
                           playerY - (mJump.mStart.GetY() + toStart.GetY()),
                           playerZ - (mJump.mStart.GetZ() + toStart.GetZ()));
          float nearDistance = 1.1f * toStart.Magnitude();
          if (beyond.Magnitude() > nearDistance) {
            mJump.mHasTarget = true;
          } else {
            mJump.mHasTarget = false;
          }
        }
      }
      BodyController()->CommandMgr().DeliverCmd(CBCUnknown18Cmd(mJump.mHasTarget == true));
    }
    break;
  case kSM_AreaLoaded:
    if (GetActive() && mTeamAiMgrId == kInvalidUniqueId) {
      mTeamAiMgrId = CScriptTeamAiMgr::GetAssociatedTeamId(*this, mgr);
      JoinTeam(mgr);
    }
    InitPathArea(mgr);
    UpdatePitchBend();
    mUnknowna8a = IsIngPossessed();
    if (mSac.mSacOnCable == true && GetActive() == true) {
      SetCableMovement(mgr, true);
      CreateSac(mgr);
      mSac.mGrabbedId = FindConnectedId(mgr);
    }
    if (mUnknown934_7) {
      SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
          GetMaterialFilter().GetIncludeList(),
          GetMaterialFilter().GetExcludeList().Union(CMaterialList(kMT_AIPassthrough))));
    }
    break;
  default:
    break;
  }
  CPatterned::AcceptScriptMsg(mgr, msg);
}

void CSplinter::PreRender(CStateManager& mgr) {
  CPatterned::PreRender(mgr);
  if (mSac.mSacId != kInvalidUniqueId) {
    CSplinterAcidSac* sac = GetSac(mgr);
    if (sac == nullptr) {
      mSac.mSacId = kInvalidUniqueId;
    } else if (!sac->IsPopped()) {
      sac->SetTranslation(GetLctrTransform(rstl::string_l(skWebAttachLocator)).GetTranslation());
    }
  }
  mSac.mSacOffset = GetLctrTransform(rstl::string_l(skAcidSackLocator)).GetTranslation();
  mEyeDirection = GetLctrTransform(rstl::string_l(skEyeLocator)).GetForward();
  if (IsMega() == true) {
    mSpit->mPosition = GetLctrTransform(rstl::string_l(skEyeLocator)).GetTranslation();
  }
}

rstl::optional_object< CAABox > CSplinter::GetTouchBounds() const {
  rstl::optional_object< CAABox > bounds = CPatterned::GetTouchBounds();
  if (mSac.mHoldingSac == true && mSac.mGrabbedId != kInvalidUniqueId) {
    bounds->AccumulateBounds(mSac.mSacOffset + CVector3f(0.8f, 0.8f, 0.7f));
    bounds->AccumulateBounds(mSac.mSacOffset - CVector3f(0.8f, 0.8f, 0.7f));
  }
  return bounds;
}

void CSplinter::Render(const CStateManager& mgr) const {
  CPatterned::Render(mgr);
  if (mSac.mHoldingSac == true) {
    const CActor* grabbed = GetGrabbed(mgr);
    if (grabbed != nullptr) {
      CTransform4f xf(grabbed->GetTransform());
      xf.SetTranslation(CVector3f(grabbed->GetTranslation().GetX(),
                                  grabbed->GetTranslation().GetY(), mSac.mSacOffset.GetZ()));
      grabbed->GetModelData()->Render(mgr, xf, grabbed->GetActorLights(), grabbed->GetModelFlags());
    }
  }
}

bool CSplinter::IsMega() const { return mSpit; }

CEntity* LoadSplinter(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrSplinter sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrSplinter.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  if (sldrThis.patterned.stateMachine2 != kInvalidAssetId) {
    sldrThis.patterned.stateMachine = kInvalidAssetId;
  }

  return rs_new CSplinter(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      *modelData, LdrToPatternedInfo(sldrThis.patterned, &sldrThis.ingPossessionData),
      sldrThis.patterned.stateMachine2, sldrThis.unknown_0xb8ed9ffa, sldrThis.unknown_0x5e8d301b,
      sldrThis.unknown_0xb98bb88f, sldrThis.unknown_0x5feb176e, sldrThis.unknown_0x72edeb7d,
      sldrThis.unknown_0x51be00d3, sldrThis.unknown_0xb7deaf32,
      LdrToDamageInfo(sldrThis.attackDamage), (sldrThis.unknown_0xb63b810c & 2) != 0,
      (sldrThis.unknown_0xb63b810c & 0x10) != 0, sldrThis.unknown_0x726cd31d,
      sldrThis.unknown_0x376e909f, (sldrThis.unknown_0xb63b810c & 1) != 0,
      (sldrThis.unknown_0xb63b810c & 4) != 0, (sldrThis.unknown_0xb63b810c & 8) != 0,
      (sldrThis.unknown_0xb63b810c & 0x20) != 0, sldrThis.unknown_0x6d752efc.ancs,
      sldrThis.unknown_0x6d752efc.character_index, sldrThis.unknown_0x6d752efc.initial_anim,
      sldrThis.unknown_0x0d6ab7b5.initial_anim, sldrThis.pART, LdrToDamageInfo(sldrThis.damageInfo),
      sldrThis.isMegaSplinter, sldrThis.megaSplinterSpitProjectile,
      LdrToDamageInfo(sldrThis.megaSplinterSpitProjectileDamage),
      sldrThis.megaSplinterSpitVisorEffect, LdrToActorParameters(sldrThis.actorInformation));
}

#ifndef MONOLITHIC
static void SetFuncPtrs() {
  static SSplinter_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &LoadSplinter;
  SetSSplinter_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSSplinter_FuncPtrs(nullptr); }
#endif
