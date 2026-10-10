#include "MetroidPrime/Enemies/CMinorIng.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrMinorIng.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "REL/REL_Setup.h"

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"PathShagged", static_cast< CPatterned::StateMachine::TriggerFunc >(&CMinorIng::PathShagged)},
    {"ShouldMoveUnderBall",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMinorIng::ShouldMoveUnderBall)},
    {"HasPatrolPath",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMinorIng::HasPatrolPath)},
    {"PlayerLeashReached",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMinorIng::PlayerLeashReached)},
    {"LeavePatrol", static_cast< CPatterned::StateMachine::TriggerFunc >(&CMinorIng::LeavePatrol)},
    {"LineOfSight", static_cast< CPatterned::StateMachine::TriggerFunc >(&CMinorIng::LineOfSight)},
    {"InMaxRange", static_cast< CPatterned::StateMachine::TriggerFunc >(&CMinorIng::InMaxRange)},
    {"ShouldFire", static_cast< CPatterned::StateMachine::TriggerFunc >(&CMinorIng::ShouldFire)},
    {"PuddleHitByWeapon",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMinorIng::PuddleHitByWeapon)},
    {"CancelStun", static_cast< CPatterned::StateMachine::TriggerFunc >(&CMinorIng::CancelStun)},
    {"IsBeingKnockedback",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMinorIng::IsBeingKnockedback)},
    {"TargetInSafeZone",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMinorIng::TargetInSafeZone)},
    {"HasCoverPoint",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMinorIng::HasCoverPoint)},
    {"PointPathOver",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMinorIng::PointPathOver)},
    {"FoundPointPath",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMinorIng::FoundPointPath)},
    {"IsPointPathFinding",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMinorIng::IsPointPathFinding)},
    {"StunOver", static_cast< CPatterned::StateMachine::TriggerFunc >(&CMinorIng::StunOver)},
    {"ShouldPathFind",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMinorIng::ShouldPathFind)},
    {"CancelAnim", static_cast< CPatterned::StateMachine::TriggerFunc >(&CMinorIng::CancelAnim)},
    {"HasAttackPattern",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMinorIng::HasAttackPattern)},
    {"AttackPatternOver",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMinorIng::AttackPatternOver)},
    {"IsFollowingAttackPath",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMinorIng::IsFollowingAttackPath)},
    {"AllowAttackPatternProjectile", static_cast< CPatterned::StateMachine::TriggerFunc >(
                                         &CMinorIng::AllowAttackPatternProjectile)},
    {"AllowAttackPatternStun",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMinorIng::AllowAttackPatternStun)},
    {"RecalculatePath",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMinorIng::RecalculatePath)},
    {"InsideSafeZone",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMinorIng::InsideSafeZone)},
    {"IsMoveAwayFromSafeZoneOver",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMinorIng::IsMoveAwayFromSafeZoneOver)},
    {"FoundMovementPos",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMinorIng::FoundMovementPos)},
    {"ShouldEvaporate",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMinorIng::ShouldEvaporate)},
    {"IsInCorporealForm",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMinorIng::IsInCorporealForm)},
    {"StayOnPointPathFinding",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMinorIng::StayOnPointPathFinding)},
    {"AllowProjectileDuringStayOnPointPathFinding",
     static_cast< CPatterned::StateMachine::TriggerFunc >(
         &CMinorIng::AllowProjectileDuringStayOnPointPathFinding)},
    {"StateOver", static_cast< CPatterned::StateMachine::TriggerFunc >(&CMinorIng::StateOver)},
    {"IsLuredBySafeZone",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CMinorIng::IsLuredBySafeZone)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"PathFind", static_cast< CPatterned::StateMachine::StateFunc >(&CMinorIng::PathFind)},
    {"PointPathFind",
     static_cast< CPatterned::StateMachine::StateFunc >(&CMinorIng::PointPathFind)},
    {"Stunned", static_cast< CPatterned::StateMachine::StateFunc >(&CMinorIng::Stunned)},
    {"ProjectileAttack",
     static_cast< CPatterned::StateMachine::StateFunc >(&CMinorIng::ProjectileAttack)},
    {"Patrol", static_cast< CPatterned::StateMachine::StateFunc >(&CMinorIng::Patrol)},
    {"Dead", static_cast< CPatterned::StateMachine::StateFunc >(&CMinorIng::Dead)},
    {"SetWallPointCoverDest",
     static_cast< CPatterned::StateMachine::StateFunc >(&CMinorIng::SetWallPointCoverDest)},
    {"SetCoverDest", static_cast< CPatterned::StateMachine::StateFunc >(&CMinorIng::SetCoverDest)},
    {"IntoPuddleForm",
     static_cast< CPatterned::StateMachine::StateFunc >(&CMinorIng::IntoPuddleForm)},
    {"IntoCorporealForm",
     static_cast< CPatterned::StateMachine::StateFunc >(&CMinorIng::IntoCorporealForm)},
    {"Idle", static_cast< CPatterned::StateMachine::StateFunc >(&CMinorIng::Idle)},
    {"FollowAttackPattern",
     static_cast< CPatterned::StateMachine::StateFunc >(&CMinorIng::FollowAttackPattern)},
    {"MoveAwayFromSafeZone",
     static_cast< CPatterned::StateMachine::StateFunc >(&CMinorIng::MoveAwayFromSafeZone)},
    {"AlwaysPointPathFind",
     static_cast< CPatterned::StateMachine::StateFunc >(&CMinorIng::AlwaysPointPathFind)},
    {"WaitForFSMTransition",
     static_cast< CPatterned::StateMachine::StateFunc >(&CMinorIng::WaitForFSMTransition)},
};

static CPatterned::StateMachine::SCodeFunction skCodeFuncs[] = {
    {"SelectTarget", static_cast< CPatterned::StateMachine::CodeFunc >(&CMinorIng::SelectTarget)},
    {"FindCoverPoint",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CMinorIng::FindCoverPoint)},
    {"SetPointCoverDest",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CMinorIng::SetPointCoverDest)},
    {"SetLuredDest", static_cast< CPatterned::StateMachine::CodeFunc >(&CMinorIng::SetLuredDest)},
    {"SetLuredPointDest",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CMinorIng::SetLuredPointDest)},
};

static EMaterialTypes skDamageMaterial = kMT_Solid;                // Guessed name
static EMaterialTypes skLineOfSightMaterial1 = kMT_Solid;          // Guessed name
static EMaterialTypes skLineOfSightMaterial2 = kMT_Player;         // Guessed name
static EMaterialTypes skLineOfSightMaterial3 = kMT_CollisionActor; // Guessed name

static CMaterialFilter skLineOfSightFilter = CMaterialFilter::MakeIncludeExclude(
    CMaterialList(skLineOfSightMaterial1),
    CMaterialList(skLineOfSightMaterial2, skLineOfSightMaterial3));
static rstl::string skLockOnLocator = rstl::string_l("lockon_target_SDK");
static rstl::string skEyeLocator = rstl::string_l("Eye_LCTR");

CMinorIng::CMinorIng(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                     const CTransform4f& xf, const CModelData& modelData,
                     const CPatternedInfo& patternedInfo, const CActorParameters& actorParams,
                     const SMinorIngData& data)
: CPatterned(kPAI_MinorIng, uid, name, kFT_Zero, info, xf, modelData, patternedInfo, kMT_Ground,
             kCT_One, kBT_BiPedal, actorParams)
, mData(data)
, mProjectileInfo(data.projectile, data.projectileDamage)
, mNormalHitEffect(mData.ingSpot.GetNormalHitEffect() != kInvalidAssetId
                       ? rstl::optional_object< TLockedToken< CGenDescription > >(
                             TLockedToken< CGenDescription >(gpSimplePool->GetObj(
                                 SObjectTag('PART', mData.ingSpot.GetNormalHitEffect()))))
                       : rstl::optional_object< TLockedToken< CGenDescription > >())
, mHeavyHitEffect(mData.ingSpot.GetHeavyHitEffect() != kInvalidAssetId
                      ? rstl::optional_object< TLockedToken< CGenDescription > >(
                            TLockedToken< CGenDescription >(gpSimplePool->GetObj(
                                SObjectTag('PART', mData.ingSpot.GetHeavyHitEffect()))))
                      : rstl::optional_object< TLockedToken< CGenDescription > >())
, mPathFindSearch(nullptr, 0x301, patternedInfo.GetPathfindingIndex(), 1.f, 1.f, 0,
                  CPFRegion::kRP_Center)
, mPointSearch(nullptr)
, mSurfaceAlignment()
, mPointNavigation()
, mUnknown0xc40(CVector3f::Zero())
, mPuddleBounds()
, mBodyBounds()
, mLineOfSight(GetUniqueId(), CSegId::Invalid(), 0.1f, 0.05f)
, mForm(-1)
, mTeamManagerId(kInvalidUniqueId)
, mTargetId(kInvalidUniqueId)
, mCoverPointId(kInvalidUniqueId)
, mPreviousCoverPointId(kInvalidUniqueId)
, mUniqueIdCd4(kInvalidUniqueId)
, mUniqueIdCd6(kInvalidUniqueId)
, mUniqueIdCd8(kInvalidUniqueId)
, mSafeZoneId(kInvalidUniqueId)
, mBlobEffectId(kInvalidUniqueId)
, mCollisionActorId(kInvalidUniqueId)
, mSfxHandles()
, mVectorCec(CVector3f::Zero())
, mScale(CVector3f::One())
, mFloatD04(0.f)
, mFloatD08(0.f)
, mFloatD0c(0.f)
, mFloatD10(0.f)
, mFloatD14(patternedInfo.GetHealthInfo().GetHP())
, mFloatD18(0.f)
, mFloatD1c(0.f)
, mFloatD20(0.f)
, mFloatD24(0.f)
, mIntD28(0)
, mCorporealFormAnim(-1)
, mStunnedAnim(-1)
, mFloatD34(0.f)
, mFloatD38(0.f)
, mLockOnSegment(CSegId::Invalid())
, mFlagD3d0(false)
, mFlagD3d1(false)
, mFlagD3d2(false)
, mFlagD3d3(false)
, mFlagD3d4(false)
, mFlagD3d5(false)
, mFlagD3d6(false)
, mFlagD3d7(false)
, mFlagD3e0(false)
, mFlagD3e1(false)
, mFlagD3e2(false)
, mFlagD3e3(false)
, mFlagD3e4(false)
, mFlagD3e5(false)
, mFlagD3e6(false) {
  mScale = GetModelData()->GetScale();
  mProjectileInfo.Token().Lock();
  mSurfaceAlignment.SetMode(CSurfaceAlignmentHelper::kM_NearbySurface);
  mPathFindSearch.SetCharacterRadius(3.f * mScale.GetY());
  mPathFindSearch.SetCharacterHeight(5.f * mScale.GetZ());
  KnockBackController().SetHurlVelocityEnabled(false);
  KnockBackController().EnableKnockBackPhysics(false);
  KnockBackController().SetAnimReactionRange(CKnockBackMgr::kAR_Flinch, CKnockBackMgr::kAR_Flinch);
  KnockBackController().EnableBurn(false);

  mLockOnSegment = GetAnimationData()->GetLocatorSegId(skLockOnLocator);
  if (mLockOnSegment != CSegId::Invalid()) {
    mLockOnTarget = mLockOnSegment;
  }
  mLineOfSight.SetSegment(GetAnimationData()->GetLocatorSegId(skEyeLocator));
  mLineOfSight.SetRayFilter(skLineOfSightFilter);
  mLookAtDeathDir = false;
  SetMomentumWR(CVector3f::Zero());

  const CPASAnimParmData corporealParms(pas::kAS_Generate, CPASAnimParm::FromEnum(0));
  mCorporealFormAnim =
      GetAnimationData()->GetPASDatabase().FindBestAnimation(corporealParms, -1).second;
  const CPASAnimParmData stunnedParms(pas::kAS_LoopReaction, CPASAnimParm::FromEnum(0),
                                      CPASAnimParm::FromEnum(3));
  mStunnedAnim = GetAnimationData()->GetPASDatabase().FindBestAnimation(stunnedParms, -1).second;
  mSfxHandles.resize(2, CSfxHandle());
}

CMinorIng::~CMinorIng() {}

bool CMinorIng::IsMoveAwayFromSafeZoneOver(CStateManager& mgr, const CTriggerData& data) const {
  bool over = false;
  if (mFlagD3d3) {
    bool finished;
    if (mFlagD3d2) {
      finished = PointPathOver(mgr, data);
    } else {
      finished = PathShagged(mgr, data) | CPatterned::PathOver(mgr, data);
    }
    if (finished) {
      over = true;
    }
  }
  return over;
}

bool CMinorIng::HasAttackPattern(CStateManager& mgr, const CTriggerData& data) const {
  return GetConnectedObject(mgr, kSS_Attack, kSM_Follow) != kInvalidUniqueId;
}

bool CMinorIng::AllowAttackPatternProjectile(CStateManager& mgr, const CTriggerData& data) const {
  return mFlagD3d7 || mData.allowProjectileDuringAttackPattern;
}

bool CMinorIng::AllowAttackPatternStun(CStateManager& mgr, const CTriggerData& data) const {
  return mData.unknown_0xbce16644;
}

bool CMinorIng::RecalculatePath(CStateManager& mgr, const CTriggerData& data) const {
  return mFlagD3d4;
}

bool CMinorIng::InsideSafeZone(CStateManager& mgr, const CTriggerData& data) const {
  return mFlagD3e5;
}

bool CMinorIng::HasPatrolPath(CStateManager& mgr, const CTriggerData& data) const {
  return mFlagD3d0;
}

bool CMinorIng::PlayerLeashReached(CStateManager& mgr, const CTriggerData& data) const {
  return mTargetId == kInvalidUniqueId || mFloatD08 > mPlayerLeashTime;
}

bool CMinorIng::LeavePatrol(CStateManager& mgr, const CTriggerData& data) const {
  bool leave = false;
  if (mTargetId != kInvalidUniqueId || InDetectionRange(mgr, data)) {
    leave = true;
  }
  return leave;
}

bool CMinorIng::LineOfSight(CStateManager& mgr, const CTriggerData& data) const {
  return mLineOfSight.HasLineOfSight() && mLineOfSight.GetClearTime() > 0.f;
}

bool CMinorIng::PuddleHitByWeapon(CStateManager& mgr, const CTriggerData& data) const {
  return mFlagD3e0;
}

bool CMinorIng::CancelStun(CStateManager& mgr, const CTriggerData& data) const { return mFlagD3e1; }

bool CMinorIng::IsBeingKnockedback(CStateManager& mgr, const CTriggerData& data) const {
  return mBodyController->GetCurrentStateId() == pas::kAS_KnockBack;
}

bool CMinorIng::ShouldMoveUnderBall(CStateManager& mgr, const CTriggerData& data) const {
  return mFlagD3e4;
}

bool CMinorIng::ShouldEvaporate(CStateManager& mgr, const CTriggerData& data) const {
  return mFlagD3e6;
}

bool CMinorIng::IsInCorporealForm(CStateManager& mgr, const CTriggerData& data) const {
  return mForm == kF_Corporeal || mForm == kF_IntoCorporeal;
}

bool CMinorIng::StayOnPointPathFinding(CStateManager& mgr, const CTriggerData& data) const {
  return mData.stayOnPointPathFinding;
}

bool CMinorIng::AllowProjectileDuringStayOnPointPathFinding(CStateManager& mgr,
                                                            const CTriggerData& data) const {
  return mData.allowProjectileDuringStayOnPointPathFinding;
}

bool CMinorIng::IsLuredBySafeZone(CStateManager& mgr, const CTriggerData& data) const {
  bool lured = false;
  if (mSafeZoneId != kInvalidUniqueId) {
    switch (mForm) {
    case kF_Corporeal:
    case kF_Puddle:
      lured = true;
      break;
    default:
      lured = false;
      break;
    }
  }
  return lured;
}

bool CMinorIng::IsPointPathFinding(CStateManager& mgr, const CTriggerData& data) const {
  return mFlagD3d2;
}

bool CMinorIng::FoundPointPath(CStateManager& mgr, const CTriggerData& data) const {
  return mPointNavigation.HasPath(*this);
}

void CMinorIng::Touch(CActor& actor, CStateManager& mgr) {
  if (mCurDamageRemTime <= 0.f && actor.GetUniqueId() == mTargetId) {
    if (mForm == kF_Puddle) {
      mgr.ApplyDamage(
          GetUniqueId(), mTargetId, GetUniqueId(), mData.ingSpotDamage,
          CMaterialFilter::MakeIncludeExclude(CMaterialList(skDamageMaterial), CMaterialList()),
          CVector3f::Zero());
    } else {
      mgr.ApplyDamage(
          GetUniqueId(), mTargetId, GetUniqueId(), GetContactDamage(),
          CMaterialFilter::MakeIncludeExclude(CMaterialList(skDamageMaterial), CMaterialList()),
          CVector3f::Zero());
    }
    mFlagD3e3 = true;
    mCurDamageRemTime = mDamageWaitTime;
  }
  CPatterned::Touch(actor, mgr);
}

rstl::optional_object< CAABox > CMinorIng::GetTouchBounds() const {
  if (mForm == kF_Puddle) {
    return mPuddleBounds;
  }
  return mBodyBounds;
}

void CMinorIng::PreRender(CStateManager& mgr) { CPatterned::PreRender(mgr); }

void CMinorIng::AddToRenderer(const CStateManager& mgr) const { CPatterned::AddToRenderer(mgr); }

void CMinorIng::Render(const CStateManager& mgr) const {
  if (mForm != kF_Puddle) {
    CPatterned::Render(mgr);
  }
}

const CDamageVulnerability* CMinorIng::GetDamageVulnerability() const {
  if (mForm == kF_Puddle) {
    return &mData.ingSpot.GetVulnerability();
  }
  return CPatterned::GetDamageVulnerability();
}

CVector3f CMinorIng::GetAimPosition(const CStateManager& mgr, float dt) const {
  return CPatterned::GetAimPosition(mgr, dt);
}

bool CMinorIng::StunOver(CStateManager& mgr, const CTriggerData& data) const {
  return mFloatD0c <= 0.f;
}

bool CMinorIng::CancelAnim(CStateManager& mgr, const CTriggerData& data) const {
  return mFlagD3e2 || mFlagD3e6;
}

bool CMinorIng::FoundMovementPos(CStateManager& mgr, const CTriggerData& data) const {
  return mFlagD3d3;
}

bool CMinorIng::StateOver(CStateManager& mgr, const CTriggerData& data) const {
  return mAnimationState.IsOver();
}

CEntity* REL_LoadMinorIng(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrMinorIng sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrMinorIng.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  const CDamageVulnerability vulnerability =
      LdrToDamageVulnerability(sldrThis.ingSpot.vulnerability);
  const CIngSpotData ingSpot(sldrThis.ingSpot.blobEffect, sldrThis.ingSpot.hitNormalDamage,
                             sldrThis.ingSpot.hitHeavyDamage, sldrThis.ingSpot.death,
                             sldrThis.ingSpot.maxSpeed, sldrThis.ingSpot.maxWallSpeed,
                             sldrThis.ingSpot.ballPursuitSpeed, sldrThis.ingSpot.maxSpeed,
                             sldrThis.ingSpot.turnSpeed, vulnerability, sldrThis.ingSpot.sound_Idle,
                             sldrThis.ingSpot.sound_Move, sldrThis.ingSpot.sound_HitNormalDamage,
                             sldrThis.ingSpot.sound_HitHeavyDamage, sldrThis.ingSpot.sound_Death);
  const CBouncyGrenadeData grenade(
      sldrThis.unknown_0x3da35851.mass, sldrThis.unknown_0x3da35851.bounciness,
      LdrToDamageInfo(sldrThis.unknown_0x3da35851.damage), sldrThis.unknown_0x3da35851.numBounces,
      sldrThis.unknown_0x3da35851.explosion, sldrThis.unknown_0x3da35851.explosion,
      sldrThis.unknown_0x3da35851.trail, sldrThis.unknown_0x3da35851.effect,
      sldrThis.unknown_0x3da35851.sound_Bounce, sldrThis.unknown_0x3da35851.sound_Explode, 0.1f,
      150.f, 0.1f, 150.f, true);
  const SMinorIngData data(
      sldrThis.projectile, LdrToDamageInfo(sldrThis.projectileDamage),
      LdrToDamageInfo(sldrThis.ingSpot.damage), ingSpot,
      LdrToHealthInfo(sldrThis.unknown_0x3da35851.health), grenade,
      sldrThis.unknown_0x3da35851.minGeneration, sldrThis.unknown_0x3da35851.maxGeneration,
      sldrThis.unknown_0xa03e450c, sldrThis.ingSpot.speedModifier,
      sldrThis.ingSpot.morphballPursuitDistance, sldrThis.ingSpot.bombStunDuration,
      sldrThis.ingSpot.unknown_0x7569fdba, sldrThis.ingSpot.sfxFallOff,
      sldrThis.unknown_0x3da35851.minLaunchSpeed, sldrThis.unknown_0x3da35851.maxLaunchSpeed,
      sldrThis.unknown_0x3da35851.maxTurnAngle, sldrThis.unknown_0x3da35851.unknown_0xfbf8ea0a,
      sldrThis.unknown_0x3da35851.unknown_0x47f99fbc, sldrThis.attackAngleLimit,
      sldrThis.lineOfSightHeightOffset, sldrThis.unknown_0xd6c8eac2, sldrThis.unknown_0x2a5449ba,
      sldrThis.hearingRadius, sldrThis.unknown_0x3da35851.allowLockOn, sldrThis.allowPuddleLockOn,
      sldrThis.unknown_0x09207f51, sldrThis.allowProjectileDuringAttackPattern,
      sldrThis.unknown_0xbce16644, sldrThis.unknown_0x142433d3, sldrThis.stayOnPointPathFinding,
      sldrThis.allowProjectileDuringStayOnPointPathFinding);

  return rs_new CMinorIng(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                          LdrToEntityInfo(info, sldrThis.editorProperties),
                          LdrToTransform4f(sldrThis.editorProperties), *modelData,
                          LdrToPatternedInfo(sldrThis.patterned, nullptr),
                          LdrToActorParameters(sldrThis.actorInformation), data);
}

static void SetFuncPtrs() {
  static SMinorIng_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &REL_LoadMinorIng;
  SetSMinorIng_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSMinorIng_FuncPtrs(nullptr); }
