#include "MetroidPrime/Enemies/CCommandoPirate.hpp"

#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CEffectWaypointPredicate.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CKnockBackInfo.hpp"
#include "MetroidPrime/CRagDoll.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Enemies/CPirateRagDoll.hpp"
#include "MetroidPrime/Enemies/CTeamAiRole.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrCommandoPirate.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIWaypoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAiJumpPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCoverPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTeamAiMgr.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CImpactVisorEffect.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "REL/REL_Setup.h"

#include "float.h"

typedef CPatterned::StateMachine::TriggerFunc TriggerFunc;
typedef CPatterned::StateMachine::StateFunc StateFunc;

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"StateOver", static_cast< TriggerFunc >(&CCommandoPirate::StateOver)},
    {"ShouldWarpIn", static_cast< TriggerFunc >(&CCommandoPirate::ShouldWarpIn)},
    {"ShouldMeleeAttack", static_cast< TriggerFunc >(&CCommandoPirate::ShouldMeleeAttack)},
    {"ShouldFireEGrenade", static_cast< TriggerFunc >(&CCommandoPirate::ShouldFireEGrenade)},
    {"ShouldRetreat", static_cast< TriggerFunc >(&CCommandoPirate::ShouldRetreat)},
    {"ShouldJumpBack", static_cast< TriggerFunc >(&CCommandoPirate::ShouldJumpBack)},
    {"ShouldAmbush", static_cast< TriggerFunc >(&CCommandoPirate::ShouldAmbush)},
    {"BreakAmbush", static_cast< TriggerFunc >(&CCommandoPirate::BreakAmbush)},
    {"HasLineOfSight", static_cast< TriggerFunc >(&CCommandoPirate::HasLineOfSight)},
    {"UnderFire", static_cast< TriggerFunc >(&CCommandoPirate::UnderFire)},
    {"HeardShot", static_cast< TriggerFunc >(&CCommandoPirate::HeardShot)},
    {"HasTarget", static_cast< TriggerFunc >(&CCommandoPirate::HasTarget)},
    {"HasNewTarget", static_cast< TriggerFunc >(&CCommandoPirate::HasNewTarget)},
    {"PathShagged", static_cast< TriggerFunc >(&CCommandoPirate::PathShagged)},
    {"PathOver", static_cast< TriggerFunc >(&CCommandoPirate::PathOver)},
    {"IsOffPath", static_cast< TriggerFunc >(&CCommandoPirate::IsOffPath)},
    {"HasAttackPattern", static_cast< TriggerFunc >(&CCommandoPirate::HasAttackPattern)},
    {"IsFacingTarget", static_cast< TriggerFunc >(&CCommandoPirate::IsFacingTarget)},
    {"AttackPatternOver", static_cast< TriggerFunc >(&CCommandoPirate::AttackPatternOver)},
    {"TooClose", static_cast< TriggerFunc >(&CCommandoPirate::TooClose)},
    {"ShouldDodge", static_cast< TriggerFunc >(&CCommandoPirate::ShouldDodge)},
    {"ShouldArmShield", static_cast< TriggerFunc >(&CCommandoPirate::ShouldArmShield)},
    {"ShouldShieldCharge", static_cast< TriggerFunc >(&CCommandoPirate::ShouldShieldCharge)},
    {"ShouldBoost", static_cast< TriggerFunc >(&CCommandoPirate::ShouldBoost)},
    {"AbortShieldCharge", static_cast< TriggerFunc >(&CCommandoPirate::AbortShieldCharge)},
    {"IsAggressive", static_cast< TriggerFunc >(&CCommandoPirate::IsAggressive)},
    {"FoundJumpPoint", static_cast< TriggerFunc >(&CCommandoPirate::FoundJumpPoint)},
    {"ShouldCrouch", static_cast< TriggerFunc >(&CCommandoPirate::ShouldCrouch)},
    {"ShouldWallHang", static_cast< TriggerFunc >(&CCommandoPirate::ShouldWallHang)},
    {"ShouldCover", static_cast< TriggerFunc >(&CCommandoPirate::ShouldCover)},
    {"FoundCover", static_cast< TriggerFunc >(&CCommandoPirate::FoundCover)},
    {"ShouldCoverAttack", static_cast< TriggerFunc >(&CCommandoPirate::ShouldCoverAttack)},
    {"CoverBlown", static_cast< TriggerFunc >(&CCommandoPirate::CoverBlown)},
    {"AbortSeekCover", static_cast< TriggerFunc >(&CCommandoPirate::AbortSeekCover)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Lurk", static_cast< StateFunc >(&CCommandoPirate::Lurk)},
    {"Ambush", static_cast< StateFunc >(&CCommandoPirate::Ambush)},
    {"Alert", static_cast< StateFunc >(&CCommandoPirate::Alert)},
    {"FaceTarget", static_cast< StateFunc >(&CCommandoPirate::FaceTarget)},
    {"SelectTarget", static_cast< StateFunc >(&CCommandoPirate::SelectTarget)},
    {"SetTargetDest", static_cast< StateFunc >(&CCommandoPirate::SetTargetDest)},
    {"SetRetreatDest", static_cast< StateFunc >(&CCommandoPirate::SetRetreatDest)},
    {"SetJumpDest", static_cast< StateFunc >(&CCommandoPirate::SetJumpDest)},
    {"SetPathMeshDest", static_cast< StateFunc >(&CCommandoPirate::SetPathMeshDest)},
    {"MeleeAttack", static_cast< StateFunc >(&CCommandoPirate::MeleeAttack)},
    {"EGrenadeAttack", static_cast< StateFunc >(&CCommandoPirate::EGrenadeAttack)},
    {"PostEGrenadeAttack", static_cast< StateFunc >(&CCommandoPirate::PostEGrenadeAttack)},
    {"JumpPointFind", static_cast< StateFunc >(&CCommandoPirate::JumpPointFind)},
    {"JetBoost", static_cast< StateFunc >(&CCommandoPirate::JetBoost)},
    {"PathFind", static_cast< StateFunc >(&CCommandoPirate::PathFind)},
    {"Patrol", static_cast< StateFunc >(&CCommandoPirate::Patrol)},
    {"FollowAttackPattern", static_cast< StateFunc >(&CCommandoPirate::FollowAttackPattern)},
    {"WarpIn", static_cast< StateFunc >(&CCommandoPirate::WarpIn)},
    {"WarpOut", static_cast< StateFunc >(&CCommandoPirate::WarpOut)},
    {"PostWarpOut", static_cast< StateFunc >(&CCommandoPirate::PostWarpOut)},
    {"JumpBack", static_cast< StateFunc >(&CCommandoPirate::JumpBack)},
    {"Dodge", static_cast< StateFunc >(&CCommandoPirate::Dodge)},
    {"ArmShield", static_cast< StateFunc >(&CCommandoPirate::ArmShield)},
    {"ShieldCharge", static_cast< StateFunc >(&CCommandoPirate::ShieldCharge)},
    {"ScriptedShieldCharge", static_cast< StateFunc >(&CCommandoPirate::ScriptedShieldCharge)},
    {"RestoreOrientation", static_cast< StateFunc >(&CCommandoPirate::RestoreOrientation)},
    {"Crouch", static_cast< StateFunc >(&CCommandoPirate::Crouch)},
    {"WallHang", static_cast< StateFunc >(&CCommandoPirate::WallHang)},
    {"WallDetach", static_cast< StateFunc >(&CCommandoPirate::WallDetach)},
    {"CoverFind", static_cast< StateFunc >(&CCommandoPirate::CoverFind)},
    {"SetCoverDest", static_cast< StateFunc >(&CCommandoPirate::SetCoverDest)},
    {"Cover", static_cast< StateFunc >(&CCommandoPirate::Cover)},
    {"CoverAttack", static_cast< StateFunc >(&CCommandoPirate::CoverAttack)},
    {"BreakCover", static_cast< StateFunc >(&CCommandoPirate::BreakCover)},
    {"GetUp", static_cast< StateFunc >(&CCommandoPirate::GetUp)},
    {"Dead", static_cast< StateFunc >(&CCommandoPirate::Dead)},
};

const SBurst CCommandoPirate::skBurstsA[] = {
    {15, {6, 5, 3, 2, 1, -1, 0, 0}, 0.1f, 0.05f}, {20, {1, 2, 3, 4, 5, -1, 0, 0}, 0.1f, 0.05f},
    {20, {7, 6, 5, 4, 3, -1, 0, 0}, 0.1f, 0.05f}, {15, {3, 4, 5, 6, 7, -1, 0, 0}, 0.1f, 0.05f},
    {15, {6, 5, 4, 3, 2, -1, 0, 0}, 0.1f, 0.05f}, {15, {2, 3, 4, 5, 6, -1, 0, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.f, 0.f},
};

const SBurst CCommandoPirate::skBurstsB[] = {
    {20, {16, 4, 8, -1, 0, 0, 0, 0}, 0.1f, 0.05f},
    {40, {5, 7, 9, -1, 0, 0, 0, 0}, 0.1f, 0.05f},
    {40, {1, 5, 10, -1, 0, 0, 0, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.f, 0.f},
};

const SBurst CCommandoPirate::skBurstsC[] = {
    {26, {16, 5, 8, 11, 14, -1, 0, 0}, 0.1f, 0.05f},
    {26, {16, 13, 12, 11, 8, -1, 0, 0}, 0.1f, 0.05f},
    {16, {9, 11, 13, 15, 2, -1, 0, 0}, 0.1f, 0.05f},
    {16, {14, 13, 12, 11, 10, -1, 0, 0}, 0.1f, 0.05f},
    {8, {10, 11, 12, 13, 14, -1, 0, 0}, 0.1f, 0.05f},
    {8, {6, 8, 11, 13, 15, -1, 0, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.f, 0.f},
};

const SBurst CCommandoPirate::skBurstsD[] = {
    {40, {7, 9, 13, -1, 0, 0, 0, 0}, 0.1f, 0.05f},
    {40, {9, 5, 1, -1, 0, 0, 0, 0}, 0.1f, 0.05f},
    {20, {16, 14, 12, -1, 0, 0, 0, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.f, 0.f},
};

const SBurst* CCommandoPirate::skBursts[] = {skBurstsA, skBurstsB, skBurstsC, skBurstsD, nullptr};

static EMaterialTypes GrenadeSolidMaterial = kMT_Solid;
static EMaterialTypes GrenadeProjectileMaterial = kMT_Projectile;
static CMaterialList skGrenadeMaterials(GrenadeSolidMaterial, GrenadeProjectileMaterial);
static int sRelUseCount = 0;

// Guessed name: the melee attack variants, selected by the angle to the target.
struct SMeleeVariant {
  pas::ESeverity mSeverity;
  float mMaxAngle;
  float mWeight;
};

static const SMeleeVariant skMeleeVariants[] = {
    {pas::kS_Zero, M_PIF / 2.f, 30.f},
    {pas::kS_One, M_PIF / 3.f, 30.f},
    {pas::kS_Three, M_PIF / 6.f, 40.f},
};

static const float skRagDollParticleRadii[] = {0.45f, 0.52f, 0.35f, 0.1f,  0.15f, 0.35f, 0.1f,
                                               0.15f, 0.15f, 0.15f, 0.15f, 0.15f, 0.15f, 0.15f};

static rstl::string skRELName = rstl::string_l("CommandoPirate.rel");

CCommandoPirateGrenade::CCommandoPirateGrenade(TUniqueId uid, const rstl::string& name,
                                               const CEntityInfo& info, const CTransform4f& xf,
                                               const CModelData& modelData,
                                               const CActorParameters& actorParams,
                                               TUniqueId parentId, const CCommandoGrenadeData& data,
                                               float velocity)
: CBouncyGrenade(uid, name, info, xf, modelData, actorParams, parentId, velocity, data, 0.f,
                 CAABox::MakeMaxInvertedBox(), kInvalidUniqueId, 0.f, 0, &skGrenadeMaterials,
                 nullptr)
, mData(data)
, mEMPTime(0.f)
, mRelToken(skRELName, 0) {
  ++sRelUseCount;
}

CCommandoPirateGrenade::~CCommandoPirateGrenade() { --sRelUseCount; }

void CCommandoPirateGrenade::Think(float dt, CStateManager& mgr) {
  UpdateGrenadeFX(dt, mgr);
  if (HasExploded()) {
    mEMPTime += dt;
    if (mData.mEMPDuration > 0.f) {
      for (uint i = 0; i < mgr.GetNumPlayers(); ++i) {
        const float distance = (mgr.GetPlayer(i)->GetTranslation() - GetTranslation()).Magnitude();
        if (distance < 20.f) {
          const float magnitude = CMath::Clamp(
              0.f,
              ((20.f - distance) / 20.f) * ((mData.mEMPDuration - mEMPTime) / mData.mEMPDuration),
              1.f);
          mgr.PlayerState(i)->StaticInterference().AddSource(GetUniqueId(), magnitude, 0.2f);
        }
      }
    }
    if (mEMPTime >= mData.mEMPDuration) {
      mgr.DeleteObjectRequest(GetUniqueId());
    }
  }
}

static CElementGen* CreateEffect(CAssetId id) {
  if (id == kInvalidAssetId) {
    return nullptr;
  }
  TLockedToken< CGenDescription > desc(gpSimplePool->GetObj(SObjectTag('PART', id)));
  return rs_new CElementGen(desc, CElementGen::kMOT_Normal, CElementGen::kOSF_One);
}

CCommandoPirate::CCommandoPirate(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                                 const CTransform4f& xf, const CModelData& modelData,
                                 const CActorParameters& actorParams,
                                 const CPatternedInfo& patternedInfo,
                                 const CCommandoPirateData& data)
: CPatterned(kPAI_CommandoPirate, uid, name, kFT_Zero, info, xf, modelData, patternedInfo,
             kMT_Ground, kCT_One, kBT_BiPedal, actorParams)
, mData(data)
, mWarpPhase(-1)
, mActionPhase(-1)
, mPathFindSearch(nullptr, 1, patternedInfo.GetPathfindingIndex(), 1.f, 1.f, 0,
                  CPFRegion::kRP_Center)
, mProjectileInfo(data.mProjectile, data.mProjectileDamage)
, mShieldCollisionMgr(nullptr)
, mBladeCollisionMgr(nullptr)
, mShieldVulnerability(data.mShield.mVulnerability)
, mBoneTracking(*GetAnimationData(), rstl::string_l("Head_1"), 70.f * (M_PIF / 180.f), M_PIF,
                kBTF_None)
, mLineOfSightTracker(GetUniqueId(), CSegId::Invalid(), 0.1f, 0.05f)
, mBurstFire(skBursts, 0)
, mBurstCount(0)
, mNextShieldChargeBurst(0)
, mNextArmShieldBurst(0)
, mBurstCooldown(0.f)
, mTimeSinceHeardShot(1.5f)
, mTimeSinceHit(0.75f)
, mDodgeCheckTimer(0.f)
, mCoverCheckTimer(0.f)
, mAggressionCheckTimer(0.f)
, mGrenadeAttackTimer(data.mGrenade.mMinAttackInterval)
, mJumpPointSearchTimer(2.f)
, mTimeSinceMelee(1.f)
, mArmShieldTimer(0.f)
, mChargeBlockedTime(0.f)
, mChargeDuration(0.f)
, mSeekCoverTime(0.f)
, mSeekCoverTimeLimit(0.f)
, mTargetPosition(CVector3f::Zero())
, mFaceDirection(CVector3f::Zero())
, mChargeUp(CVector3f::Zero())
, mChargeCollisionNormal(CVector3f::Zero())
, mTeamAiMgrId(kInvalidUniqueId)
, mShieldCollisionActorId(kInvalidUniqueId)
, mBladeCollisionActorId(kInvalidUniqueId)
, mTargetId(kInvalidUniqueId)
, mAlertedTargetId(kInvalidUniqueId)
, mRetreatWaypointId(kInvalidUniqueId)
, mJumpPointId(kInvalidUniqueId)
, mCoverPointId(kInvalidUniqueId)
, mLastCoverPointId(kInvalidUniqueId)
, mWallHangWaypointId(kInvalidUniqueId)
, mSpawnedObjectId(kInvalidUniqueId)
, mAttackPatternWaypointId(kInvalidUniqueId)
, mDodgeDir(pas::kSD_Invalid)
, mJumpBackDistance(5.f)
, mDodgeDistance(5.f)
, mJumpDistance(0.75f)
, mJumpApexHeight(0.f)
, mJumpDestination(CVector3f::Zero())
, mRagDoll(nullptr)
, mRagDollTimer(0.f)
, mHurledSfxHandle()
, mShieldExplodeEffect(
      mData.mShield.mExplodeEffect != kInvalidAssetId
          ? rstl::optional_object< TLockedToken< CGenDescription > >(
                gpSimplePool->GetObj(SObjectTag('PART', mData.mShield.mExplodeEffect)))
          : rstl::optional_object_null())
, mArmShieldExplodeEffect(
      mData.mShield.mArmExplodeEffect != kInvalidAssetId
          ? rstl::optional_object< TLockedToken< CGenDescription > >(
                gpSimplePool->GetObj(SObjectTag('PART', mData.mShield.mArmExplodeEffect)))
          : rstl::optional_object_null())
, mDrowningEffect(
      CAssetId(mData.mDrowningEffect) != kInvalidAssetId
          ? rstl::optional_object< TLockedToken< CGenDescription > >(
                gpSimplePool->GetObj(SObjectTag('PART', CAssetId(mData.mDrowningEffect))))
          : rstl::optional_object_null())
, mSfxHandle()
, mArmShieldEffect(CreateEffect(mData.mShield.mArmEffect))
, mShieldChargeEffect(CreateEffect(mData.mShield.mChargeEffect))
, mShieldEffectScale(0.f)
, mShieldEffectTargetScale(0.f)
, mShieldEffectType(-1)
, mRecentDamage(0.f)
, mMeleeVariant(-1)
, mWarpAnimTime(0.f)
, mHeadSeg(CSegId::Invalid())
, mLaunchSeg(CSegId::Invalid())
, mGunSeg(CSegId::Invalid())
, mGrenadeSeg(CSegId::Invalid())
, mRightWristSeg(CSegId::Invalid())
, mRightElbowSeg(CSegId::Invalid())
, mLeftWristSeg(CSegId::Invalid())
, mPathBlocked(false)
, mEnableRetreat(false)
, mWarpInRequested(false)
, mDeleteAfterWarpOut(false)
, mJumpVelSet(false)
, mEnableDodge(false)
, mEnableGrenadeAttack(false)
, mCoverCheck(false)
, mArmShieldWanted(false)
, mAggressive(false)
, mStopped(false)
, mAdditiveAimEnabled(false)
, mChargeStarted(false)
, mAbortShield(false)
, mChargeBlocked(false)
, mShieldBroken(false)
, mPathDestValid(true)
, mWarpTimeCaptured(false)
, mMeleeAttacking(false)
, mGettingUp(false)
, mArmingShield(false)
, mShieldCharging(false)
, mInWallHang(false)
, mEGrenadeAttacking(false)
, mFollowingAttackPattern(false)
, mSeekingCover(false)
, mShieldOn(false) {
  mProjectileInfo.Token().Lock();
  KnockBackController().EnableBurn(true);
  KnockBackController().EnableKnockBackPhysics(!mData.mNoKnockbackImpulseReset);

  const CAnimData* animData = GetAnimationData();
  mHeadSeg = animData->GetLocatorSegId(rstl::string_l("Head_1"));
  mLaunchSeg = animData->GetLocatorSegId(rstl::string_l("Mid_Launch_LCTR"));
  mGunSeg = animData->GetLocatorSegId(rstl::string_l("gun_LCTR"));
  mGrenadeSeg = animData->GetLocatorSegId(rstl::string_l("gun_LCTR"));
  mRightWristSeg = animData->GetLocatorSegId(rstl::string_l("R_wrist"));
  mRightElbowSeg = animData->GetLocatorSegId(rstl::string_l("R_elbow"));
  mLeftWristSeg = animData->GetLocatorSegId(rstl::string_l("L_wrist"));
  mLineOfSightTracker.SetSegment(mHeadSeg);

  mBoneTracking.SetDisableTrackingDistance(25.f * GetModelData()->GetScale().GetZ());
  mBurstFire.SetBurstType(0);

  const CPASAnimParmData jump(pas::kAS_Jump, CPASAnimParm::FromEnum(0), CPASAnimParm::FromEnum(0),
                              CPASAnimParm::FromEnum(0));
  mJumpDistance = GetModelData()->GetScale().GetY() * GetAnimationDistance(jump);
  const CPASAnimParmData dodge(pas::kAS_Step, CPASAnimParm::FromEnum(3), CPASAnimParm::FromEnum(2));
  mDodgeDistance = GetModelData()->GetScale().GetX() * GetAnimationDistance(dodge);
  const CPASAnimParmData step(pas::kAS_Step, CPASAnimParm::FromEnum(1), CPASAnimParm::FromEnum(2));
  mJumpBackDistance = GetModelData()->GetScale().GetY() * GetAnimationDistance(step);

  mShieldVulnerability = CPatterned::GetDamageVulnerability()->MakeIgnoreRadius();

  const float speed = GetBodyController()->GetBodyStateInfo().GetLocomotionSpeed(pas::kLA_BackUp);
  if (speed > 0.f) {
    mSeekCoverTimeLimit = 4.f * (mData.mSearchRadius / speed);
  }

  const CAABox widthBounds = GetBoundingBox();
  mPathFindSearch.SetCharacterRadius(widthBounds.GetMaxPoint().GetX() -
                                     widthBounds.GetMinPoint().GetX());
  const CAABox heightBounds = GetBoundingBox();
  mPathFindSearch.SetCharacterHeight(heightBounds.GetMaxPoint().GetZ() -
                                     heightBounds.GetMinPoint().GetZ());
}

CCommandoPirate::~CCommandoPirate() {}

CProjectileInfo* CCommandoPirate::ProjectileInfo() { return &mProjectileInfo; }

void CCommandoPirate::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const bool active = GetActive();
  const TUniqueId senderId = msg.GetSenderId();
  const EScriptObjectMessage message = msg.GetMessage();
  CPatterned::AcceptScriptMsg(mgr, msg);

  switch (message) {
  case kSM_Create:
    mBodyController->Activate(mgr, pas::kAS_Invalid);
    SetupCollisionActors(mgr);
    break;
  case kSM_Delete:
    mShieldCollisionMgr->Destroy(mgr);
    mBladeCollisionMgr->Destroy(mgr);
    ReleaseCoverPoint(mgr, mCoverPointId, true);
    mgr.DeleteObjectRequest(mSpawnedObjectId);
    QuitTeam(mgr);
    break;
  case kSM_Activate:
    if (mData.mTrooper) {
      if (!active && mWarpPhase == -1) {
        mColor.SetAlpha(0.f);
        mAlphaDelta = 0.f;
      }
      mWarpInRequested = true;
    }
    break;
  case kSM_Deactivate:
    if (active) {
      if (mShieldCollisionMgr.get() != nullptr) {
        mShieldCollisionMgr->SetActive(mgr, false);
      }
      if (mBladeCollisionMgr.get() != nullptr) {
        mBladeCollisionMgr->SetActive(mgr, false);
      }
    }
    QuitTeam(mgr);
    break;
  case kSM_AIUpdateDisabled:
    if (mShieldCollisionMgr.get() != nullptr) {
      mShieldCollisionMgr->SetPhysicsActive(mgr, false);
    }
    if (mBladeCollisionMgr.get() != nullptr) {
      mBladeCollisionMgr->SetPhysicsActive(mgr, false);
    }
    break;
  case kSM_AreaLoaded:
    if (mData.mFloatingCorpse) {
      mRagDollTimer = 0.01f;
      RemoveMaterial(kMT_Character, kMT_Solid, kMT_Target, kMT_Orbit, mgr);
      mAlive = false;
      HealthInfo()->SetHP(-1.f);
    } else {
      mRetreatWaypointId =
          FindConnectedObject_if(mgr, kSS_Retreat, kSM_Follow, CEffectWaypointPredicate());
      mPathFindSearch.SetArea(
          mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()).GetPostConstructed()->mPathArea);
    }
    break;
  case kSM_SetToZero:
    if (active && !mWarpInRequested) {
      mEnableRetreat = true;
      RequestWarpOut(mgr, false);
    }
    break;
  case kSM_Escape:
    if (active && !mWarpInRequested) {
      RequestWarpOut(mgr, true);
    }
    break;
  case kSM_OffGround:
    if (!mBodyController->IsFrozen()) {
      const float mass = GetMass();
      SetMomentumWR(CVector3f(0.f, 0.f, -GetGravityConstant() * mass));
    }
    mBurstFire.SetBurstType(1);
    break;
  case kSM_Launching:
    if (mBodyController->GetCurrentStateId() != pas::kAS_Hurled) {
      CPatterned::AcceptScriptMsg(mgr, CScriptMsg(senderId, GetUniqueId(), kSM_OffGround));
      const float mass = GetMass();
      SetMomentumWR(CVector3f(0.f, 0.f, -GetGravityConstant() * mass));
      SetVelocityForJump();
    }
    break;
  case kSM_Landed:
    mBurstFire.SetBurstType(0);
    mJumpVelSet = false;
    break;
  case kSM_Start:
    mStopped = false;
    break;
  case kSM_Stop:
    mStopped = true;
    break;
  case kSM_Alert:
    mHitByPlayerProjectile = true;
    break;
  case kSM_Damage:
    if (mArmingShield || mShieldCharging) {
      HandleShieldHit(mgr, senderId);
    }
    if (mSeekingCover) {
      mSeekCoverTime += 5.f;
    }
    if (const CWeapon* weapon = TCastToConstPtr< CWeapon >(mgr.GetObjectById(senderId))) {
      mRecentDamage += weapon->GetCurrentDamageInfo().GetDamage(*GetDamageVulnerability());
    }
    mgr.InformListeners(GetTranslation(), kLNT_PlayerFire);
    if (!mAdditiveAimEnabled) {
      mBodyController->AbortScriptedAnimations();
    }
    break;
  case kSM_ResistedDamage:
    if ((mArmingShield || mShieldCharging) && senderId == mShieldCollisionActorId) {
      if (const CCollisionActor* actor =
              TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(mShieldCollisionActorId))) {
        const TUniqueId touchedId = actor->GetLastTouchedObject();
        const CDamageVulnerability vulnerability = *actor->GetDamageVulnerability();
        if (const CWeapon* weapon = TCastToConstPtr< CWeapon >(mgr.GetObjectById(touchedId))) {
          if (vulnerability.WeaponHurts(weapon->GetCurrentDamageInfo().GetWeaponMode())) {
            HandleShieldHit(mgr, mShieldCollisionActorId);
          }
        }
      }
    }
    mgr.InformListeners(GetTranslation(), kLNT_PlayerFire);
    if (!mAdditiveAimEnabled) {
      mBodyController->AbortScriptedAnimations();
    }
    break;
  case kSM_HitObject:
    ApplyChargeDamage(senderId, mgr);
    break;
  default:
    break;
  }
}

void CCommandoPirate::PreThink(float dt, CStateManager& mgr) {
  if (!mRagDoll.get() || !mRagDoll->IsPrimed()) {
    mBoneTracking.PreThink(*AnimationData());
  }
  CPatterned::PreThink(dt, mgr);
}

void CCommandoPirate::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  const bool noRagDoll = !mRagDoll.get();
  if (noRagDoll || !mRagDoll->IsPrimed()) {
    CPatterned::Think(dt, mgr);
    mShieldCollisionMgr->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
    mBladeCollisionMgr->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
    if (!mBodyController->IsFrozen()) {
      mBoneTracking.Think(dt);
    }
    mLineOfSightTracker.Update(dt, mgr);
    UpdateBurstFire(dt, mgr);
    UpdateAdditiveAim(mgr);
    UpdateTimers(dt, mgr);
    CheckDrowning(mgr);
  } else {
    CActor::Think(dt, mgr);
    UpdateAlphaDelta(mgr, dt);
    UpdateHitDamageTime(dt);
    if (mBodyController->IsFrozen()) {
      mBodyController->UnFreeze();
    }
  }

  UpdateShieldEffects(mgr, dt);
  UpdateEmitters();
  ThinkRagDoll(dt, mgr, noRagDoll);
}

void CCommandoPirate::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                      EUserEventType type, float dt) {
  bool handled = false;
  switch (type) {
  case kUE_DeGenerate:
  case kUE_BecomeRagDoll:
    if (GetHealthInfo()->GetHP() <= 0.f) {
      mRagDollTimer = 0.05f * mgr.Random()->Float() + 0.001f;
    }
    handled = true;
    break;
  case kUE_Projectile:
    LaunchGrenade(mgr);
    handled = true;
    break;
  case kUE_TakeOff:
    if (mShieldCharging) {
      mChargeStarted = true;
    }
    handled = true;
    break;
  case kUE_Landing:
    if (mShieldCharging) {
      mChargeStarted = false;
    }
    handled = true;
    break;
  case kUE_Activate:
    if (mShieldCharging || mArmingShield) {
      TurnShieldOn();
    }
    handled = true;
    break;
  case kUE_Deactivate:
    if (mShieldCharging || mArmingShield) {
      TurnShieldOff();
    }
    handled = true;
    break;
  case kUE_DamageOn:
    mCurDamageRemTime = 0.f;
    handled = true;
    break;
  case kUE_BreakLockOn:
    handled = true;
    break;
  default:
    break;
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

void CCommandoPirate::PreRender(CStateManager& mgr) {
  if (mRagDoll.get() != nullptr && mRagDoll->IsPrimed()) {
    mRagDoll->PreRender(GetTranslation(), *ModelData());
  }
  CPatterned::PreRender(mgr);
  if (mRagDoll.get() == nullptr || !mRagDoll->IsPrimed()) {
    mBoneTracking.PreRender(mgr, *AnimationData(), GetTransform(), GetModelData()->GetScale(),
                            *mBodyController);
  }
}

void CCommandoPirate::Render(const CStateManager& mgr) const {
  const float alpha = mColor.GetAlpha();
  if (mAlive && mWarpPhase == 1) {
    const float warp = CMath::FastSinR(M_PIF * alpha);
    if (warp > 0.f) {
      mgr.DrawSpaceWarp(GetBoundingBox().GetCenterPoint(), warp);
    }
  }
  if (alpha > 0.f) {
    CPatterned::Render(mgr);
    switch (mShieldEffectType) {
    case 0:
      if (mArmShieldEffect.get() != nullptr) {
        mArmShieldEffect->Render();
      }
      break;
    case 1:
      if (mShieldChargeEffect.get() != nullptr) {
        mShieldChargeEffect->Render();
      }
      break;
    }
  }
}

void CCommandoPirate::PreRenderAllViewports(CStateManager& mgr) {
  if (mRagDoll.get() != nullptr && mRagDoll->IsPrimed()) {
    mRagDoll->PreRenderAllViewports(*this, 0.2f);
    UpdatePortalSystemState(mgr);
  } else {
    CPatterned::PreRenderAllViewports(mgr);
  }
}

void CCommandoPirate::KnockBack(CStateManager& mgr, const CKnockBackInfo& info) {
  if (mData.mFloatingCorpse && mRagDoll.get()) {
    const float power = info.GetDamageInfo().GetKnockBackPower(*GetDamageVulnerability(), 0.f);
    mRagDoll->TorsoImpulse() += (200000.f * power) * info.GetDirection();
  } else if (!mRagDoll.get()) {
    KnockBackController().EnableAnimReaction(CKnockBackMgr::kAR_KnockBack, IsOnGround());
    CPatterned::KnockBack(mgr, info);
    if (mAlive) {
      if (KnockBackController().GetActiveReaction() == CKnockBackMgr::kAR_Hurled) {
        mStateMachine->SetState(mgr, *this, rstl::string_l("GetUp"));
        mHurledSfxHandle = CSfxManager::AddEmitter(mData.mSound_Hurled, GetTranslation(), 127,
                                                   GetCurrentAreaId().Value(), true, false,
                                                   CSfxManager::kMedPriority);
      }
    } else if (KnockBackController().GetActiveReaction() == CKnockBackMgr::kAR_Hurled &&
               KnockBackController().GetFollowUp() != CKnockBackMgr::kFU_LaggedBurnDeath &&
               KnockBackController().GetFollowUp() != CKnockBackMgr::kFU_BurnDeath &&
               KnockBackController().GetFollowUp() != CKnockBackMgr::kFU_ExplodeDeath &&
               KnockBackController().GetFollowUp() != CKnockBackMgr::kFU_IceDeath) {
      mHurledSfxHandle = CSfxManager::AddEmitter(mData.mSound_Death, GetTranslation(), 127,
                                                 GetCurrentAreaId().Value(), true, false,
                                                 CSfxManager::kMedPriority);
    }
  }
}

const CDamageVulnerability* CCommandoPirate::GetDamageVulnerability() const {
  if (mShieldCharging) {
    return &mShieldVulnerability;
  }
  return CPatterned::GetDamageVulnerability();
}

CDamageInfo CCommandoPirate::GetContactDamage() const {
  if (mShieldCharging) {
    return mData.mShield.mChargeDamage;
  }
  if (mMeleeAttacking) {
    return mData.mBladeDamage;
  }
  return CPatterned::GetContactDamage();
}

static EMaterialTypes ChargeCollideSolidMaterial = kMT_Solid;
static EMaterialTypes ChargeBlockerCeilingMaterial = kMT_Ceiling;
static EMaterialTypes ChargeBlockerWallMaterial = kMT_Wall;

void CCommandoPirate::CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                                   CStateManager& mgr) {
  CPatterned::CollidedWith(id, list, mgr);
  if (mShieldCharging) {
    if (id != kInvalidUniqueId) {
      if (TCastToConstPtr< CPlayer >(mgr.GetObjectById(id)) == nullptr) {
        mgr.ApplyDamage(id, id, GetUniqueId(), mData.mShield.mChargeDamage,
                        CMaterialFilter::MakeInclude(CMaterialList(ChargeCollideSolidMaterial)),
                        GetTransform().GetColumn(kDY));
      }
      mChargeBlocked = true;
    } else {
      static CMaterialList blockers(ChargeBlockerCeilingMaterial, ChargeBlockerWallMaterial);
      const CVector3f up = GetTransform().GetColumn(kDZ);
      for (int i = 0; i < list.GetCount(); ++i) {
        const CCollisionInfo& info = list[i];
        const CVector3f& normal = info.GetNormalLeft();
        if (info.GetMaterialLeft().SharesMaterials(blockers) ||
            CVector3f::Dot(up, normal) < 0.707f) {
          mChargeBlocked = true;
          mChargeCollisionNormal = normal;
          return;
        }
        if (info.GetMaterialLeft().HasMaterial(kMT_Floor)) {
          mChargeCollisionNormal = normal;
        }
      }
    }
  }
}

bool CCommandoPirate::Listen(CStateManager& mgr, const CVector3f& position, EListenNoiseType type) {
  bool heard = false;
  if (mAlive) {
    switch (type) {
    case kLNT_PathObstruction: {
      const CVector3f diff = position - GetTranslation();
      if (diff.MagSquared() < 1600.f) {
        mPathBlocked = heard = true;
      }
      break;
    }
    case kLNT_PlayerFire: {
      const float radiusSq = mData.mHearingRadius * mData.mHearingRadius;
      const CVector3f diff = position - GetTranslation();
      const float distSq = diff.MagSquared();
      if (distSq < radiusSq) {
        const float rangeSq = mDetectionHeightRange * mDetectionHeightRange;
        if (mDetectionHeightRange == 0.f || distSq < rangeSq) {
          heard = true;
          mTimeSinceHeardShot = 0.f;
        }
      }
      break;
    }
    default:
      break;
    }
  }
  return heard;
}

void CCommandoPirate::SetupStateMachine(CStateManager& mgr) {
  StateMachine* stateMachine = mStateMachine.get();
  CPatterned::SetupStateMachine(mgr);
  stateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
  stateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
}

bool CCommandoPirate::StateOver(CStateManager&, const CTriggerData&) const {
  return mAnimationState.GetState() == CAnimationState::kAS_Over;
}

bool CCommandoPirate::ShouldWarpIn(CStateManager&, const CTriggerData&) const {
  return mWarpPhase == -1 && mData.mTrooper && mWarpInRequested;
}

bool CCommandoPirate::ShouldMeleeAttack(CStateManager& mgr, const CTriggerData&) const {
  if (!mData.mNoMeleeAttack && mTimeSinceMelee > 1.f) {
    const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
    if (target != nullptr) {
      const float minSq = mMinAttackRange * mMinAttackRange;
      const float maxSq = mMaxAttackRange * mMaxAttackRange;
      const CVector3f diff = target->GetTranslation() - GetTranslation();
      const float distSq = diff.MagSquared();
      if (distSq >= minSq && distSq <= maxSq) {
        return fabsf(diff.GetZ()) < 3.f;
      }
    }
  }
  return false;
}

bool CCommandoPirate::ShouldFireEGrenade(CStateManager& mgr, const CTriggerData&) const {
  if (mEnableGrenadeAttack && sRelUseCount == 0 && !mData.mNoGrenadeAttack) {
    CVector3f position = CVector3f::Zero();
    if (GetTargetAimPosition(mgr, position, 0.f)) {
      const float minSq = mData.mGrenade.mMinAttackDist * mData.mGrenade.mMinAttackDist;
      const float maxSq = mData.mGrenade.mMaxAttackDist * mData.mGrenade.mMaxAttackDist;
      const CVector3f diff = position - GetTranslation();
      const float distSq = diff.MagSquared();
      if (distSq >= minSq && distSq <= maxSq) {
        return true;
      }
    }
  }
  return false;
}

bool CCommandoPirate::ShouldRetreat(CStateManager&, const CTriggerData&) const {
  if (mEnableRetreat && !mData.mTrooper && !mData.mStationary) {
    return mRetreatWaypointId != kInvalidUniqueId;
  }
  return false;
}

bool CCommandoPirate::ShouldJumpBack(CStateManager& mgr, const CTriggerData&) const {
  if (!mData.mStationary) {
    return IsPathClear(mgr, -GetTransform().GetForward(), mJumpBackDistance);
  }
  return false;
}

bool CCommandoPirate::ShouldAmbush(CStateManager&, const CTriggerData&) const {
  return mData.mPendingAmbush;
}

bool CCommandoPirate::BreakAmbush(CStateManager&, const CTriggerData&) const {
  return mData.mBreakAmbush;
}

bool CCommandoPirate::HasLineOfSight(CStateManager&, const CTriggerData&) const {
  return mLineOfSightTracker.HasLineOfSight();
}

bool CCommandoPirate::UnderFire(CStateManager&, const CTriggerData&) const {
  return mTimeSinceHit < 0.75f;
}

bool CCommandoPirate::HeardShot(CStateManager&, const CTriggerData&) const {
  return mTimeSinceHeardShot < 1.5f;
}

bool CCommandoPirate::HasTarget(CStateManager& mgr, const CTriggerData&) const {
  return mgr.GetObjectById(mTargetId) != nullptr;
}

bool CCommandoPirate::HasNewTarget(CStateManager& mgr, const CTriggerData& data) const {
  bool hasNewTarget = false;
  if (HasTarget(mgr, data) && mAlertedTargetId != mTargetId) {
    hasNewTarget = true;
  }
  return hasNewTarget;
}

bool CCommandoPirate::PathShagged(CStateManager& mgr, const CTriggerData& data) const {
  bool pathShagged = false;
  if (mPathBlocked || CPatterned::PathShagged(mgr, data)) {
    pathShagged = true;
  }
  return pathShagged;
}

bool CCommandoPirate::PathOver(CStateManager& mgr, const CTriggerData& data) const {
  bool pathOver = false;
  if (!mPathDestValid || CPatterned::PathOver(mgr, data)) {
    pathOver = true;
  }
  return pathOver;
}

bool CCommandoPirate::IsOffPath(CStateManager&, const CTriggerData&) const {
  return mPathFindSearch.OnPath(GetTranslation()) != CPathFindSearch::kR_Success;
}

bool CCommandoPirate::HasAttackPattern(CStateManager& mgr, const CTriggerData&) const {
  return GetConnectedObject(mgr, kSS_Attack, kSM_Follow) != kInvalidUniqueId;
}

bool CCommandoPirate::IsFacingTarget(CStateManager& mgr, const CTriggerData&) const {
  CVector3f position = CVector3f::Zero();
  if (GetTargetAimPosition(mgr, position, 0.f)) {
    CVector3f toTarget = position - GetTranslation();
    toTarget.SetZ(0.f);
    return CVector3f::GetAngleDiff(GetTransform().GetForward(), toTarget) <= 20.f * (M_PIF / 180.f);
  }
  return true;
}

bool CCommandoPirate::AttackPatternOver(CStateManager&, const CTriggerData&) const {
  return mWaypointNavigation.GetDestination() == kInvalidUniqueId;
}

bool CCommandoPirate::TooClose(CStateManager& mgr, const CTriggerData&) const {
  const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
  if (target != nullptr) {
    const float minSq = mMinAttackRange * mMinAttackRange;
    const CVector3f diff = target->GetTranslation() - GetTranslation();
    return diff.MagSquared() < minSq;
  }
  return false;
}

bool CCommandoPirate::ShouldDodge(CStateManager& mgr, const CTriggerData&) const {
  if (!mData.mNoDodge && mEnableDodge) {
    const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
    if (target != nullptr) {
      const CVector3f toTarget = target->GetTranslation() - GetTranslation();
      const CVector3f forward = GetTransform().GetForward();
      if (CVector3f::Dot(toTarget, forward) > 0.f &&
          (mTimeSinceHit < 0.75f || mTimeSinceHeardShot < 1.5f)) {
        return true;
      }
    }
  }
  return false;
}

bool CCommandoPirate::ShouldArmShield(CStateManager&, const CTriggerData&) const {
  return mArmShieldWanted && !mData.mNoArmShield && !mShieldBroken;
}

static EMaterialTypes ShieldChargeSolidMaterial = kMT_Solid;
static EMaterialTypes ShieldChargePlayerMaterial = kMT_Player;
static EMaterialTypes ShieldChargePlatformMaterial = kMT_Platform;

bool CCommandoPirate::ShouldShieldCharge(CStateManager& mgr, const CTriggerData&) const {
  if (mAggressive && !mData.mNoShieldCharge && !mShieldBroken &&
      mBurstCount > mNextShieldChargeBurst) {
    const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
    if (target != nullptr) {
      const CVector3f diff = target->GetTranslation() - GetTranslation();
      if (fabsf(diff.GetZ()) <= 5.f) {
        const float minSq = mData.mShield.mChargeMinAttackDist * mData.mShield.mChargeMinAttackDist;
        const float maxSq = mData.mShield.mChargeMaxAttackDist * mData.mShield.mChargeMaxAttackDist;
        const float distSq = diff.MagSquared();
        if (distSq >= minSq && distSq <= maxSq && diff.IsMagnitudeSafe()) {
          if (mTeamAiMgrId == kInvalidUniqueId ||
              CScriptTeamAiMgr::CanStartAttack(CScriptTeamAiMgr::kAT_Melee, mgr, mTeamAiMgrId,
                                               GetUniqueId())) {
            const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
                CMaterialList(ShieldChargeSolidMaterial),
                CMaterialList(ShieldChargePlayerMaterial, ShieldChargePlatformMaterial));
            const CVector3f dir = diff.AsNormalized();
            const CVector3f right = GetTransform().GetRight();
            const CAABox bounds = GetBoundingBox();
            const float width = bounds.GetMaxPoint().GetX() - bounds.GetMinPoint().GetX();
            const CVector3f center = GetBoundingBox().GetCenterPoint();
            const CVector3f rightPoint = center + width * right;
            const CVector3f leftPoint = center - width * right;
            const float dist = CMath::SqrtF(distSq);
            if (mgr.RayCollideWorld(center, center + dist * dir, filter, this) &&
                mgr.RayCollideWorld(rightPoint, rightPoint + dist * dir, filter, this) &&
                mgr.RayCollideWorld(leftPoint, leftPoint + dist * dir, filter, this)) {
              return true;
            }
          }
        }
      }
    }
  }
  return false;
}

bool CCommandoPirate::ShouldBoost(CStateManager& mgr, const CTriggerData&) const {
  if (mFollowingAttackPattern && !mShieldBroken) {
    const CScriptAIWaypoint* waypoint =
        TCastToPtr< CScriptAIWaypoint >(mgr.ObjectById(mWaypointNavigation.GetDestination()));
    if (waypoint != nullptr && (waypoint->GetFlags() & 0x20) != 0) {
      return true;
    }
  }
  return false;
}

bool CCommandoPirate::AbortShieldCharge(CStateManager&, const CTriggerData&) const {
  return mAbortShield;
}

bool CCommandoPirate::IsAggressive(CStateManager&, const CTriggerData&) const {
  return mAggressive;
}

bool CCommandoPirate::FoundJumpPoint(CStateManager&, const CTriggerData&) const {
  return mJumpPointId != kInvalidUniqueId;
}

bool CCommandoPirate::ShouldCrouch(CStateManager& mgr, const CTriggerData&) const {
  const CScriptCoverPoint* coverPoint = GetCoverPoint(mgr, mCoverPointId);
  return coverPoint != nullptr ? coverPoint->ShouldCrouch() : false;
}

bool CCommandoPirate::ShouldWallHang(CStateManager& mgr, const CTriggerData&) const {
  const CScriptCoverPoint* coverPoint = GetCoverPoint(mgr, mCoverPointId);
  return coverPoint != nullptr && coverPoint->ShouldWallHang();
}

bool CCommandoPirate::ShouldCover(CStateManager&, const CTriggerData&) const {
  if (mData.mStationary) {
    return false;
  }
  return mCoverCheck;
}

bool CCommandoPirate::FoundCover(CStateManager&, const CTriggerData&) const {
  return mCoverPointId != kInvalidUniqueId;
}

bool CCommandoPirate::ShouldCoverAttack(CStateManager&, const CTriggerData&) const {
  return mBurstCooldown <= 0.f;
}

bool CCommandoPirate::CoverBlown(CStateManager& mgr, const CTriggerData&) const {
  CVector3f position = CVector3f::Zero();
  if (GetTargetAimPosition(mgr, position, 1.f)) {
    const float minSq = mMinAttackRange * mMinAttackRange;
    const CVector3f diff = position - GetTranslation();
    if (diff.MagSquared() > minSq) {
      const CScriptCoverPoint* coverPoint = GetCoverPoint(mgr, mCoverPointId);
      if (coverPoint != nullptr) {
        return coverPoint->Blown(position);
      }
    }
  }
  return true;
}

bool CCommandoPirate::AbortSeekCover(CStateManager&, const CTriggerData&) const {
  return mSeekCoverTime >= mSeekCoverTimeLimit;
}

void CCommandoPirate::Alert(CStateManager&, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mAlertedTargetId = mTargetId;
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_Taunt)) {
      if (mBodyController->GetLocomotionType() == pas::kLT_Relaxed) {
        mBodyController->CommandMgr().DeliverCmd(CBCTauntCmd(pas::kTT_Two));
      } else {
        mBodyController->CommandMgr().DeliverCmd(CBCTauntCmd(pas::kTT_Four));
      }
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  default:
    break;
  }
}

void CCommandoPirate::Lurk(CStateManager&, EStateMsg msg, float) {
  if (msg == kStateMsg_Activate && mTargetId != kInvalidUniqueId) {
    mAdditiveAimEnabled = true;
  }
}

void CCommandoPirate::Ambush(CStateManager&, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mBodyController->SetLocomotionType(pas::kLT_Crouch);
    break;
  default:
    break;
  }
}

void CCommandoPirate::FaceTarget(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    if (GetTargetAimPosition(mgr, mTargetPosition, 0.5f)) {
      mTargetPosition.SetZ(GetTranslation().GetZ());
      const CVector3f toTarget = mTargetPosition - GetTranslation();
      if (CVector3f::GetAngleDiff(GetTransform().GetForward(), toTarget) > 20.f * (M_PIF / 180.f)) {
        mAnimationState.SetState(CAnimationState::kAS_Ready);
      } else {
        mAnimationState.SetState(CAnimationState::kAS_Over);
      }
    } else {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    }
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_Turn)) {
      const CVector3f toTarget = mTargetPosition - GetTranslation();
      if (toTarget.IsMagnitudeSafe()) {
        mBodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), toTarget.AsNormalized(), 1.f));
      } else {
        mAnimationState.SetState(CAnimationState::kAS_Over);
      }
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  default:
    break;
  }
}

void CCommandoPirate::SelectTarget(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    if (mgr.GetObjectById(mTargetId) == nullptr) {
      mTargetId = mgr.GetPlayer(0)->GetUniqueId();
      mLineOfSightTracker.SetTarget(mTargetId);
      mBoneTracking.SetActive(true);
      mBoneTracking.SetTarget(mTargetId);
      SendScriptMsgs(kSS_Attack, mgr, kInvalidUniqueId, kSM_None);
      mBodyController->SetLocomotionType(pas::kLT_Combat);
    }
    break;
  default:
    break;
  }
}

void CCommandoPirate::SetTargetDest(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == kStateMsg_Activate) {
    CVector3f destination = GetTranslation();
    mPathDestValid = false;
    const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
    if (target != nullptr && (mAggressive || !mLineOfSightTracker.HasLineOfSight())) {
      destination = target->GetTranslation();
      mPathDestValid = true;
    }
    mPathFindNavigation.SetDestination(destination);
    mPathFindNavigation.SetFaceTarget(kInvalidUniqueId);
    JoinTeam(mgr);
  }
}

void CCommandoPirate::SetRetreatDest(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == kStateMsg_Activate) {
    const CActor* waypoint = static_cast< const CActor* >(mgr.GetObjectById(mRetreatWaypointId));
    mPathDestValid = false;
    if (waypoint != nullptr) {
      mPathFindNavigation.SetDestination(waypoint->GetTranslation());
      mPathDestValid = true;
    } else {
      mPathFindNavigation.SetDestination(GetTranslation());
    }
    mPathFindNavigation.SetFaceTarget(kInvalidUniqueId);
    mCoverCheck = true;
  }
}

void CCommandoPirate::SetJumpDest(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == kStateMsg_Activate) {
    const CActor* jumpPoint = static_cast< const CActor* >(mgr.GetObjectById(mJumpPointId));
    mPathDestValid = false;
    if (jumpPoint != nullptr) {
      mPathFindNavigation.SetDestination(jumpPoint->GetTranslation());
      mPathDestValid = true;
    } else {
      mPathFindNavigation.SetDestination(GetTranslation());
    }
    mPathFindNavigation.SetFaceTarget(kInvalidUniqueId);
  }
}

void CCommandoPirate::SetPathMeshDest(CStateManager&, EStateMsg msg, float) {
  if (msg == kStateMsg_Activate) {
    CVector3f destination = CVector3f::Zero();
    mPathDestValid = false;
    if (mPathFindSearch.FindClosestReachablePoint(GetTranslation(), destination) ==
        CPathFindSearch::kR_Success) {
      mPathFindNavigation.SetDestination(destination);
      mPathDestValid = true;
    } else {
      mPathFindNavigation.SetDestination(GetTranslation());
    }
    mPathFindNavigation.SetFaceTarget(kInvalidUniqueId);
  }
}

void CCommandoPirate::SetCoverDest(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == kStateMsg_Activate) {
    const CScriptCoverPoint* coverPoint = GetCoverPoint(mgr, mCoverPointId);
    if (coverPoint != nullptr) {
      mPathFindNavigation.SetDestination(coverPoint->GetTranslation());
    } else {
      mPathFindNavigation.SetDestination(GetTranslation());
    }
    mPathFindNavigation.SetFaceTarget(mTargetId);
    if (!mSeekingCover) {
      mSeekingCover = true;
      mSeekCoverTime = 0.f;
    }
  }
}

void CCommandoPirate::MeleeAttack(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mMeleeVariant = SelectMeleeVariant(mgr);
    if (GetTargetAimPosition(mgr, mTargetPosition, 0.f) && mMeleeVariant != -1) {
      if (mTeamAiMgrId == kInvalidUniqueId ||
          CScriptTeamAiMgr::StartAttack(CScriptTeamAiMgr::kAT_Melee, mgr, mTeamAiMgrId,
                                        GetUniqueId())) {
        mAnimationState.SetState(CAnimationState::kAS_Ready);
        mBodyController->SetLocomotionType(pas::kLT_Combat);
        mMeleeAttacking = true;
      } else {
        mAnimationState.SetState(CAnimationState::kAS_Over);
      }
    } else {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    }
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_MeleeAttack)) {
      mBodyController->CommandMgr().DeliverCmd(
          CBCMeleeAttackCmd(skMeleeVariants[mMeleeVariant].mSeverity));
      mBladeCollisionMgr->SetActive(mgr, true);
    } else {
      mBodyController->CommandMgr().SetTargetVector(mTargetPosition - GetTranslation());
    }
    break;
  case kStateMsg_Deactivate:
    mMeleeAttacking = false;
    mTimeSinceMelee = 0.f;
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    CScriptTeamAiMgr::EndAttack(CScriptTeamAiMgr::kAT_Melee, mgr, mTeamAiMgrId, GetUniqueId(),
                                false);
    mBladeCollisionMgr->SetActive(mgr, false);
    break;
  default:
    break;
  }
}

void CCommandoPirate::EGrenadeAttack(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mEGrenadeAttacking = true;
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_ProjectileAttack)) {
      if (GetTargetAimPosition(mgr, mTargetPosition, 0.f)) {
        mBodyController->CommandMgr().DeliverCmd(
            CBCProjectileAttackCmd(pas::kS_Two, mTargetPosition, false));
      }
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mGrenadeAttackTimer = 0.f;
    mEGrenadeAttacking = false;
    break;
  default:
    break;
  }
}

void CCommandoPirate::PostEGrenadeAttack(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    if (mgr.GetObjectById(mTargetId) != nullptr) {
      mAnimationState.SetState(CAnimationState::kAS_Ready);
    } else {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    }
    break;
  case kStateMsg_Update:
    if (mStateMachine->GetTime() >= mData.mGrenade.mPostAttackPause) {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    } else if (GetTargetAimPosition(mgr, mTargetPosition, 0.5f)) {
      const CVector3f toTarget = mTargetPosition - GetTranslation();
      if (toTarget.IsMagnitudeSafe()) {
        if (CVector3f::GetAngleDiff(GetTransform().GetForward(), toTarget) >
            20.f * (M_PIF / 180.f)) {
          mBodyController->CommandMgr().DeliverCmd(
              CBCLocomotionCmd(CVector3f::Zero(), toTarget.AsNormalized(), 1.f));
        }
      }
    } else {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  default:
    break;
  }
}

void CCommandoPirate::JumpPointFind(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == kStateMsg_Activate) {
    mJumpPointId = kInvalidUniqueId;
    if (mJumpPointSearchTimer >= 2.f && GetTargetAimPosition(mgr, mTargetPosition, 0.f)) {
      mJumpPointSearchTimer = 0.f;
      float closestDistSq = FLT_MAX;
      CObjectList& list = mgr.ObjectListById(kOL_AiWaypoint);
      const CVector3f position = GetTranslation();
      for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
        CScriptAiJumpPoint* jumpPoint = TCastToPtr< CScriptAiJumpPoint >(list[i]);
        if (jumpPoint != nullptr && jumpPoint->GetActive() && !jumpPoint->GetInUse(GetUniqueId()) &&
            jumpPoint->GetType() == 0 && jumpPoint->GetJumpTarget() == kInvalidUniqueId &&
            jumpPoint->GetCurrentAreaId() == GetCurrentAreaId()) {
          const CScriptWaypoint* waypoint =
              TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(jumpPoint->GetJumpPoint()));
          if (waypoint != nullptr) {
            const CVector3f diff = waypoint->GetTranslation() - mTargetPosition;
            const float distSq = diff.MagSquared();
            if (distSq < closestDistSq &&
                GetSearchPath()->PathExists(position, jumpPoint->GetTranslation()) ==
                    CPathFindSearch::kR_Success) {
              closestDistSq = distSq;
              mJumpPointId = jumpPoint->GetUniqueId();
            }
          }
        }
      }
    }
  }
}

void CCommandoPirate::JetBoost(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    mAnimationState.SetState(CAnimationState::kAS_Over);
    const CScriptAiJumpPoint* jumpPoint =
        TCastToConstPtr< CScriptAiJumpPoint >(mgr.GetObjectById(mJumpPointId));
    if (jumpPoint != nullptr) {
      const CScriptWaypoint* waypoint =
          TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(jumpPoint->GetJumpPoint()));
      if (waypoint != nullptr) {
        mJumpDestination = waypoint->GetTranslation();
        mFaceDirection = jumpPoint->GetTransform().GetForward();
        mJumpApexHeight = jumpPoint->GetJumpApex();
        mAnimationState.SetState(CAnimationState::kAS_Ready);
      }
    }
    break;
  }
  case kStateMsg_Update: {
    bool canJump = true;
    if (mAnimationState.GetState() == CAnimationState::kAS_Ready) {
      if (CVector3f::GetAngleDiff(GetTransform().GetForward(), mFaceDirection) >
          20.f * (M_PIF / 180.f)) {
        canJump = false;
        mBodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), mFaceDirection, 1.f));
      }
    }
    if (canJump && mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_Jump)) {
      mBodyController->CommandMgr().DeliverCmd(CBCJumpCmd(
          mJumpDestination, pas::kJT_Normal, pas::kJS_IntoJump, 0, CBCJumpCmd::kFF_AmbushJump));
    }
    break;
  }
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  default:
    break;
  }
}

void CCommandoPirate::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mBodyController->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
    mBodyController->CommandMgr().SetSteeringSpeedRange(1.f, 1.f);
    mPathBlocked = false;
    break;
  case kStateMsg_Deactivate:
    mBodyController->CommandMgr().SetSteeringBlendMode(kSBM_Normal);
    mPathDestValid = true;
    break;
  default:
    break;
  }
  if (!mData.mStationary && mPathDestValid) {
    mPathFindNavigation.PathFind(mgr, msg, dt, *this);
    ApplySeparation(mgr);
  }
}

void CCommandoPirate::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    mBodyController->SetLocomotionType(pas::kLT_Relaxed);
    mBodyController->SetTurnSpeed(mBodyController->GetTurnSpeed() * 0.5f);
    CBodyController* controller = mBodyController.get();
    const float runSpeed = controller->GetBodyStateInfo().GetLocomotionSpeed(pas::kLA_Run);
    const float walkSpeed = controller->GetBodyStateInfo().GetLocomotionSpeed(pas::kLA_Walk);
    const float speedRatio = walkSpeed / runSpeed;
    mBodyController->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
    mBodyController->CommandMgr().SetSteeringSpeedRange(speedRatio, speedRatio);
    break;
  }
  case kStateMsg_Deactivate:
    mBodyController->SetLocomotionType(pas::kLT_Combat);
    mBodyController->SetTurnSpeed(2.f * mBodyController->GetTurnSpeed());
    mBodyController->CommandMgr().SetSteeringBlendMode(kSBM_Normal);
    break;
  default:
    break;
  }
  mWaypointNavigation.Patrol(mgr, msg, dt, *this);
}

void CCommandoPirate::FollowAttackPattern(CStateManager& mgr, EStateMsg msg, float dt) {
  mWaypointNavigation.Patrol(mgr, msg, dt, *this);
  switch (msg) {
  case kStateMsg_Activate: {
    const TUniqueId destination = mAttackPatternWaypointId != kInvalidUniqueId
                                      ? mAttackPatternWaypointId
                                      : GetConnectedObject(mgr, kSS_Attack, kSM_Follow);
    mWaypointNavigation.SetDestination(destination);
    const CScriptWaypoint* waypoint = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(destination));
    if (waypoint != nullptr) {
      const CVector3f toWaypoint = waypoint->GetTranslation() - GetTranslation();
      if (CVector3f::Dot(GetTransform().GetForward(), toWaypoint) <= 0.f) {
        mWaypointNavigation.SetInPosition(true);
      }
    }
    mFollowingAttackPattern = true;
    break;
  }
  case kStateMsg_Update: {
    const CScriptAIWaypoint* aiWaypoint =
        TCastToPtr< CScriptAIWaypoint >(mgr.ObjectById(mWaypointNavigation.GetDestination()));
    if (aiWaypoint != nullptr) {
      const uint jump = (aiWaypoint->GetFlags() >> 1) & 1;
      const uint drop = (aiWaypoint->GetFlags() >> 2) & 1;
      if (jump || drop) {
        const float maxSpeed = mBodyController->GetBodyStateInfo().GetMaxSpeed();
        const float distance =
            maxSpeed * ((1.5f * dt + 0.1f) * GetModelData()->GetScale().GetY()) + mJumpDistance;
        const CVector3f diff = GetTranslation() - aiWaypoint->GetTranslation();
        if (diff.MagSquared() < distance * distance) {
          mWaypointNavigation.SetInPosition(true);
          mJumpApexHeight = jump ? 3.f : 0.f;
        }
      }
    }
    if (mBodyController->GetCurrentStateId() == pas::kAS_Jump) {
      bool targetPlayer = true;
      if (aiWaypoint != nullptr) {
        if (aiWaypoint->CheckConnectedObject(mgr, kSS_Arrived, kSM_Next) != kInvalidUniqueId) {
          targetPlayer = false;
        }
      }
      if (targetPlayer && GetTargetAimPosition(mgr, mTargetPosition, 0.f)) {
        mBodyController->CommandMgr().SetTargetVector(mTargetPosition - GetTranslation());
      }
    }
    mJumpDestination = mWaypointNavigation.GetDestinationPosition();
    break;
  }
  case kStateMsg_Deactivate:
    mFollowingAttackPattern = false;
    break;
  default:
    break;
  }
}

static EMaterialTypes WarpInSolidMaterial = kMT_Solid;
static EMaterialTypes WarpInPlayerMaterial = kMT_Player;
static EMaterialTypes WarpInCharacterMaterial = kMT_Character;

void CCommandoPirate::WarpIn(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mColor.SetAlpha(0.f);
    mAlphaDelta = 0.f;
    mWarpPhase = 0;
    mWarpTimeCaptured = false;
    RemoveMaterial(kMT_Character, kMT_Solid, kMT_Target, kMT_Orbit, mgr);
    break;
  case kStateMsg_Update:
    switch (mWarpPhase) {
    case 0: {
      rstl::vector< const CScriptWaypoint* > waypoints;
      waypoints.reserve(GetConnectionList().size());
      rstl::vector< SConnection >::const_iterator it = GetConnectionList().begin();
      for (; it != GetConnectionList().end(); ++it) {
        if (it->state == kSS_GRNT && it->msg == kSM_Follow) {
          TUniqueId id = mgr.GetIdForScript(it->objId);
          const CScriptWaypoint* waypoint =
              TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(id));
          if (waypoint != nullptr) {
            waypoints.push_back_unsafe(waypoint);
          }
        }
      }
      if (!waypoints.empty()) {
        const int index = mgr.Random()->Range(0, waypoints.size() - 1);
        SetTranslation(waypoints[index]->GetTranslation());
        SetTransform(CQuaternion::FromMatrix(waypoints[index]->GetTransform())
                         .BuildTransform4f(GetTranslation()));
      }
      rstl::reserved_vector< TUniqueId, 1024 > nearList;
      const CMaterialFilter filter = CMaterialFilter::MakeInclude(
          CMaterialList(WarpInSolidMaterial, WarpInPlayerMaterial, WarpInCharacterMaterial));
      const CVector3f extent(10.f, 10.f, 10.f);
      const CAABox bounds(GetTranslation() - extent, GetTranslation() + extent);
      mgr.BuildNearList(nearList, bounds, filter, this);
      if (!CGameCollision::DetectDynamicCollisionBoolean(*GetCollisionPrimitive(), GetTransform(),
                                                         nearList, mgr)) {
        mWarpPhase = 1;
        AddMaterial(kMT_Character, kMT_Solid, kMT_Target, mgr);
      }
      break;
    }
    case 1:
      if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_Generate)) {
        mBodyController->CommandMgr().DeliverCmd(
            CBCGenerateCmd(pas::kGType_Zero, CVector3f::Zero()));
      } else if (mBodyController->GetCurrentStateId() == pas::kAS_Generate) {
        if (!mWarpTimeCaptured) {
          mWarpTimeCaptured = true;
          mWarpAnimTime = mBodyController->GetAnimTimeRemaining();
        } else if (mWarpAnimTime > FLT_EPSILON) {
          const float ratio = mBodyController->GetAnimTimeRemaining() / mWarpAnimTime;
          mColor.SetAlpha(CMath::Max(0.f, 1.f - ratio));
        }
      }
      break;
    default:
      break;
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    AddMaterial(kMT_Character, kMT_Solid, kMT_Target, kMT_Orbit, mgr);
    mColor.SetAlpha(1.f);
    mAlphaDelta = 0.f;
    mWarpPhase = -1;
    mWarpInRequested = false;
    break;
  default:
    break;
  }
}

void CCommandoPirate::WarpOut(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mWarpPhase = 1;
    mWarpTimeCaptured = false;
    ReleaseCoverPoint(mgr, mCoverPointId, true);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_Generate)) {
      mBodyController->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_One, CVector3f::Zero()));
    } else if (mBodyController->GetCurrentStateId() == pas::kAS_Generate) {
      if (!mWarpTimeCaptured) {
        mWarpTimeCaptured = true;
        mWarpAnimTime = mBodyController->GetAnimTimeRemaining();
      } else if (mWarpAnimTime > FLT_EPSILON) {
        const float ratio = mBodyController->GetAnimTimeRemaining() / mWarpAnimTime;
        mColor.SetAlpha(CMath::Min(1.f, ratio));
      }
    }
    break;
  case kStateMsg_Deactivate:
    mColor.SetAlpha(0.f);
    mAlphaDelta = 0.f;
    break;
  default:
    break;
  }
}

void CCommandoPirate::PostWarpOut(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    if (!mWarpInRequested) {
      mgr.DeliverScriptMsg(CScriptMsg(GetUniqueId(), GetUniqueId(), kSM_Deactivate));
    }
    mWarpPhase = -1;
    if (mDeleteAfterWarpOut) {
      mgr.DeleteObjectRequest(GetUniqueId());
    }
    break;
  default:
    break;
  }
}

void CCommandoPirate::JumpBack(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    if (!GetTargetAimPosition(mgr, mTargetPosition, 0.f)) {
      mTargetPosition = GetTranslation() + GetTransform().GetForward();
    }
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_Step)) {
      mBodyController->CommandMgr().DeliverCmd(CBCStepCmd(pas::kSD_Backward, pas::kStep_Normal));
    } else if (mBodyController->GetCurrentStateId() == pas::kAS_Step) {
      mBodyController->CommandMgr().SetTargetVector(mTargetPosition - GetTranslation());
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  default:
    break;
  }
}

void CCommandoPirate::Dodge(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mDodgeDir = ChooseDodgeDirection(mgr);
    if (mDodgeDir != pas::kSD_Invalid) {
      mAnimationState.SetState(CAnimationState::kAS_Ready);
    }
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_Step)) {
      mBodyController->CommandMgr().DeliverCmd(CBCStepCmd(mDodgeDir, pas::kStep_BreakDodge));
    } else if (mBodyController->GetCurrentStateId() == pas::kAS_Step) {
      if (GetTargetAimPosition(mgr, mTargetPosition, 0.f)) {
        mBodyController->CommandMgr().SetTargetVector(mTargetPosition - GetTranslation());
      }
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mDodgeDir = pas::kSD_Invalid;
    mEnableDodge = false;
    mDodgeCheckTimer = 0.f;
    break;
  default:
    break;
  }
}

void CCommandoPirate::ArmShield(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mArmingShield = true;
    mShieldOn = false;
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mBodyController->SetLocomotionType(pas::kLT_Lurk);
    mArmShieldTimer =
        mgr.Random()->Float() * mData.mShield.mArmTimeVariation + mData.mShield.mArmTime;
    mAbortShield = false;
    mShieldCollisionMgr->SetActive(mgr, true);
    break;
  case kStateMsg_Update:
    mArmShieldTimer -= dt;
    if (mArmShieldTimer <= 0.f || mAbortShield) {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    } else if (mShieldOn) {
      if (GetTargetAimPosition(mgr, mTargetPosition, 0.f)) {
        CVector3f toTarget = mTargetPosition - GetTranslation();
        toTarget.SetZ(0.f);
        if (toTarget.IsMagnitudeSafe()) {
          if (CVector3f::GetAngleDiff(GetTransform().GetForward(), toTarget) >
              20.f * (M_PIF / 180.f)) {
            mBodyController->CommandMgr().DeliverCmd(
                CBCLocomotionCmd(CVector3f::Zero(), toTarget.AsNormalized(), 1.f));
          }
        }
      }
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mBodyController->SetLocomotionType(pas::kLT_Combat);
    mArmShieldWanted = false;
    mNextArmShieldBurst = mBurstCount + 1;
    mShieldCollisionMgr->SetActive(mgr, false);
    TurnShieldOff();
    mArmingShield = false;
    break;
  default:
    break;
  }
}

void CCommandoPirate::ShieldCharge(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mShieldCharging = true;
    mShieldOn = false;
    mChargeStarted = false;
    mAbortShield = false;
    mChargeBlockedTime = 0.f;
    mActionPhase = 0;
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mNextShieldChargeBurst = mBurstCount + 1;
    if (mTeamAiMgrId == kInvalidUniqueId ||
        CScriptTeamAiMgr::StartAttack(CScriptTeamAiMgr::kAT_Melee, mgr, mTeamAiMgrId,
                                      GetUniqueId())) {
      const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
      if (target != nullptr) {
        const CVector3f position = GetTranslation();
        CVector3f toTarget = target->GetTranslation() - position;
        toTarget.SetZ(0.f);
        mFaceDirection =
            toTarget.IsMagnitudeSafe() ? toTarget.AsNormalized() : GetTransform().GetForward();
        mTargetPosition = position + mData.mShield.mChargeMaxAttackDist * mFaceDirection;
        mChargeCollisionNormal = CVector3f::Zero();
      } else {
        mAnimationState.SetState(CAnimationState::kAS_Over);
      }
    } else {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    }
    break;
  case kStateMsg_Update:
    switch (mActionPhase) {
    case 0:
      if (CVector3f::GetAngleDiff(GetTransform().GetForward(), mFaceDirection) >
          10.f * (M_PIF / 180.f)) {
        mBodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), mFaceDirection, 1.f));
      } else {
        mActionPhase = 1;
      }
      break;
    case 1:
      if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_LoopAttack)) {
        mBodyController->CommandMgr().DeliverCmd(CBCLoopAttackCmd(pas::kLAT_Four, false));
        mShieldCollisionMgr->SetActive(mgr, true);
      } else if (mBodyController->GetCurrentStateId() == pas::kAS_LoopAttack) {
        const CVector3f toDest = mTargetPosition - GetTranslation();
        const CVector3f forward = GetTransform().GetForward();
        float chargeTime = 0.f;
        if (mData.mShield.mChargeSpeed > 0.f) {
          chargeTime = mData.mShield.mChargeMaxAttackDist / mData.mShield.mChargeSpeed;
        }
        if (CVector3f::Dot(forward, toDest) <= 0.f || mAbortShield ||
            mStateMachine->GetTime() > 3.f + chargeTime) {
          mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
        } else if (mChargeStarted) {
          const float speed = mData.mShield.mChargeSpeed;
          MoveInOneFrameOR(dt * (speed * GetTransform().TransposeRotate(mFaceDirection)), dt);
          if (mChargeCollisionNormal.IsNonZero()) {
            MoveInOneFrameOR(dt * (mData.mShield.mChargeSpeed *
                                   GetTransform().TransposeRotate(mChargeCollisionNormal)),
                             dt);
            mChargeCollisionNormal = CVector3f::Zero();
          }
        }
      }
      break;
    default:
      break;
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
    if (mAbortShield) {
      mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_AbortScripted));
    }
    mActionPhase = -1;
    mShieldCollisionMgr->SetActive(mgr, false);
    TurnShieldOff();
    CScriptTeamAiMgr::EndAttack(CScriptTeamAiMgr::kAT_Melee, mgr, mTeamAiMgrId, GetUniqueId(),
                                false);
    mChargeStarted = false;
    mShieldCharging = false;
    break;
  default:
    break;
  }
}

static EMaterialTypes ShieldChargeExcludeCeilingMaterial = kMT_Ceiling;
static EMaterialTypes ShieldChargeExcludeWallMaterial = kMT_Wall;
static EMaterialTypes ShieldChargeRestoreCeilingMaterial = kMT_Ceiling;
static EMaterialTypes ShieldChargeRestoreWallMaterial = kMT_Wall;

void CCommandoPirate::ScriptedShieldCharge(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mShieldCharging = true;
    mShieldOn = false;
    mChargeStarted = false;
    mAbortShield = false;
    mChargeBlockedTime = 0.f;
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mNextShieldChargeBurst = mBurstCount + 1;
    if (FindShieldChargeDest(mgr, false)) {
      CMaterialFilter filter = GetMaterialFilter();
      filter.ExcludeList().Add(
          CMaterialList(ShieldChargeExcludeCeilingMaterial, ShieldChargeExcludeWallMaterial));
      SetMaterialFilter(filter);
      mChargeCollisionNormal = CVector3f::Zero();
    } else {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    }
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_LoopAttack)) {
      mVerticalMovement = true;
      RemoveMaterial(kMT_GroundCollider, mgr);
      mBodyController->CommandMgr().DeliverCmd(CBCLoopAttackCmd(pas::kLAT_Four, false));
      mShieldCollisionMgr->SetActive(mgr, true);
    } else if (mBodyController->GetCurrentStateId() == pas::kAS_LoopAttack) {
      mChargeBlockedTime = 0.f;
      if (mAbortShield) {
        mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
      } else {
        const CVector3f toDest = mTargetPosition - GetTranslation();
        const float distSq = toDest.MagSquared();
        const CVector3f forward = GetTransform().GetForward();
        const CAABox box = GetBoundingBox();
        const float width = box.GetMaxPoint().GetY() - box.GetMinPoint().GetY();
        if (CVector3f::Dot(forward, toDest) <= 0.f || distSq <= width * width) {
          if (!FindShieldChargeDest(mgr, true)) {
            mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
          }
        } else if (mChargeStarted) {
          UpdateShieldCharge(dt);
        }
      }
    }
    break;
  case kStateMsg_Deactivate: {
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
    if (mAbortShield) {
      mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_AbortScripted));
    }
    mAttackPatternWaypointId = mWaypointNavigation.GetDestination();
    mVerticalMovement = false;
    AddMaterial(kMT_GroundCollider, mgr);
    CMaterialFilter filter = GetMaterialFilter();
    filter.ExcludeList().Remove(
        CMaterialList(ShieldChargeRestoreCeilingMaterial, ShieldChargeRestoreWallMaterial));
    SetMaterialFilter(filter);
    mShieldCollisionMgr->SetActive(mgr, false);
    TurnShieldOff();
    mChargeStarted = false;
    mShieldCharging = false;
    break;
  }
  default:
    break;
  }
}

void CCommandoPirate::RestoreOrientation(CStateManager&, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    break;
  case kStateMsg_Update: {
    const CVector3f up = GetTransform().GetUp();
    if (!(fabsf(up.GetZ() - 1.f) < 0.00001f)) {
      const float angleToUp = CMath::Rad2Deg(CVector3f::GetAngleDiff(up, CVector3f::Up()));
      const float maxStep = dt * mBodyController->GetTurnSpeed();
      if (angleToUp > maxStep) {
        const CQuaternion rotation = CQuaternion::LookAt(CUnitVector3f(up), CVector3f::Up(),
                                                         CRelAngle::FromDegrees(maxStep));
        const CQuaternion newRotation = CQuaternion::FromMatrix(GetTransform()) * rotation;
        SetTransform(newRotation.BuildNormalized().BuildTransform4f(GetTranslation()));
      } else {
        mAnimationState.SetState(CAnimationState::kAS_Over);
      }
    } else {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    }
    break;
  }
  case kStateMsg_Deactivate: {
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    const CVector3f forward = GetTransform().GetForward();
    const CVector3f position = GetTranslation();
    const CTransform4f xf = CTransform4f::LookAt(
        position, position + CVector3f(forward.GetX(), forward.GetY(), 0.f), CVector3f::Up());
    SetTransform(xf);
    break;
  }
  default:
    break;
  }
}

void CCommandoPirate::Crouch(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mBodyController->SetLocomotionType(pas::kLT_Crouch);
    mSeekingCover = false;
    break;
  case kStateMsg_Update: {
    const CScriptCoverPoint* coverPoint = GetCoverPoint(mgr, mCoverPointId);
    if (coverPoint != nullptr) {
      const CVector3f forward = GetTransform().GetForward();
      if (CVector3f::GetAngleDiff(forward, coverPoint->GetTransform().GetForward()) >
          20.f * (M_PIF / 180.f)) {
        mBodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), coverPoint->GetTransform().GetForward(), 1.f));
      }
    }
    break;
  }
  case kStateMsg_Deactivate:
    mBodyController->SetLocomotionType(pas::kLT_Combat);
    break;
  default:
    break;
  }
}

// Guessed name. Cover points only follow script waypoints.
class CCoverWaypointPredicate : public CValidEntityPredicate {
public:
  ~CCoverWaypointPredicate() override {}

  bool IsValid(const CStateManager& mgr, TUniqueId id) const override {
    return TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(id)) != nullptr;
  }
};

void CCommandoPirate::WallHang(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    mInWallHang = true;
    mSeekingCover = false;
    mActionPhase = 0;
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    CScriptCoverPoint* coverPoint = GetCoverPoint(mgr, mCoverPointId);
    if (coverPoint != nullptr) {
      const CScriptWaypoint* waypoint =
          TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(coverPoint->CheckConnectedObject_if(
              mgr, kSS_Arrived, kSM_Next, CCoverWaypointPredicate())));
      if (waypoint != nullptr) {
        mWallHangWaypointId = waypoint->GetUniqueId();
      }
      mFaceDirection = coverPoint->GetTransform().GetForward();
    }
    break;
  }
  case kStateMsg_Update:
    mBurstFire.SetBurstType(0);
    switch (mActionPhase) {
    case 0:
      if (CVector3f::GetAngleDiff(GetTransform().GetForward(), mFaceDirection) >
          10.f * (M_PIF / 180.f)) {
        mBodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), mFaceDirection, 1.f));
      } else {
        mActionPhase = 1;
      }
      break;
    case 1:
      if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_WallHang)) {
        mBodyController->CommandMgr().DeliverCmd(CBCWallHangCmd(mWallHangWaypointId));
      } else if (mBodyController->GetCurrentStateId() == pas::kAS_WallHang) {
        if (GetTargetAimPosition(mgr, mTargetPosition, 0.f)) {
          mBodyController->CommandMgr().SetTargetVector(mTargetPosition - GetTranslation());
          mTargetPosition.SetZ(0.f);
        }
      }
      break;
    default:
      break;
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mInWallHang = false;
    mActionPhase = -1;
    break;
  default:
    break;
  }
}

void CCommandoPirate::WallDetach(CStateManager&, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mInWallHang = true;
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    break;
  case kStateMsg_Update:
    mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
    if (mBodyController->GetCurrentStateId() != pas::kAS_WallHang) {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mInWallHang = false;
    break;
  default:
    break;
  }
}

void CCommandoPirate::CoverFind(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == kStateMsg_Activate) {
    mEnableRetreat = false;
    mCoverCheck = false;
    if (GetTargetAimPosition(mgr, mTargetPosition, 0.f)) {
      CScriptCoverPoint* closest = nullptr;
      CObjectList& list = mgr.ObjectListById(kOL_AiWaypoint);
      float minDistSq = mData.mSearchRadius * mData.mSearchRadius;
      const CVector3f position = GetTranslation();
      for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
        CScriptCoverPoint* coverPoint = TCastToPtr< CScriptCoverPoint >(list[i]);
        if (coverPoint != nullptr && coverPoint->GetActive() && !coverPoint->ShouldLandHere() &&
            !coverPoint->GetInUse(GetUniqueId()) &&
            coverPoint->GetCurrentAreaId() == GetCurrentAreaId() &&
            coverPoint->GetUniqueId() != mLastCoverPointId) {
          const CVector3f diff = position - coverPoint->GetTranslation();
          const float distSq = diff.MagSquared();
          if (distSq < minDistSq && !coverPoint->Blown(mTargetPosition) &&
              GetSearchPath()->PathExists(position, coverPoint->GetTranslation()) ==
                  CPathFindSearch::kR_Success) {
            minDistSq = distSq;
            closest = coverPoint;
          }
        }
      }
      if (closest != nullptr) {
        SetCoverPoint(closest, mCoverPointId);
        mLastCoverPointId = mCoverPointId;
      }
    }
  }
}

void CCommandoPirate::Cover(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    if (mBodyController->GetCurrentStateId() != pas::kAS_Cover) {
      mAnimationState.SetState(CAnimationState::kAS_Ready);
    }
    mSeekingCover = false;
    break;
  case kStateMsg_Update: {
    CScriptCoverPoint* coverPoint = GetCoverPoint(mgr, mCoverPointId);
    if (coverPoint != nullptr) {
      const uint attackDir = static_cast< uint >(coverPoint->GetAttackDirection());
      const pas::ECoverDirection coverDir =
          ((attackDir >> 2) & 1) != 0 ? pas::kCD_Right : pas::kCD_Left;
      if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_Cover)) {
        mBodyController->CommandMgr().DeliverCmd(CBCCoverCmd(
            coverDir, coverPoint->GetTranslation(), -coverPoint->GetTransform().GetForward()));
      } else if (!mBodyController->GetBodyStateInfo().GetCurrentState()->CanShoot()) {
        mBodyController->CommandMgr().SetTargetVector(-coverPoint->GetTransform().GetForward());
        const CVector3f toCover = coverPoint->GetTranslation() - GetTranslation();
        MoveInOneFrameOR(0.05f * GetTransform().TransposeRotate(toCover), dt);
      }
    }
    break;
  }
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  default:
    break;
  }
}

void CCommandoPirate::CoverAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_LeanFromCover));
    break;
  case kStateMsg_Update:
    if (!mBodyController->GetBodyStateInfo().GetCurrentState()->CanShoot()) {
      const CScriptCoverPoint* coverPoint = GetCoverPoint(mgr, mCoverPointId);
      if (coverPoint != nullptr) {
        const CVector3f toCover = coverPoint->GetTranslation() - GetTranslation();
        MoveInOneFrameOR(0.05f * GetTransform().TransposeRotate(toCover), dt);
      }
    }
    break;
  default:
    break;
  }
}

void CCommandoPirate::BreakCover(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
    ReleaseCoverPoint(mgr, mCoverPointId, true);
    mSeekingCover = false;
    break;
  default:
    break;
  }
}

void CCommandoPirate::GetUp(CStateManager&, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mGettingUp = true;
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    break;
  case kStateMsg_Update:
    if (mBodyController->GetCurrentStateId() == pas::kAS_LieOnGround &&
        mPathFindSearch.Search(GetTranslation(), GetTranslation()) ==
            CPathFindSearch::kR_NoSourcePoint) {
      mPendingDeath = true;
    } else if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_Getup)) {
      mBodyController->CommandMgr().DeliverCmd(CBCGetupCmd(pas::kGetup_Zero));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mGettingUp = false;
    break;
  default:
    break;
  }
}

void CCommandoPirate::Dead(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::Dead(mgr, msg, dt);
  switch (msg) {
  case kStateMsg_Activate:
    mSeekingCover = false;
    if (mShieldOn) {
      BreakShield(mgr);
    }
    break;
  default:
    break;
  }
}

void CCommandoPirate::UpdateTimers(float dt, CStateManager& mgr) {
  if (mAlive) {
    mAggressionCheckTimer += dt;
    if (mAggressionCheckTimer >= 5.f) {
      mAggressive = mData.mAggressiveness >= mgr.Random()->Range(0.f, 100.f);
      mAggressionCheckTimer = 0.f;
    }

    mDodgeCheckTimer += dt;
    if (mDodgeCheckTimer >= 2.f) {
      mEnableDodge = !mAggressive ? mData.mDodgeCheck >= mgr.Random()->Range(0.f, 100.f) : false;
      mDodgeCheckTimer = 0.f;
    }

    mCoverCheckTimer += dt;
    if (mCoverCheckTimer >= 5.f) {
      mCoverCheck = !mAggressive ? mData.mCoverCheck >= mgr.Random()->Range(0.f, 100.f) : false;
      mCoverCheckTimer = 0.f;
    }

    mGrenadeAttackTimer += dt;
    if (sRelUseCount > 0) {
      mGrenadeAttackTimer = 0.f;
    } else if (mGrenadeAttackTimer >= mData.mGrenade.mMinAttackInterval) {
      mEnableGrenadeAttack =
          !mAggressive ? mData.mGrenade.mAttackChance >= mgr.Random()->Range(0.f, 100.f) : false;
      mGrenadeAttackTimer = 0.f;
    }

    if (mChargeBlocked) {
      mChargeBlockedTime += dt;
      mChargeBlocked = false;
      if (mChargeBlockedTime >= 0.3f) {
        mAbortShield = true;
      }
    }

    if (mSeekingCover) {
      mSeekCoverTime += dt;
    }

    mTimeSinceHeardShot += dt;
    mJumpPointSearchTimer += dt;
    mTimeSinceMelee += dt;

    if (mData.mShield.mArmDamageDecayTime > 0.f) {
      mRecentDamage -= dt * mData.mShield.mArmDamageThreshold / mData.mShield.mArmDamageDecayTime;
      mRecentDamage = rstl::max_val(0.f, mRecentDamage);
    }

    if (!mArmShieldWanted && mData.mShield.mArmDamageThreshold > 0.f) {
      mArmShieldWanted = mRecentDamage > mData.mShield.mArmDamageThreshold;
    }

    if (mHitByPlayerProjectile) {
      mTimeSinceHit = 0.f;
      mHitByPlayerProjectile = false;
    } else {
      mTimeSinceHit += dt;
    }
  }
}

void CCommandoPirate::UpdateBurstFire(float dt, CStateManager& mgr) {
  if (mBurstCooldown > 0.f) {
    mBurstCooldown -= dt;
  }

  if (!mArmShieldWanted && mBurstCount > mNextArmShieldBurst) {
    const float armChance = mData.mShield.mArmChance;
    if (armChance >= mgr.Random()->Range(0.f, 100.f)) {
      mArmShieldWanted = true;
    } else {
      mNextArmShieldBurst = mBurstCount + 1;
    }
  }

  if (!mAlive || CanFireAtTarget()) {
    if (mBurstFire.GetBurstType() != -1) {
      const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
      if (mAlive && target != nullptr) {
        const CVector3f toTarget = target->GetTranslation() - GetTranslation();
        const CVector3f forward = GetTransform().GetForward();
        if (mBurstCooldown <= 0.f && (mInWallHang || CVector3f::Dot(forward, toTarget) > 0.f)) {
          const CVector3f toSelf = GetTranslation() - target->GetTranslation();
          const CVector3f targetForward = target->GetTransform().GetForward();
          if (CVector3f::Dot(targetForward, toSelf) < 0.f && mBurstFire.GetBurstType() < 2) {
            mBurstFire.SetBurstType(mBurstFire.GetBurstType() + 2);
          }
          mBurstFire.Start(mgr);
          float variation = mAttackTimeVariation;
          mBurstCooldown = variation * mgr.Random()->Float() + GetAverageAttackTime();
          ++mBurstCount;
        }
      }

      mBurstFire.Update(mgr, dt);
      if (mBurstFire.ShouldFire()) {
        FireProjectile(dt, mgr);
        mBurstFire.SetTimeToNextShot((mgr.Random()->Float() - 0.5f) *
                                         mData.mIntraBurstShotVariation +
                                     mData.mIntraBurstShotTime);
      }
    }
  }
}

static EMaterialTypes FireSolidMaterial = kMT_Solid;
static EMaterialTypes FirePlayerMaterial = kMT_Player;
static EMaterialTypes FirePassthroughMaterial = kMT_ProjectilePassthrough;

bool CCommandoPirate::FireProjectile(float dt, CStateManager& mgr) {
  bool fired = false;
  const CTransform4f xf = GetLctrTransform(mGunSeg);
  if (mAlive) {
    const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
    if (target != nullptr) {
      CVector3f origin = target->GetTranslation();
      bool direct = false;
      if (const CPlayer* player = TCastToConstPtr< CPlayer >(target)) {
        if (player->GetTurretState() == CPlayer::kTS_Active) {
          origin = player->GetTranslation();
          direct = true;
        } else {
          origin = ProjectileInfo()->PredictInterceptPos(
              xf.GetTranslation(), player->GetAimPosition(mgr, 0.f), *player, true, dt);
        }
      }

      const CVector3f delta = origin - xf.GetTranslation();
      const float distance = delta.Magnitude();
      const CTransform4f wristXf = GetLctrTransform(mRightWristSeg);
      const CTransform4f elbowXf = GetLctrTransform(mRightElbowSeg);
      const float angle =
          CVector3f::GetAngleDiff(wristXf.GetTranslation() - elbowXf.GetTranslation(), delta);
      if (angle <= M_PIF / 6.f || (distance <= 6.f && angle <= M_PIF / 4.f)) {
        const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
            CMaterialList(FireSolidMaterial),
            CMaterialList(FirePlayerMaterial, FirePassthroughMaterial));
        if (direct || mgr.RayCollideWorld(xf.GetTranslation(), origin, filter, this)) {
          origin += GetTransform().Rotate(mBurstFire.GetDistanceCompensatedError(distance, 6.f));
          const CTransform4f aimXf =
              CTransform4f::LookAt(xf.GetTranslation(), origin, CVector3f::Up());
          LaunchProjectile(aimXf, mgr, 6, CWeapon::kPA_None, false, CImpactVisorEffect::None(),
                           CVector3f(1.f, 1.f, 1.f));
          fired = true;
        } else {
          mLineOfSightTracker.ClearLineOfSight();
        }
      }
    }
  } else {
    LaunchProjectile(xf, mgr, 6, CWeapon::kPA_None, false, CImpactVisorEffect::None(),
                     CVector3f(1.f, 1.f, 1.f));
    fired = true;
  }

  if (fired) {
    int reactionType = 2;
    if (GetBodyController()->GetCurrentStateId() == pas::kAS_Locomotion &&
        GetBodyController()->GetBodyStateInfo().GetCurrentState()->IsMoving()) {
      reactionType = 8;
    }
    const CPASAnimParmData parms(pas::kAS_AdditiveReaction, CPASAnimParm::FromEnum(reactionType));
    const rstl::pair< float, int > anim =
        GetBodyController()->GetPASDatabase().FindBestAnimation(parms, *mgr.Random(), -1);
    if (anim.second != -1) {
      ModelData()->AnimationData()->AddAdditiveAnimation(anim.second, 1.f, false, true);
    }
    CSfxManager::AddEmitter(mData.mSound_Projectile, GetTranslation(), GetCurrentAreaId().Value(),
                            true, false, CSfxManager::kMedPriority);
  }
  return fired;
}

void CCommandoPirate::LaunchGrenade(CStateManager& mgr) {
  const CTransform4f launchXf = GetLctrTransform(mLaunchSeg);
  const CVector3f origin = launchXf.GetTranslation();
  float angle = 20.f * (M_PIF / 180.f);
  float speed = mData.mGrenade.mMinLaunchSpeed;
  if (const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId))) {
    const CVector3f aim = GetGrenadeTargetPosition(mgr, *target);
    SolveGrenadeLaunch(aim, origin, angle, speed);

    CVector3f flat = aim - origin;
    flat.SetZ(0.f);
    const CVector3f forward = GetTransform().GetColumn(kDY);
    CVector3f horizontal = flat.CanBeNormalized() ? flat.AsNormalized() : forward;
    if (CVector3f::GetAngleDiff(forward, horizontal) > M_PIF / 6.f) {
      horizontal = CVector3f::Slerp(forward, horizontal, CRelAngle::FromRadians(M_PIF / 6.f));
    }
    const CVector3f launchDir =
        CVector3f::Slerp(horizontal, CVector3f::Up(), CRelAngle::FromRadians(angle));
    const CTransform4f grenadeXf =
        CTransform4f::LookAt(origin, origin + launchDir, CVector3f::Up());

    CCommandoPirateGrenade* grenade = rs_new CCommandoPirateGrenade(
        mgr.AllocateUniqueId(), rstl::string_l("Commando E-Grenade"),
        CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true), grenadeXf,
        CModelData::CModelDataNull(), CActorParameters::None(), GetUniqueId(), mData.mGrenade,
        speed);
    if (grenade) {
      mgr.AddObject(grenade);
      mEnableGrenadeAttack = false;
    }
  }
}

CVector3f CCommandoPirate::GetGrenadeTargetPosition(const CStateManager& mgr,
                                                    const CActor& target) const {
  CVector3f aim = target.GetAimPosition(mgr, 0.5f);
  const CPlayer* player = TCastToConstPtr< CPlayer >(target);
  if (player && player->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed) {
    aim -= CVector3f(0.f, 0.f, 0.5f * player->GetEyeHeight());
  }
  const CVector3f diff = GetTranslation() - aim;
  const float distance = diff.Magnitude();
  if (distance > 6.f) {
    aim += (3.f / distance) * diff;
  }
  return aim;
}

void CCommandoPirate::SolveGrenadeLaunch(const CVector3f& target, const CVector3f& origin,
                                         float& outAngle, float& outSpeed) const {
  float angle = 20.f * (M_PIF / 180.f);
  float speed = mData.mGrenade.mMinLaunchSpeed;
  float bestError = FLT_MAX;
  const float heightDelta = target.GetZ() - origin.GetZ();
  const float distance =
      CVector2f(target.GetX() - origin.GetX(), target.GetY() - origin.GetY()).Magnitude();
  const float halfGravityDistSq = 0.5f * kDefaultGravityAccel * distance * distance;
  const float minSpeedSq = mData.mGrenade.mMinLaunchSpeed * mData.mGrenade.mMinLaunchSpeed;
  const float maxSpeedSq = mData.mGrenade.mMaxLaunchSpeed * mData.mGrenade.mMaxLaunchSpeed;
  float startAngle = 20.f * (M_PIF / 180.f);
  float stepAngle = M_PIF / 72.f;
  if (target.GetZ() > origin.GetZ()) {
    startAngle = M_PIF / 4.f;
    stepAngle = -stepAngle;
  }
  for (float i = 0.f; i < 10.f; i += 1.f) {
    const float candidate = stepAngle * i + startAngle;
    const float cosine = CMath::FastCosR(candidate);
    const float sine = CMath::FastSinR(candidate);
    const float denominator = distance * (cosine * sine) - heightDelta * (cosine * cosine);
    if (denominator > FLT_EPSILON) {
      const float speedSq = halfGravityDistSq / denominator;
      if (speedSq >= minSpeedSq && speedSq <= maxSpeedSq) {
        angle = candidate;
        speed = CMath::SqrtF(speedSq);
        break;
      }
      const float error = speedSq > maxSpeedSq ? speedSq - maxSpeedSq : minSpeedSq - speedSq;
      if (error < bestError) {
        angle = candidate;
        speed = CMath::SqrtF(speedSq);
        bestError = error;
      }
    }
  }
  outAngle = angle;
  outSpeed = speed;
}

void CCommandoPirate::UpdateAdditiveAim(CStateManager& mgr) {
  if (mAlive && !mRagDoll.get() && CanFireAtTarget()) {
    CVector3f target = CVector3f::Zero();
    if (GetTargetAimPosition(mgr, target, 0.f)) {
      mBodyController->CommandMgr().DeliverCmd(CBCAdditiveAimCmd(mInWallHang));
      CTransform4f xf = GetTransform();
      const CTransform4f gunXf = GetLctrTransform(mGunSeg);
      xf.SetTranslation(gunXf.GetTranslation());
      CVector3f aim = xf.TransposeRotate(target - gunXf.GetTranslation());
      if (mInWallHang) {
        aim = CVector3f(-aim.GetX(), -aim.GetY(), aim.GetZ());
      }
      if (aim.GetY() > 0.f) {
        mBodyController->CommandMgr().DeliverAdditiveTargetVector(aim);
      } else {
        mBodyController->CommandMgr().DeliverAdditiveTargetVector(CVector3f::Forward());
      }
    }
  } else if (mAdditiveAimEnabled && !mData.mMelee) {
    mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_AdditiveIdle));
  }
}

bool CCommandoPirate::CanFireAtTarget() const {
  if (GetBodyController()->GetBodyStateInfo().GetCurrentState()->CanShoot() &&
      mAdditiveAimEnabled && !mData.mMelee && !mStopped && !mEGrenadeAttacking &&
      !mShieldCharging && !mArmingShield && mShieldEffectType == -1 && mTimeSinceMelee > 1.f &&
      !GetBodyController()->IsFrozen() && !GetBodyController()->IsElectrocuting()) {
    return true;
  }
  return false;
}

void CCommandoPirate::UpdateEmitters() {
  if (mHurledSfxHandle) {
    if (CSfxManager::IsPlaying(mHurledSfxHandle) || CSfxManager::IsQueued(mHurledSfxHandle)) {
      CSfxManager::UpdateEmitter(mHurledSfxHandle, GetTranslation(), GetTransform().GetForward(),
                                 127);
    } else {
      mHurledSfxHandle = CSfxHandle();
    }
  }
  if (mSfxHandle) {
    CSfxManager::UpdateEmitter(mSfxHandle, GetTranslation(), GetTransform().GetForward(), 127);
  }
}

void CCommandoPirate::SetupCollisionActors(CStateManager& mgr) {
  rstl::vector< CJointCollisionDescription > joints;
  joints.reserve(1);
  joints.push_back_unsafe(CJointCollisionDescription::SphereCollision(
      mLeftWristSeg, CVector3f::Zero(), 1.5f, rstl::string_l("Shield_CollisionActor"), 1000.f));
  mShieldCollisionMgr =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), joints, false);
  mShieldCollisionActorId = mShieldCollisionMgr->GetCollisionDescFromIndex(0).GetCollisionActorId();
  if (CCollisionActor* shieldActor =
          TCastToPtr< CCollisionActor >(mgr.ObjectById(mShieldCollisionActorId))) {
    shieldActor->SetDamageVulnerability(mData.mShield.mVulnerability);
    shieldActor->SetResponseType(kWCR_EnemyShielded);
  }

  joints.clear();
  joints.push_back_unsafe(CJointCollisionDescription::SphereCollision(
      mGrenadeSeg, CVector3f::Zero(), 1.f, rstl::string_l("Blade_CollisionActor"), 1000.f));
  mBladeCollisionMgr =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), joints, false);
  mBladeCollisionActorId = mBladeCollisionMgr->GetCollisionDescFromIndex(0).GetCollisionActorId();
  if (CCollisionActor* bladeActor =
          TCastToPtr< CCollisionActor >(mgr.ObjectById(mBladeCollisionActorId))) {
    bladeActor->SetDamageVulnerability(CDamageVulnerability::PassThroughVulnerabilty());
  }

  CMaterialFilter filter = GetMaterialFilter();
  filter.ExcludeList().Add(kMT_CollisionActor);
  SetMaterialFilter(filter);
}

void CCommandoPirate::ThinkRagDoll(float dt, CStateManager& mgr, bool noRagDoll) {
  if (!noRagDoll) {
    if (!mRagDoll->IsPrimed()) {
      mRagDoll->Prime(mgr, GetTransform(), *ModelData());
      const CVector3f position = GetTranslation();
      SetTransform(CTransform4f::Identity());
      SetTranslation(position);
      mBodyController->SetPlaybackRate(0.f);
    } else {
      float waterTop = -FLT_MAX;
      if (InFluidId() != kInvalidUniqueId) {
        const CScriptWater* water = TCastToConstPtr< CScriptWater >(mgr.GetObjectById(InFluidId()));
        if (water && water->GetActive()) {
          waterTop = water->GetTriggerBoundsWR().GetMaxPoint().GetZ();
        }
      }
      mRagDoll->Update(mgr, dt * GetDeathTimeScale(), waterTop);
      ModelData()->AdvanceParticles(GetTransform(), dt, mgr);
    }

    if (mRagDoll->IsOver() && !mRagDoll->WillContinueSmallMovements() && !mFadeToDeath) {
      mFadeToDeath = true;
      mAlphaDelta = -1.f / 3.f;
      AddMaterial(kMT_ProjectilePassthrough, mgr);
      SetMomentumWR(CVector3f::Zero());
      Stop();
    }
  }

  if (mRagDollTimer > 0.f) {
    mRagDollTimer -= dt;
    if (mRagDollTimer <= 0.f) {
      if (!mRagDoll.get()) {
        const rstl::reserved_vector< float, 14 > radii(skRagDollParticleRadii,
                                                       skRagDollParticleRadii + 14);
        const ushort impactSound = mData.mSound_Impact;
        const float floatingGravity = -3.f;
        mRagDoll = rs_new CPirateRagDoll(mgr, this, impactSound, mData.mFloatingCorpse ? 3 : 0,
                                         GetGravityConstant(), floatingGravity, radii);
        RemoveMaterial(kMT_Orbit, kMT_Target, mgr);
      }
      mRagDollTimer = 0.f;
    }
  }
}

void CCommandoPirate::RequestWarpOut(CStateManager& mgr, bool deleteAfter) {
  if (mAlive && !mGettingUp && mWarpPhase == -1 && mData.mTrooper) {
    mStateMachine->SetState(mgr, *this, rstl::string_l("WarpOut"));
    mDeleteAfterWarpOut = deleteAfter;
  }
}

bool CCommandoPirate::GetTargetAimPosition(CStateManager& mgr, CVector3f& position,
                                           float dt) const {
  const CEntity* target = mgr.GetObjectById(mTargetId);
  if (target != nullptr) {
    position = static_cast< const CActor* >(target)->GetAimPosition(mgr, dt);
    return true;
  }
  return false;
}

static EMaterialTypes ChargeDamageSolidMaterial = kMT_Solid;
static EMaterialTypes BladeDamageSolidMaterial = kMT_Solid;

void CCommandoPirate::ApplyChargeDamage(const TUniqueId& id, CStateManager& mgr) {
  if (mShieldCharging && id == mShieldCollisionActorId) {
    if (const CCollisionActor* colAct = TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(id))) {
      const TUniqueId touchedId = colAct->GetLastTouchedObject();
      if (TCastToConstPtr< CPlayer >(mgr.GetObjectById(touchedId)) != nullptr &&
          mCurDamageRemTime <= 0.f) {
        mgr.ApplyDamage(GetUniqueId(), touchedId, GetUniqueId(), mData.mShield.mChargeDamage,
                        CMaterialFilter::MakeInclude(CMaterialList(ChargeDamageSolidMaterial)),
                        GetTransform().GetForward());
        mCurDamageRemTime = mDamageWaitTime;
      }
    }
  } else if (mMeleeAttacking && id == mBladeCollisionActorId) {
    if (const CCollisionActor* colAct = TCastToPtr< CCollisionActor >(mgr.ObjectById(id))) {
      const TUniqueId touchedId = colAct->GetLastTouchedObject();
      if (TCastToPtr< CPlayer >(mgr.ObjectById(touchedId)) != nullptr && mCurDamageRemTime <= 0.f) {
        mgr.ApplyDamage(GetUniqueId(), touchedId, GetUniqueId(), mData.mBladeDamage,
                        CMaterialFilter::MakeInclude(CMaterialList(BladeDamageSolidMaterial)),
                        GetTransform().GetForward());
        mCurDamageRemTime = mDamageWaitTime;
      }
    }
  }
}

void CCommandoPirate::SetVelocityForJump() {
  if (!mJumpVelSet) {
    CVector3f velocity = CVector3f::Zero();
    const CVector3f delta = mJumpDestination - GetTranslation();
    const float gravity = GetGravityConstant();
    const float jumpZ =
        mJumpApexHeight + CMath::Max(mJumpDestination.GetZ(), GetTranslation().GetZ());
    velocity.SetZ(CMath::SqrtF(2.f * gravity * (jumpZ - GetTranslation().GetZ())));
    float time = velocity.GetZ() / gravity;
    time += CMath::SqrtF(2.f * (jumpZ - mJumpDestination.GetZ()) / gravity);
    const float invTime = 1.f / time;
    velocity.SetX(invTime * delta.GetX());
    velocity.SetY(invTime * delta.GetY());
    SetVelocityWR(velocity);
    mJumpVelSet = true;
  }
}

pas::EStepDirection CCommandoPirate::ChooseDodgeDirection(CStateManager& mgr) {
  const float stepDistanceSquared = mDodgeDistance * mDodgeDistance;
  const CVector3f position = GetTranslation();
  const CVector3f right = GetTransform().GetRight();
  bool leftFree = true;
  bool rightFree = true;
  pas::EStepDirection direction = pas::kSD_Invalid;
  const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    if (const CActor* actor = static_cast< const CActor* >(list[i])) {
      if (actor != this && actor->GetCurrentAreaId() == GetCurrentAreaId()) {
        const CVector3f delta = actor->GetTranslation() - position;
        if (delta.MagSquared() < stepDistanceSquared) {
          if (CVector3f::Dot(delta, right) >= 0.f) {
            if (rightFree && CVector3f::GetAngleDiff(right, delta) < (M_PIF / 3.f)) {
              rightFree = false;
            }
          } else if (leftFree && CVector3f::GetAngleDiff(-right, delta) < (M_PIF / 3.f)) {
            leftFree = false;
          }
        }
      }
    }
  }
  if (rightFree) {
    rightFree = IsPathClear(mgr, right, mDodgeDistance);
  }
  if (leftFree) {
    leftFree = IsPathClear(mgr, -right, mDodgeDistance);
  }
  if (leftFree && rightFree) {
    if (mgr.Random()->Next() & 0x4000) {
      leftFree = false;
    } else {
      rightFree = false;
    }
  }

  if (leftFree) {
    direction = pas::kSD_Left;
  } else if (rightFree) {
    direction = pas::kSD_Right;
  }
  return direction;
}

int CCommandoPirate::SelectMeleeVariant(CStateManager& mgr) const {
  int variant = -1;
  CVector3f aimPosition = CVector3f::Zero();
  if (GetTargetAimPosition(mgr, aimPosition, 0.5f)) {
    const CVector3f forward = GetTransform().GetForward();
    CVector3f toTarget = aimPosition - GetTranslation();
    toTarget.SetZ(0.f);
    const float angle = CVector3f::GetAngleDiff(toTarget, forward);

    float totalWeight = 0.f;
    for (int i = 0; i < 3; ++i) {
      if (skMeleeVariants[i].mMaxAngle >= angle) {
        totalWeight += skMeleeVariants[i].mWeight;
      }
    }

    float roll = mgr.Random()->Float() * totalWeight;
    for (int i = 0; i < 3; ++i) {
      if (skMeleeVariants[i].mMaxAngle >= angle) {
        if (skMeleeVariants[i].mWeight >= roll) {
          variant = i;
          break;
        }
        roll -= skMeleeVariants[i].mWeight;
      }
    }
  }
  return variant;
}

static EMaterialTypes PathClearSolidMaterial = kMT_Solid;
static EMaterialTypes PathClearCollisionActorMaterial = kMT_CollisionActor;

bool CCommandoPirate::IsPathClear(CStateManager& mgr, const CVector3f& direction,
                                  float distance) const {
  const CVector3f center = GetBoundingBox().GetCenterPoint();
  const CVector3f end = center + distance * direction;
  const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
      CMaterialList(PathClearSolidMaterial), CMaterialList(PathClearCollisionActorMaterial));
  return mgr.RayCollideWorld(center, end, filter, this) &&
         mPathFindSearch.OnPath(end) == CPathFindSearch::kR_Success;
}

void CCommandoPirate::CheckDrowning(CStateManager& mgr) {
  if (mAlive) {
    if (InFluidId() != kInvalidUniqueId) {
      if (const CScriptWater* water =
              TCastToConstPtr< CScriptWater >(mgr.GetObjectById(InFluidId()))) {
        if (water->GetActive()) {
          const float waterTop = water->GetTriggerBoundsWR().GetMaxPoint().GetZ();
          const CTransform4f headXf = GetLctrTransform(mHeadSeg);
          if (headXf.GetTranslation().GetZ() < waterTop) {
            mRagDollTimer = 0.01f;
            HealthInfo()->SetHP(0.f);
            Death(mgr, GetTransform().GetForward(), kSS_DeathRattle);
            if (mDrowningEffect) {
              CExplosion* explosion =
                  rs_new CExplosion(*mDrowningEffect, mgr.AllocateUniqueId(),
                                    CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList,
                                                true, kInvalidEditorId),
                                    rstl::string_l("Commando Drowning Fx"), GetTransform(), 0,
                                    GetModelData()->GetScale(), CColor::White(), -1);
              if (explosion != nullptr) {
                mgr.AddObject(*explosion);
              }
            }
            CSfxManager::AddEmitter(mData.mSound_Drowning, GetTranslation(), 127,
                                    GetCurrentAreaId().Value(), true, false,
                                    CSfxManager::kMedPriority);
          }
        }
      }
    }
  }
}

static EMaterialTypes DeathFallSolidMaterial = kMT_Solid;
static EMaterialTypes DeathFallCharacterMaterial = kMT_Character;
static EMaterialTypes DeathFallPlayerMaterial = kMT_Player;
static EMaterialTypes DeathFallCollisionActorMaterial = kMT_CollisionActor;
static EMaterialTypes DeathFallPlatformMaterial = kMT_Platform;

void CCommandoPirate::HandleShieldHit(CStateManager& mgr, const TUniqueId& id) {
  const CWeapon* weapon = nullptr;
  if (id == mShieldCollisionActorId) {
    if (const CCollisionActor* colAct = TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(id))) {
      weapon = TCastToConstPtr< CWeapon >(mgr.GetObjectById(colAct->GetLastTouchedObject()));
      if (mShieldOn) {
        BreakShield(mgr);
      }
    }
  } else {
    weapon = TCastToPtr< CWeapon >(mgr.ObjectById(id));
  }
  if (weapon != nullptr) {
    const CKnockBackInfo info(GetTransform().GetForward(), weapon->GetUniqueId(),
                              weapon->GetOwnerId(), weapon->GetCurrentDamageInfo(), true);
    if (mShieldBroken) {
      mKnockBackController.SetAnimReactionRange(CKnockBackMgr::kAR_Hurled, CKnockBackMgr::kAR_Fall);
    }
    KnockBack(mgr, info);
    mKnockBackController.SetAnimReactionRange(CKnockBackMgr::kAR_None, CKnockBackMgr::kAR_Fall);
  }
  switch (mKnockBackController.GetActiveReaction()) {
  case CKnockBackMgr::kAR_KnockBack:
  case CKnockBackMgr::kAR_Hurled:
  case CKnockBackMgr::kAR_Fall:
    mAbortShield = true;
    break;
  default:
    mAbortShield = mAbortShield || mShieldBroken;
    break;
  }
  const CVector3f start = GetTranslation() + CVector3f::Up();
  const CVector3f end = start + 6.f * CVector3f::Down();
  if (mgr.RayCollideWorld(
          start, end,
          CMaterialFilter::MakeIncludeExclude(
              CMaterialList(DeathFallSolidMaterial),
              CMaterialList(DeathFallCharacterMaterial, DeathFallPlayerMaterial,
                            DeathFallCollisionActorMaterial, DeathFallPlatformMaterial)),
          this)) {
    mRagDollTimer = 0.01f;
    Death(mgr, GetTransform().GetForward(), kSS_DeathRattle);
    HealthInfo()->SetKnockbackResistance(0.f);
    if (weapon != nullptr) {
      mgr.RecordDamageSource(*this, weapon->GetUniqueId(), weapon->GetCurrentDamageInfo(), true,
                             false);
    }
    mAbortShield = true;
  }
}

bool CCommandoPirate::FindShieldChargeDest(CStateManager& mgr, bool useNextWaypoint) {
  const float chargeSpeed = mData.mShield.mChargeSpeed;
  if (chargeSpeed > 0.f) {
    const CScriptAIWaypoint* waypoint =
        TCastToPtr< CScriptAIWaypoint >(mgr.ObjectById(mWaypointNavigation.GetDestination()));
    if (waypoint != nullptr && (waypoint->GetFlags() & 0x20) != 0) {
      if (useNextWaypoint) {
        const TUniqueId nextId = waypoint->FindConnectedObject_if(mgr, kSS_Arrived, kSM_Next,
                                                                  CEffectWaypointPredicate());
        if (const CScriptWaypoint* next = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(nextId))) {
          mWaypointNavigation.SetDestination(nextId);
          mTargetPosition = next->GetTranslation();
          mChargeUp = next->GetTransform().GetUp();
          mChargeDuration = (mTargetPosition - GetTranslation()).Magnitude() / chargeSpeed;
          return true;
        }
      } else {
        mTargetPosition = waypoint->GetTranslation();
        mChargeUp = waypoint->GetTransform().GetUp();
        mChargeDuration = (mTargetPosition - GetTranslation()).Magnitude() / chargeSpeed;
        return true;
      }
    }
  }
  return false;
}

void CCommandoPirate::UpdateShieldCharge(float dt) {
  const float chargeSpeed = mData.mShield.mChargeSpeed;
  if (chargeSpeed > 0.f) {
    const CVector3f position = GetTranslation();
    const CQuaternion currentRotation = CQuaternion::FromMatrix(GetTransform());
    const CVector3f forward = GetTransform().GetForward();
    const CVector3f delta = mTargetPosition - position;
    const CVector3f direction =
        delta.IsMagnitudeSafe() ? delta.AsNormalized() : GetTransform().GetForward();
    const float timeLeft = delta.Magnitude() / chargeSpeed;
    const CTransform4f lookXf = CTransform4f::LookAt(position, position + direction, mChargeUp);
    const CQuaternion targetRotation = CQuaternion::FromMatrix(lookXf);
    const float t = CMath::Max(CMath::Min(1.f, 1.f - timeLeft / mChargeDuration), 0.f);
    const CQuaternion slerped = CQuaternion::SlerpLocal(currentRotation, targetRotation, t);
    SetTransform(slerped.BuildNormalized().BuildTransform4f(GetTranslation()));
    if (CVector3f::GetAngleDiff(direction, forward) > 10.f * (M_PIF / 180.f)) {
      if (CVector3f::Dot(direction, GetTransform().GetRight()) > 0.f) {
        mBodyController->CommandMgr().DeliverCmd(CBCLoopAttackCmd(pas::kLAT_Six, false, true));
      } else {
        mBodyController->CommandMgr().DeliverCmd(CBCLoopAttackCmd(pas::kLAT_Five, false, true));
      }
    } else {
      mBodyController->CommandMgr().DeliverCmd(CBCLoopAttackCmd(pas::kLAT_Four, false, true));
    }
    MoveInOneFrameOR(dt * (chargeSpeed * GetTransform().TransposeRotate(direction)), dt);
    if (mChargeCollisionNormal.IsNonZero()) {
      MoveInOneFrameOR(dt * (chargeSpeed * GetTransform().TransposeRotate(mChargeCollisionNormal)),
                       dt);
      mChargeCollisionNormal = CVector3f::Zero();
    }
  }
}

void CCommandoPirate::BreakShield(CStateManager& mgr) {
  if (!mShieldBroken) {
    mShieldBroken = true;
    CExplosion* explosion = nullptr;
    const CTransform4f xf = GetLctrTransform(mLeftWristSeg);
    if (mShieldEffectType == 1 && mShieldExplodeEffect) {
      explosion = rs_new CExplosion(
          *mShieldExplodeEffect, mgr.AllocateUniqueId(),
          CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true, kInvalidEditorId),
          rstl::string_l("Commando Explosion Fx"), xf, 0, GetModelData()->GetScale(),
          CColor::White(), -1);
    } else if (mShieldEffectType == 0 && mArmShieldExplodeEffect) {
      explosion = rs_new CExplosion(
          *mArmShieldExplodeEffect, mgr.AllocateUniqueId(),
          CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true, kInvalidEditorId),
          rstl::string_l("Commando Explosion Fx"), xf, 0, GetModelData()->GetScale(),
          CColor::White(), -1);
    }
    if (explosion != nullptr) {
      mgr.AddObject(explosion);
      CSfxManager::SfxStart(mData.mShield.mSound_Explode, CAudioSys::kMaxVolume, 64,
                            CSfxManager::kAllAreas, false, false, CSfxManager::kMedPriority);
      if (mSfxHandle) {
        CSfxManager::RemoveEmitter(mSfxHandle);
        mSfxHandle = CSfxHandle();
      }
    }
    mShieldEffectType = -1;
    mShieldEffectTargetScale = 0.f;
    mShieldEffectScale = 0.f;
  }
}

void CCommandoPirate::TurnShieldOn() {
  if (!mShieldBroken) {
    mShieldOn = true;
    if (!mSfxHandle) {
      mSfxHandle = CSfxManager::AddEmitter(mData.mShield.mSound_TurnOn, GetTranslation(), 127,
                                           GetCurrentAreaId().Value(), true, true,
                                           CSfxManager::kMedPriority);
    }
    mShieldEffectType = mShieldCharging ? 1 : (mArmingShield ? 0 : -1);
    mShieldEffectScale = 0.1f;
    mShieldEffectTargetScale = GetModelData()->GetScale().GetX();
  }
}

void CCommandoPirate::TurnShieldOff() {
  if (!mShieldBroken) {
    if (mSfxHandle) {
      CSfxManager::RemoveEmitter(mSfxHandle);
      mSfxHandle = CSfxHandle();
      CSfxManager::AddEmitter(mData.mShield.mSound_TurnOff, GetTranslation(), 127,
                              GetCurrentAreaId().Value(), true, false, CSfxManager::kMedPriority);
    }
    mShieldEffectTargetScale = 0.f;
    mRecentDamage = 0.f;
  }
}

void CCommandoPirate::UpdateShieldEffects(CStateManager& mgr, float dt) {
  if (mShieldEffectType != -1) {
    if (mShieldOn && mBodyController->IsFrozen()) {
      mAbortShield = true;
      mShieldCollisionMgr->SetActive(mgr, false);
    }
    if (mShieldChargeEffect.get() != nullptr) {
      mShieldChargeEffect->SetParticleEmission(false);
    }
    if (mArmShieldEffect.get() != nullptr) {
      mArmShieldEffect->SetParticleEmission(false);
    }
    if (dt < 0.5f) {
      mShieldEffectScale += dt * (mShieldEffectTargetScale - mShieldEffectScale) / 0.5f;
    } else {
      mShieldEffectScale = mShieldEffectTargetScale;
    }
    if (mShieldEffectScale < 0.1f && mShieldEffectTargetScale < 0.1f) {
      mShieldEffectType = -1;
    }
    switch (mShieldEffectType) {
    case 1:
      if (mShieldChargeEffect.get() != nullptr) {
        mShieldChargeEffect->SetParticleEmission(true);
        const CTransform4f xf = GetLctrTransform(mLeftWristSeg);
        mShieldChargeEffect->SetOrientation(xf.GetRotation());
        mShieldChargeEffect->SetGlobalTranslation(xf.GetTranslation());
        mShieldChargeEffect->SetGlobalScale(
            CVector3f(mShieldEffectScale, mShieldEffectScale, mShieldEffectScale));
        mShieldChargeEffect->Update(dt);
      }
      // fallthrough
    case 0:
      if (mArmShieldEffect.get() != nullptr) {
        mArmShieldEffect->SetParticleEmission(true);
        const CTransform4f xf = GetLctrTransform(mLeftWristSeg);
        mArmShieldEffect->SetOrientation(xf.GetRotation());
        mArmShieldEffect->SetGlobalTranslation(xf.GetTranslation());
        mArmShieldEffect->SetGlobalScale(
            CVector3f(mShieldEffectScale, mShieldEffectScale, mShieldEffectScale));
        mArmShieldEffect->Update(dt);
      }
      break;
    default:
      break;
    }
  }
}

void CCommandoPirate::ApplySeparation(CStateManager& mgr) {
  const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    if (const CPatterned* ai = TCastToConstPtr< CPatterned >(list[i])) {
      if (ai != this && ai->GetCurrentAreaId() == GetCurrentAreaId()) {
        const float radius = 5.f * GetModelData()->GetScale().GetX();
        const CVector3f separation =
            mSteeringBehaviors.Separation(*this, ai->GetTranslation(), radius);
        if (separation.IsMagnitudeSafe()) {
          mBodyController->CommandMgr().DeliverCmd(
              CBCLocomotionCmd(separation.AsNormalized(), CVector3f::Zero(), 0.5f));
        }
      }
    }
  }
}

void CCommandoPirate::JoinTeam(CStateManager& mgr) {
  if (mTeamAiMgrId == kInvalidUniqueId && (!mData.mNoMeleeAttack || !mData.mNoShieldCharge)) {
    mTeamAiMgrId = CScriptTeamAiMgr::GetAssociatedTeamId(*this, mgr);
    if (mTeamAiMgrId != kInvalidUniqueId) {
      if (CScriptTeamAiMgr* team = TCastToPtr< CScriptTeamAiMgr >(mgr.ObjectById(mTeamAiMgrId))) {
        team->JoinTeam(*this, CTeamAiRole::kTAR_Melee, CTeamAiRole::kTAR_Invalid,
                       CTeamAiRole::kTAR_Invalid);
      }
    }
  }
}

void CCommandoPirate::QuitTeam(CStateManager& mgr) {
  if (mTeamAiMgrId != kInvalidUniqueId) {
    if (CScriptTeamAiMgr* team = TCastToPtr< CScriptTeamAiMgr >(mgr.ObjectById(mTeamAiMgrId))) {
      if (team->IsPartOfTeam(GetUniqueId())) {
        team->QuitTeam(GetUniqueId());
        mTeamAiMgrId = kInvalidUniqueId;
      }
    }
  }
}

CEntity* LoadCommandoPirate(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrCommandoPirate sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrCommandoPirate.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  const CBouncyGrenadeData grenade(
      sldrThis.unknown_0xfb435257.grenadeMass, sldrThis.unknown_0xfb435257.unknown_0xed086ce0,
      LdrToDamageInfo(sldrThis.unknown_0xfb435257.grenadeDamage),
      sldrThis.unknown_0xfb435257.grenadeNumBounces, sldrThis.unknown_0xfb435257.grenadeExplosion,
      sldrThis.unknown_0xfb435257.grenadeExplosion, sldrThis.unknown_0xfb435257.grenadeTrail,
      sldrThis.unknown_0xfb435257.grenadeEffect, sldrThis.unknown_0xfb435257.sound_GrenadeBounce,
      sldrThis.unknown_0xfb435257.sound_GrenadeExplode, 0.1f, 150.f, 0.1f, 150.f, true);
  const CCommandoGrenadeData grenadeData(grenade, sldrThis.unknown_0xfb435257.eMPDuration,
                                         sldrThis.unknown_0xfb435257.grenadeMinAttackInterval,
                                         sldrThis.unknown_0xfb435257.grenadePostAttackPause,
                                         sldrThis.unknown_0xfb435257.grenadeAttackChance,
                                         sldrThis.unknown_0xfb435257.grenadeMinAttackDist,
                                         sldrThis.unknown_0xfb435257.grenadeMaxAttackDist,
                                         sldrThis.unknown_0xfb435257.grenadeMinLaunchSpeed,
                                         sldrThis.unknown_0xfb435257.grenadeMaxLaunchSpeed);
  const CCommandoShieldData shieldData(
      LdrToDamageInfo(sldrThis.shieldInfo.shieldChargeDamage),
      LdrToDamageVulnerability(sldrThis.shieldInfo.shieldVulnerability),
      sldrThis.shieldInfo.shieldChargeMinAttackDist, sldrThis.shieldInfo.shieldChargeMaxAttackDist,
      sldrThis.shieldInfo.shieldChargeSpeed, sldrThis.shieldInfo.shieldExplodeEffect,
      sldrThis.shieldInfo.sound_ShieldExplode, sldrThis.shieldInfo.unknown_0x6cb0da5a,
      sldrThis.shieldInfo.unknown_0xc3938663, sldrThis.shieldInfo.armShieldChance,
      sldrThis.shieldInfo.armShieldTime, sldrThis.shieldInfo.armShieldTimeVariation,
      sldrThis.shieldInfo.armShieldExplodeEffect, sldrThis.shieldInfo.shieldChargeEffect,
      sldrThis.shieldInfo.armShieldEffect, sldrThis.shieldInfo.sound_ShieldTurnOn,
      sldrThis.shieldInfo.sound_ShieldTurnOff);
  const CCommandoPirateData data(
      sldrThis.sound, sldrThis.aggressiveness, sldrThis.coverCheck, sldrThis.searchRadius,
      sldrThis.dodgeCheck, sldrThis.sound_Impact, sldrThis.sound_Hurled, sldrThis.sound_Death,
      sldrThis.alwaysFF, sldrThis.alwaysFF_0x467c3d94, LdrToDamageInfo(sldrThis.bladeDamage),
      sldrThis.projectile, LdrToDamageInfo(sldrThis.projectileDamage), sldrThis.sound_Projectile,
      sldrThis.hearingRadius, sldrThis.intraBurstShotTime, sldrThis.intraBurstShotVariation,
      grenadeData, shieldData);

  return rs_new CCommandoPirate(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      *modelData, LdrToActorParameters(sldrThis.actorInformation),
      LdrToPatternedInfo(sldrThis.patterned, &sldrThis.ingPossessionData), data);
}

static void SetFuncPtrs() {
  static SCommandoPirate_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &LoadCommandoPirate;
  SetSCommandoPirate_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSCommandoPirate_FuncPtrs(nullptr); }
