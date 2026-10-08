#include "MetroidPrime/Enemies/CFlyingPirate.hpp"

#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAxisAngle.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CKnockBackInfo.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Enemies/CSpacePirate.hpp"
#include "MetroidPrime/Enemies/CTeamAiRole.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrFlyingPirate.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIWaypoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCoverPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTeamAiMgr.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "MetroidPrime/Weapons/CImpactVisorEffect.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "REL/REL_Setup.h"
#include "rstl/math.hpp"

static const SBurst skBurstsFlying[] = {
    {10, {3, 4, 11, 12, -1, 0, 0, 0}, 0.1f, 0.05f},
    {20, {2, 3, 4, 5, -1, 0, 0, 0}, 0.1f, 0.05f},
    {20, {10, 11, 12, 13, -1, 0, 0, 0}, 0.1f, 0.05f},
    {25, {15, 16, 1, 2, -1, 0, 0, 0}, 0.1f, 0.05f},
    {25, {5, 6, 7, 8, -1, 0, 0, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.f, 0.f},
};

static const SBurst skBurstsFlyingOutOfView[] = {
    {5, {3, 4, 11, 12, -1, 0, 0, 0}, 0.1f, 0.05f},
    {10, {2, 3, 4, 5, -1, 0, 0, 0}, 0.1f, 0.05f},
    {10, {10, 11, 12, 13, -1, 0, 0, 0}, 0.1f, 0.05f},
    {40, {15, 16, 1, 2, -1, 0, 0, 0}, 0.1f, 0.05f},
    {35, {5, 6, 7, 8, -1, 0, 0, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.f, 0.f},
};

static const SBurst skBurstsLanded[] = {
    {30, {3, 4, 5, 11, 12, 4, -1, 0}, 0.1f, 0.05f},  {20, {2, 3, 4, 5, 4, 3, -1, 0}, 0.1f, 0.05f},
    {20, {5, 4, 3, 13, 12, 11, -1, 0}, 0.1f, 0.05f}, {30, {1, 2, 3, 4, 5, 6, -1, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.f, 0.f},
};

static const SBurst skBurstsLandedOutOfView[] = {
    {10, {6, 5, 4, 14, 13, 12, -1, 0}, 0.1f, 0.05f},
    {20, {14, 13, 12, 11, 10, 9, -1, 0}, 0.1f, 0.05f},
    {20, {14, 15, 16, 11, 10, 9, -1, 0}, 0.1f, 0.05f},
    {50, {11, 10, 9, 8, 7, 6, -1, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.f, 0.f},
};

const float CFlyingPirate::skGravityConstant = 50.f;
const float CFlyingPirate::skAquaGravityConstant = 5.f;

static EMaterialTypes skSolidMaterial = kMT_Unknown59;

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"HearShot", static_cast< CPatterned::StateMachine::TriggerFunc >(&CFlyingPirate::HearShot)},
    {"HearPlayer",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CFlyingPirate::HearPlayer)},
    {"ShouldSpecialAttack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CFlyingPirate::ShouldSpecialAttack)},
    {"ShouldAttack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CFlyingPirate::ShouldAttack)},
    {"LineOfSight",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CFlyingPirate::LineOfSight)},
    {"PatternOver",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CFlyingPirate::PatternOver)},
    {"PatternShagged",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CFlyingPirate::PatternShagged)},
    {"SpotPlayer",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CFlyingPirate::SpotPlayer)},
    {"ShouldDodge",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CFlyingPirate::ShouldDodge)},
    {"ShotAt", static_cast< CPatterned::StateMachine::TriggerFunc >(&CFlyingPirate::ShotAt)},
    {"Attacked", static_cast< CPatterned::StateMachine::TriggerFunc >(&CFlyingPirate::Attacked)},
    {"CoverCheck",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CFlyingPirate::CoverCheck)},
    {"CoverFind", static_cast< CPatterned::StateMachine::TriggerFunc >(&CFlyingPirate::CoverFind)},
    {"Landed", static_cast< CPatterned::StateMachine::TriggerFunc >(&CFlyingPirate::Landed)},
    {"ShouldMove",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CFlyingPirate::ShouldMove)},
    {"Stuck", static_cast< CPatterned::StateMachine::TriggerFunc >(&CFlyingPirate::Stuck)},
    {"DeathOver", static_cast< CPatterned::StateMachine::TriggerFunc >(&CFlyingPirate::DeathOver)},
    {"AnimOver", static_cast< CPatterned::StateMachine::TriggerFunc >(&CFlyingPirate::AnimOver)},
    {"InRange", static_cast< CPatterned::StateMachine::TriggerFunc >(&CFlyingPirate::InRange)},
    {"InPosition",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CFlyingPirate::InPosition)},
    {"AggressionCheck",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CFlyingPirate::AggressionCheck)},
    {"ShouldRetreat",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CFlyingPirate::ShouldRetreat)},
    {"HasAttackPattern",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CFlyingPirate::HasAttackPattern)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Attack", static_cast< CPatterned::StateMachine::StateFunc >(&CFlyingPirate::Attack)},
    {"PathFind", static_cast< CPatterned::StateMachine::StateFunc >(&CFlyingPirate::PathFind)},
    {"Patrol", static_cast< CPatterned::StateMachine::StateFunc >(&CFlyingPirate::Patrol)},
    {"TargetPatrol",
     static_cast< CPatterned::StateMachine::StateFunc >(&CFlyingPirate::TargetPatrol)},
    {"TurnAround", static_cast< CPatterned::StateMachine::StateFunc >(&CFlyingPirate::TurnAround)},
    {"Dodge", static_cast< CPatterned::StateMachine::StateFunc >(&CFlyingPirate::Dodge)},
    {"Lurk", static_cast< CPatterned::StateMachine::StateFunc >(&CFlyingPirate::Lurk)},
    {"Taunt", static_cast< CPatterned::StateMachine::StateFunc >(&CFlyingPirate::Taunt)},
    {"Dead", static_cast< CPatterned::StateMachine::StateFunc >(&CFlyingPirate::Dead)},
    {"GetUp", static_cast< CPatterned::StateMachine::StateFunc >(&CFlyingPirate::GetUp)},
    {"GetUpNow", static_cast< CPatterned::StateMachine::StateFunc >(&CFlyingPirate::GetUpNow)},
    {"Jump", static_cast< CPatterned::StateMachine::StateFunc >(&CFlyingPirate::Jump)},
    {"ProjectileAttack",
     static_cast< CPatterned::StateMachine::StateFunc >(&CFlyingPirate::ProjectileAttack)},
    {"Land", static_cast< CPatterned::StateMachine::StateFunc >(&CFlyingPirate::Land)},
    {"Walk", static_cast< CPatterned::StateMachine::StateFunc >(&CFlyingPirate::Walk)},
    {"Retreat", static_cast< CPatterned::StateMachine::StateFunc >(&CFlyingPirate::Retreat)},
    {"Explode", static_cast< CPatterned::StateMachine::StateFunc >(&CFlyingPirate::Explode)},
    {"Enraged", static_cast< CPatterned::StateMachine::StateFunc >(&CFlyingPirate::Enraged)},
    {"Bounce", static_cast< CPatterned::StateMachine::StateFunc >(&CFlyingPirate::Bounce)},
    {"Deactivate", static_cast< CPatterned::StateMachine::StateFunc >(&CFlyingPirate::Deactivate)},
    {"ChooseWaypoint",
     static_cast< CPatterned::StateMachine::StateFunc >(&CFlyingPirate::ChooseWaypoint)},
};

static const SBurst* skBursts[] = {
    skBurstsFlying, skBurstsFlyingOutOfView, skBurstsLanded, skBurstsLandedOutOfView, nullptr,
};

static rstl::string skJetPack = rstl::string_l("JetPack");
static rstl::string skScubaGear = rstl::string_l("ScubaGear");
static rstl::string skScubaBubbles = rstl::string_l("ScubaBubbles");
static rstl::string skSparks = rstl::string_l("Sparks");
static rstl::string skLandingSmoke = rstl::string_l("LandingSmoke");
static rstl::string skEyes = rstl::string_l("Eyes");

CFlyingPirate::CFlyingPirateData::CFlyingPirateData(
    float maxCoverDistance, float hearingDistance, uint type, CAssetId projectile,
    const CDamageInfo& projectileDamage, ushort gunSfx, CAssetId missile,
    const CDamageInfo& missileDamage, CAssetId wpsc, float knockBackDelay, float flyingHeight,
    CAssetId rocketPackExplosion, const CDamageInfo& rocketPackExplosionDamage, float spiralChance,
    float minimumMissileTime, float missileTimeVariation, float flightThrust, ushort impactSfx,
    ushort spiralSfx, float coverCheckChance, float intraBurstShotTime,
    float intraBurstShotVariation, CAssetId landingCloudDirt, CAssetId landingCloudDust,
    CAssetId landingCloudSnow, ushort hurledSfx, ushort deathSfx, float aggressionChance,
    float jumpAggressionChance, float projectileHomingDistance, float unknown_0xccf05648,
    float unknown_0x2a90f9a9, float unknown_0x9ca8f357, float unknown_0x7ac85cb6)
: mMaxCoverDistance(maxCoverDistance)
, mHearingDistance(hearingDistance)
, mType(type)
, mProjectile(projectile)
, mProjectileDamage(projectileDamage)
, mGunSfx(gunSfx)
, mMissile(missile)
, mMissileDamage(missileDamage)
, mWpsc(wpsc)
, mWpscDamage(CDamageInfo())
, mKnockBackDelay(knockBackDelay)
, mFlyingHeight(flyingHeight)
, mRocketPackExplosion(rocketPackExplosion)
, mDInfo(rocketPackExplosionDamage)
, mSpiralChance(spiralChance)
, mMinimumMissileTime(minimumMissileTime)
, mMissileTimeVariation(missileTimeVariation)
, mFlightThrust(flightThrust)
, mRagDollSfx1(impactSfx)
, mRagDollSfx2(spiralSfx)
, mCoverCheckChance(coverCheckChance)
, mIntraBurstShotTime(intraBurstShotTime)
, mIntraBurstShotVariation(intraBurstShotVariation)
, mParticleGen1(landingCloudDirt)
, mParticleGen2(landingCloudDust)
, mParticleGen3(landingCloudSnow)
, mKnockBackSfx(hurledSfx)
, mDeathSfx(deathSfx)
, mAggressionChance(aggressionChance)
, mJumpAggressionChance(jumpAggressionChance)
, mProjectileHomingDistance(projectileHomingDistance)
, unknown_0xccf05648(unknown_0xccf05648)
, unknown_0x2a90f9a9(unknown_0x2a90f9a9)
, unknown_0x9ca8f357(unknown_0x9ca8f357)
, unknown_0x7ac85cb6(unknown_0x7ac85cb6) {}

CFlyingPirate::CFlyingPirate(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                             const CTransform4f& xf, const CModelData& modelData,
                             const CActorParameters& actParms, const CPatternedInfo& pInfo,
                             const CFlyingPirateData& data)
: CPatterned(kPAI_FlyingPirate, uid, name, kFT_Zero, info, xf, modelData, pInfo, kMT_Flyer, kCT_One,
             kBT_AiMovedFlyer, actParms)
, mData(data)
, mGunProjectileInfo(data.mProjectile, data.mProjectileDamage)
, mAltProjectileInfo1(data.mMissile, data.mMissileDamage)
, mAltProjectileInfo2(data.mWpsc, data.mWpscDamage)
, mParticleGenDesc(gpSimplePool->GetObj(SObjectTag('PART', data.mRocketPackExplosion)))
, mCurrentCoverPoint(kInvalidUniqueId)
, mPathFindSearch(nullptr, (mData.mType & 2) ? 4 : 3, pInfo.GetPathfindingIndex(),
                  pInfo.GetHalfExtent() * modelData.GetScale().GetX(),
                  pInfo.GetHeight() * modelData.GetScale().GetZ(), 1, CPFRegion::kRP_Center)
, x78c_(0.f)
, x790_(0)
, mInitialHealth(pInfo.GetHealthInfo().GetHP())
, mHeadSegId(CSegId::Invalid())
, mBoneTracking(*GetModelData()->GetAnimationData(), rstl::string_l("Head_1"), CMath::Deg2Rad(80.f),
                CMath::Deg2Rad(180.f), kBTF_None)
, mGunSegId(CSegId::Invalid())
, mBurstTimer(1.f)
, mTargetId(kInvalidUniqueId)
, mBurstFire(skBursts, 0)
, mDodgeDirection(pas::kSD_Invalid)
, mHeight(3.f)
, mTimeSinceAttacked(3.4028235e38f)
, mTimeSinceShotAt(3.4028235e38f)
, mAttackObjectId(kInvalidUniqueId)
, mSpecialAttackTimer(15.f)
, mMissileTimer(0.f)
, mFlightVelocity(CVector3f::Zero())
, mFlightAcceleration(CVector3f::Zero())
, mCoverCheckTimer(10.f)
, mRagDollTimer(3.f)
, mTeamAiMgr(kInvalidUniqueId)
, mPitchBend(1.f)
, mTargetPitchBend(1.f)
, mPatrolTarget(kInvalidUniqueId)
, mPatrolSpeed(0.f)
, mLineOfSightTracker(GetUniqueId(), CSegId::Invalid(), 0.2f, 0.05f)
, mFireMissilesCheck(0)
, mFireMissilesCheckInterval(11)
, mCanFireMissiles(false)
, mIsFlyingPirate(mData.mType & 1)
, mIsAquaPirate(mData.mType & 2)
, mHearShot(false)
, mCanPatrol(false)
, mAimAtTarget(false)
, mCheckForProjectiles(false)
, mShotAt(false)
, mPrevInCineCam(false)
, x6a1_25_(false)
, mIsAttackingObject(false)
, x6a1_27_(false)
, mMissilePathBlocked(false)
, mIsMoving(false)
, mSpinToDeath(false)
, mStopped(false)
, mAggressive(false)
, mAggressionChecked(false)
, mJetpackActive(false)
, mSparksActive(false)
, mRetreatRequested(false)
, mDeathSpinFinished(false)
, mBecameRagDoll(false) {
  mGunProjectileInfo.Token().Lock();
  mAltProjectileInfo1.Token().Lock();
  mAltProjectileInfo2.Token().Lock();
  const CAnimData* animData = GetModelData()->GetAnimationData();
  mHeadSegId = animData->GetLocatorSegId(rstl::string_l("Head_1"));
  mGunSegId = animData->GetLocatorSegId(rstl::string_l("L_gun_LCTR"));
  mMissileSegments.push_back(animData->GetLocatorSegId(rstl::string_l("L_Missile_LCTR")));
  mMissileSegments.push_back(animData->GetLocatorSegId(rstl::string_l("R_Missile_LCTR")));
  mBoneTracking.SetDisableTrackingDistance(25.f * GetModelData()->GetScale().GetZ());
  const CPASAnimParmData parms(pas::kAS_Step, CPASAnimParm::FromEnum(3), CPASAnimParm::FromEnum(1));
  mHeight = GetModelData()->GetScale().GetX() * GetAnimationDistance(parms);
  if (mData.mParticleGen1 != kInvalidAssetId && mData.mParticleGen2 != kInvalidAssetId &&
      mData.mParticleGen3 != kInvalidAssetId) {
    mParticleGenDescs.push_back(gpSimplePool->GetObj(SObjectTag('PART', mData.mParticleGen1)));
    mParticleGenDescs.push_back(gpSimplePool->GetObj(SObjectTag('PART', mData.mParticleGen2)));
    mParticleGenDescs.push_back(gpSimplePool->GetObj(SObjectTag('PART', mData.mParticleGen3)));
    for (int i = 0; i < mParticleGenDescs.size(); ++i) {
      mParticleGens.push_back(rs_new CElementGen(mParticleGenDescs[i]));
      mParticleGens[i]->SetParticleEmission(false);
    }
  }
  KnockBackController().SetLocomotionDuringElectrocution(true);
  mOnGround = !mIsFlyingPirate;
  mLineOfSightTracker.SetSegment(mHeadSegId);
  mLineOfSightTracker.SetRayFilter(CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Unknown59, kMT_Character),
      CMaterialList(kMT_Player, kMT_CollisionActor, kMT_NoPlatformCollision,
                    kMT_ExcludeFromLineOfSightTest)));
}

void CFlyingPirate::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  switch (message) {
  case kSM_Alert:
    if (GetActive()) {
      mHitByPlayerProjectile = true;
    }
    break;
  case kSM_Activate:
    AddToTeam(mgr);
    break;
  case kSM_Deactivate:
  case kSM_Delete:
    RemoveFromTeam(mgr);
    break;
  }
  CPatterned::AcceptScriptMsg(mgr, msg);
  switch (message) {
  case kSM_AreaLoaded: {
    for (AUTO(it, GetConnectionList().begin()); it != GetConnectionList().end(); ++it) {
      if (it->state == kSS_Retreat) {
        const TUniqueId id = mgr.GetIdForScript(it->objId);
        if (CScriptCoverPoint* cover = TCastToPtr< CScriptCoverPoint >(mgr.ObjectById(id))) {
          cover->Reserve(GetUniqueId());
        }
      } else if (it->state == kSS_Patrol && it->msg == kSM_Follow) {
        mCanPatrol = true;
      } else if (it->state == kSS_Attack && it->msg == kSM_Action) {
        mAttackObjectId = mgr.GetIdForScript(it->objId);
      }
    }
    mPathFindSearch.SetArea(
        mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()).GetPostConstructed()->mPathArea);
    if (GetActive()) {
      AddToTeam(mgr);
    }
    const bool jetpackActive = mIsFlyingPirate;
    UpdateParticleEffects(mgr, 0.f, jetpackActive);
    AnimationData()->SetEffectState(skEyes, true, mgr);
    mLineOfSightTracker.SetTarget(mgr.GetPlayer(0)->GetUniqueId());
    break;
  }
  case kSM_Create: {
    const float range = mData.mMissileTimeVariation;
    const float delay = mData.mMinimumMissileTime;
    mMissileTimer = range * mgr.Random()->Float() + delay;
    break;
  }
  case kSM_Falling:
    if (GetBodyController()->GetPercentageFrozen() == 0.f && !mFadeToDeath && !mSpinToDeath) {
      SetMomentumWR(CVector3f(0.f, 0.f, -GetGravityConstant() * GetMass()));
    }
    mBurstFire.SetBurstType(0);
    break;
  case kSM_Landed:
    mBurstFire.SetBurstType(2);
    break;
  case kSM_Launching:
    if (CScriptCoverPoint* cover = GetCoverPoint(mgr, mCurrentCoverPoint)) {
      mVerticalMovement = false;
      SetMomentumWR(CVector3f(0.f, 0.f, -GetMass() * GetGravityConstant()));
      AddMaterial(kMT_GroundCollider, mgr);
      SetDestPos(cover->GetTranslation());
      const CVector3f delta = cover->GetTranslation() - GetTranslation();
      if (delta.GetZ() < 0.f) {
        CVector3f velocity = GetVelocityWR();
        const float gravity = GetGravityConstant();
        const float root =
            CMath::FastSqrtF(-(2.f * gravity * delta.GetZ() - velocity[kDZ] * velocity[kDZ]));
        float verticalVelocity = -velocity[kDZ];
        verticalVelocity += root;
        const float time = verticalVelocity / gravity;
        if (time > 0.f) {
          const CVector2f normal(delta.ToVec2f().AsNormalized());
          const float speed = delta.ToVec2f().Magnitude() / time;
          velocity.SetX(speed * normal[0]);
          velocity.SetY(speed * normal[1]);
          SetVelocityWR(velocity);
          mFlightVelocity = CVector3f::Zero();
          mFlightAcceleration = CVector3f::Zero();
          mTargetPitchBend = 1.f;
        }
      }
    }
    break;
  case kSM_Start:
    mStopped = false;
    break;
  case kSM_Stop:
    mStopped = true;
    break;
  case kSM_SetToZero:
    mRetreatRequested = true;
    break;
  }
}

bool CFlyingPirate::Listen(CStateManager& mgr, const CVector3f& pos, EListenNoiseType type) {
  bool heard = false;
  if (mAlive) {
    const float hearingDistance = mData.mHearingDistance * mData.mHearingDistance;
    const CVector3f delta = pos - GetTranslation();
    if (delta.MagSquared() < hearingDistance &&
        (mDetectionHeightRange == 0.f ||
         delta.GetZ() * delta.GetZ() < mDetectionHeightRange * mDetectionHeightRange)) {
      mHearShot = true;
      heard = true;
    }
    if (type == kLNT_PlayerFire) {
      mCheckForProjectiles = true;
    }
  }
  const bool result = heard;
  return result;
}

void CFlyingPirate::DeliverGetUp() {
  if (BodyController()->GetCurrentStateId() == pas::kAS_LieOnGround) {
    BodyController()->CommandMgr().DeliverCmd(CBCGetupCmd(pas::kGetup_Zero));
  }
}

void CFlyingPirate::UpdateParticleEffects(CStateManager& mgr, float intensity, bool active) {
  CAnimData* animData = AnimationData();
  const rstl::string& name = mIsAquaPirate ? skScubaGear : skJetPack;
  if (active != mJetpackActive) {
    animData->SetEffectState(name, active, mgr);
    if (mIsAquaPirate) {
      animData->SetEffectState(skScubaBubbles, active, mgr);
    }
    mJetpackActive = active;
  }
  if (active) {
    animData->SetEffectComponentExternalParam(
        name, 0,
        intensity * (mData.unknown_0x2a90f9a9 - mData.unknown_0xccf05648) +
            mData.unknown_0xccf05648);
    animData->SetEffectComponentExternalParam(
        name, 1,
        intensity * (mData.unknown_0x7ac85cb6 - mData.unknown_0x9ca8f357) +
            mData.unknown_0x9ca8f357);
  }
  if (!mIsAquaPirate) {
    bool sparks = active && intensity > 0.8f;
    if (sparks != mSparksActive) {
      animData->SetEffectState(skSparks, sparks, mgr);
      mSparksActive = sparks;
    }
  }
}

void CFlyingPirate::UpdateLandingSmoke(CStateManager& mgr, bool active) {
  if (active) {
    if (!mParticleGens.empty()) {
      float particleLevel = GetTranslation().GetZ() - 5.f;
      CScriptCoverPoint* cover = GetCoverPoint(mgr, mCurrentCoverPoint);
      if (cover != nullptr) {
        particleLevel = cover->GetTranslation().GetZ() - 1.f;
      }
      const CRayCastResult result = mgr.RayStaticIntersection(
          GetTranslation(), CVector3f::Down(), GetTranslation().GetZ() - particleLevel,
          CMaterialFilter::MakeInclude(CMaterialList(kMT_Unknown59)));
      int index = 1;
      if (result.IsValid()) {
        const CMaterialList& material = result.GetMaterial();
        if (material.HasMaterial(kMT_Dirt) || material.HasMaterial(kMT_Organic) ||
            material.HasMaterial(kMT_Sand)) {
          index = 0;
        }
        particleLevel = GetTranslation().GetZ() - result.GetTime();
      }
      mParticleGens[index]->SetParticleEmission(true);
      const CVector3f& origin = GetTranslation();
      mParticleGens[index]->SetTranslation(CVector3f(origin.GetX(), origin.GetY(), particleLevel));
    }
    AnimationData()->SetEffectState(skLandingSmoke, true, mgr);
  } else {
    for (int i = 0; i < mParticleGens.size(); ++i) {
      mParticleGens[i]->SetParticleEmission(false);
    }
    AnimationData()->SetEffectState(skLandingSmoke, false, mgr);
  }
}

CVector3f CFlyingPirate::GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                                   const CVector3f& aimPos) const {
  return GetTranslation();
}

void CFlyingPirate::AddToTeam(CStateManager& mgr) {
  if (mTeamAiMgr == kInvalidUniqueId) {
    mTeamAiMgr = CScriptTeamAiMgr::GetAssociatedTeamId(*this, mgr);
  }
  if (mTeamAiMgr != kInvalidUniqueId) {
    if (CScriptTeamAiMgr* team = TCastToPtr< CScriptTeamAiMgr >(mgr.ObjectById(mTeamAiMgr))) {
      team->JoinTeam(*this, CTeamAiRole::kTAR_Projectile, CTeamAiRole::kTAR_Unknown,
                     CTeamAiRole::kTAR_Invalid);
    }
  }
}

void CFlyingPirate::RemoveFromTeam(CStateManager& mgr) {
  if (mTeamAiMgr != kInvalidUniqueId) {
    if (CScriptTeamAiMgr* team = TCastToPtr< CScriptTeamAiMgr >(mgr.ObjectById(mTeamAiMgr))) {
      if (team->IsPartOfTeam(GetUniqueId())) {
        team->QuitTeam(GetUniqueId());
        mTeamAiMgr = kInvalidUniqueId;
      }
    }
  }
}

void CFlyingPirate::CheckForProjectiles(CStateManager& mgr) {
  if (mCheckForProjectiles) {
    const CVector3f playerPos = mgr.GetPlayer(0)->GetTranslation();
    const CVector3f extent(5.f, 5.f, 5.f);
    const CAABox box(playerPos - extent, playerPos + extent);
    mShotAt = false;
    rstl::reserved_vector< TUniqueId, 1024 > nearList;
    mgr.BuildNearList(nearList, box, CMaterialFilter::MakeInclude(CMaterialList(kMT_Projectile)),
                      nullptr);
    for (int i = 0; i < nearList.size(); ++i) {
      if (const CGameProjectile* const projectile =
              TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(nearList[i]))) {
        CVector3f delta = GetBoundingBox().GetCenterPoint() - projectile->GetTranslation();
        if (delta.IsMagnitudeSafe()) {
          if (CVector3f::Dot(GetTransform().GetForward(), delta) < 0.f) {
            delta.Normalize();
            CVector3f movement = projectile->GetTranslation() - projectile->GetPreviousPos();
            if (movement.IsMagnitudeSafe()) {
              movement.Normalize();
              if (CVector3f::Dot(movement, delta) > 0.939f) {
                mShotAt = true;
              }
            }
          }
        } else {
          mShotAt = true;
        }
        if (mShotAt) {
          break;
        }
      }
    }
    mCheckForProjectiles = false;
  }
}

bool CFlyingPirate::LineOfSightTest(CStateManager& mgr, const CVector3f& start,
                                    const CVector3f& end, const CMaterialList& exclude) {
  const CMaterialFilter filter =
      CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Unknown59), exclude);
  return mgr.RayCollideWorld(start, end, filter, this);
}

CVector3f CFlyingPirate::AvoidActors(CStateManager& mgr) {
  CVector3f separation = CVector3f::Zero();
  const CVector3f extent(8.f, 8.f, 8.f);
  const CAABox box(GetTranslation() - extent, GetTranslation() + extent);
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  mgr.BuildNearList(nearList, box, CMaterialFilter::MakeInclude(CMaterialList(kMT_Character)),
                    this);
  for (int i = 0; i < nearList.size(); ++i) {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(nearList[i]))) {
      separation += mSteeringBehaviors.Separation(*this, actor->GetTranslation(), 10.f);
    }
  }
  CVector3f delta = mgr.GetPlayer(0)->GetTranslation() - GetTranslation();
  delta.SetZ(0.f);
  separation += mSteeringBehaviors.Separation(*this, GetTranslation() + delta, 20.f);
  return separation;
}

pas::EStepDirection CFlyingPirate::GetDodgeDirection(CStateManager& mgr, float arg) {
  const float argSquared = arg * arg;
  bool canDodgeLeft = true;
  bool canDodgeRight = true;
  bool canDodgeUp = true;
  bool canDodgeDown = true;
  pas::EStepDirection direction = pas::kSD_Invalid;
  const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    if (const CPhysicsActor* actor = TCastToConstPtr< CPhysicsActor >(list[i])) {
      if (actor != this && actor->GetCurrentAreaId() == GetCurrentAreaId()) {
        const CVector3f delta = actor->GetTranslation() - GetTranslation();
        const float magSquared = delta.MagSquared();
        if (magSquared < argSquared) {
          float rightDot = CVector3f::Dot(delta, GetTransform().GetRight());
          if (rightDot > 0.866f * magSquared || (rightDot > 0.f && magSquared < 3.f)) {
            canDodgeRight = false;
          } else if (rightDot < 0.866f * -magSquared || (rightDot < 0.f && magSquared < 3.f)) {
            canDodgeLeft = false;
          }
          float upDot = CVector3f::Dot(delta, GetTransform().GetUp());
          if (upDot > 0.866f * magSquared || (upDot > 0.f && magSquared < 3.f)) {
            canDodgeUp = false;
          } else if (upDot < 0.866f * -magSquared || (upDot < 0.f && magSquared < 3.f)) {
            canDodgeDown = false;
          }
        }
      }
    }
  }
  const CVector3f center = GetBoundingBox().GetCenterPoint();
  if (canDodgeRight) {
    canDodgeRight = mPathFindSearch.OnPath(center + arg * GetTransform().GetRight()) ==
                    CPathFindSearch::kR_Success;
  }
  if (canDodgeLeft) {
    canDodgeLeft = mPathFindSearch.OnPath(center - arg * GetTransform().GetRight()) ==
                   CPathFindSearch::kR_Success;
  }
  if (canDodgeUp) {
    canDodgeUp = mPathFindSearch.OnPath(center + arg * GetTransform().GetUp()) ==
                 CPathFindSearch::kR_Success;
  }
  if (canDodgeDown) {
    canDodgeDown = mPathFindSearch.OnPath(center - arg * GetTransform().GetUp()) ==
                   CPathFindSearch::kR_Success;
  }
  if ((canDodgeLeft || canDodgeRight) && (canDodgeUp || canDodgeDown)) {
    if ((mgr.Random()->Next() & 0x4000) != 0) {
      canDodgeLeft = false;
      canDodgeRight = false;
    } else {
      canDodgeUp = false;
      canDodgeDown = false;
    }
  }
  if (canDodgeLeft && canDodgeRight) {
    if ((mgr.Random()->Next() & 0x4000) != 0) {
      canDodgeLeft = false;
    } else {
      canDodgeRight = false;
    }
  }
  if (canDodgeUp && canDodgeDown) {
    const float height = mData.mFlyingHeight;
    if (GetTargetPos(mgr).GetZ() - (GetTranslation()[kDZ] - -height) > 0.f) {
      canDodgeDown = false;
    } else {
      canDodgeUp = false;
    }
  }
  if (canDodgeUp) {
    direction = pas::kSD_Up;
  } else if (canDodgeDown) {
    direction = pas::kSD_Down;
  } else if (canDodgeLeft) {
    direction = pas::kSD_Left;
  } else if (canDodgeRight) {
    direction = pas::kSD_Right;
  }
  return direction;
}

CVector3f CFlyingPirate::GetTargetPos(CStateManager& mgr) {
  if (mTargetId != mgr.GetPlayer(0)->GetUniqueId()) {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mTargetId))) {
      if (actor->GetActive()) {
        return actor->GetTranslation();
      }
    }
    mBoneTracking.SetTarget(mgr.GetPlayer(0)->GetUniqueId());
    mTargetId = mgr.GetPlayer(0)->GetUniqueId();
  }
  return mgr.GetPlayer(0)->GetAimPosition(mgr, 0.f);
}

void CFlyingPirate::CheckFireMissiles(CStateManager& mgr) {
  if (mFireMissilesCheck < 4) {
    if (mFireMissilesCheck == 0) {
      mCanFireMissiles = false;
      mMissilePathBlocked = false;
    }
    if (!mMissilePathBlocked) {
      const int step = mFireMissilesCheck & 1;
      const CTransform4f xf = GetLctrTransform(mMissileSegments[mFireMissilesCheck >> 1]);
      const CVector3f end = xf.GetTranslation() + 3.f * xf.GetForward();
      switch (step) {
      case 0:
        mMissilePathBlocked = !LineOfSightTest(mgr, end, GetTargetPos(mgr),
                                               CMaterialList(kMT_Player, kMT_NoPlatformCollision));
        break;
      case 1:
        mMissilePathBlocked = !LineOfSightTest(mgr, xf.GetTranslation(), end,
                                               CMaterialList(kMT_Player, kMT_NoPlatformCollision));
        break;
      }
    }
    if (mFireMissilesCheck == 3) {
      mCanFireMissiles = !mMissilePathBlocked;
    }
  }
  ++mFireMissilesCheck;
  mFireMissilesCheck %= mFireMissilesCheckInterval;
}

void CFlyingPirate::KnockBack(CStateManager& mgr, const CKnockBackInfo& info) {
  if (mAlive) {
    KnockBackController().SetSeverity(mVerticalMovement ? pas::kS_Zero : pas::kS_One);
  } else if (!IsOnGround()) {
    const float chance = mData.mSpiralChance;
    if (mgr.Random()->Range(0.f, 100.f) <= chance) {
      mSpinToDeath = true;
      SetMomentumWR(CVector3f::Zero());
    } else {
      UpdateParticleEffects(mgr, 0.f, false);
      SetMomentumWR(CVector3f(0.f, 0.f, -GetGravityConstant() * GetMass()));
    }
    mBoneTracking.SetActive(false);
    mVerticalMovement = false;
  }
  CPatterned::KnockBack(mgr, info);
  if (mAlive) {
    switch (GetKnockBackController().GetActiveReaction()) {
    case CKnockBackMgr::kAR_Hurled:
      if (!GetBodyController()->IsFrozen()) {
        mStateMachine->SetState(mgr, *this, rstl::string_l("GetUpNow"));
        mStateMachine->SetDelay(mData.mKnockBackDelay);
      }
      mMissilePathBlocked = false;
      mVerticalMovement = false;
      CSfxManager::AddEmitter(mData.mKnockBackSfx, GetTranslation(), 127,
                              GetCurrentAreaId().Value(), true, false, CSfxManager::kMedPriority);
      break;
    }
  } else {
    if (!IsOnGround() && (mBurning || mLaggedBurnDeath)) {
      mSpinToDeath = false;
      mVerticalMovement = true;
      SetMomentumWR(CVector3f::Zero());
    } else {
      switch (GetKnockBackController().GetActiveReaction()) {
      case CKnockBackMgr::kAR_Hurled:
        CSfxManager::AddEmitter(mData.mDeathSfx, GetTranslation(), 127, GetCurrentAreaId().Value(),
                                true, false, CSfxManager::kMedPriority);
        if (mFadeToDeath) {
          mSpinToDeath = false;
          UpdateParticleEffects(mgr, 0.f, false);
          SetMomentumWR(CVector3f(0.f, 0.f, -GetGravityConstant() * GetMass()));
        }
        break;
      }
    }
    if (mSpinToDeath) {
      StartSpinToDeath(mgr);
    }
  }
}

void CFlyingPirate::MassiveDeath(CStateManager& mgr) {
  CExplosion* explosion = rs_new CExplosion(
      mParticleGenDesc, mgr.AllocateUniqueId(),
      CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true), rstl::string_l(""),
      GetTransform(), 0, CVector3f(1.5f, 1.5f, 1.5f), CColor::White(), -1);
  if (explosion != nullptr) {
    mgr.AddObject(*explosion);
    mgr.ApplyDamageToWorld(
        GetUniqueId(), *this, GetTranslation(), mData.mDInfo,
        CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Unknown59), CMaterialList()));
    for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
    }
  }
  CPatterned::MassiveDeath(mgr);
}

void CFlyingPirate::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  if (mCanPatrol) {
    CPatterned::Patrol(mgr, msg, dt);
    switch (msg) {
    case kStateMsg_Activate:
      BodyController()->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
      mPatrolTarget = mWaypointNavigation.GetDestination();
      mPatrolSpeed = 0.f;
      break;
    case kStateMsg_Update:
      if (mWaypointNavigation.GetDestination() != mPatrolTarget) {
        mPatrolTarget = mWaypointNavigation.GetDestination();
        mPatrolSpeed = 0.f;
      }
      if (mWaypointNavigation.IsMoving()) {
        const CVector3f delta = mWaypointNavigation.GetDestinationPosition() - GetTranslation();
        const CVector3f direction =
            delta.IsMagnitudeSafe() ? delta.AsNormalized() : GetTransform().GetForward();
        const float moveSpeed = mWaypointNavigation.GetMoveSpeed();
        const float speed = moveSpeed * mData.mFlightThrust;
        mPatrolSpeed = dt * speed + mPatrolSpeed;
        mPatrolSpeed = rstl::min_val(speed, mPatrolSpeed);
        mFlightAcceleration = (dt * (mPatrolSpeed * dt)) * direction;
        mTargetPitchBend = 1.5f * mWaypointNavigation.GetMoveSpeed();
        mFlightVelocity += mFlightAcceleration;
      }
      UpdatePatrolFacing(mgr);
      mLineOfSightTracker.Update(dt, mgr);
      break;
    case kStateMsg_Deactivate:
      BodyController()->CommandMgr().SetSteeringBlendMode(kSBM_Normal);
      break;
    }
  }
}

void CFlyingPirate::TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::Patrol(mgr, msg, dt);
  switch (msg) {
  case kStateMsg_Activate: {
    BodyController()->CommandMgr().SetSteeringBlendMode(kSBM_Normal);
    mWaypointNavigation.SetDestination(GetConnectedObject(mgr, kSS_Attack, kSM_Follow));
    if (mWaypointNavigation.GetDestination() != kInvalidUniqueId) {
      if (const CScriptAIWaypoint* waypoint = TCastToConstPtr< CScriptAIWaypoint >(
              mgr.GetObjectById(mWaypointNavigation.GetDestination()))) {
        mWaypointNavigation.SetMoveSpeed(waypoint->GetSpeed());
      }
    }
    mPatrolTarget = mWaypointNavigation.GetDestination();
    mPatrolSpeed = 0.f;
    break;
  }
  case kStateMsg_Update:
    if (mWaypointNavigation.GetDestination() != mPatrolTarget) {
      mPatrolTarget = mWaypointNavigation.GetDestination();
      mPatrolSpeed = 0.f;
    }
    if (mWaypointNavigation.IsMoving()) {
      const CVector3f delta = mWaypointNavigation.GetDestinationPosition() - GetTranslation();
      const CVector3f direction =
          delta.IsMagnitudeSafe() ? delta.AsNormalized() : GetTransform().GetForward();
      const float moveSpeed = mWaypointNavigation.GetMoveSpeed();
      const float speed = moveSpeed * mData.mFlightThrust;
      mPatrolSpeed = dt * speed + mPatrolSpeed;
      mPatrolSpeed = rstl::min_val(speed, mPatrolSpeed);
      mFlightAcceleration = (dt * (mPatrolSpeed * dt)) * direction;
      mTargetPitchBend = 1.5f * mWaypointNavigation.GetMoveSpeed();
      mFlightVelocity += mFlightAcceleration;
    }
    UpdatePatrolFacing(mgr);
    mLineOfSightTracker.Update(dt, mgr);
    break;
  }
}

void CFlyingPirate::UpdatePatrolFacing(CStateManager& mgr) {
  if (const CScriptAIWaypoint* waypoint = TCastToConstPtr< CScriptAIWaypoint >(
          mgr.GetObjectById(mWaypointNavigation.GetDestination()))) {
    const uint flags = waypoint->GetFlags();
    if ((flags & 8) != 0) {
      const CVector3f delta = mWaypointNavigation.GetDestinationPosition() - GetTranslation();
      if (delta.IsMagnitudeSafe()) {
        BodyController()->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), delta.AsNormalized(), 1.f));
      }
    } else if ((flags & 0x10) != 0) {
      const CVector3f delta = mgr.GetPlayer(0)->GetTranslation() - GetTranslation();
      if (delta.IsMagnitudeSafe()) {
        BodyController()->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), delta.AsNormalized(), 1.f));
      }
    }
  }
}

void CFlyingPirate::StartSpinToDeath(CStateManager& mgr) {
  BodyController()->CommandMgr().Reset();
  BodyController()->CommandMgr().DeliverCmd(CBCLoopHitReactionCmd(pas::EReactionType(1)));
  const CVector3f homingPos = mgr.GetPlayer(0)->GetHomingPosition(mgr, 0.f);
  const CVector3f pos = GetTranslation();
  SetMomentumWR(CVector3f::Zero());
  const CVector3f delta = homingPos - pos;
  CVector3f cross = CVector3f::Cross(delta, CVector3f::Up());
  if (close_enough(cross, CVector3f::Zero(), 0.0001f)) {
    cross = CVector3f::Cross(delta, CVector3f::Forward());
  }
  cross = mgr.Random()->Range(-5.f, 5.f) * cross.AsNormalized();
  const CVector3f dir = (homingPos + cross) - pos;
  if (dir.IsMagnitudeSafe()) {
    const CVector3f velocity = 25.f * dir.AsNormalized();
    SetVelocityWR(velocity);
    SetTransform(CTransform4f::LookAt(pos, pos + dir, CVector3f::Up()));
  } else {
    const CVector3f fallDir = GetTransform().GetForward() - 0.5f * GetTransform().GetUp();
    SetVelocityWR(25.f * fallDir.AsNormalized());
  }
}

bool CFlyingPirate::PatternOver(CStateManager& mgr, const CTriggerData& data) const {
  return mWaypointNavigation.GetDestination() == kInvalidUniqueId;
}

bool CFlyingPirate::PatternShagged(CStateManager& mgr, const CTriggerData& data) const {
  return CPatterned::Stuck(mgr, data);
}

bool CFlyingPirate::HearShot(CStateManager& mgr, const CTriggerData& data) const {
  const bool heard = mHearShot;
  const_cast< CFlyingPirate* >(this)->mHearShot = false;
  return heard;
}

bool CFlyingPirate::HearPlayer(CStateManager& mgr, const CTriggerData& data) const {
  const CPlayer& player = *mgr.GetPlayer(0);
  bool heard = false;
  if (player.GetVelocityWR().MagSquared() > 0.1f) {
    const CVector3f delta = player.GetTranslation() - GetTranslation();
    if (delta.MagSquared() < mData.mHearingDistance * mData.mHearingDistance) {
      heard = true;
    }
  }
  return heard;
}

void CFlyingPirate::Taunt(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAimAtTarget = true;
    mBoneTracking.SetActive(true);
    mBoneTracking.SetTarget(mgr.GetPlayer(0)->GetUniqueId());
    if (!mIsFlyingPirate) {
      mAnimationState.SetState(CAnimationState::kAS_Ready);
    } else {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    }
    if (mTargetId == kInvalidUniqueId) {
      mTargetId = mgr.GetPlayer(0)->GetUniqueId();
    }
    SendScriptMsgs(kSS_Attack, mgr, kInvalidUniqueId, kSM_None);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*GetBodyController(), pas::kAS_Taunt)) {
      BodyController()->CommandMgr().DeliverCmd(CBCTauntCmd(pas::ETauntType(4)));
    }
    break;
  case kStateMsg_Deactivate: {
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
    for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
      if (const CSpacePirate* pirate = TCastToConstPtr< CSpacePirate >(list[i])) {
        if (!pirate->GetEnableAim() && pirate->GetAlive() &&
            pirate->GetCurrentAreaId() == GetCurrentAreaId() &&
            (pirate->GetTranslation() - GetTranslation()).MagSquared() <
                mData.mHearingDistance * mData.mHearingDistance) {
          mgr.InformListeners(GetTranslation(), kLNT_PlayerFire);
          return;
        }
      }
    }
    break;
  }
  }
}

void CFlyingPirate::GetUp(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    CScriptTeamAiMgr::EndAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamAiMgr, GetUniqueId(),
                                true);
    break;
  case kStateMsg_Update:
    if (GetBodyController()->GetCurrentStateId() == pas::kAS_LieOnGround) {
      if (mPathFindSearch.Search(GetTranslation(), GetTranslation()) ==
          CPathFindSearch::kR_NoSourcePoint) {
        mPendingDeath = true;
        return;
      }
    }
    if (mAnimationState.CanIssueCommand(*GetBodyController(), pas::kAS_Getup)) {
      BodyController()->CommandMgr().DeliverCmd(CBCGetupCmd(pas::kGetup_Zero));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CFlyingPirate::GetUpNow(CStateManager& mgr, EStateMsg msg, float dt) {}

void CFlyingPirate::Bounce(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    CScriptTeamAiMgr::EndAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamAiMgr, GetUniqueId(),
                                true);
    break;
  case kStateMsg_Update:
    switch (GetBodyController()->GetCurrentStateId()) {
    case pas::kAS_Hurled:
      BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
      mVerticalMovement = true;
      break;
    case pas::kAS_LieOnGround:
      BodyController()->CommandMgr().DeliverCmd(CBCGetupCmd(pas::kGetup_Zero));
      break;
    case pas::kAS_Locomotion:
      static_cast< TStateMachineState< CPatterned >* >(mStateMachine.get())->SetCodeTrigger();
      break;
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CFlyingPirate::Lurk(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    ReleaseCoverPoint(mgr, mCurrentCoverPoint, true);
    mLineOfSightTracker.SetTarget(mgr.GetPlayer(0)->GetUniqueId());
    ResetFireMissilesCheck();
    CScriptTeamAiMgr::EndAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamAiMgr, GetUniqueId(),
                                true);
    mStateMachine->SetDelay(3.f);
    UpdateParticleEffects(mgr, 0.f, true);
    mAggressionChecked = false;
    break;
  case kStateMsg_Update:
    mLineOfSightTracker.Update(dt, mgr);
    UpdateCanFireMissiles(mgr);
    if (mAnimationState.GetState() != CAnimationState::kAS_NotReady &&
        mAnimationState.CanIssueCommand(*GetBodyController(), pas::kAS_Turn)) {
      const CVector3f delta = mDestPos - GetTranslation();
      if (delta.IsMagnitudeSafe()) {
        BodyController()->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), delta.AsNormalized(), 1.f));
      }
    }
    if (mAnimationState.GetState() != CAnimationState::kAS_Repeat) {
      mDestPos = GetTargetPos(mgr);
      CVector3f delta = mDestPos - GetTranslation();
      delta.SetZ(0.f);
      if (CVector3f::Dot(GetTransform().GetForward(), delta.AsNormalized()) < 0.8f) {
        mAnimationState.SetState(CAnimationState::kAS_Ready);
      }
    }
    break;
  case kStateMsg_Deactivate:
    x6a1_25_ = false;
    mMissilePathBlocked = false;
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

bool CFlyingPirate::CoverCheck(CStateManager& mgr, const CTriggerData& data) const {
  const float zero = 0.f;
  if (mCoverCheckTimer <= zero) {
    const_cast< CFlyingPirate* >(this)->mCoverCheckTimer = 10.f;
    const float chance = mData.mCoverCheckChance;
    return mgr.Random()->Range(zero, 100.f) < chance;
  }
  return false;
}

bool CFlyingPirate::CoverFind(CStateManager& mgr, const CTriggerData& data) const {
  bool found = false;
  float closestMag = mData.mMaxCoverDistance * mData.mMaxCoverDistance;
  const CScriptCoverPoint* closest = nullptr;
  CFlyingPirate* self = const_cast< CFlyingPirate* >(this);
  const CObjectList& list = mgr.GetObjectListById(kOL_AiWaypoint);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    if (const CScriptCoverPoint* cover = TCastToConstPtr< CScriptCoverPoint >(list[i])) {
      if (cover->GetActive() && cover->ShouldLandHere() && !cover->GetInUse(GetUniqueId()) &&
          cover->GetCurrentAreaId() == GetCurrentAreaId()) {
        float mag = (GetTranslation() - cover->GetTranslation()).MagSquared();
        if (mag < closestMag) {
          closestMag = mag;
          closest = cover;
        }
      }
    }
  }
  self->ReleaseCoverPoint(mgr, self->mCurrentCoverPoint, true);
  if (closest != nullptr) {
    if (CScriptCoverPoint* cover =
            TCastToPtr< CScriptCoverPoint >(mgr.ObjectById(closest->GetUniqueId()))) {
      self->SetCoverPoint(cover, self->mCurrentCoverPoint);
      found = true;
    }
  }
  return found;
}

void CFlyingPirate::ChooseWaypoint(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    float closestDist = 1000.f;
    const CScriptCoverPoint* closest = nullptr;
    const CObjectList& list = mgr.GetObjectListById(kOL_AiWaypoint);
    for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
      if (const CScriptCoverPoint* cover = TCastToConstPtr< CScriptCoverPoint >(list[i])) {
        if (cover->GetActive() && !cover->GetInUse(GetUniqueId()) && !cover->ShouldLandHere() &&
            cover->GetCurrentAreaId() == GetCurrentAreaId() &&
            (GetTranslation() - cover->GetTranslation()).Magnitude() < mData.mMaxCoverDistance) {
          const float dist =
              (mgr.GetPlayer(0)->GetTranslation() - cover->GetTranslation()).Magnitude();
          if (dist < closestDist) {
            closestDist = dist;
            closest = cover;
          }
        }
      }
    }
    ReleaseCoverPoint(mgr, mCurrentCoverPoint, true);
    if (closest != nullptr) {
      if (CScriptCoverPoint* cover =
              TCastToPtr< CScriptCoverPoint >(mgr.ObjectById(closest->GetUniqueId()))) {
        SetCoverPoint(cover, mCurrentCoverPoint);
        GetSearchPath()->SetPadding(20.f);
      }
    }
    break;
  }
  }
}

void CFlyingPirate::ResetFireMissilesCheck() {
  mCanFireMissiles = false;
  mMissilePathBlocked = false;
  mFireMissilesCheck = 0;
}

void CFlyingPirate::UpdateCanFireMissiles(CStateManager& mgr) {
  const CTeamAiRole* role = CScriptTeamAiMgr::GetTeamAiRole(mgr, mTeamAiMgr, GetUniqueId());
  if ((role == nullptr || role->GetTeamAiRole() == CTeamAiRole::kTAR_Projectile) &&
      mTargetId == mgr.GetPlayer(0)->GetUniqueId() &&
      (mMissileTimer <= 0.f || mTimeSinceAttacked < 1.f)) {
    const CVector3f delta = mgr.GetPlayer(0)->GetTranslation() - GetTranslation();
    if (delta.GetZ() * delta.GetZ() < delta.GetX() * delta.GetX() + delta.GetY() * delta.GetY()) {
      CheckFireMissiles(mgr);
    }
  }
}

bool CFlyingPirate::ShouldAttack(CStateManager& mgr, const CTriggerData& data) const {
  bool shouldAttack = false;
  if (mCanFireMissiles && (mTeamAiMgr == kInvalidUniqueId ||
                           CScriptTeamAiMgr::StartAttack(CScriptTeamAiMgr::kAT_Projectile, mgr,
                                                         mTeamAiMgr, GetUniqueId()))) {
    const float range = mData.mMissileTimeVariation;
    const float delay = mData.mMinimumMissileTime;
    shouldAttack = true;
    const_cast< CFlyingPirate* >(this)->mMissileTimer = range * mgr.Random()->Float() + delay;
  }
  return shouldAttack;
}

void CFlyingPirate::Attack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    if (!mAggressionChecked) {
      float chance;
      if (mTimeSinceShotAt < 3.f) {
        chance = 2.f * mData.mAggressionChance;
      } else {
        chance = mData.mAggressionChance;
      }
      mAggressive = mgr.Random()->Range(0.f, 100.f) < chance;
      mAggressionChecked = true;
    } else {
      mAggressive = false;
    }
    break;
  case kStateMsg_Update: {
    if (mAnimationState.CanIssueCommand(*GetBodyController(), pas::kAS_ProjectileAttack)) {
      BodyController()->CommandMgr().DeliverCmd(
          CBCProjectileAttackCmd(pas::kS_One, mDestPos, false));
    }
    const CVector3f delta = mgr.GetPlayer(0)->GetTranslation() - GetTranslation();
    if (delta.IsMagnitudeSafe()) {
      BodyController()->FaceDirection(delta.AsNormalized(), dt);
    }
    DeliverGetUp();
    mLineOfSightTracker.Update(dt, mgr);
    break;
  }
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mAggressive = false;
    break;
  }
}

bool CFlyingPirate::SpotPlayer(CStateManager& mgr, const CTriggerData& data) const {
  const CVector3f dir = mgr.GetPlayer(0)->GetAimPosition(mgr, 0.f) - GetGunEyePos();
  float distance = dir.Magnitude();
  float angle = mDetectionAngle;
  return CVector3f::Dot(dir, GetTransform().GetForward()) > distance * angle;
}

bool CFlyingPirate::InRange(CStateManager& mgr, const CTriggerData& data) const {
  const CPlayer& player = *mgr.GetPlayer(0);
  const CVector3f pos = player.GetTranslation();
  return CMath::AbsF(pos.GetZ()) < mMinAttackRange &&
         pos.MagSquared() < mMaxAttackRange * mMaxAttackRange;
}

void CFlyingPirate::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    CVector3f target = mgr.GetPlayer(0)->GetAimPosition(mgr, 0.f);
    if (mIsMoving) {
      target = mDestPos;
    } else if (const CScriptCoverPoint* cover = GetCoverPoint(mgr, mCurrentCoverPoint)) {
      target = cover->GetTranslation();
    }
    if (GetSearchPath()->Search(GetTranslation(), target) != CPathFindSearch::kR_Success &&
        (GetSearchPath()->GetResult() == CPathFindSearch::kR_NoDestPoint ||
         GetSearchPath()->GetResult() == CPathFindSearch::kR_NoPath)) {
      if (GetSearchPath()->FindClosestReachablePoint(GetTranslation(), target) ==
          CPathFindSearch::kR_Success) {
        GetSearchPath()->Search(GetTranslation(), target);
      }
    }
    UpdateParticleEffects(mgr, 0.5f, true);
    break;
  }
  case kStateMsg_Update: {
    CVector3f move = CVector3f::Zero();
    if (!GetSearchPath()->IsShagged() && !GetSearchPath()->IsOver()) {
      CVector3f out = GetTranslation() + GetTransform().GetForward();
      GetSearchPath()->GetSplinePointWithLookahead(out, GetTranslation(), 3.f);
      if (GetSearchPath()->SegmentOver(out)) {
        GetSearchPath()->Advance();
      }
      move = out - GetTranslation();
      if (move.CanBeNormalized()) {
        move.Normalize();
      }
    }
    move += 3.f * AvoidActors(mgr);
    if (move.CanBeNormalized()) {
      move.Normalize();
    }
    float speed = mTimeSinceShotAt < 2.f ? 4.f : 1.f;
    const float multiplier = 1.5f * speed;
    speed = dt * (dt * (speed * mData.mFlightThrust));
    mFlightAcceleration = speed * move;
    mTargetPitchBend = multiplier;
    mFlightVelocity += mFlightAcceleration;
    const CVector3f face = GetTargetPos(mgr) - GetTranslation();
    if (face.IsMagnitudeSafe()) {
      BodyController()->CommandMgr().DeliverCmd(CBCLocomotionCmd(move, face.AsNormalized(), 1.f));
    }
    mLineOfSightTracker.Update(dt, mgr);
    break;
  }
  case kStateMsg_Deactivate:
    mIsMoving = false;
    break;
  }
}

void CFlyingPirate::Retreat(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    const CVector3f playerPos = mgr.GetPlayer(0)->GetTranslation();
    CVector3f target =
        GetTranslation() - mMinAttackRange * (playerPos - GetTranslation()).AsNormalized();
    float targetZ = playerPos.GetZ();
    targetZ += mData.mFlyingHeight;
    target.SetZ(targetZ);
    if (GetSearchPath()->OnPath(target) == CPathFindSearch::kR_NoSourcePoint) {
      GetSearchPath()->FindClosestReachablePoint(GetTranslation(), target);
      target[kDZ] += mData.mFlyingHeight;
      if ((playerPos - target).MagSquared() < 0.25f * mMinAttackRange * mMinAttackRange) {
        target = GetTranslation() + mMinAttackRange * (playerPos - GetTranslation()).AsNormalized();
        float targetZ = playerPos.GetZ();
        targetZ += mData.mFlyingHeight;
        target.SetZ(targetZ);
        if (GetSearchPath()->OnPath(target) == CPathFindSearch::kR_NoSourcePoint) {
          GetSearchPath()->FindClosestReachablePoint(GetTranslation(), target);
          target[kDZ] += mData.mFlyingHeight;
        }
      }
    }
    GetSearchPath()->Search(GetTranslation(), target);
    UpdateParticleEffects(mgr, 0.5f, true);
    break;
  }
  case kStateMsg_Update: {
    CVector3f move = CVector3f::Zero();
    if (!GetSearchPath()->IsOver()) {
      CVector3f out = GetTranslation() + GetTransform().GetForward();
      GetSearchPath()->GetSplinePointWithLookahead(out, GetTranslation(), 3.f);
      if (GetSearchPath()->SegmentOver(out)) {
        GetSearchPath()->Advance();
      }
      move = out - GetTranslation();
      if (move.CanBeNormalized()) {
        move.Normalize();
      }
    }
    move += 3.f * AvoidActors(mgr);
    if (move.CanBeNormalized()) {
      move.Normalize();
    }
    float speed = mTimeSinceShotAt < 2.f ? 4.f : 1.f;
    const float multiplier = 1.5f * speed;
    speed = dt * (dt * (speed * mData.mFlightThrust));
    mFlightAcceleration = speed * move;
    mTargetPitchBend = multiplier;
    mFlightVelocity += mFlightAcceleration;
    const CVector3f face = GetTargetPos(mgr) - GetTranslation();
    if (face.IsMagnitudeSafe()) {
      BodyController()->CommandMgr().DeliverCmd(CBCLocomotionCmd(move, face.AsNormalized(), 1.f));
    }
    mLineOfSightTracker.Update(dt, mgr);
    break;
  }
  case kStateMsg_Deactivate:
    break;
  }
}

void CFlyingPirate::TurnAround(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    mDestPos = GetTargetPos(mgr);
    CVector3f delta = mDestPos - GetTranslation();
    delta.SetZ(0.f);
    if (delta.IsMagnitudeSafe() &&
        CVector3f::Dot(GetTransform().GetForward(), delta.AsNormalized()) < 0.8f) {
      mAnimationState.SetState(CAnimationState::kAS_Ready);
    }
    break;
  }
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*GetBodyController(), pas::kAS_Turn)) {
      const CVector3f delta = mDestPos - GetTranslation();
      if (delta.IsMagnitudeSafe()) {
        BodyController()->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), delta.AsNormalized(), 1.f));
      } else {
        mAnimationState.SetState(CAnimationState::kAS_Over);
      }
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

bool CFlyingPirate::ShouldDodge(CStateManager& mgr, const CTriggerData& data) const {
  bool dodge = mMissilePathBlocked;
  if (!mMissilePathBlocked && !x6a1_25_) {
    const CVector3f delta =
        const_cast< CFlyingPirate* >(this)->GetTargetPos(mgr) - GetTranslation();
    if (CVector3f::Dot(delta, GetTransform().GetForward()) > 0.f &&
        (mTimeSinceAttacked < 0.33f || mTimeSinceShotAt < 0.33f) &&
        mLineOfSightTracker.GetClearTime() < 0.5f) {
      dodge = true;
    }
  }
  return dodge;
}

void CFlyingPirate::Dodge(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mDodgeDirection = GetDodgeDirection(mgr, mHeight);
    if (mDodgeDirection == pas::kSD_Invalid) {
      mDodgeDirection = (mgr.Random()->Next() & 0x4000) != 0 ? pas::kSD_Left : pas::kSD_Right;
    }
    UpdateParticleEffects(mgr, 1.f, true);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*GetBodyController(), pas::kAS_Step)) {
      BodyController()->CommandMgr().DeliverCmd(CBCStepCmd(mDodgeDirection, pas::kStep_Dodge));
    }
    mLineOfSightTracker.Update(dt, mgr);
    mTargetPitchBend = rstl::max_val(2.f - mStateMachine->GetTime(), 1.f);
    DeliverGetUp();
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mMissilePathBlocked = false;
    break;
  }
}

bool CFlyingPirate::ShotAt(CStateManager& mgr, const CTriggerData& data) const {
  return mTimeSinceShotAt < (data.GetFloat() ? data.GetFloat() : 0.5f);
}

bool CFlyingPirate::Attacked(CStateManager& mgr, const CTriggerData& data) const {
  return mTimeSinceAttacked < (data.GetFloat() ? data.GetFloat() : 0.5f);
}

bool CFlyingPirate::ShouldSpecialAttack(CStateManager& mgr, const CTriggerData& data) const {
  if (mFlavor == kFT_One && mAttackObjectId != kInvalidUniqueId && mSpecialAttackTimer <= 0.f) {
    const_cast< CFlyingPirate* >(this)->mSpecialAttackTimer = 15.f * mgr.Random()->Float() + 15.f;
    if (!mgr.GetPlayer(0)->CheckOrbitDisableSourceList()) {
      if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mAttackObjectId))) {
        if (mTeamAiMgr == kInvalidUniqueId ||
            CScriptTeamAiMgr::StartAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamAiMgr,
                                          GetUniqueId())) {
          const_cast< CFlyingPirate* >(this)->SetDestPos(actor->GetTranslation() +
                                                         15.f * CVector3f::Down());
          const_cast< CFlyingPirate* >(this)->mIsMoving = true;
          return true;
        }
      }
    }
  }
  return false;
}

bool CFlyingPirate::LineOfSight(CStateManager& mgr, const CTriggerData& data) const {
  return mLineOfSightTracker.HasLineOfSight();
}

void CFlyingPirate::Jump(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    BodyController()->SetLocomotionType(pas::kLT_Combat);
    mVerticalMovement = true;
    RemoveMaterial(kMT_GroundCollider, mgr);
    SetMomentumWR(CVector3f::Zero());
    mCoverCheckTimer = 10.f;
    UpdateParticleEffects(mgr, 1.f, true);
    UpdateLandingSmoke(mgr, true);
    float chance = mData.mJumpAggressionChance;
    mAggressive = mgr.Random()->Range(0.f, 100.f) < chance;
    break;
  }
  case kStateMsg_Update:
    break;
  case kStateMsg_Deactivate:
    UpdateParticleEffects(mgr, 0.5f, true);
    UpdateLandingSmoke(mgr, false);
    mAggressive = false;
    break;
  }
}

bool CFlyingPirate::Landed(CStateManager& mgr, const CTriggerData& data) const {
  return GetBodyController()->GetCurrentStateId() == pas::kAS_LieOnGround;
}

bool CFlyingPirate::InPosition(CStateManager& mgr, const CTriggerData& data) const {
  CScriptCoverPoint* cover = GetCoverPoint(mgr, mCurrentCoverPoint);
  if (cover != nullptr) {
    const CVector3f delta = cover->GetTranslation() - GetTranslation();
    return delta.GetZ() < 0.f && delta.Magnitude() < 4.f;
  }
  return true;
}

void CFlyingPirate::Land(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    UpdateLandingSmoke(mgr, true);
    UpdateParticleEffects(mgr, 1.f, true);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*GetBodyController(), pas::kAS_Jump)) {
      BodyController()->CommandMgr().DeliverCmd(
          CBCJumpCmd(mDestPos, pas::kJT_Normal, pas::kJS_IntoJump, 0, CBCJumpCmd::kFF_AmbushJump));
    }
    if (mAnimationState.GetState() == CAnimationState::kAS_Repeat) {
      BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    UpdateLandingSmoke(mgr, false);
    UpdateParticleEffects(mgr, 0.f, false);
    break;
  }
}

void CFlyingPirate::Walk(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    UpdateParticleEffects(mgr, 0.f, false);
    break;
  case kStateMsg_Update:
    if (mAnimationState.GetState() != CAnimationState::kAS_NotReady &&
        mAnimationState.CanIssueCommand(*GetBodyController(), pas::kAS_Turn)) {
      const CVector3f delta = mDestPos - GetTranslation();
      if (delta.IsMagnitudeSafe()) {
        BodyController()->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), delta.AsNormalized(), 1.f));
      }
    }
    if (mAnimationState.GetState() != CAnimationState::kAS_Repeat) {
      mDestPos = GetTargetPos(mgr);
      CVector3f delta = mDestPos - GetTranslation();
      delta.SetZ(0.f);
      if (delta.IsMagnitudeSafe() &&
          CVector3f::Dot(GetTransform().GetForward(), delta.AsNormalized()) < 0.8f) {
        mAnimationState.SetState(CAnimationState::kAS_Ready);
      }
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    BodyController()->SetLocomotionType(pas::kLT_Combat);
    mVerticalMovement = true;
    SetMomentumWR(CVector3f::Zero());
    break;
  }
}

void CFlyingPirate::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mIsAttackingObject = true;
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*GetBodyController(), pas::kAS_ProjectileAttack)) {
      BodyController()->CommandMgr().DeliverCmd(
          CBCProjectileAttackCmd(pas::kS_Zero, mDestPos, false));
    }
    DeliverGetUp();
    break;
  case kStateMsg_Deactivate:
    mIsAttackingObject = false;
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

bool CFlyingPirate::ShouldMove(CStateManager& mgr, const CTriggerData& data) const {
  CVector3f delta = GetTranslation() - mgr.GetPlayer(0)->GetTranslation();
  float random = mgr.Random()->Float();
  if (random < 0.5f) {
    random = mgr.Random()->Range(-25.f, -15.f);
  } else {
    random = mgr.Random()->Range(15.f, 25.f);
  }
  CVector3f cross = CVector3f::Cross(delta, CVector3f::Up()).AsNormalized();
  CVector3f dest = GetTranslation() + random * cross;
  dest[kDZ] = mgr.GetPlayer(0)->GetTranslation()[kDZ] + mData.GetFlyingHeight();
  const_cast< CFlyingPirate* >(this)->SetDestPos(dest);
  const_cast< CFlyingPirate* >(this)->mIsMoving = true;
  return true;
}

bool CFlyingPirate::Stuck(CStateManager& mgr, const CTriggerData& data) const {
  return mStateMachine->GetTime() > 0.5f &&
         (CPatterned::Stuck(mgr, data) ||
          const_cast< CFlyingPirate* >(this)->GetSearchPath()->GetResult() !=
              CPathFindSearch::kR_Success);
}

bool CFlyingPirate::AggressionCheck(CStateManager& mgr, const CTriggerData& data) const {
  return mAggressive;
}

void CFlyingPirate::Enraged(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Update:
    mFlightAcceleration = (dt * (dt * mData.mFlightThrust)) * CVector3f::Up();
    mTargetPitchBend = 1.5f;
    mFlightVelocity += mFlightAcceleration;
    BodyController()->CommandMgr().DeliverCmd(CBCLocomotionCmd(
        CVector3f::Up(), (GetTargetPos(mgr) - GetTranslation()).AsNormalized(), 1.f));
    break;
  }
}

void CFlyingPirate::Dead(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::Dead(mgr, msg, dt);
  switch (msg) {
  case kStateMsg_Activate:
    mBoneTracking.SetActive(false);
    mDeathSpinFinished = false;
    AnimationData()->SetEffectState(skEyes, false, mgr);
    CScriptTeamAiMgr::EndAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamAiMgr, GetUniqueId(),
                                true);
    break;
  case kStateMsg_Update:
    SetMomentumWR(CVector3f::Zero());
    if (mSpinToDeath) {
      const CTransform4f xf = GetLctrTransform(mHeadSegId);
      const CVector3f offset = xf.GetTranslation() - GetTranslation();
      const CAABox box(offset, offset + 2.f * CVector3f::One());
      SetBoundingBox(box);
      SetCollisionPrimitive(CCollidableAABox(box, GetMaterialList()));
      if (mStateMachine->GetTime() >= 6.f) {
        mDeathSpinFinished = true;
      }
    }
    break;
  }
}

bool CFlyingPirate::DeathOver(CStateManager& mgr, const CTriggerData& data) const {
  if (mBecameRagDoll) {
    return true;
  }
  if (mSpinToDeath) {
    return mDeathSpinFinished || mSolidCollision;
  }
  return CPatterned::AnimOver(mgr, data);
}

bool CFlyingPirate::AnimOver(CStateManager& mgr, const CTriggerData& data) const {
  return CPatterned::AnimOver(mgr, data);
}

void CFlyingPirate::Explode(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    RemoveMaterial(kMT_Target, kMT_Orbit, kMT_GroundCollider, kMT_Unknown59, mgr);
    SetMomentumWR(CVector3f::Zero());
    if (!mFadeToDeath) {
      MassiveDeath(mgr);
    }
    break;
  case kStateMsg_Update:
    if (mStateMachine->GetTime() > 0.1f) {
      DeathDelete(mgr);
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CFlyingPirate::Deactivate(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    mPendingDeath = true;
  }
}

bool CFlyingPirate::ShouldRetreat(CStateManager& mgr, const CTriggerData& data) const {
  bool shouldRetreat = false;
  CFlyingPirate* self = const_cast< CFlyingPirate* >(this);
  if (mRetreatRequested) {
    TUniqueId id = GetConnectedObject(mgr, kSS_Patrol, kSM_Follow);
    const CScriptWaypoint* waypoint = TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(id));
    if (waypoint == nullptr) {
      id = GetConnectedObject(mgr, kSS_Retreat, kSM_Follow);
      waypoint = TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(id));
    }
    if (waypoint != nullptr) {
      self->mRetreatRequested = false;
      shouldRetreat = true;
      self->mDestObj = id;
      self->SetDestPos(waypoint->GetTranslation());
      self->mReflectedDestPos = GetTranslation();
      self->mInPosition = false;
      self->mIsMoving = true;
      self->mHearShot = false;
      self->mAimAtTarget = false;
      self->mHitByPlayerProjectile = false;
    }
  }
  return shouldRetreat;
}

bool CFlyingPirate::HasAttackPattern(CStateManager& mgr, const CTriggerData& data) const {
  return GetConnectedObject(mgr, kSS_Attack, kSM_Follow) != kInvalidUniqueId;
}

CProjectileInfo* CFlyingPirate::ProjectileInfo() { return &mGunProjectileInfo; }

bool CFlyingPirate::FireProjectile(CStateManager& mgr, float dt) {
  bool fired = false;
  const CTransform4f xf = GetLctrTransform(mGunSegId);
  if (!mAlive) {
    LaunchProjectile(xf, mgr, 8, CWeapon::kPA_None, false, CImpactVisorEffect::None(),
                     CVector3f(1.f, 1.f, 1.f));
    fired = true;
  } else {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mTargetId))) {
      CVector3f origin = actor->GetTranslation();
      const CPlayer& player = *mgr.GetPlayer(0);
      if (mTargetId == player.GetUniqueId()) {
        origin = ProjectileInfo()->PredictInterceptPos(
            xf.GetTranslation(), player.GetAimPosition(mgr, 0.f), player, true, dt);
      }
      CVector3f delta = origin - xf.GetTranslation();
      float distance = delta.Magnitude();
      delta *= 1.f / distance;
      float dot = CVector3f::Dot(xf.GetForward(), delta);
      if (dot > 0.707f || (distance < 6.f && dot > 0.5f)) {
        if (LineOfSightTest(mgr, xf.GetTranslation(), origin,
                            CMaterialList(kMT_Player, kMT_NoPlatformCollision))) {
          origin += GetTransform().Rotate(mBurstFire.GetDistanceCompensatedError(distance, 6.f));
          const CTransform4f aimXf =
              CTransform4f::LookAt(xf.GetTranslation(), origin, CVector3f::Up());
          LaunchProjectile(aimXf, mgr, 8, CWeapon::kPA_None, false, CImpactVisorEffect::None(),
                           CVector3f(1.f, 1.f, 1.f));
          fired = true;
        }
      }
    }
  }
  if (fired) {
    const CPASDatabase& database = GetBodyController()->GetPASDatabase();
    const CPASAnimParmData parms(pas::kAS_AdditiveReaction, CPASAnimParm::FromEnum(2));
    const rstl::pair< float, int > anim = database.FindBestAnimation(parms, *mgr.Random(), -1);
    if (anim.first > 0.f) {
      ModelData()->AnimationData()->AddAdditiveAnimation(anim.second, 1.f, false, true);
    }
    CSfxManager::AddEmitter(mData.mGunSfx, GetTranslation(), GetCurrentAreaId().Value(), true,
                            false, CSfxManager::kMedPriority);
  }
  const bool result = fired;
  return result;
}

void CFlyingPirate::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                    EUserEventType type, float dt) {
  bool handled = false;
  switch (type) {
  case kUE_Projectile: {
    CProjectileInfo& info = mIsAttackingObject ? mAltProjectileInfo2 : mAltProjectileInfo1;
    if (info.Token().IsLoaded() && mgr.CanCreateProjectile(GetUniqueId(), kWT_AI, 16)) {
      const CTransform4f xf = GetLctrTransform(node.GetLocatorName());
      CEnergyProjectile* projectile = rs_new CEnergyProjectile(
          true, info.Token(), kWT_AI, xf, kMT_Character, info.GetDamage(), mgr.AllocateUniqueId(),
          GetCurrentAreaId(), GetUniqueId(),
          mIsAttackingObject ? TUniqueId(mAttackObjectId) : mgr.GetPlayer(0)->GetUniqueId(),
          CWeapon::kPA_None, false, CVector3f::One(), CImpactVisorEffect::None(), false, true,
          false, 1.f, 4.f, 4.f);
      if (projectile != nullptr) {
        mgr.AddObject(projectile);
        if (!mIsAttackingObject && mIsAquaPirate) {
          projectile->SetMinHomingDistance(mData.mProjectileHomingDistance);
        }
      }
    }
    handled = true;
    break;
  }
  case kUE_BecomeRagDoll:
    mBecameRagDoll = true;
    handled = true;
    break;
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

void CFlyingPirate::PreRenderAllViewports(CStateManager& mgr) {
  CPatterned::PreRenderAllViewports(mgr);
}

void CFlyingPirate::AddToRenderer(const CStateManager& mgr) const {
  for (int i = 0; i < mParticleGens.size(); ++i) {
    CElementGen* gen = mParticleGens[i].get();
    if (mgr.GetFrustumPlanes().BoxInFrustumPlanes(gen->GetBounds())) {
      gpRender->AddParticleGen(*gen);
    }
  }
  CPatterned::AddToRenderer(mgr);
}

void CFlyingPirate::PreRender(CStateManager& mgr) {
  CPatterned::PreRender(mgr);
  mBoneTracking.PreRender(mgr, *AnimationData(), GetTransform(), GetModelData()->GetScale(),
                          *BodyController());
}

void CFlyingPirate::Render(const CStateManager& mgr) const { CPatterned::Render(mgr); }

void CFlyingPirate::CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                                 CStateManager& mgr) {
  CPatterned::CollidedWith(id, list, mgr);
  if (!mAlive) {
    if (id == kInvalidUniqueId) {
      static const CMaterialList skWorldMaterials(kMT_Unknown59, kMT_Wall, kMT_Floor);
      for (int i = 0; i < list.GetCount(); ++i) {
        if (list[i].GetMaterialLeft().SharesMaterials(skWorldMaterials)) {
          mDeathSpinFinished = true;
          break;
        }
      }
    } else {
      mDeathSpinFinished = true;
    }
  }
}

void CFlyingPirate::PreThink(float dt, CStateManager& mgr) {
  mBoneTracking.PreThink(*AnimationData());
  CPatterned::PreThink(dt, mgr);
}

void CFlyingPirate::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  if (!GetBodyController()->GetIsActive()) {
    BodyController()->Activate(mgr, pas::kAS_Invalid);
    if (mIsFlyingPirate) {
      BodyController()->SetLocomotionType(pas::kLT_Combat);
      mVerticalMovement = true;
    }
  }
  bool inCineCam = false;
  if (!mgr.IsMultiplayer() && mgr.GetCameraManager(0)->IsInCinematicCamera()) {
    inCineCam = true;
  }
  if (inCineCam && !mPrevInCineCam) {
    RemoveMaterial(kMT_AIBlock, mgr);
    CMaterialList include = GetMaterialFilter().GetIncludeList();
    include.Remove(kMT_AIBlock);
    SetMaterialFilter(
        CMaterialFilter::MakeIncludeExclude(include, GetMaterialFilter().GetExcludeList()));
  } else if (!inCineCam && mPrevInCineCam) {
    AddMaterial(kMT_AIBlock, mgr);
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
        CMaterialList(GetMaterialFilter().GetIncludeList()).Union(CMaterialList(kMT_AIBlock)),
        GetMaterialFilter().GetExcludeList()));
  }
  mPrevInCineCam = inCineCam;
  for (int i = 0; i < mParticleGens.size(); ++i) {
    mParticleGens[i]->Update(dt);
  }
  x78c_ = rstl::max_val(x78c_ - dt, 0.f);
  if (mAlive) {
    mTimeSinceAttacked += dt;
    mTimeSinceShotAt += dt;
    if (mShotAt) {
      mTimeSinceShotAt = 0.f;
      mShotAt = false;
    }
    if (mHitByPlayerProjectile) {
      mTimeSinceAttacked = 0.f;
      mHitByPlayerProjectile = false;
    }
    if (!mIsAquaPirate && InFluidId() != kInvalidUniqueId) {
      if (const CScriptWater* water =
              TCastToConstPtr< CScriptWater >(mgr.GetObjectById(InFluidId()))) {
        const float height = GetTranslation().GetZ();
        if (water->GetTriggerBoundsWR().GetMaxPoint().GetZ() > 2.f + height) {
          mPendingDeath = true;
        }
      }
    }
  }
  const float zero = 0.f;
  if (GetBodyController()->GetPercentageFrozen() == zero) {
    mMissileTimer = rstl::max_val(mMissileTimer - dt, zero);
    mSpecialAttackTimer = rstl::max_val(mSpecialAttackTimer - dt, 0.f);
    mCoverCheckTimer = rstl::max_val(mCoverCheckTimer - dt, 0.f);
    if (mAlive) {
      CheckForProjectiles(mgr);
    }
    if (!mIsAquaPirate &&
        (!mAlive ||
         (GetBodyController()->GetBodyStateInfo().GetCurrentState()->CanShoot() && mAimAtTarget &&
          GetBodyController()->GetCurrentStateId() != pas::kAS_ProjectileAttack && !mStopped &&
          !GetBodyController()->IsElectrocuting())) &&
        mBurstFire.GetBurstType() != -1) {
      mBurstTimer -= dt;
      if (mBurstTimer < 0.f) {
        int type = mBurstFire.GetBurstType() & ~1;
        if (!mLineOfSightTracker.HasLineOfSight() || !IsOnScreen(mgr)) {
          ++type;
        }
        mBurstFire.SetBurstType(type);
        mBurstFire.Start(mgr);
        if (mAlive) {
          const float variation = mAttackTimeVariation;
          mBurstTimer = variation * mgr.Random()->Float() + GetAverageAttackTime();
          const CVector3f delta =
              (GetBoundingBox().GetCenterPoint() - mgr.GetPlayer(0)->GetAimPosition(mgr, 0.f))
                  .AsNormalized();
          if (CVector3f::Dot(delta, mgr.GetPlayer(0)->GetTransform().GetForward()) < 0.9f) {
            const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
            for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
              const CPhysicsActor* actor = TCastToConstPtr< CPhysicsActor >(list[i]);
              if (actor != nullptr && actor->GetActive() &&
                  actor->GetCurrentAreaId() == GetCurrentAreaId()) {
                mBurstTimer += 0.2f;
              }
            }
          }
        } else {
          mBurstTimer = 22050.f;
        }
      }
      mBurstFire.Update(mgr, dt);
      if (mBurstFire.ShouldFire()) {
        FireProjectile(mgr, dt);
        const float variation = mData.mIntraBurstShotVariation;
        const float delay = mData.mIntraBurstShotTime;
        mBurstFire.SetTimeToNextShot(variation * (mgr.Random()->Float() - 0.5f) + delay);
      }
    }
  }
  if (mAlive && !GetBodyController()->IsFrozen() && !GetBodyController()->IsElectrocuting() &&
      mAimAtTarget && !IsAquaPirate()) {
    BodyController()->CommandMgr().DeliverCmd(CBCAdditiveAimCmd());
    const CVector3f aim = GetTransform().TransposeMultiply(GetTargetPos(mgr));
    BodyController()->CommandMgr().DeliverAdditiveTargetVector(aim);
  } else {
    BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_AdditiveIdle));
  }
  if (mFlightVelocity.MagSquared() > 0.f) {
    const float mag = mFlightVelocity.Magnitude();
    const CVector3f direction = (1.f / mag) * mFlightVelocity;
    float damping = 0.2f;
    if (mFlightAcceleration.MagSquared() == 0.f) {
      damping *= 3.f;
    }
    const float speed = -(dt * (mag * (damping * mag)) - mag);
    mFlightVelocity = speed * direction;
  }
  if (mAlive && GetBodyController()->GetCurrentStateId() != pas::kAS_LoopReaction &&
      GetBodyController()->GetCurrentStateId() != pas::kAS_Hurled &&
      GetBodyController()->GetCurrentStateId() != pas::kAS_LieOnGround &&
      GetBodyController()->GetCurrentStateId() != pas::kAS_Getup) {
    ApplyImpulseWR(GetMass() * mFlightVelocity, CAxisAngle::Identity());
  } else {
    mFlightVelocity = CVector3f::Zero();
    mFlightAcceleration = CVector3f::Zero();
  }
  mTargetPitchBend = CMath::Clamp(1.f, mTargetPitchBend, 1.999f);
  float change = mTargetPitchBend - mPitchBend;
  change = CMath::Clamp(-dt, change, dt);
  mPitchBend += change;
  SetSoundEventPitchBend(static_cast< int >(8192.f * mPitchBend));
  mFlightAcceleration = CVector3f::Zero();
  mTargetPitchBend = 1.f;
  CPatterned::Think(dt, mgr);
  CVector3f movement = mFlightAcceleration;
  if (movement.CanBeNormalized()) {
    movement.Normalize();
  }
  const float maxTilt = 0.333f;
  const float tilt = rstl::min_val(maxTilt, maxTilt * mFlightAcceleration.Magnitude());
  const CVector3f targetUp = (CVector3f::Up() + tilt * movement).AsNormalized();
  const CVector3f currentUp = GetTransform().GetUp();
  const float angle = CMath::AbsF(CVector3f::GetAngleDiff(currentUp, targetUp));
  if (angle > 0.f) {
    const float maxStep = 30.f * ((M_PIF * dt) / 180.f);
    const float step = rstl::min_val(maxStep, angle);
    const CVector3f up = (step * targetUp + (angle - step) * currentUp).AsNormalized();
    CVector3f right = CVector3f::Cross(GetTransform().GetForward(), up);
    const CVector3f forward = CVector3f::Cross(up, right).AsNormalized();
    right = CVector3f::Cross(forward, up);
    SetTransform(CTransform4f::FromColumns(right, forward, up, GetTranslation()));
  }
  if (!GetBodyController()->IsFrozen()) {
    mBoneTracking.Think(dt);
  }
}

void CFlyingPirate::SetupStateMachine(CStateManager& mgr) {
  StateMachine* stateMachine = mStateMachine.get();
  CPatterned::SetupStateMachine(mgr);
  stateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
  stateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
}

CEntity* REL_LoadFlyingPirate(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrFlyingPirate sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrFlyingPirate.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  const CFlyingPirate::CFlyingPirateData data(
      sldrThis.searchRadius, sldrThis.hearingRadius, sldrThis.unknown_0x20daf45e,
      sldrThis.projectile, LdrToDamageInfo(sldrThis.projectileDamage), sldrThis.sound_Projectile,
      sldrThis.missile, LdrToDamageInfo(sldrThis.missileDamage), sldrThis.wPSC,
      sldrThis.hurlRecoverTime, sldrThis.hoverHeight, sldrThis.rocketPackExplosion,
      LdrToDamageInfo(sldrThis.rocketPackExplosionDamage), sldrThis.spiralChance,
      sldrThis.minimumMissileTime, sldrThis.missileTimeVariation, sldrThis.flightThrust,
      sldrThis.sound_Impact, sldrThis.sound_Spiral, sldrThis.landChance,
      sldrThis.intraBurstShotTime, sldrThis.intraBurstShotVariation, sldrThis.landingCloudDirt,
      sldrThis.landingCloudDust, sldrThis.landingCloudSnow, sldrThis.sound_Hurled,
      sldrThis.sound_Death, sldrThis.doubleAttackChance, sldrThis.unknown_0x3427d27f,
      sldrThis.stopHomingRange, sldrThis.unknown_0xccf05648, sldrThis.unknown_0x2a90f9a9,
      sldrThis.unknown_0x9ca8f357, sldrThis.unknown_0x7ac85cb6);

  return rs_new CFlyingPirate(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                              LdrToEntityInfo(info, sldrThis.editorProperties),
                              LdrToTransform4f(sldrThis.editorProperties), *modelData,
                              LdrToActorParameters(sldrThis.actorInformation),
                              LdrToPatternedInfo(sldrThis.patterned, nullptr), data);
}

static SFlyingPirate_FuncPtrs REL_loader_FlyingPirate;

void SetRelLoaderFunctionToLoader() {
  REL_loader_FlyingPirate.mLoader = REL_LoadFlyingPirate;
  SetSFlyingPirate_FuncPtrs(&REL_loader_FlyingPirate);
}

extern "C" void RELMain() { SetRelLoaderFunctionToLoader(); }

extern "C" void RELExit() { SetSFlyingPirate_FuncPtrs(nullptr); }
