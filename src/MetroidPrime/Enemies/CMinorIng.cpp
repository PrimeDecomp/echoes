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
#include "MetroidPrime/ScriptObjects/CScriptAIHint.hpp"
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

// ---- Raw matching-decompiler output from a local tree (reference only, not cleaned up) ----

extern float lbl_44_rodata_7C;
extern "C" void fn_44_657C();
extern "C" void fn_44_5D70(int, int, int);
extern "C" void fn_44_2F24();
extern "C" int fn_44_2598(int, int, long long*);
extern "C" void fn_44_660C(int, unsigned char*, unsigned char*, unsigned char*);
extern int lbl_44_data_79C;
extern "C" void fn_44_3B0C(int, int, int);
extern "C" void fn_44_9CC0(int, int);
extern int lbl_44_data_790;
extern int lbl_44_data_784;
extern int lbl_44_data_778;
extern "C" void fn_44_115C();
extern "C" void fn_44_6748();
extern "C" void fn_44_34B0(unsigned char*, int, int);
extern "C" unsigned char fn_44_60C4(int, int, int);
extern "C" void fn_44_8BF4(CVector3f*, int, int);

extern "C" void fn_44_6A3C() {}

extern "C" int fn_44_6F98(int arg0) { return *(int*)(arg0 + 0x254); }

extern "C" bool fn_44_3430(int arg0) { return *(unsigned char*)(arg0 + 0xd3d) >> 4 & 1; }

extern "C" bool fn_44_4A68(int arg0) { return *(unsigned char*)(arg0 + 0xd3d) >> 5 & 1; }

extern "C" bool fn_44_5CF0(int arg0) { return *(unsigned char*)(arg0 + 0x916) >> 3 & 1; }

extern "C" bool fn_44_5CFC(int arg0) { return *(unsigned char*)(arg0 + 0x916) >> 4 & 1; }

extern "C" bool fn_44_5D28(int arg0) { return *(unsigned char*)(arg0 + 0xd3e) >> 1 & 1; }

extern "C" bool fn_44_5D34(int arg0) { return *(unsigned char*)(arg0 + 0xd3e) >> 3 & 1; }

extern "C" bool fn_44_5D58(int arg0) { return *(unsigned char*)(arg0 + 0xd3e) >> 6 & 1; }

extern "C" bool fn_44_5D64(int arg0) { return *(unsigned char*)(arg0 + 0xd3e) >> 7 & 1; }

extern "C" bool fn_44_6178(int arg0) { return *(unsigned char*)(arg0 + 0xd3d) >> 7 & 1; }

extern "C" bool fn_44_6210(int arg0) { return *(unsigned char*)(arg0 + 0xd3d) >> 1 & 1; }

extern "C" bool fn_44_621C(int arg0) { return *(unsigned char*)(arg0 + 0xd3e) >> 2 & 1; }

extern "C" bool fn_44_6228(int arg0) { return *(unsigned char*)(arg0 + 0xd3d) >> 3 & 1; }

extern "C" bool fn_44_6234(int arg0) { return *(unsigned char*)(arg0 + 0x916) >> 6 & 1; }

extern "C" int fn_44_2038(int arg0) { return (*(int*)(arg0 + 0x6b4) == 3) ? 1 : 0; }

extern "C" int fn_44_5D40(int arg0) {
  return (6 == *(int*)(0x37c + (*(int*)(arg0 + 0x48c)))) ? 1 : 0;
}

extern "C" void fn_44_1114() {
  void fn_44_1134();
  fn_44_1134();
}

extern "C" bool fn_44_3E00(int arg0) { return *(float*)(arg0 + 0xd0c) <= lbl_44_rodata_7C; }

extern "C" void fn_44_655C() { fn_44_657C(); }

extern "C" void fn_44_6700() {
  void fn_44_6720();
  fn_44_6720();
}

extern "C" CModelData fn_44_76AC(int arg0) { return CModelData(); }

extern "C" void fn_44_5E70(int arg0, int arg1) { fn_44_5D70(arg0, arg1, arg0 + 84); }

extern "C" void fn_44_4A40(int arg0) {
  ((CIngSpotPathFindNavigation*)(arg0 + 3120))->HasPath(*(const CPatterned*)arg0);
}

extern "C" int fn_44_6240(int arg0) {
  s32 var_r4 = false;
  if ((*(unsigned char*)(arg0 + 0xd3d) & 1) || (*(unsigned char*)(arg0 + 0x916) >> 7 & 1)) {
    var_r4 = true;
  }
  return var_r4;
}

extern "C" int fn_44_2EF4(int arg0) {
  fn_44_2F24();
  return arg0;
}

extern "C" int fn_44_6144(int arg0) {
  int var_r6 = false;
  if ((*(unsigned short*)(arg0 + 0xcce)) == kInvalidUniqueId.value ||
      *(float*)(arg0 + 0xd08) > *(float*)(arg0 + 0x3f4)) {
    var_r6 = true;
  }
  return var_r6;
}

extern "C" bool fn_44_4D4C(int arg0, int arg1) {
  long long stack_8;
  *(unsigned short*)&stack_8 = *(unsigned short*)(arg0 + 0xcd0);
  return fn_44_2598(arg0, arg1, &stack_8) != 0;
}

extern "C" int fn_44_2C9C(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_44_6888(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_44_68C4(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_44_8AFC(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_44_A644(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_44_A804(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_44_A8CC(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_44_A908(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" void fn_44_7740(int arg0, int arg1, int arg2, float arg3) {
  switch (arg2) {
  case 0:
    *(unsigned short*)(arg0 + 0xcce) = kInvalidUniqueId.value;
    break;
  case 1:
  case 2:
    break;
  }
  ((CPatterned*)arg0)->Patrol(*(CStateManager*)arg1, (EStateMsg)arg2, arg3);
}

extern "C" int fn_44_A944(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" void fn_44_2550(int arg0, int arg1, int arg2) {
  ((CScriptAIHint*)arg1)->SetInUse(true);
  *(unsigned short*)arg2 = *(unsigned short*)(arg1 + 0x8);
}

extern "C" void fn_44_65C4(int arg0) {
  void fn_44_6500(unsigned char*, int);
  unsigned char stack_24[28];
  unsigned char stack_14[16];
  unsigned char stack_8[12];
  *(unsigned char*)(stack_24 + 0x14) = 0;
  *(unsigned char*)(stack_14 + 0xc) = 0;
  *(unsigned char*)(stack_8 + 0x8) = 0;
  fn_44_660C(arg0, stack_24, stack_14, stack_8);
  fn_44_6500(stack_24, -1);
}

extern "C" int fn_44_7918(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_44_data_79C;
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_44_2930(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_44_60F4(int arg0, int arg1, int arg2) {
  int var_r31 = false;
  if ((*(unsigned short*)(arg0 + 0xcce)) != kInvalidUniqueId.value ||
      ((CPatterned*)arg0)->InDetectionRange(*(CStateManager*)arg1, *(const CTriggerData*)arg2)) {
    var_r31 = true;
  }
  return var_r31;
}

extern "C" int fn_44_E64(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_44_107C(int arg0, int arg1) {
  if (arg0) {
    ((CDamageVulnerability*)(arg0 + 36))->~CDamageVulnerability();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_44_396C(int arg0) {
  int var_r31;
  for (var_r31 = 0; var_r31 < (*(int*)(arg0 + 0xce0)); var_r31 = var_r31 + 1) {
    fn_44_3B0C(arg0, var_r31, 0);
  }
}

extern "C" int fn_44_9BB8(int arg0, int arg1) {
  void fn_44_9C10(int, int);
  if (arg0) {
    fn_44_9C10(arg0 + 80, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_44_9C68(int arg0, int arg1) {
  if (arg0) {
    fn_44_9CC0(arg0 + 24, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_44_3CB4(int arg0, float arg1) {
  float temp_f2;
  if ((*(int*)(arg0 + 0xcc8)) == 1) {
    temp_f2 = *(float*)(arg0 + 0x440);
    if (temp_f2 > lbl_44_rodata_7C) {
      *(float*)(arg0 + 0x440) = temp_f2 - arg1;
    }
  }
  *(float*)(arg0 + 0xd04) = *(float*)(arg0 + 0xd04) - arg1;
  *(float*)(arg0 + 0xd10) = *(float*)(arg0 + 0xd10) - arg1;
  *(float*)(arg0 + 0xd1c) = *(float*)(arg0 + 0xd1c) - arg1;
  *(float*)(arg0 + 0xd38) = *(float*)(arg0 + 0xd38) - arg1;
}

extern "C" int fn_44_78BC(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_44_data_790;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_44_data_79C;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_44_8404(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_44_data_784;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_44_data_79C;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_44_87E0(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_44_data_778;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_44_data_79C;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_44_13BC(int arg0, int arg1) {
  if (arg0) {
    if ((*(unsigned char*)(arg0 + 0x4c))) {
      ((CModelData*)arg0)->~CModelData();
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_44_1660(int arg0, int arg1) {
  if (arg0) {
    ((SLdrDamageInfo*)(arg0 + 8))->~SLdrDamageInfo();
    ((SLdrHealthInfo*)arg0)->~SLdrHealthInfo();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_44_17CC(int arg0, int arg1) {
  if (arg0) {
    ((SLdrDamageVulnerability*)(arg0 + 88))->~SLdrDamageVulnerability();
    ((SLdrDamageInfo*)arg0)->~SLdrDamageInfo();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_44_1358(int arg0, int arg1) {
  if (arg0) {
    if ((*(unsigned char*)arg0)) {
      delete (CAnimData*)*(int*)(arg0 + 0x4);
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_44_343C(int arg0, int arg1) {
  float temp_f31;
  float temp_f1;
  *(float*)(arg0 + 0xd04) = ((CPatterned*)arg0)->GetAverageAttackTime();
  if ((unsigned int)(*(unsigned char*)(arg1 + 0x16e8) >> 7 & 1) == 1) {
    temp_f31 = *(float*)(arg0 + 0x3e8);
    temp_f1 = ((CRandom16*)(arg1 + 5860))->Float();
    *(float*)(arg0 + 0xd04) = temp_f1 * temp_f31 + *(float*)(arg0 + 0xd04);
  }
}

extern "C" void fn_44_1134(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_44_115C();
  }
}

extern "C" void fn_44_6720(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_44_6748();
  }
}

extern "C" int fn_44_64AC(int arg0, int arg1) {
  void fn_44_6500(int, int);
  if (arg0) {
    fn_44_6500(arg0, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_44_28D8(int arg0, int arg1) {
  void fn_44_2930(int, int);
  if (arg0) {
    fn_44_2930(arg0 + 4, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_44_9C10(int arg0, int arg1) {
  void fn_44_9C68(int, int);
  if (arg0) {
    fn_44_9C68(*(int*)arg0, 1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_44_6500(int arg0, int arg1) {
  void fn_44_655C();
  if (arg0) {
    if ((*(unsigned char*)(arg0 + 0x14))) {
      fn_44_655C();
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_44_3670(int arg0, int arg1) {
  float temp_f1;
  float temp_f2;
  CVector3f stack_14;
  unsigned char stack_8[12];
  fn_44_34B0(stack_8, arg0, arg1);
  float temp_f3 = *(float*)stack_8;
  temp_f2 = *(float*)((char*)stack_8 + 0x4);
  temp_f1 = *(float*)((char*)stack_8 + 0x8);
  stack_14.SetX(temp_f3);
  stack_14.SetY(temp_f2);
  stack_14.SetZ(temp_f1);
  bool var_r0 = 0.0f != temp_f3 || 0.0f != temp_f2 || 0.0f != temp_f1;
  if (var_r0) {
    ((CActor*)arg0)->SetTranslation(stack_14);
  }
}

extern "C" int fn_44_141C(int arg0, int arg1) {
  if (arg0) {
    if (arg0 + 1248) {
      ((SLdrDamageInfo*)(arg0 + 1256))->~SLdrDamageInfo();
      ((SLdrHealthInfo*)(arg0 + 1248))->~SLdrHealthInfo();
    }
    if (arg0 + 812) {
      ((SLdrDamageVulnerability*)(arg0 + 900))->~SLdrDamageVulnerability();
      ((SLdrDamageInfo*)(arg0 + 812))->~SLdrDamageInfo();
    }
    ((SLdrDamageInfo*)(arg0 + 760))->~SLdrDamageInfo();
    ((SLdrActorParameters*)(arg0 + 640))->~SLdrActorParameters();
    ((SLdrPatternedAITypedef*)(arg0 + 60))->~SLdrPatternedAITypedef();
    ((SLdrEditorProperties*)arg0)->~SLdrEditorProperties();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" bool fn_44_3D10(int arg0, int arg1, int arg2) {
  unsigned char fn_44_5D34(int, int, int);
  unsigned char fn_44_5E70();
  CVector3f stack_8;
  if (!fn_44_5E70()) {
    return true;
  }
  if (!fn_44_60C4(arg0, arg1, arg2)) {
    return true;
  }
  if (fn_44_5D34(arg0, arg1, arg2)) {
    return true;
  }
  if (!((*(unsigned char*)((char*)arg0 + 0xd3d)) >> 5 & 1)) {
    fn_44_8BF4(&stack_8, arg0, arg0 + 84);
    if (((CPathFindSearch*)(arg0 + 2400))->OnPath(stack_8)) {
      return true;
    }
  }
  return false;
}

// ---- End of raw matching-decompiler output ----
