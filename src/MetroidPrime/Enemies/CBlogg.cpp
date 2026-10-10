#include "MetroidPrime/Enemies/CBlogg.hpp"

#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Collision/CSpatialPrimitive.hpp"
#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CKnockBackInfo.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Collision/CJointCollisionDescription.hpp"
#include "MetroidPrime/Enemies/CPatternedInfo.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrBlogg.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CFishCloud.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTeamAiMgr.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CBloggProjectile.hpp"
#include "MetroidPrime/Weapons/CBomb.hpp"
#include "MetroidPrime/Weapons/CImpactVisorEffect.hpp"
#include "MetroidPrime/Weapons/CPowerBomb.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "REL/REL_Setup.h"
#include "rstl/StringExtras.hpp"

#include <math.h>

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"ShouldPatrol", static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::ShouldPatrol)},
    {"AnimOver", static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::AnimOver)},
    {"ShouldPrepareToAttack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::ShouldPrepareToAttack)},
    {"InAttackPosition",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::InAttackPosition)},
    {"InValidPosition",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::InValidPosition)},
    {"IsFacingPlayer",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::IsFacingPlayer)},
    {"IsPlayerStunned",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::IsPlayerStunned)},
    {"ProjectileAttackDelay",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::ProjectileAttackDelay)},
    {"ShouldCharge", static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::ShouldCharge)},
    {"IsChargeOver", static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::IsChargeOver)},
    {"CanMeleeAttack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::CanMeleeAttack)},
    {"CanRangedAttack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::CanRangedAttack)},
    {"CanTaunt", static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::CanTaunt)},
    {"InProjectileRange",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::InProjectileRange)},
    {"CollidedWithWall",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::CollidedWithWall)},
    {"CanBitePlayer", static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::CanBitePlayer)},
    {"InMeleeRange", static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::InMeleeRange)},
    {"InBiteRange", static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::InBiteRange)},
    {"CantMoveToPlayer",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::CantMoveToPlayer)},
    {"ShouldEndPursuit",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::ShouldEndPursuit)},
    {"ShouldEndBallPursuit",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::ShouldEndBallPursuit)},
    {"PlayerInBallMode",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::PlayerInBallMode)},
    {"DetectBall", static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::DetectBall)},
    {"CanGrabBall", static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::CanGrabBall)},
    {"BallGrabbed", static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::BallGrabbed)},
    {"IsPlayerReachable",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::IsPlayerReachable)},
    {"ShouldAbortBallGrab",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::ShouldAbortBallGrab)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Patrol", static_cast< CPatterned::StateMachine::StateFunc >(&CBlogg::Patrol)},
    {"MoveToAttackPosition",
     static_cast< CPatterned::StateMachine::StateFunc >(&CBlogg::MoveToAttackPosition)},
    {"MoveToValidPosition",
     static_cast< CPatterned::StateMachine::StateFunc >(&CBlogg::MoveToValidPosition)},
    {"FacePlayer", static_cast< CPatterned::StateMachine::StateFunc >(&CBlogg::FacePlayer)},
    {"ProjectileAttack",
     static_cast< CPatterned::StateMachine::StateFunc >(&CBlogg::ProjectileAttack)},
    {"ChargeTelegraph",
     static_cast< CPatterned::StateMachine::StateFunc >(&CBlogg::ChargeTelegraph)},
    {"ChargeAttack", static_cast< CPatterned::StateMachine::StateFunc >(&CBlogg::ChargeAttack)},
    {"MeleeAttack", static_cast< CPatterned::StateMachine::StateFunc >(&CBlogg::MeleeAttack)},
    {"Stunned", static_cast< CPatterned::StateMachine::StateFunc >(&CBlogg::Stunned)},
    {"Taunt", static_cast< CPatterned::StateMachine::StateFunc >(&CBlogg::Taunt)},
    {"MoveToPlayer", static_cast< CPatterned::StateMachine::StateFunc >(&CBlogg::MoveToPlayer)},
    {"GrabBall", static_cast< CPatterned::StateMachine::StateFunc >(&CBlogg::GrabBall)},
    {"Thrash", static_cast< CPatterned::StateMachine::StateFunc >(&CBlogg::Thrash)},
    {"SpitBall", static_cast< CPatterned::StateMachine::StateFunc >(&CBlogg::SpitBall)},
    {"Dead", static_cast< CPatterned::StateMachine::StateFunc >(&CBlogg::Dead)},
};

static CPatterned::StateMachine::SCodeFunction skCodeFuncs[] = {
    {"ComputeAttackPositions",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CBlogg::ComputeAttackPositions)},
    {"ComputeTauntProbability",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CBlogg::ComputeTauntProbability)},
    {"EndMeleePursuit",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CBlogg::EndMeleePursuit)},
};

static const char* const skPivotLocatorName = "Skeleton_Root";         // Guessed name
static const char* const skMouthLocatorName = "mouth_LCTR";            // Guessed name
static const char* const skBallAttachLocatorName = "ball_attach_LCTR"; // Guessed name

static EMaterialTypes skHintRayInclude = kMT_Solid;                       // Guessed name
static EMaterialTypes skHintRayExclude0 = kMT_Character;                  // Guessed name
static EMaterialTypes skHintRayExclude1 = kMT_Player;                     // Guessed name
static EMaterialTypes skHintRayExclude2 = kMT_CollisionActor;             // Guessed name
static EMaterialTypes skHintRayExclude3 = kMT_AIPassthrough;              // Guessed name
static EMaterialTypes skHintRayExclude4 = kMT_ExcludeFromLineOfSightTest; // Guessed name

static float skAttackAngleStep = M_PIF / 6.f;                  // Guessed name
static CVector3f skWaterTestOffset = CVector3f(0.f, 0.f, 5.f); // Guessed name
static EMaterialTypes skGrabBallMaterial = kMT_Player;         // Guessed name
static EMaterialTypes skSpitBallMaterial = kMT_Player;         // Guessed name
static EMaterialTypes skContactDamageSolid = kMT_Solid;        // Guessed name
static EMaterialTypes skCollisionCeiling = kMT_Ceiling;        // Guessed name
static EMaterialTypes skCollisionWall = kMT_Wall;              // Guessed name
static EMaterialTypes skCollisionFloor = kMT_Floor;            // Guessed name
static EMaterialTypes skCollisionCharacter = kMT_Character;    // Guessed name

static inline bool RollChance(CStateManager& mgr, float chance) { // Guessed name
  if (chance == 1.f) {
    return true;
  }
  return mgr.Random()->Float() <= chance;
}

static inline void FindAnimation(const CPASDatabase& pasDatabase, const CPASAnimParmData& parms,
                                 int& anim) { // Guessed name
  const rstl::pair< float, int > best = pasDatabase.FindBestAnimation(parms, -1);
  if (best.first > 0.0000001192f) {
    anim = best.second;
  }
}

static float sLocomotionSpeedA; // Guessed name
static float sLocomotionSpeedB; // Guessed name

SBloggPhaseData::SBloggPhaseData(const SLdrBloggStruct& data)
: mMin(data.min_________________________)
, mMax(data.max_________________________)
, mUnknownA(data.unknown_0x6e603df2)
, mUnknownB(data.unknown_0x1e74f1ec)
, mUnknownC(data.unknown_0xecba9fb2) {}

CBloggBodyVulnerability::CBloggBodyVulnerability(const CDamageVulnerability& vulnerability,
                                                 const CDamageVulnerability& otherVulnerability,
                                                 bool flag)
: mVulnerability(vulnerability)
, mOtherVulnerability(otherVulnerability)
, mOwner(nullptr)
, mFlag(flag) {}

const CDamageVulnerability* CBloggBodyVulnerability::GetDamageVulnerability(
    const CDamageVulnerability* defaultVuln, const CVector3f& position, const CVector3f& direction,
    const CDamageInfo& damage) {
  if (mOwner != nullptr && mOwner->IsIngPossessed()) {
    return mOwner->GetIngPossessedArmorVulnerability();
  }
  return &mVulnerability;
}

bool CBloggBodyVulnerability::GetCollisionResponseType(const CVector3f& position,
                                                       const CVector3f& direction,
                                                       const CWeaponMode& mode, int attributes,
                                                       EWeaponCollisionResponseTypes& response) {
  switch (mVulnerability.GetEffect(mode)) {
  case CWeaponTypeVulnerability::kE_Reflect:
    response = kWCR_EnemyShielded;
    break;
  default:
    response = kWCR_EnemyNormal;
    break;
  }
  return true;
}

CBloggMouthVulnerability::CBloggMouthVulnerability(const CDamageVulnerability& vulnerability,
                                                   const CDamageVulnerability& otherVulnerability,
                                                   bool flag)
: CBloggBodyVulnerability(vulnerability, otherVulnerability, flag)
, mResponseType(kWCR_EnemyShielded) {}

const CDamageVulnerability* CBloggMouthVulnerability::GetDamageVulnerability(
    const CDamageVulnerability* defaultVuln, const CVector3f& position, const CVector3f& direction,
    const CDamageInfo& damage) {
  switch (mVulnerability.GetEffect(damage.GetWeaponMode())) {
  case CWeaponTypeVulnerability::kE_Reflect:
    mResponseType = kWCR_EnemyShielded;
    break;
  default:
    mResponseType = kWCR_EnemyNormal;
    break;
  }

  if (mOwner != nullptr && mOwner->IsHitInMouthDirection(direction) && !mOwner->IsMouthClosed()) {
    mResponseType = kWCR_EnemyNormal;
    return mOwner->GetDamageVulnerability();
  }

  if (mOwner != nullptr && mOwner->IsIngPossessed()) {
    return mOwner->GetIngPossessedArmorVulnerability();
  }
  return &mVulnerability;
}

bool CBloggMouthVulnerability::GetCollisionResponseType(const CVector3f& position,
                                                        const CVector3f& direction,
                                                        const CWeaponMode& mode, int attributes,
                                                        EWeaponCollisionResponseTypes& response) {
  response = mResponseType;
  return true;
}

CBlogg::CBlogg(TUniqueId uid, const rstl::string& name, CEntityInfo& info, const CTransform4f& xf,
               const CModelData& modelData, const CPatternedInfo& patternedInfo,
               const CActorParameters& actorParams, float minAttackAngle, float maxAttackAngle,
               float minProjectileDelay, float maxProjectileDelay, uchar unknown_0xa19d5f62,
               CAssetId projectileParticleEffect, const CDamageInfo& projectileDamage,
               const CDamageVulnerability& armorVulnerability, float bodyDamageMultiplier,
               float mouthDamageMultiplier, float mouthDamageAngle, float chargeDamageRadius,
               float chargeDamage, float biteDamage, float ballSpitDamage,
               float fishAttractionRadius, float fishAttractionPriority, float aggressiveness,
               float unknown_0x479ccc37, float unknown_0x689a803f, float unknown_0x800a2b0d,
               float chargeTurnSpeed, float chargeSpeedMultiplier, float maxMeleeRange,
               float maxBallDetectionRange, float maxPlayerPursuitTime, float maxBallPursuitTime,
               ushort mouthOpenSound, float minDelayBetweenMeleeAttacks, float maxCollisionTime,
               bool isMegaBlogg, float projectileBlurRadius, float projectileBlurTime,
               const CDamageVulnerability& ingPossessedArmorVulnerability,
               const SBloggPhaseData& phase0, const SBloggPhaseData& phase1,
               const SBloggPhaseData& phase2)
: CPatterned(kPAI_Blogg, uid, name, kFT_Zero, info, xf, modelData, patternedInfo, kMT_Flyer,
             kCT_One, kBT_PitchableFlyer, actorParams)
, mAimAnimLeft(0)
, mAimAnimRight(0)
, mAimAnimUp(0)
, mAimAnimDown(0)
, x7d4_(0)
, mUnknown26Anim(0)
, mAimWeightLeft(0.f)
, mAimWeightRight(0.f)
, mAimWeightUp(0.f)
, mAimWeightDown(0.f)
, mLastForward(xf.GetForward())
, mTargetForward(xf.GetForward())
, mContactDamage(patternedInfo.GetContactDamage())
, mMinAttackAngle((M_PIF / 180.f) * minAttackAngle)
, mMaxAttackAngle((M_PIF / 180.f) * maxAttackAngle)
, mMinAttackRange(patternedInfo.GetMinAttackRange())
, mMaxAttackRange(patternedInfo.GetMaxAttackRange())
, mCurrentAttackAngle((M_PIF / 180.f) * minAttackAngle)
, mCurrentAttackRange(patternedInfo.GetMinAttackRange())
, mAttackPosition(CVector3f::Zero())
, mHintId(kInvalidUniqueId)
, mPathFindSearch(nullptr, 4, 0, 1.f, 1.f, 0, CPFRegion::kRP_Center)
, mProjectileDirection(CVector3f::Forward())
, mMinProjectileDelay(minProjectileDelay)
, mMaxProjectileDelay(maxProjectileDelay)
, mProjectileDelay(minProjectileDelay)
, mUnknown_0xa19d5f62(unknown_0xa19d5f62)
, x951_(0)
, mProjectileInfo(projectileParticleEffect, projectileDamage)
, x97c_(CVector3f::Zero())
, mBaseTurnSpeed(mTurnSpeed)
, mProjectileScale(3.f)
, mPlayerId(kInvalidUniqueId)
, mTeamManagerId(kInvalidUniqueId)
, mPositionHistory(CVector3f::Zero())
, mBodyDamageMultiplier(bodyDamageMultiplier)
, mMouthDamageMultiplier(mouthDamageMultiplier)
, mArmorVulnerability(armorVulnerability)
, mIngPossessedArmorVulnerability(ingPossessedArmorVulnerability)
, xac4_(kInvalidUniqueId)
, mMouthDamageAngle((M_PIF / 180.f) * mouthDamageAngle)
, mChargeDamageRadius(chargeDamageRadius)
, mChargeDamage(chargeDamage)
, mChargeTurnSpeed(chargeTurnSpeed)
, mChargeSpeedMultiplier(chargeSpeedMultiplier)
, mBiteDamage(biteDamage)
, mBallSpitDamage(ballSpitDamage)
, mMaxMeleeRange(maxMeleeRange)
, mMaxBallDetectionRange(maxBallDetectionRange)
, mMaxPlayerPursuitTime(maxPlayerPursuitTime)
, mMaxBallPursuitTime(maxBallPursuitTime)
, mBallPursuitTime(0.f)
, mPlayerPursuitTime(0.f)
, mFishAttractionRadius(fishAttractionRadius)
, mFishAttractionPriority(fishAttractionPriority)
, mAggressiveness(aggressiveness)
, mUnknown_0x479ccc37(unknown_0x479ccc37)
, mUnknown_0x689a803f(unknown_0x689a803f)
, mUnknown_0x800a2b0d(unknown_0x800a2b0d)
, mCollisionTime(0.f)
, mMaxCollisionTime(maxCollisionTime)
, mBallGrabTime(0.f)
, mLocomotionChangeTimer(0.f)
, mLocomotionChangeInterval(10.f)
, mMouthOpenSound(mouthOpenSound)
, mBaseSpeed(patternedInfo.GetSpeed())
, mMeleeDelayTimer(0.f)
, mMinDelayBetweenMeleeAttacks(minDelayBetweenMeleeAttacks)
, mMouthVulnerability(rs_new CBloggMouthVulnerability(armorVulnerability, armorVulnerability, true))
, mBodyVulnerability(rs_new CBloggBodyVulnerability(armorVulnerability, armorVulnerability, true))
, mProjectileBlurRadius(projectileBlurRadius)
, mProjectileBlurTime(projectileBlurTime)
, xb68_(0.f)
, mLineOfSightTracker(GetUniqueId(), CSegId(1), 0.3f, 0.f)
, mPhaseValue(0)
, xbb1_(0)
, xbb2_(0)
, xbc4_24_(false)
, xbc4_25_(false)
, mCanBite(false)
, xbc4_27_(false)
, mBallGrabbed(false)
, mTauntReady(false)
, mChargeOver(false)
, mIsMegaBlogg(isMegaBlogg)
, mMeleePursuitEnded(false)
, mAbortBallGrab(false)
, xbc5_26_(false)
, xbc5_27_(true)
, xbc5_28_(true)
, xbc5_29_(false)
, xbc5_30_(false) {
  mProjectileInfo.Token().Lock();

  const CPASDatabase& pasDatabase = AnimationData()->GetPASDatabase();
  FindAnimation(
      pasDatabase,
      CPASAnimParmData(pas::kAS_AdditiveAim, CPASAnimParm::FromEnum(0), CPASAnimParm::FromEnum(0)),
      mAimAnimLeft);
  FindAnimation(
      pasDatabase,
      CPASAnimParmData(pas::kAS_AdditiveAim, CPASAnimParm::FromEnum(1), CPASAnimParm::FromEnum(0)),
      mAimAnimRight);
  FindAnimation(
      pasDatabase,
      CPASAnimParmData(pas::kAS_AdditiveAim, CPASAnimParm::FromEnum(2), CPASAnimParm::FromEnum(0)),
      mAimAnimUp);
  FindAnimation(
      pasDatabase,
      CPASAnimParmData(pas::kAS_AdditiveAim, CPASAnimParm::FromEnum(3), CPASAnimParm::FromEnum(0)),
      mAimAnimDown);
  FindAnimation(pasDatabase, CPASAnimParmData(pas::kAS_Unknown26), mUnknown26Anim);

  sLocomotionSpeedA = GetLocomotionSpeed(
      CPASAnimParmData(pas::kAS_Locomotion, CPASAnimParm::FromEnum(2), CPASAnimParm::FromEnum(3)));
  sLocomotionSpeedB = GetLocomotionSpeed(
      CPASAnimParmData(pas::kAS_Locomotion, CPASAnimParm::FromEnum(2), CPASAnimParm::FromEnum(2)));

  SetDrawShadow(false);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Character, kMT_CollisionActor, kMT_AIPassthrough, kMT_Player, kMT_Solid),
      CMaterialList()));
  BodyController()->BodyStateInfo().SetMaximumPitch(80.f * (M_PIF / 180.f));

  rstl::rc_ptr< CBloggMouthVulnerability > mouthVulnerability(mMouthVulnerability);
  if (mouthVulnerability) {
    mouthVulnerability->SetOwner(this);
  }
  rstl::rc_ptr< CBloggBodyVulnerability > bodyVulnerability(mBodyVulnerability);
  if (bodyVulnerability) {
    bodyVulnerability->SetOwner(this);
  }

  mKnockBackController.EnableAllAnimReactions(false);
  mKnockBackController.EnableKnockBackPhysics(false);
  mKnockBackController.EnableBurn(false);
  mKnockBackController.EnableFreeze(false);
  mKnockBackController.EnableSlow(false);

  if (mIsMegaBlogg) {
    mPhases.reserve(3);
    mPhases.push_back_unsafe(phase0);
    mPhases.push_back_unsafe(phase1);
    mPhases.push_back_unsafe(phase2);
  }
}

int CBlogg::GetHealthPhase() const {
  const float health = GetHealthInfo()->GetHP();
  const float fraction = health / GetHealthInfo()->GetInitialHP();
  if (fraction < 0.33f) {
    return 2;
  }
  return fraction < 0.66f;
}

void CBlogg::ChoosePhaseValue(CStateManager& mgr) {
  const uchar phaseIndex = GetHealthPhase();
  if (phaseIndex < mPhases.size()) {
    const SBloggPhaseData& phase = mPhases[phaseIndex];
    uint value = phase.mMin;
    const uchar range = phase.mMax - phase.mMin;
    if (range != 0) {
      value = phase.mMin + mgr.Random()->Next() % (range + 1);
    }
    mPhaseValue = value;
  }
}

void CBlogg::SyncCollisionActorHealth(CStateManager& mgr) {
  for (uint i = 0; i < mCollisionActorManager->GetNumCollisionActors(); ++i) {
    const TUniqueId id = mCollisionActorManager->GetCollisionDescFromIndex(i).GetCollisionActorId();
    if (CActor* actor = static_cast< CActor* >(mgr.ObjectById(id))) {
      *actor->HealthInfo() = *GetHealthInfo();
    }
  }
}

void CBlogg::SetIngPossessed(bool possessed, float duration, CStateManager& mgr) {
  CPatterned::SetIngPossessed(possessed, duration, mgr);
  SyncCollisionActorHealth(mgr);
}

void CBlogg::SetIngPossessed(bool possessed, CStateManager& mgr) {
  CPatterned::SetIngPossessed(possessed, mgr);
  SyncCollisionActorHealth(mgr);
}

CVector3f CBlogg::GetIngSnatchingPoint(float t) const {
  const CAABox localBounds = GetModelData()->GetBounds();
  CAABox bounds = GetModelData()->GetBounds(GetTransform());
  const CVector3f normal = GetIngSnatchingNormal(t);
  const CVector3f center = bounds.GetCenterPoint();
  const float height = localBounds.GetMaxPoint().GetY() - localBounds.GetMinPoint().GetY();
  return center + (0.5f - t) * height * normal;
}

CVector3f CBlogg::GetIngSnatchingNormal(float) const { return -GetTransform().GetForward(); }

CBlogg::~CBlogg() {}

CPlayer* CBlogg::GetPlayer(CStateManager& mgr) const {
  return mPlayerId != kInvalidUniqueId ? TCastToPtr< CPlayer >(mgr.ObjectById(mPlayerId)) : nullptr;
}

bool CBlogg::IsPlayerWithin(CStateManager& mgr, float distance) const {
  CPlayer* player = GetPlayer(mgr);
  if (player != nullptr) {
    const CVector3f playerPosition = player->GetAimPosition(mgr, 0.f);
    const float dx = GetTranslation().GetX() - playerPosition.GetX();
    const float dy = GetTranslation().GetY() - playerPosition.GetY();
    const float dz = GetTranslation().GetZ() - playerPosition.GetZ();
    if (dx * dx + dy * dy + dz * dz < distance * distance) {
      return true;
    }
  }
  return false;
}

bool CBlogg::IsHitInMouthDirection(const CVector3f& direction) const {
  const CTransform4f mouthTransform =
      GetTransform() * GetScaledLocatorTransform(rstl::string_l(skMouthLocatorName));
  const float dot = CVector3f::Dot(direction, mouthTransform.GetForward());
  return dot < static_cast< float >(cos(rstl::max_val(M_PIF / 2.f, M_PIF - mMouthDamageAngle)));
}

uchar CBlogg::HasCollisionTimeElapsed() const { return mCollisionTime >= mMaxCollisionTime; }

bool CBlogg::IsAtAttackPosition() const {
  return CPatterned::GetSearchPath()->IsOver() && mState == kBS_MoveToAttackPosition;
}

void CBlogg::PreRender(CStateManager& mgr) { CPatterned::PreRender(mgr); }

void CBlogg::AddToRenderer(const CStateManager& mgr) const { CPatterned::AddToRenderer(mgr); }

void CBlogg::Render(const CStateManager& mgr) const { CPatterned::Render(mgr); }

void CBlogg::SetupStateMachine(CStateManager& mgr) {
  StateMachine* stateMachine = mStateMachine.get();
  CPatterned::SetupStateMachine(mgr);
  stateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
  stateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
  stateMachine->SetCodeFunctions(skCodeFuncs, ARRAY_SIZE(skCodeFuncs));
}

void CBlogg::ComputeTauntProbability(CStateManager& mgr, float dt) {
  if (mgr.Random()->Float() < 0.2f) {
    mTauntReady = true;
  }
}

void CBlogg::EndMeleePursuit(CStateManager& mgr, float dt) { mMeleePursuitEnded = true; }

bool CBlogg::ShouldPatrol(CStateManager& mgr, const CTriggerData& data) const { return false; }

bool CBlogg::AnimOver(CStateManager& mgr, const CTriggerData& data) const {
  return CPatterned::AnimOver(mgr, data);
}

bool CBlogg::ShouldPrepareToAttack(CStateManager& mgr, const CTriggerData& data) const {
  return xbc4_24_ || xbc5_26_ || xbc5_30_;
}

bool CBlogg::IsPlayerStunned(CStateManager& mgr, const CTriggerData& data) const { return false; }

bool CBlogg::CollidedWithWall(CStateManager& mgr, const CTriggerData& data) const {
  return HasCollisionTimeElapsed();
}

bool CBlogg::CanBitePlayer(CStateManager& mgr, const CTriggerData& data) const { return mCanBite; }

bool CBlogg::InMeleeRange(CStateManager& mgr, const CTriggerData& data) const {
  return IsPlayerWithin(mgr, mMaxMeleeRange);
}

bool CBlogg::InBiteRange(CStateManager& mgr, const CTriggerData& data) const {
  return IsPlayerWithin(mgr, 3.f * GetModelData()->GetScale().GetX());
}

bool CBlogg::InProjectileRange(CStateManager& mgr, const CTriggerData& data) const {
  return IsPlayerWithin(mgr, 2.f * mMaxAttackRange);
}

bool CBlogg::CantMoveToPlayer(CStateManager& mgr, const CTriggerData& data) const { return false; }

bool CBlogg::BallGrabbed(CStateManager& mgr, const CTriggerData& data) const {
  return mBallGrabbed;
}

bool CBlogg::ShouldAbortBallGrab(CStateManager& mgr, const CTriggerData& data) const {
  return mAbortBallGrab;
}

CVector3f CBlogg::GetDirectionToPlayer(CStateManager& mgr) const {
  const CVector3f position = GetTransform().GetTranslation();
  CPlayer* player = GetPlayer(mgr);
  if (player != nullptr) {
    return (player->GetAimPosition(mgr, 0.f) - position).AsNormalized();
  }
  return CVector3f::Forward();
}

void CBlogg::FindFluid(CStateManager& mgr, CAABox& bounds, TUniqueId& waterId) const {
  if (GetFluidCount() != 0) {
    waterId = InFluidId();
    if (const CScriptWater* water = TCastToConstPtr< CScriptWater >(mgr.GetObjectById(waterId))) {
      bounds = water->GetTriggerBoundsWR();
    } else {
      waterId = kInvalidUniqueId;
    }
  }
}

bool CBlogg::IsInhabitingFluid(CStateManager& mgr, TUniqueId waterId, TUniqueId uid) const {
  const CScriptWater* water = TCastToConstPtr< CScriptWater >(mgr.GetObjectById(waterId));
  if (water != nullptr && water->HasInhabitant(uid)) {
    return true;
  }
  return false;
}

bool CBlogg::CanReachPlayer(CStateManager& mgr, CPlayer* player) const {
  TUniqueId waterId = kInvalidUniqueId;
  CAABox waterBounds = CAABox::MakeNullBox();
  FindFluid(mgr, waterBounds, waterId);
  const CVector3f position = player->GetAimPosition(mgr, 0.f);
  if (IsInhabitingFluid(mgr, waterId, player->GetUniqueId()) &&
      mPathFindSearch.OnPath(position) == CPathFindSearch::kR_Success) {
    return true;
  }
  return false;
}

bool CBlogg::InAttackPosition(CStateManager& mgr, const CTriggerData& data) const {
  return IsAtAttackPosition() || xbc5_26_ || xbc5_30_;
}

bool CBlogg::InValidPosition(CStateManager& mgr, const CTriggerData& data) const {
  bool result = true;
  const bool valid = mStateMachine->GetTime() > 1.f && !HasCollisionTimeElapsed();
  if (!valid && !CPatterned::GetSearchPath()->IsOver()) {
    result = false;
  }
  return result;
}

bool CBlogg::IsFacingPlayer(CStateManager& mgr, const CTriggerData& data) const {
  const CVector3f direction(GetDirectionToPlayer(mgr));
  const CVector3f forward = GetTransform().GetForward();
  return CVector3f::GetAngleDiff(direction, forward) < (10.f * (M_PIF / 180.f));
}

bool CBlogg::ProjectileAttackDelay(CStateManager& mgr, const CTriggerData& data) const {
  return mStateMachine->GetTime() > mProjectileDelay;
}

bool CBlogg::ShouldCharge(CStateManager& mgr, const CTriggerData& data) const {
  if (mIsMegaBlogg && xbc5_29_ && !xbc5_27_ && !xbc5_26_ && !xbc5_30_) {
    return false;
  }
  CPlayer* player = GetPlayer(mgr);
  if (player != nullptr && player->GetFrozenState()) {
    return true;
  }
  bool result = false;
  if (x951_ >= mUnknown_0xa19d5f62 || xbc4_25_ || mAggressiveness == 1.f || xbc5_26_ || xbc5_30_) {
    result = true;
  }
  return result;
}

bool CBlogg::IsChargeOver(CStateManager& mgr, const CTriggerData& data) const {
  return mChargeOver != 0;
}

bool CBlogg::CanMeleeAttack(CStateManager& mgr, const CTriggerData& data) const {
  if (mMeleePursuitEnded || mChargeOver) {
    return false;
  }
  if (mIsMegaBlogg && !xbc5_27_ && !xbc5_26_ && !xbc5_30_) {
    return false;
  }
  bool teamAllows = true;
  if (mTeamManagerId != kInvalidUniqueId) {
    bool canStart = false;
    if (CScriptTeamAiMgr::CanStartAttack(CScriptTeamAiMgr::kAT_Melee, mgr, mTeamManagerId,
                                         GetUniqueId())) {
      if (CScriptTeamAiMgr::CanStartAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamManagerId,
                                           GetUniqueId())) {
        canStart = true;
      }
    }
    if (!canStart) {
      teamAllows = false;
    }
  }
  return teamAllows || xbc5_26_ || xbc5_30_;
}

bool CBlogg::CanRangedAttack(CStateManager& mgr, const CTriggerData& data) const {
  bool result = false;
  bool teamAllows = true;
  if (mTeamManagerId != kInvalidUniqueId) {
    bool canStart = false;
    if (CScriptTeamAiMgr::CanStartAttack(CScriptTeamAiMgr::kAT_Melee, mgr, mTeamManagerId,
                                         GetUniqueId())) {
      if (CScriptTeamAiMgr::CanStartAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamManagerId,
                                           GetUniqueId())) {
        canStart = true;
      }
    }
    if (!canStart) {
      teamAllows = false;
    }
  }
  if (teamAllows) {
    bool underLimit = false;
    if (x951_ < mUnknown_0xa19d5f62 && !xbc4_25_) {
      underLimit = true;
    }
    if (underLimit) {
      result = true;
    }
  }
  return result;
}

bool CBlogg::CanTaunt(CStateManager& mgr, const CTriggerData& data) const {
  return !CanMeleeAttack(mgr, data) && !CanRangedAttack(mgr, data) && mTauntReady;
}

bool CBlogg::ShouldEndPursuit(CStateManager& mgr, const CTriggerData& data) const {
  return !CanMeleeAttack(mgr, data) || mPlayerPursuitTime >= mMaxPlayerPursuitTime;
}

bool CBlogg::ShouldEndBallPursuit(CStateManager& mgr, const CTriggerData& data) const {
  CPlayer* player = GetPlayer(mgr);
  if (player != nullptr) {
    const CPlayer::EPlayerMorphBallState state =
        player->GetSpawnedMorphballState() == CPlayer::kMS_Unmorphed
            ? player->GetMorphballTransitionState()
            : CPlayer::kMS_Unmorphed;
    if (state != CPlayer::kMS_Morphed) {
      return true;
    }
  }
  return !CanMeleeAttack(mgr, data) || mBallPursuitTime >= mMaxBallPursuitTime;
}

bool CBlogg::PlayerInBallMode(CStateManager& mgr, const CTriggerData& data) const {
  CPlayer* player = GetPlayer(mgr);
  if (player != nullptr) {
    return (player->GetSpawnedMorphballState() == CPlayer::kMS_Unmorphed
                ? player->GetMorphballTransitionState()
                : CPlayer::kMS_Unmorphed) == CPlayer::kMS_Morphed;
  }
  return false;
}

bool CBlogg::DetectBall(CStateManager& mgr, const CTriggerData& data) const {
  return mIsMegaBlogg ? false : mCanBite && IsPlayerWithin(mgr, mMaxBallDetectionRange);
}

bool CBlogg::CanGrabBall(CStateManager& mgr, const CTriggerData& data) const {
  CPlayer* player = GetPlayer(mgr);
  if (player != nullptr) {
    const CTransform4f locator = GetScaledLocatorTransform(rstl::string_l(skBallAttachLocatorName));
    const CTransform4f xf = GetTransform() * locator;
    const CVector3f offset = player->GetAimPosition(mgr, 0.f) - xf.GetTranslation();
    if (offset.MagSquared() < 4.f) {
      return true;
    }
  }
  return false;
}

bool CBlogg::IsPlayerReachable(CStateManager& mgr, const CTriggerData& data) const {
  CPlayer* player = GetPlayer(mgr);
  return player != nullptr ? CanReachPlayer(mgr, player) : false;
}

void CBlogg::ReleaseHints(CStateManager& mgr) {
  const uint count = mHintIds.size();
  for (uint i = 0; i < count; ++i) {
    const TUniqueId id = mHintIds[i];
    CScriptAIHint* hint = static_cast< CScriptAIHint* >(mgr.ObjectById(id));
    if (hint != nullptr) {
      hint->SetInUse(false);
      hint->SetTimeRemaining(0.f);
    }
  }
}

TUniqueId CBlogg::FindNearestHint(CStateManager& mgr, const CVector3f& position,
                                  bool checkLineOfSight) const {
  TUniqueId nearest = kInvalidUniqueId;
  const uint count = mHintIds.size();
  float nearestDistance = 1000000000.f;
  float bestDot = -1000000000.f;
  TUniqueId facing = nearest;
  const CVector3f forward = GetTransform().GetForward();
  const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
      CMaterialList(skHintRayInclude),
      CMaterialList(skHintRayExclude0, skHintRayExclude1, skHintRayExclude2, skHintRayExclude3,
                    skHintRayExclude4));
  for (uint i = 0; i < count; ++i) {
    const TUniqueId id = mHintIds[i];
    const CScriptAIHint* hint = static_cast< const CScriptAIHint* >(mgr.GetObjectById(id));
    if (hint != nullptr && hint->GetActive() && !hint->GetInUse(kInvalidUniqueId)) {
      const CVector3f offset(hint->GetTranslation() - position);
      const float distance = offset.MagSquared();
      if (distance < nearestDistance) {
        if (checkLineOfSight) {
          const CRayCastResult result = CGameCollision::RayStaticIntersection(
              mgr, position, offset.AsNormalized(), offset.Magnitude(), filter);
          if (!result.IsValid()) {
            nearestDistance = distance;
            nearest = id;
          }
        } else {
          nearestDistance = distance;
          nearest = id;
        }
      }
      const CVector3f direction = offset.AsNormalized();
      const float dot = CVector3f::Dot(direction, forward);
      if (dot > bestDot) {
        facing = id;
        bestDot = dot;
      }
    }
  }
  if (facing != kInvalidUniqueId) {
    return facing;
  }
  return nearest;
}

void CBlogg::CollectHints(CStateManager& mgr) {
  CObjectList& list = mgr.ObjectListById(kOL_AiWaypoint);
  mHintIds.reserve(32);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    CScriptAIHint* hint = TCastToPtr< CScriptAIHint >(list[i]);
    if (hint != nullptr && hint->GetHintType() == CScriptAIHint::kHT_BloggHint &&
        hint->GetActive() && hint->GetCurrentAreaId() == GetCurrentAreaId() &&
        !hint->GetInUse(kInvalidUniqueId) && mHintIds.size() < 32u) {
      mHintIds.push_back_unsafe(hint->GetUniqueId());
    }
  }
}

void CBlogg::UpdateCollisionActorMaterials(CStateManager& mgr, const CMaterialList& materials,
                                           EMaterialAction action) {
  for (uint i = 0; i < mCollisionActorManager->GetNumCollisionActors(); ++i) {
    const TUniqueId id = mCollisionActorManager->GetCollisionDescFromIndex(i).GetCollisionActorId();
    CCollisionActor* actor = static_cast< CCollisionActor* >(mgr.ObjectById(id));
    if (actor != nullptr) {
      switch (action) {
      case kMA_Add:
        actor->MaterialList().Add(materials);
        break;
      case kMA_Remove:
        actor->MaterialList().Remove(materials);
        break;
      }
    }
  }
}

void CBlogg::StopPlayer(CStateManager& mgr) {
  CPlayer* player = GetPlayer(mgr);
  if (player != nullptr) {
    player->Stop();
    player->SetAngularVelocityWR(CAxisAngle::Identity());
    player->SetVelocityWR(CVector3f::Zero());
    player->EnableLeaveMorphBall(false);
  }
}

void CBlogg::AttachPlayerToMouth(CStateManager& mgr) {
  StopPlayer(mgr);
  CPlayer* player = GetPlayer(mgr);
  if (player != nullptr) {
    const CTransform4f locator = GetScaledLocatorTransform(rstl::string_l(skBallAttachLocatorName));
    const CTransform4f xf = GetTransform() * locator;
    const CVector3f attach = xf.GetTranslation();
    const CVector3f offset = player->GetAimPosition(mgr, 0.f) - player->GetTranslation();
    player->SetTranslation(attach - offset);
  }
}

uchar CBlogg::GetNextPositionIndex() const {
  const int count = mPositionHistory.size();
  if (count > 0) {
    switch (xb14_) {
    case 1:
      if (xb18_ - 1 < 0) {
        return count - 1;
      }
      return 0;
    case 0:
      if (xb18_ + 1 >= count) {
        return 0;
      }
      return 0;
    default:
      return 0;
    }
  }
  return 0;
}

void CBlogg::PathToAttackPosition(CStateManager& mgr, float dt) {
  CVector3f destination = GetTranslation();
  if (mHintIds.size() != 0u) {
    const TUniqueId id = FindNearestHint(mgr, GetTranslation(), false);
    if (id != kInvalidUniqueId) {
      CScriptAIHint* hint = static_cast< CScriptAIHint* >(mgr.ObjectById(id));
      if (hint != nullptr) {
        ReleaseHints(mgr);
        hint->SetInUse(true);
        destination = hint->GetTranslation();
        mAttackPosition = destination;
        mHintId = id;
      }
    }
  } else {
    xb18_ = GetNextPositionIndex();
    mHintId = kInvalidUniqueId;
    if (xb18_ < mPositionHistory.size()) {
      destination = mPositionHistory[xb18_];
      mAttackPosition = destination;
    }
  }
  mPathFindNavigation.SetDestination(destination);
  mPathFindNavigation.PathFind(mgr, kStateMsg_Activate, dt, *this);
}

void CBlogg::LeaveTeam(CStateManager& mgr) {
  if (mTeamManagerId != kInvalidUniqueId) {
    CScriptTeamAiMgr* team = TCastToPtr< CScriptTeamAiMgr >(mgr.ObjectById(mTeamManagerId));
    if (team != nullptr) {
      if (team->IsPartOfTeam(GetUniqueId())) {
        team->QuitTeam(GetUniqueId());
        mTeamManagerId = kInvalidUniqueId;
      }
    }
  }
}

void CBlogg::JoinTeam(CStateManager& mgr) {
  if (mTeamManagerId == kInvalidUniqueId) {
    mTeamManagerId = CScriptTeamAiMgr::GetAssociatedTeamId(*this, mgr);
    if (mTeamManagerId != kInvalidUniqueId) {
      CScriptTeamAiMgr* team = TCastToPtr< CScriptTeamAiMgr >(mgr.ObjectById(mTeamManagerId));
      if (team != nullptr) {
        team->JoinTeam(*this, CTeamAiRole::kTAR_Melee, CTeamAiRole::kTAR_Projectile,
                       CTeamAiRole::kTAR_Invalid);
      }
    }
  }
}

void CBlogg::ApplyContactDamage(CStateManager& mgr, CPlayer& player, const CDamageInfo& damage) {
  if (mCurDamageRemTime <= 0.f) {
    CVector3f direction = CVector3f::Forward();
    direction = GetTransform().BuildMatrix3f() * direction;
    mgr.ApplyDamage(
        GetUniqueId(), player.GetUniqueId(), GetUniqueId(), damage,
        CMaterialFilter::MakeIncludeExclude(CMaterialList(skContactDamageSolid), CMaterialList()),
        CVector3f::Zero());
    mCurDamageRemTime = mDamageWaitTime;
    if (mState == kBS_ChargeAttack && player.GetFrozenState()) {
      player.BreakFrozenState(mgr, CPlayer::kBFS_BreakWithEffects, false);
    }
  }
}

bool CBlogg::IsPlayerInMouthRange(CStateManager& mgr) const {
  CPlayer* player = GetPlayer(mgr);
  if (player != nullptr) {
    const CVector3f playerPosition = player->GetTranslation();
    const CTransform4f xf =
        GetTransform() * GetScaledLocatorTransform(rstl::string_l(skMouthLocatorName));
    const CVector3f offset = playerPosition - xf.GetTranslation();
    if (offset.MagSquared() < mChargeDamageRadius * mChargeDamageRadius) {
      return true;
    }
  }
  return false;
}

void CBlogg::CollidedWith(const TUniqueId& id, const CCollisionInfoList& list, CStateManager& mgr) {
  if (mState == kBS_ChargeAttack || mState == kBS_MoveToPlayer || mState == kBS_Patrol ||
      mState == kBS_MoveToAttackPosition || mState == kBS_MoveToValidPosition) {
    static const CMaterialList testList(skCollisionCeiling, skCollisionWall, skCollisionFloor,
                                        skCollisionCharacter);
    for (int i = 0; i < list.GetCount(); ++i) {
      if (list[i].GetMaterialLeft().SharesMaterials(testList)) {
        mCollisionTime += mPreThinkDt;
        break;
      }
    }
  }
  CPatterned::CollidedWith(id, list, mgr);
}

void CBlogg::Touch(CActor& actor, CStateManager& mgr) {
  CFishCloud* fishCloud = TCastToPtr< CFishCloud >(actor);
  if (fishCloud != nullptr) {
    if (mState == kBS_ChargeAttack) {
      fishCloud->AddRepulsor(GetUniqueId(), false, 20.f, 0.5f);
    } else {
      fishCloud->AddAttractor(GetUniqueId(), false, 20.f, 0.5f);
    }
  }
}

void CBlogg::Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) {
  CPlayer* player = GetPlayer(mgr);
  if (player != nullptr) {
    if ((player->GetSpawnedMorphballState() == CPlayer::kMS_Unmorphed
             ? player->GetMorphballTransitionState()
             : CPlayer::kMS_Unmorphed) == CPlayer::kMS_Morphed) {
      player->SetMoveState(NPlayer::kMS_ApplyJump, mgr);
      player->EnableLeaveMorphBall(true);
    }
  }
  if (mBallGrabbed) {
    mBallGrabbed = false;
    mPendingMassiveDeath = true;
    RemoveMaterial(kMT_Solid, mgr);
    RemoveMaterial(kMT_Target, mgr);
    RemoveMaterial(kMT_Orbit, mgr);
    mCollisionActorManager->SetActive(mgr, false);
  }
  if (mTeamManagerId != kInvalidUniqueId) {
    CScriptTeamAiMgr::EndAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamManagerId,
                                GetUniqueId(), false);
    CScriptTeamAiMgr::EndAttack(CScriptTeamAiMgr::kAT_Melee, mgr, mTeamManagerId, GetUniqueId(),
                                false);
    LeaveTeam(mgr);
  }
  CPatterned::Death(mgr, direction, state);
  mVerticalMovement = true;
}

void CBlogg::ApplyCollisionActorDamage(CStateManager& mgr, TUniqueId senderId, float multiplier) {
  if (mAlive) {
    CCollisionActor* collisionActor = TCastToPtr< CCollisionActor >(mgr.ObjectById(senderId));
    if (collisionActor != nullptr) {
      const TUniqueId touchedId = collisionActor->GetLastTouchedObject();
      CHealthInfo* collisionHealth = collisionActor->HealthInfo();
      CHealthInfo* health = HealthInfo();
      const float initialHealth = health->GetInitialHP();
      const CWeapon* weapon = TCastToConstPtr< CWeapon >(mgr.GetObjectById(touchedId));
      const CBomb* bomb = TCastToConstPtr< CBomb >(mgr.GetObjectById(touchedId));
      const CPowerBomb* powerBomb = TCastToConstPtr< CPowerBomb >(mgr.GetObjectById(touchedId));
      if (weapon != nullptr || bomb != nullptr || powerBomb != nullptr) {
        CVector3f direction = CVector3f::Forward();
        TUniqueId ownerId = kInvalidUniqueId;
        CDamageInfo damage;
        if (weapon != nullptr) {
          damage = weapon->GetCurrentDamageInfo();
        } else if (bomb != nullptr) {
          ownerId = bomb->GetOwnerId();
          damage = bomb->GetCurrentDamageInfo();
        } else if (powerBomb != nullptr) {
          ownerId = powerBomb->GetOwnerId();
          damage = powerBomb->GetCurrentDamageInfo();
        }
        const float currentHealth = health->GetHP();
        const float amount = multiplier * (initialHealth - collisionHealth->GetHP());
        health->SetHP(currentHealth - amount);
        TakeDamage(direction, amount);
        if (currentHealth <= amount) {
          Death(mgr, direction, kSS_DeathRattle);
        }
        if (damage.GetWeaponMode1() != -1) {
          const CKnockBackInfo knockBack(direction, touchedId, ownerId, damage, true);
          KnockBack(mgr, knockBack);
        }
      }
      collisionHealth->SetHP(initialHealth);
    }
  }
}

void CBlogg::MoveToValidPosition(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    mState = kBS_MoveToValidPosition;
    const TUniqueId hintId = FindNearestHint(mgr, GetTranslation(), true);
    if (hintId != kInvalidUniqueId) {
      const CEntity* hint = mgr.GetObjectById(hintId);
      if (hint != nullptr) {
        mPathFindNavigation.SetDestination(static_cast< const CActor* >(hint)->GetTranslation());
        mPathFindNavigation.PathFind(mgr, msg, dt, *this);
        mLineOfSightTracker.SetTarget(hintId);
      }
    }
    mCollisionTime = 0.f;
    break;
  }
  case kStateMsg_Update:
    if (mLineOfSightTracker.HasLineOfSight()) {
      const CVector3f move = mPathFindNavigation.GetDestinationPosition() - GetTranslation();
      BodyController()->CommandMgr().DeliverCmd(CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
      mCollisionTime = 0.f;
    } else {
      mPathFindNavigation.PathFind(mgr, msg, dt, *this);
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CBlogg::MoveToAttackPosition(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = kBS_MoveToAttackPosition;
    BodyController()->SetLocomotionType(pas::kLT_Lurk);
    JoinTeam(mgr);
    BodyController()->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
    BodyController()->CommandMgr().SetSteeringSpeedRange(1.f, 1.f);
    mMeleePursuitEnded = false;
    if (mIsMegaBlogg && !xbc5_27_ && !xbc5_26_ && !xbc5_30_) {
      x951_ = 0;
    }
    break;
  case kStateMsg_Update:
    mPathFindNavigation.PathFind(mgr, msg, dt, *this);
    if (IsAtAttackPosition() && !xbc5_26_ && !xbc5_30_) {
      const float random = mgr.Random()->Float();
      const CVector3f offset = mgr.GetPlayer(0)->GetTranslation() - GetTranslation();
      if (random < mUnknown_0x800a2b0d || offset.MagSquared() < mMinAttackRange * mMinAttackRange) {
        PathToAttackPosition(mgr, dt);
      }
    }
    if (HasCollisionTimeElapsed()) {
      PathToAttackPosition(mgr, dt);
      mCollisionTime = 0.f;
    }
    break;
  case kStateMsg_Deactivate:
    ReleaseHints(mgr);
    break;
  }
}

void CBlogg::FacePlayer(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    mState = kBS_FacePlayer;
    if (mIsMegaBlogg) {
      const uchar phaseIndex = GetHealthPhase();
      if (phaseIndex < mPhases.size()) {
        const SBloggPhaseData& phase = mPhases[phaseIndex];
        if (xbc5_29_) {
          xbc5_27_ = RollChance(mgr, phase.mUnknownB);
          xbc5_29_ = false;
        } else {
          xbc4_25_ = RollChance(mgr, phase.mUnknownA);
        }
      }
    } else {
      xbc4_25_ = mAggressiveness == 1.f ? true : mgr.Random()->Float() <= mAggressiveness;
    }
    mChargeOver = false;
    break;
  case kStateMsg_Update: {
    const CVector3f direction = GetDirectionToPlayer(mgr);
    BodyController()->CommandMgr().DeliverCmd(CBCLocomotionCmd(CVector3f::Zero(), direction, 1.f));
    CPlayer* player = GetPlayer(mgr);
    if (player != nullptr) {
      x97c_ = player->GetAimPosition(mgr, 0.f);
    }
    break;
  }
  case kStateMsg_Deactivate:
    break;
  }
}

void CBlogg::ChargeTelegraph(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->SetLocomotionType(pas::kLT_Combat);
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Zero, -1));
    mState = kBS_ChargeTelegraph;
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Generate)) {
      BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Zero, -1));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CBlogg::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    if (mTeamManagerId == kInvalidUniqueId ||
        CScriptTeamAiMgr::StartAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamManagerId,
                                      GetUniqueId())) {
      mAnimationState.SetState(CAnimationState::kAS_Ready);
      CPlayer* player = GetPlayer(mgr);
      if (player != nullptr) {
        x97c_ = player->GetAimPosition(mgr, 0.f);
      }
      BodyController()->CommandMgr().DeliverCmd(CBCProjectileAttackCmd(pas::kS_Zero, x97c_, false));
      BodyController()->CommandMgr().SetTargetVector(GetDirectionToPlayer(mgr));
      mState = kBS_ProjectileAttack;
      ++x951_;
    } else {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    }
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_ProjectileAttack)) {
      CPlayer* player = GetPlayer(mgr);
      if (player != nullptr) {
        x97c_ = player->GetAimPosition(mgr, 0.f);
      }
      BodyController()->CommandMgr().DeliverCmd(CBCProjectileAttackCmd(pas::kS_Zero, x97c_, false));
      BodyController()->CommandMgr().SetTargetVector(GetDirectionToPlayer(mgr));
    }
    break;
  case kStateMsg_Deactivate: {
    const float range = mMaxProjectileDelay - mMinProjectileDelay;
    mProjectileDelay = mMinProjectileDelay;
    if (mgr.IsRandomAvailable() == true) {
      mProjectileDelay += range * mgr.Random()->Float();
    }
    CScriptTeamAiMgr::EndAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamManagerId,
                                GetUniqueId(), false);
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mMeleePursuitEnded = false;
    xbc5_29_ = true;
    break;
  }
  }
}

void CBlogg::Stunned(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->SetLocomotionType(pas::kLT_Lurk);
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    BodyController()->CommandMgr().DeliverCmd(
        CBCGenerateCmd(pas::kGType_Five, CVector3f::Zero(), false, false));
    mState = kBS_Stunned;
    mCollisionTime = 0.f;
    if (IsIngPossessed()) {
      SendScriptMsgs(kSS_InternalState0, mgr);
    } else {
      SendScriptMsgs(kSS_Zero, mgr);
    }
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Generate)) {
      BodyController()->CommandMgr().DeliverCmd(
          CBCGenerateCmd(pas::kGType_Five, CVector3f::Zero(), false, false));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CBlogg::MoveToPlayer(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    mState = kBS_MoveToPlayer;
    BodyController()->SetLocomotionType(pas::kLT_Lurk);
    JoinTeam(mgr);
    BodyController()->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
    BodyController()->CommandMgr().SetSteeringSpeedRange(1.f, 1.f);
    CPlayer* player = GetPlayer(mgr);
    if (player != nullptr) {
      x97c_ = player->GetAimPosition(mgr, 0.f);
    }
    mPathFindNavigation.SetDestination(x97c_);
    mPathFindNavigation.PathFind(mgr, msg, dt, *this);
    BodyController()->CommandMgr().SetTargetVector(GetDirectionToPlayer(mgr));
    mBallPursuitTime = 0.f;
    mPlayerPursuitTime = 0.f;
    break;
  }
  case kStateMsg_Update: {
    CPlayer* player = GetPlayer(mgr);
    if (player != nullptr) {
      x97c_ = player->GetAimPosition(mgr, 0.f);
      mPathFindNavigation.SetDestination(x97c_);
      mPathFindNavigation.PathFind(mgr, kStateMsg_Activate, dt, *this);
      BodyController()->CommandMgr().SetTargetVector(GetDirectionToPlayer(mgr));
      const CPlayer::EPlayerMorphBallState state =
          player->GetSpawnedMorphballState() == CPlayer::kMS_Unmorphed
              ? player->GetMorphballTransitionState()
              : CPlayer::kMS_Unmorphed;
      if (state == CPlayer::kMS_Morphed) {
        mBallPursuitTime = CMath::Min(mMaxBallPursuitTime, mBallPursuitTime + dt);
      } else {
        mPlayerPursuitTime = CMath::Min(mMaxPlayerPursuitTime, mPlayerPursuitTime + dt);
      }
    }
    if (HasCollisionTimeElapsed()) {
      PathToAttackPosition(mgr, dt);
      mCollisionTime = 0.f;
    }
    break;
  }
  case kStateMsg_Deactivate:
    break;
  }
}

void CBlogg::GrabBall(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    if (mTeamManagerId == kInvalidUniqueId ||
        CScriptTeamAiMgr::StartAttack(CScriptTeamAiMgr::kAT_Melee, mgr, mTeamManagerId,
                                      GetUniqueId())) {
      mState = kBS_GrabBall;
      if (GetPlayer(mgr) != nullptr) {
        StopPlayer(mgr);
      }
      UpdateCollisionActorMaterials(mgr, CMaterialList(skGrabBallMaterial), kMA_Remove);
    } else {
      mAbortBallGrab = true;
    }
    break;
  case kStateMsg_Update: {
    StopPlayer(mgr);
    CPlayer* player = GetPlayer(mgr);
    if (player != nullptr) {
      mBallGrabTime = CMath::Min(mBallGrabTime + dt, 0.2f);
      const float t = CMath::Min(1.f, mBallGrabTime / 0.2f);
      const CTransform4f locator =
          GetScaledLocatorTransform(rstl::string_l(skBallAttachLocatorName));
      const CTransform4f xf = GetTransform() * locator;
      const CVector3f attach = xf.GetTranslation();
      const CVector3f aim = player->GetAimPosition(mgr, 0.f);
      const CVector3f playerOffset = aim - player->GetTranslation();
      const CVector3f blended = CVector3f::Lerp(aim - attach, CVector3f::Zero(), t);
      AnimationData()->AddAdditiveAnimation(mUnknown26Anim, 0.2f * t + (1.f - t), false, false);
      player->SetTranslation(attach + blended - playerOffset);
      if (t == 1.f) {
        mBallGrabbed = true;
      }
    }
    break;
  }
  case kStateMsg_Deactivate:
    AttachPlayerToMouth(mgr);
    x951_ = 0;
    mCanBite = false;
    mMeleeDelayTimer = 0.f;
    mAbortBallGrab = false;
    break;
  }
}

bool CBlogg::IsPlayerWithinChargeRange(CStateManager& mgr, CPlayer* player) const {
  const CVector3f offset = GetTranslation() - player->GetAimPosition(mgr, 0.f);
  const float health = GetHealthInfo()->GetHP();
  const float fraction = health / GetHealthInfo()->GetInitialHP();
  const float range = mCurrentAttackRange *
                      (mUnknown_0x689a803f * fraction + (1.f - fraction) * mUnknown_0x479ccc37);
  return offset.MagSquared() < range * range;
}

void CBlogg::Taunt(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    BodyController()->CommandMgr().DeliverCmd(
        CBCGenerateCmd(pas::kGType_Two, CVector3f::Zero(), false, false));
    mState = kBS_Taunt;
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Generate)) {
      BodyController()->CommandMgr().DeliverCmd(
          CBCGenerateCmd(pas::kGType_Two, CVector3f::Zero(), false, false));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mTauntReady = false;
    break;
  }
}

void CBlogg::ChargeAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    if (mTeamManagerId == kInvalidUniqueId ||
        CScriptTeamAiMgr::StartAttack(CScriptTeamAiMgr::kAT_Melee, mgr, mTeamManagerId,
                                      GetUniqueId())) {
      mState = kBS_ChargeAttack;
      BodyController()->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
      BodyController()->CommandMgr().SetSteeringSpeedRange(1.f, 1.f);
      CPlayer* player = GetPlayer(mgr);
      if (player != nullptr) {
        x97c_ = player->GetAimPosition(mgr, 0.f);
      }
      mPathFindNavigation.SetDestination(x97c_);
      mPathFindNavigation.PathFind(mgr, msg, dt, *this);
      BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_MaintainVelocity));
      mChargeOver = false;
      if (mIsMegaBlogg) {
        const uchar phase = GetHealthPhase();
        if (phase < mPhases.size()) {
          const float chance = mPhases[phase].mUnknownC;
          xbc5_28_ = xbc5_29_ || mgr.Random()->Float() <= chance;
        }
      }
      if (mIsMegaBlogg) {
        ++xbb1_;
      }
      mLineOfSightTracker.SetTarget(mgr.GetPlayer(0)->GetUniqueId());
    } else {
      mChargeOver = true;
    }
    break;
  case kStateMsg_Update: {
    CPlayer* statePlayer = mgr.GetPlayer(0);
    const CVector3f aim = statePlayer->GetAimPosition(mgr, 0.f);
    const bool lostSight = !mLineOfSightTracker.HasLineOfSight();
    const bool inRange = IsPlayerWithinChargeRange(mgr, statePlayer);
    if (lostSight) {
      CPlayer* player = GetPlayer(mgr);
      if (player != nullptr && !inRange) {
        x97c_ = player->GetAimPosition(mgr, 0.f);
        mPathFindNavigation.SetDestination(x97c_);
        mPathFindNavigation.PathFind(mgr, kStateMsg_Activate, dt, *this);
      } else {
        mPathFindNavigation.PathFind(mgr, msg, dt, *this);
      }
      if (GetSearchPath()->OnPath(GetTranslation()) != CPathFindSearch::kR_Success) {
        mChargeOver = true;
      }
    } else {
      if (!inRange) {
        x97c_ = aim;
        mTargetForward = BodyController()->CommandMgr().GetPreviousMoveVector();
      }
      const CVector3f toTarget = x97c_ - GetTranslation();
      const CTransform4f& xf = GetTransform();
      if (toTarget.GetZ() * xf.Get21() +
              (toTarget.GetX() * xf.Get01() + toTarget.GetY() * xf.Get11()) >
          0.f) {
        BodyController()->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(toTarget, CVector3f::Zero(), 1.f));
      } else {
        BodyController()->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(mTargetForward, CVector3f::Zero(), 1.f));
      }
    }
    BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_MaintainVelocity));
    break;
  }
  case kStateMsg_Deactivate:
    CScriptTeamAiMgr::EndAttack(CScriptTeamAiMgr::kAT_Melee, mgr, mTeamManagerId, GetUniqueId(),
                                false);
    xbc4_25_ = false;
    xbc5_28_ = true;
    x951_ = 0;
    mCanBite = false;
    mMeleeDelayTimer = 0.f;
    mMeleePursuitEnded = false;
    xbc5_26_ = false;
    break;
  }
}

void CBlogg::Thrash(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    BodyController()->CommandMgr().DeliverCmd(
        CBCGenerateCmd(pas::kGType_Seven, CVector3f::Zero(), false, false));
    mState = kBS_Thrash;
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Generate)) {
      BodyController()->CommandMgr().DeliverCmd(
          CBCGenerateCmd(pas::kGType_Seven, CVector3f::Zero(), false, false));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CBlogg::SpitBall(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    BodyController()->CommandMgr().DeliverCmd(
        CBCGenerateCmd(pas::kGType_Three, CVector3f::Zero(), false, false));
    mState = kBS_SpitBall;
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Generate)) {
      BodyController()->CommandMgr().DeliverCmd(
          CBCGenerateCmd(pas::kGType_Three, CVector3f::Zero(), false, false));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    CScriptTeamAiMgr::EndAttack(CScriptTeamAiMgr::kAT_Melee, mgr, mTeamManagerId, GetUniqueId(),
                                false);
    UpdateCollisionActorMaterials(mgr, CMaterialList(skSpitBallMaterial), kMA_Add);
    break;
  }
}

void CBlogg::Dead(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = kBS_Dead;
    BodyController()->SetLocomotionType(pas::kLT_Internal9);
    break;
  case kStateMsg_Update:
    if (GetFluidCount() == 0 && !mFadeToDeath) {
      mFadeToDeath = true;
      mAlphaDelta = -1.f / GetFadeOnDeathTime();
      RemoveMaterial(kMT_Character, kMT_Solid, kMT_Target, kMT_Orbit, mgr);
      AddMaterial(kMT_ProjectilePassthrough, mgr);
      mState = kBS_Dying;
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CBlogg::MeleeAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = kBS_MeleeAttack;
    if (mTeamManagerId == kInvalidUniqueId ||
        CScriptTeamAiMgr::StartAttack(CScriptTeamAiMgr::kAT_Melee, mgr, mTeamManagerId,
                                      GetUniqueId())) {
      mAnimationState.SetState(CAnimationState::kAS_Ready);
      BodyController()->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::kS_Zero));
    } else {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    }
    break;
  case kStateMsg_Update:
    mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_MeleeAttack);
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    CScriptTeamAiMgr::EndAttack(CScriptTeamAiMgr::kAT_Melee, mgr, mTeamManagerId, GetUniqueId(),
                                false);
    mCanBite = false;
    mMeleeDelayTimer = 0.f;
    break;
  }
}

void CBlogg::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    mState = kBS_Patrol;
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    CBodyController* controller = BodyController();
    const float runSpeed = controller->GetBodyStateInfo().GetLocomotionSpeed(pas::kLA_Run);
    const float walkSpeed = controller->GetBodyStateInfo().GetLocomotionSpeed(pas::kLA_Walk);
    const float ratio = walkSpeed / runSpeed;
    BodyController()->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
    BodyController()->CommandMgr().SetSteeringSpeedRange(ratio, ratio);
    LeaveTeam(mgr);
    CPatterned::Patrol(mgr, msg, dt);
    const TUniqueId waypointId = mWaypointNavigation.GetDestination();
    if (waypointId != kInvalidUniqueId) {
      const CScriptWaypoint* waypoint =
          TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(waypointId));
      if (waypoint != nullptr) {
        mPathFindNavigation.SetDestination(waypoint->GetTranslation());
        mPathFindNavigation.PathFind(mgr, msg, dt, *this);
      }
    }
    break;
  }
  case kStateMsg_Update: {
    mLocomotionChangeTimer += dt;
    if (mLocomotionChangeTimer > mLocomotionChangeInterval) {
      mLocomotionChangeTimer = 0.f;
      const float random = mgr.Random()->Float();
      if (random < 0.3f && BodyController()->GetLocomotionType() != pas::kLT_Internal4 &&
          BodyController()->GetLocomotionType() != pas::kLT_Internal6) {
        BodyController()->SetLocomotionType(pas::kLT_Internal4);
        BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_MaintainVelocity));
        mLocomotionChangeInterval = 1.f + mgr.Random()->Float();
      } else if (random < 0.6f && BodyController()->GetLocomotionType() != pas::kLT_Internal4 &&
                 BodyController()->GetLocomotionType() != pas::kLT_Internal6) {
        BodyController()->SetLocomotionType(pas::kLT_Internal6);
        BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_MaintainVelocity));
        mLocomotionChangeInterval = 1.f + mgr.Random()->Float();
      } else {
        BodyController()->SetLocomotionType(pas::kLT_Relaxed);
        BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_MaintainVelocity));
        mLocomotionChangeInterval = 5.f * mgr.Random()->Float() + 5.f;
      }
    }
    const CPathFindSearch* search = GetSearchPath();
    if (search->GetCurrentWaypoint() >= static_cast< int >(search->GetWaypoints().size()) - 1) {
      CPatterned::Patrol(mgr, msg, dt);
    } else {
      mPathFindNavigation.PathFind(mgr, msg, dt, *this);
    }
    break;
  }
  case kStateMsg_Deactivate:
    CPatterned::Patrol(mgr, msg, dt);
    break;
  }
}

void CBlogg::FindAttackPositions(CStateManager& mgr, const CVector3f& playerPosition,
                                 const CVector3f& bloggPosition, const CVector3f& direction,
                                 float distance) {
  TUniqueId waterId = kInvalidUniqueId;
  CAABox waterBounds = CAABox::MakeNullBox();
  FindFluid(mgr, waterBounds, waterId);
  CVector3f position = playerPosition + direction * distance;
  if (mPathFindSearch.OnPath(position) == CPathFindSearch::kR_Success) {
    if (waterId == kInvalidUniqueId || waterBounds.PointInside(position + skWaterTestOffset)) {
      mPositionHistory.push_back(position);
    }
  }
  float angle = skAttackAngleStep;
  const float angleStep = angle;
  for (; angle < M_2PIF; angle += angleStep) {
    const CVector3f rotated = CMatrix3f::RotateZ(CRelAngle::FromRadians(angle)) * direction;
    position = playerPosition + rotated * distance;
    if (mPathFindSearch.OnPath(position) == CPathFindSearch::kR_Success) {
      if (waterId == kInvalidUniqueId || waterBounds.PointInside(position + skWaterTestOffset)) {
        mPositionHistory.push_back(position);
      }
    }
  }

  rstl::vector< CTeamAiRole > roles;
  const CScriptTeamAiMgr* team =
      TCastToConstPtr< CScriptTeamAiMgr >(mgr.GetObjectById(mTeamManagerId));
  if (team != nullptr) {
    roles = team->GetRoles();
  }
  const uint positionCount = mPositionHistory.size();
  if (positionCount != 0u) {
    float closestDistanceSquared = 1000000.f;
    for (uint i = 0; i < positionCount; ++i) {
      const CVector3f& candidate = mPositionHistory[i];
      bool blocked = false;
      uint roleIndex;
      const uint roleCount = roles.size();
      for (roleIndex = 0; roleIndex < roleCount; ++roleIndex) {
        const TUniqueId memberId = roles[roleIndex].GetOwnerId();
        const CBlogg* other = TCastToConstPtr< CBlogg >(mgr.GetObjectById(memberId));
        if (other != nullptr && memberId != GetUniqueId()) {
          if (!(other->mAttackPosition == CVector3f::Zero())) {
            const CVector3f offset = candidate - other->mAttackPosition;
            if (offset.MagSquared() < 25.f) {
              blocked = true;
              break;
            }
          }
        }
      }
      if (!blocked) {
        const CVector3f offset = candidate - bloggPosition;
        const float distanceSquared = offset.MagSquared();
        if (distanceSquared < closestDistanceSquared) {
          mAttackPosition = candidate;
          closestDistanceSquared = distanceSquared;
          mHintId = kInvalidUniqueId;
          xb18_ = roleIndex;
          bool flag;
          if (mgr.Random()->Float() < 0.5f) {
            flag = false;
          } else {
            flag = true;
          }
          xb14_ = flag;
        }
      }
    }
  }
}

void CBlogg::ComputeAttackPositions(CStateManager& mgr, float dt) {
  if (mHintIds.size() != 0u) {
    const TUniqueId hintId = FindNearestHint(mgr, GetTranslation(), false);
    if (hintId != kInvalidUniqueId) {
      CEntity* hint = mgr.ObjectById(hintId);
      if (hint != nullptr) {
        ReleaseHints(mgr);
        static_cast< CScriptAIHint* >(hint)->SetInUse(true);
        mAttackPosition = static_cast< CActor* >(hint)->GetTranslation();
        mHintId = hintId;
        mPathFindNavigation.SetDestination(static_cast< CActor* >(hint)->GetTranslation());
        mPathFindNavigation.PathFind(mgr, kStateMsg_Activate, dt, *this);
      }
    }
  } else {
    CPlayer* player = GetPlayer(mgr);
    if (player != nullptr) {
      mCurrentAttackAngle =
          CMath::Max(mMaxAttackAngle - mMinAttackAngle, 0.f) * mgr.Random()->Float() +
          mMinAttackAngle;
      mCurrentAttackRange =
          CMath::Max(mMaxAttackRange - mMinAttackRange, 0.f) * mgr.Random()->Float() +
          mMinAttackRange;
      const CVector3f playerPosition = player->GetAimPosition(mgr, 0.f);
      const CVector3f bloggPosition = GetTranslation();
      const CVector3f unit = (bloggPosition - playerPosition).AsNormalized();
      const CVector3f toBlogg = unit;
      CVector3f flat = unit;
      flat.SetZ(playerPosition.GetZ());
      flat.Normalize();
      const float flatAngle = CVector3f::GetAngleDiff(toBlogg, flat);
      CVector3f directionA = flat;
      const float headingAngle = CVector3f::GetAngleDiff(flat, CVector3f::Forward());
      const CVector3f rotated =
          CMatrix3f::RotateZ(CRelAngle::FromRadians(headingAngle)) *
          (CMatrix3f::RotateX(CRelAngle::FromRadians(mCurrentAttackAngle)) * CVector3f::Forward());
      CVector3f directionB = flat;
      if (flatAngle >= 0.f) {
        directionA = rotated;
      } else if (flatAngle > mCurrentAttackAngle) {
        directionB = rotated;
      }
      mPositionHistory.clear();
      mAttackPosition = CVector3f::Zero();
      float distance = mCurrentAttackRange;
      while (mPositionHistory.size() == 0 && distance > 0.f) {
        FindAttackPositions(mgr, playerPosition, bloggPosition, directionA, distance);
        distance = CMath::Max(distance - 5.f, 0.f);
      }
      if (distance <= 0.f) {
        distance = mCurrentAttackRange;
        while (mPositionHistory.size() == 0 && distance > 0.f) {
          FindAttackPositions(mgr, playerPosition, bloggPosition, directionB, distance);
          distance = CMath::Max(distance - 5.f, 0.f);
        }
      }
      if (distance <= 0.f) {
        distance = mCurrentAttackRange;
        const CVector3f mirrored =
            CMatrix3f::RotateZ(CRelAngle::FromRadians(headingAngle)) *
            (CMatrix3f::RotateX(CRelAngle::FromRadians(-mCurrentAttackAngle)) *
             CVector3f::Forward());
        while (mPositionHistory.size() == 0 && distance > 0.f) {
          FindAttackPositions(mgr, playerPosition, bloggPosition, mirrored, distance);
          distance = CMath::Max(distance - 5.f, 0.f);
        }
      }
      if (distance > 0.f) {
        mPathFindNavigation.SetDestination(mAttackPosition);
        mPathFindNavigation.PathFind(mgr, kStateMsg_Activate, dt, *this);
      }
    }
  }
}

void CBlogg::UpdateAimWeights() {
  const CTransform4f& xf = GetTransform();
  const CTransform4f inverse(xf.GetQuickInverse());
  const CVector3f local = inverse.BuildMatrix3f() * mLastForward;
  const float step = M_PIF * (BodyController()->GetTurnSpeed() / 60.f / 180.f);
  const float up = CVector3f::Dot(local, CVector3f::Up());
  const float left = CVector3f::Dot(local, CVector3f::Left());
  const float threshold = 0.75f * step;
  if (up >= 0.f) {
    if (up > threshold) {
      mAimWeightDown = CMath::Min(mAimWeightDown + CMath::Min(up / (M_PIF / 2.f), step), 1.f);
    } else {
      mAimWeightDown = CMath::Max(mAimWeightDown - step, 0.f);
    }
    mAimWeightUp = CMath::Max(mAimWeightUp - step, 0.f);
  } else {
    const float absUp = CMath::AbsF(up);
    if (absUp > threshold) {
      mAimWeightUp = CMath::Min(mAimWeightUp + CMath::Min(absUp / (M_PIF / 2.f), step), 1.f);
    } else {
      mAimWeightUp = CMath::Max(mAimWeightUp - step, 0.f);
    }
    mAimWeightDown = CMath::Max(mAimWeightDown - step, 0.f);
  }
  if (left >= 0.f) {
    if (left > threshold) {
      mAimWeightLeft = CMath::Min(mAimWeightLeft + CMath::Min(left / (M_PIF / 2.f), step), 1.f);
    } else {
      mAimWeightLeft = CMath::Max(mAimWeightLeft - step, 0.f);
    }
    mAimWeightRight = CMath::Max(mAimWeightRight - step, 0.f);
  } else {
    const float absLeft = CMath::AbsF(left);
    if (absLeft > threshold) {
      mAimWeightRight =
          CMath::Min(mAimWeightRight + CMath::Min(absLeft / (M_PIF / 2.f), step), 1.f);
    } else {
      mAimWeightRight = CMath::Max(mAimWeightRight - step, 0.f);
    }
    mAimWeightLeft = CMath::Max(mAimWeightLeft - step, 0.f);
  }
  mLastForward = xf.GetForward();
}

void CBlogg::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  if (mTeamManagerId != kInvalidUniqueId) {
    TCastToConstPtr< CScriptTeamAiMgr >(mgr.GetObjectById(mTeamManagerId));
  }
  if (mState == kBS_ChargeAttack || mState == kBS_MoveToValidPosition) {
    mLineOfSightTracker.Update(dt, mgr);
  }
  if (mState == kBS_MoveToAttackPosition || mState == kBS_MoveToPlayer ||
      mState == kBS_FacePlayer) {
    BodyController()->SetTurnSpeed(3.f * mBaseTurnSpeed);
  } else if (mState == kBS_ChargeAttack) {
    BodyController()->SetTurnSpeed(mChargeTurnSpeed);
  } else {
    BodyController()->SetTurnSpeed(mBaseTurnSpeed);
  }
  if (mState == kBS_ChargeAttack) {
    mSpeed = mBaseSpeed * mChargeSpeedMultiplier;
  } else if (mState == kBS_ProjectileAttack) {
    mSpeed = mBaseSpeed;
  } else if (mState == kBS_Dying) {
    mSpeed = 0.f;
  } else {
    mSpeed = mBaseSpeed;
  }
  CPatterned::Think(dt, mgr);
  mCollisionActorManager->Update(dt, mgr, CCollisionActorManager::kUO_WorldSpace);
  UpdateAimWeights();
  AnimationData()->AddAdditiveAnimation(mAimAnimUp, mAimWeightUp, false, false);
  AnimationData()->AddAdditiveAnimation(mAimAnimDown, mAimWeightDown, false, false);
  AnimationData()->AddAdditiveAnimation(mAimAnimLeft, mAimWeightLeft, false, false);
  AnimationData()->AddAdditiveAnimation(mAimAnimRight, mAimWeightRight, false, false);
  CPlayer* player = GetPlayer(mgr);
  if (player != nullptr) {
    const bool inRange = IsPlayerWithinChargeRange(mgr, player);
    if (mState == kBS_ChargeAttack && inRange && mMouthClosed != 0 && xbc5_28_) {
      AnimationData()->AddAdditiveAnimation(mUnknown26Anim, 1.f, false, false);
      mMouthClosed = 0;
      const ushort mouthOpenSound = mMouthOpenSound;
      ProcessSoundEvent(mouthOpenSound, 1.f, 0, 0.1f, 1000.f, CSegId(0), 0, 0, 0.f, 20, 127,
                        GetDistanceToCamera(mgr), GetTranslation(), mgr.GetNextAreaId().Value(),
                        mgr, true);
    } else if (mState == kBS_MoveToPlayer &&
               (player->GetSpawnedMorphballState() == CPlayer::kMS_Unmorphed
                    ? player->GetMorphballTransitionState()
                    : CPlayer::kMS_Unmorphed) == CPlayer::kMS_Morphed) {
      AnimationData()->AddAdditiveAnimation(mUnknown26Anim, 1.f, false, false);
      mMouthClosed = 0;
    } else if (mState == kBS_GrabBall || mState == kBS_Thrash) {
      mMouthClosed = 0;
    } else if (mMouthClosed == 0 && mState != kBS_ChargeAttack) {
      AnimationData()->DelAdditiveAnimation(mUnknown26Anim);
      mMouthClosed = 1;
    }
  }
  if (mBallGrabbed) {
    AttachPlayerToMouth(mgr);
  }
  if (!mCanBite) {
    mMeleeDelayTimer += dt;
    if (mMeleeDelayTimer >= mMinDelayBetweenMeleeAttacks) {
      mCanBite = true;
    }
  }
  xb68_ = CMath::Max(xb68_ - dt, 0.f);
  if (xb68_ > 0.f) {
    const float t = xb68_ / skDamageHitTime;
    const CColor& color = CColor::Lerp(CColor::Black(), skHitsWithoutDamageColor, t);
    const uchar blue = color.GetBlueu8();
    const uchar green = color.GetGreenu8();
    mColor.SetRed(color.GetRedu8());
    mColor.SetGreen(green);
    mColor.SetBlue(blue);
  }
  if (mIsMegaBlogg) {
    const uchar healthPhase = GetHealthPhase();
    if (healthPhase != xbb2_) {
      ChoosePhaseValue(mgr);
    }
    if (!xbc5_30_) {
      if (mPhaseValue != 0 && xbb1_ >= mPhaseValue) {
        xbc5_30_ = true;
        xbb1_ = 0;
      }
    } else if (xbb1_ >= 3) {
      xbc5_30_ = false;
      const uchar newPhase = GetHealthPhase();
      if (newPhase < mPhases.size()) {
        ChoosePhaseValue(mgr);
        xbb1_ = 0;
      }
    }
  }
}

void CBlogg::LaunchBloggProjectile(const CTransform4f& xf, CStateManager& mgr, int maxProjectiles,
                                   uint attributes, bool homing,
                                   const CImpactVisorEffect& visorEffect, const CVector3f& scale) {
  if (ProjectileInfo()->Token().TryCache()) {
    if (mgr.CanCreateProjectile(GetUniqueId(), kWT_AI, maxProjectiles)) {
      CBloggProjectile* projectile = rs_new CBloggProjectile(
          true, ProjectileInfo()->Token(), kWT_AI, xf, kMT_Character, ProjectileInfo()->GetDamage(),
          mgr.AllocateUniqueId(), GetCurrentAreaId(), GetUniqueId(),
          homing ? mgr.GetPlayer(0)->GetUniqueId() : kInvalidUniqueId, attributes, true, scale,
          visorEffect, false, true, mProjectileScale);
      if (projectile != nullptr) {
        projectile->SetDamageDuration(mProjectileBlurTime);
        mgr.AddObject(projectile);
      }
    }
  }
}

void CBlogg::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                             float dt) {
  switch (type) {
  case kUE_Projectile: {
    const CTransform4f xf =
        GetTransform() * GetScaledLocatorTransform(rstl::string_l("mouth_LCTR"));
    const CImpactVisorEffect visorEffect = CImpactVisorEffect::BlurEffect(
        CImpactVisorEffect::SBlurEffect(1, mProjectileBlurRadius, mProjectileBlurTime));
    const CVector3f scale(1.f, 1.f, 1.f);
    CPlayer* player = GetPlayer(mgr);
    if (player != nullptr) {
      const CVector3f position = xf.GetTranslation();
      const CVector3f aim = player->GetAimPosition(mgr, 0.f);
      const CVector3f offset = position - aim;
      const float travelTime = offset.Magnitude() / ProjectileInfo()->GetProjectileSpeed();
      const CVector3f predicted = aim + 1.f * (travelTime * player->GetDampedClampedVelocityWR());
      LaunchBloggProjectile(CTransform4f::LookAt(position, predicted), mgr, 10, 0, false,
                            visorEffect, scale);
    } else {
      LaunchBloggProjectile(CTransform4f::LookAt(xf.GetTranslation(), x97c_), mgr, 10, 0, false,
                            visorEffect, scale);
    }
    break;
  }
  case kUE_ObjectDrop: {
    CPlayer* player = GetPlayer(mgr);
    if (player != nullptr) {
      const CTransform4f locator =
          GetScaledLocatorTransform(rstl::string_l(skBallAttachLocatorName));
      const CTransform4f xf = GetTransform() * locator;
      const CVector3f forward = xf.GetForward();
      player->Stop();
      player->SetVelocityWR(CVector3f::Zero());
      const CVector3f direction = forward.AsNormalized();
      player->ApplyImpulseWR(direction * player->GetMass() * 30.f, CAxisAngle::Identity());
      player->SetMoveState(NPlayer::kMS_ApplyJump, mgr);
      player->EnableLeaveMorphBall(true);
      CDamageInfo damage(mContactDamage);
      damage.SetDamage(mBallSpitDamage);
      mgr.ApplyDamage(
          GetUniqueId(), player->GetUniqueId(), GetUniqueId(), damage,
          CMaterialFilter::MakeIncludeExclude(CMaterialList(skContactDamageSolid), CMaterialList()),
          CVector3f::Zero());
      player->GetMorphBall()->SetAsProjectile(true);
    }
    mBallGrabbed = false;
  }
  default:
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
    break;
  }
}

void CBlogg::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const bool wasActive = GetActive();
  const TUniqueId senderId = msg.GetSenderId();
  switch (msg.GetMessage()) {
  case kSM_AreaLoaded: {
    rstl::vector< SConnection >::const_iterator it = GetConnectionList().begin();
    for (; it != GetConnectionList().end(); ++it) {
      mgr.GetIdForScript(it->objId);
    }
    mPathFindSearch.SetArea(
        mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()).GetPostConstructed()->mPathArea);
    mPlayerId = mgr.GetPlayer(0)->GetUniqueId();
    CollectHints(mgr);
    break;
  }
  case kSM_Create: {
    if (!BodyController()->GetIsActive()) {
      BodyController()->SetLocomotionType(pas::kLT_Relaxed);
      BodyController()->Activate(mgr, pas::kAS_Invalid);
    }
    {
      rstl::vector< CJointCollisionDescription > joints;
      if (HasAnimation() && GetAnimationData()->GetSpatialPrimitive()) {
        const CSpatialPrimitive* primitive = **GetAnimationData()->GetSpatialPrimitive();
        const rstl::vector< CSpatialPrimitive::SSphere >& spheres = primitive->GetSpheres();
        const uint sphereCount = spheres.size();
        joints.reserve(sphereCount);
        for (uint i = 0; i < sphereCount; ++i) {
          const CSpatialPrimitive::SSphere& sphere = spheres[i];
          const CSegId segId = sphere.mFirstSegment;
          const CSphere& bounds = sphere.mSphere;
          const CJointCollisionDescription desc = CJointCollisionDescription::SphereCollision(
              segId, bounds.GetCenter(), bounds.GetRadius(),
              rstl::string_l("sphere") + CStringExtras::CreateFromInteger(i), 0.001f);
          joints.push_back_unsafe(desc);
        }

        mCollisionActorManager = rs_new CCollisionActorManager(
            mgr, GetUniqueId(), GetCurrentAreaId(), joints, GetActive());
        mCollisionActorManager->AddMaterialList(
            mgr, CMaterialList(kMT_CameraPassthrough, kMT_Immovable));
        const CSegId pivotId = GetAnimationData()->GetCharLayoutInfo()->GetSegIdFromString(
            rstl::string_l(skPivotLocatorName));
        for (uint i = 0; i < mCollisionActorManager->GetNumCollisionActors(); ++i) {
          const CJointCollisionDescription& desc =
              mCollisionActorManager->GetCollisionDescFromIndex(i);
          const TUniqueId id = desc.GetCollisionActorId();
          if (CCollisionActor* colAct = static_cast< CCollisionActor* >(mgr.ObjectById(id))) {
            colAct->AddMaterial(spheres[i].x8_);
            colAct->MaterialList().Add(kMT_Player);
            colAct->MaterialList().Add(kMT_AIPassthrough);
            colAct->MaterialList().Remove(kMT_Orbit);
            colAct->MaterialList().Remove(kMT_Target);
            const u64 ownInclude = GetMaterialFilter().GetIncludeList().GetValue();
            const u64 ownExclude = GetMaterialFilter().GetExcludeList().GetValue();
            const u64 actorInclude = colAct->GetMaterialFilter().GetIncludeList().GetValue();
            const u64 actorExclude = colAct->GetMaterialFilter().GetExcludeList().GetValue();
            colAct->SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
                CMaterialList(ownInclude | actorInclude),
                CMaterialList(ownExclude | (u64(1) << kMT_Character) | actorExclude)));
            const CHealthInfo health = *GetHealthInfo();
            colAct->SetDamageVulnerability(*CPatterned::GetDamageVulnerability());
            if (desc.GetPivotId() == pivotId) {
              xac4_ = id;
              colAct->SetNonUniformVulnerability(mMouthVulnerability);
            } else {
              colAct->SetNonUniformVulnerability(mBodyVulnerability);
            }
            *colAct->HealthInfo() = health;
          }
        }
      }
    }
    AddMaterial(kMT_ProjectilePassthrough, mgr);
    if (mIsMegaBlogg && mPhases.size() > 0) {
      ChoosePhaseValue(mgr);
    }
    mLineOfSightTracker.SetTarget(mgr.GetPlayer(0)->GetUniqueId());
    break;
  }
  case kSM_AIUpdateDisabled:
    if (mCollisionActorManager.get() != nullptr) {
      mCollisionActorManager->SetPhysicsActive(mgr, false);
    }
    break;
  case kSM_Delete:
    mCollisionActorManager->Destroy(mgr);
    LeaveTeam(mgr);
    // Fallthrough
  case kSM_Deactivate:
    mCollisionActorManager->SetActive(mgr, false);
    LeaveTeam(mgr);
    break;
  case kSM_Alert:
    xbc4_24_ = true;
    // Fallthrough
  case kSM_HitObject: {
    if (CCollisionActor* colAct = TCastToPtr< CCollisionActor >(mgr.ObjectById(senderId))) {
      if (CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(colAct->GetLastTouchedObject()))) {
        CDamageInfo damage(mContactDamage);
        if (senderId == xac4_) {
          if (mState == kBS_ChargeAttack) {
            damage.SetDamage(mChargeDamage);
            xbc4_25_ = true;
          } else if (mState == kBS_MeleeAttack) {
            damage.SetDamage(mBiteDamage);
          }
        }
        ApplyContactDamage(mgr, *player, damage);
      }
    }
    break;
  }
  case kSM_Damage: {
    if (CCollisionActor* colAct = TCastToPtr< CCollisionActor >(mgr.ObjectById(senderId))) {
      const TUniqueId touchedId = colAct->GetLastTouchedObject();
      CHealthInfo* colHealth = colAct->HealthInfo();
      const float initialHP = HealthInfo()->GetInitialHP();
      if (const CWeapon* weapon = TCastToConstPtr< CWeapon >(mgr.GetObjectById(touchedId))) {
        const CVector3f position = weapon->GetTransform().GetForward();
        if (senderId == xac4_ && IsHitInMouthDirection(position) && mMouthClosed == 0) {
          ApplyCollisionActorDamage(mgr, senderId, mMouthDamageMultiplier);
          mHitByPlayerProjectile = true;
          mDamageCooldownTimer = skDamageHitTime;
          xbc4_24_ = true;
        } else {
          const CVector3f forward = GetTransform().GetForward();
          if (CVector3f::Dot(position.AsNormalized(), forward) > 0.f) {
            ApplyCollisionActorDamage(mgr, senderId, mBodyDamageMultiplier);
            mHitByPlayerProjectile = true;
            xbc4_24_ = true;
          }
        }
      } else if (mMouthClosed == 0) {
        const CBomb* bomb = TCastToConstPtr< CBomb >(mgr.GetObjectById(touchedId));
        const CPowerBomb* powerBomb = TCastToConstPtr< CPowerBomb >(mgr.GetObjectById(touchedId));
        if (bomb != nullptr || powerBomb != nullptr) {
          ApplyCollisionActorDamage(mgr, senderId, mMouthDamageMultiplier);
          mHitByPlayerProjectile = true;
          mDamageCooldownTimer = skDamageHitTime;
          xbc4_24_ = true;
        }
      }
      colHealth->SetHP(initialHP);
    }
    break;
  }
  case kSM_ResistedDamage: {
    if (CCollisionActor* colAct = TCastToPtr< CCollisionActor >(mgr.ObjectById(senderId))) {
      if (const CWeapon* weapon =
              TCastToConstPtr< CWeapon >(mgr.GetObjectById(colAct->GetLastTouchedObject()))) {
        const CDamageInfo& weaponDamage = weapon->GetCurrentDamageInfo();
        const CDamageVulnerability* vulnerability =
            colAct->GetDamageVulnerability(CVector3f::Zero(), CVector3f::Forward(), weaponDamage);
        if (weaponDamage.GetVulnerableDamage(*vulnerability) > 0.f) {
          xb68_ = skDamageHitTime;
          xbc5_26_ = true;
          BodyController()->CommandMgr().DeliverCmd(CBCAdditiveFlinchCmd(1.f));
        }
      }
    }
    break;
  }
  case kSM_InternalMessage0:
    HealthInfo()->SetHP(-1.f);
    Death(mgr, GetTransform().GetForward(), kSS_InvalidState);
    break;
  case kSM_InternalMessage1:
    xbc4_24_ = false;
    break;
  case kSM_Decrement:
  case kSM_Increment:
    break;
  }

  CPatterned::AcceptScriptMsg(mgr, msg);
  if (wasActive != GetActive() && mCollisionActorManager.get() != nullptr) {
    mCollisionActorManager->SetActive(mgr, GetActive());
  }
}

CEntity* LoadBlogg(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrBlogg sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrBlogg.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  return rs_new CBlogg(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      *modelData, LdrToPatternedInfo(sldrThis.patterned, &sldrThis.ingPossessionData),
      LdrToActorParameters(sldrThis.actorInformation), sldrThis.minAttackAngle,
      sldrThis.maxAttackAngle, sldrThis.minDelayBetweenProjectileAttacks,
      sldrThis.maxDelayBetweenProjectileAttacks, sldrThis.unknown_0xa19d5f62,
      sldrThis.projectileParticleEffect, LdrToDamageInfo(sldrThis.projectileDamage),
      LdrToDamageVulnerability(sldrThis.armorVulnerability), sldrThis.bodyDamageMultiplier,
      sldrThis.mouthDamageMultiplier, sldrThis.mouthDamageAngle, sldrThis.chargeDamageRadius,
      sldrThis.chargeDamage, sldrThis.biteDamage, sldrThis.ballSpitDamage,
      sldrThis.fishAttractionRadius, sldrThis.fishAttractionPriority, sldrThis.aggressiveness,
      sldrThis.unknown_0x479ccc37, sldrThis.unknown_0x689a803f, sldrThis.unknown_0x800a2b0d,
      sldrThis.chargeTurnSpeed, sldrThis.chargeSpeedMultiplier, sldrThis.maxMeleeRange,
      sldrThis.maxBallDetectionRange, sldrThis.maxPlayerPursuitTime, sldrThis.maxBallPursuitTime,
      sldrThis.mouthOpenSound, sldrThis.minDelayBetweenMeleeAttacks, sldrThis.maxCollisionTime,
      sldrThis.isMegaBlogg, sldrThis.projectileBlurRadius, sldrThis.projectileBlurTime,
      LdrToDamageVulnerability(sldrThis.ingPossessedArmorVulnerability),
      SBloggPhaseData(sldrThis.bloggStruct), SBloggPhaseData(sldrThis.bloggStruct_0x97dd1aa7),
      SBloggPhaseData(sldrThis.bloggStruct_0xf2ba21e1));
}

#ifndef MONOLITHIC
static void SetFuncPtrs() {
  static SBlogg_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &LoadBlogg;
  SetSBlogg_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSBlogg_FuncPtrs(nullptr); }
#endif
