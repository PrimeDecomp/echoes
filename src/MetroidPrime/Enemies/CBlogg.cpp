#include "MetroidPrime/Enemies/CBlogg.hpp"

#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CKnockBackInfo.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Enemies/CPatternedInfo.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrBlogg.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CFishCloud.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTeamAiMgr.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CBomb.hpp"
#include "MetroidPrime/Weapons/CPowerBomb.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "REL/REL_Setup.h"

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

static CPatterned::StateMachine::SCodeFunction skCodeFuncs[] = {
    {"ComputeTauntProbability",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CBlogg::ComputeTauntProbability)},
    {"EndMeleePursuit",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CBlogg::EndMeleePursuit)},
};

static const char* const skMouthLocatorName = "mouth_LCTR";            // Guessed name
static const char* const skBallAttachLocatorName = "ball_attach_LCTR"; // Guessed name

static EMaterialTypes skHintRayInclude = kMT_Solid;                       // Guessed name
static EMaterialTypes skHintRayExclude0 = kMT_Character;                  // Guessed name
static EMaterialTypes skHintRayExclude1 = kMT_Player;                     // Guessed name
static EMaterialTypes skHintRayExclude2 = kMT_CollisionActor;             // Guessed name
static EMaterialTypes skHintRayExclude3 = kMT_AIPassthrough;              // Guessed name
static EMaterialTypes skHintRayExclude4 = kMT_ExcludeFromLineOfSightTest; // Guessed name

static EMaterialTypes skGrabBallMaterial = kMT_Player;      // Guessed name
static EMaterialTypes skContactDamageSolid = kMT_Solid;     // Guessed name
static EMaterialTypes skCollisionCeiling = kMT_Ceiling;     // Guessed name
static EMaterialTypes skCollisionWall = kMT_Wall;           // Guessed name
static EMaterialTypes skCollisionFloor = kMT_Floor;         // Guessed name
static EMaterialTypes skCollisionCharacter = kMT_Character; // Guessed name

static inline bool RollChance(CStateManager& mgr, float chance) { // Guessed name
  if (chance == 1.f) {
    return true;
  }
  return mgr.Random()->Float() <= chance;
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
    const CDamageInfo& damage) const {
  if (mOwner != nullptr && mOwner->IsIngPossessed()) {
    return mOwner->GetIngPossessedArmorVulnerability();
  }
  return &mVulnerability;
}

bool CBloggBodyVulnerability::GetCollisionResponseType(
    const CVector3f& position, const CVector3f& direction, const CWeaponMode& mode, int attributes,
    EWeaponCollisionResponseTypes& response) const {
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
    const CDamageInfo& damage) const {
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

bool CBloggMouthVulnerability::GetCollisionResponseType(
    const CVector3f& position, const CVector3f& direction, const CWeaponMode& mode, int attributes,
    EWeaponCollisionResponseTypes& response) const {
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
, mMinAttackAngle(0.017453292f * minAttackAngle)
, mMaxAttackAngle(0.017453292f * maxAttackAngle)
, mMinAttackRange(patternedInfo.GetMinAttackRange())
, mMaxAttackRange(patternedInfo.GetMaxAttackRange())
, mCurrentAttackAngle(0.017453292f * minAttackAngle)
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
, mMouthDamageAngle(0.017453292f * mouthDamageAngle)
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
, xb28_(0.f)
, xb2c_(10.f)
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
  {
    const CPASAnimParmData parms(pas::kAS_AdditiveAim, CPASAnimParm::FromEnum(0),
                                 CPASAnimParm::FromEnum(0));
    const rstl::pair< float, int > best = pasDatabase.FindBestAnimation(parms, -1);
    if (best.first > 0.0000001192f) {
      mAimAnimLeft = best.second;
    }
  }
  {
    const CPASAnimParmData parms(pas::kAS_AdditiveAim, CPASAnimParm::FromEnum(1),
                                 CPASAnimParm::FromEnum(0));
    const rstl::pair< float, int > best = pasDatabase.FindBestAnimation(parms, -1);
    if (best.first > 0.0000001192f) {
      mAimAnimRight = best.second;
    }
  }
  {
    const CPASAnimParmData parms(pas::kAS_AdditiveAim, CPASAnimParm::FromEnum(2),
                                 CPASAnimParm::FromEnum(0));
    const rstl::pair< float, int > best = pasDatabase.FindBestAnimation(parms, -1);
    if (best.first > 0.0000001192f) {
      mAimAnimUp = best.second;
    }
  }
  {
    const CPASAnimParmData parms(pas::kAS_AdditiveAim, CPASAnimParm::FromEnum(3),
                                 CPASAnimParm::FromEnum(0));
    const rstl::pair< float, int > best = pasDatabase.FindBestAnimation(parms, -1);
    if (best.first > 0.0000001192f) {
      mAimAnimDown = best.second;
    }
  }
  {
    const CPASAnimParmData parms(pas::kAS_Unknown26);
    const rstl::pair< float, int > best = pasDatabase.FindBestAnimation(parms, -1);
    if (best.first > 0.0000001192f) {
      mUnknown26Anim = best.second;
    }
  }

  {
    const CPASAnimParmData parms(pas::kAS_Locomotion, CPASAnimParm::FromEnum(2),
                                 CPASAnimParm::FromEnum(3));
    const float distance = GetAnimationDistance(parms);
    const float duration = GetAnimationDuration(parms);
    sLocomotionSpeedA = distance / duration;
  }
  {
    const CPASAnimParmData parms(pas::kAS_Locomotion, CPASAnimParm::FromEnum(2),
                                 CPASAnimParm::FromEnum(2));
    const float distance = GetAnimationDistance(parms);
    const float duration = GetAnimationDuration(parms);
    sLocomotionSpeedB = distance / duration;
  }

  SetDrawShadow(false);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Character, kMT_CollisionActor, kMT_AIPassthrough, kMT_Player, kMT_Solid),
      CMaterialList()));
  BodyController()->BodyStateInfo().SetMaximumPitch(1.3962634f);

  rstl::rc_ptr< CBloggMouthVulnerability > mouthVulnerability =
      rstl::ncrc_ptr< CBloggMouthVulnerability >(mMouthVulnerability);
  if (mouthVulnerability) {
    mouthVulnerability->SetOwner(this);
  }
  rstl::rc_ptr< CBloggBodyVulnerability > bodyVulnerability =
      rstl::ncrc_ptr< CBloggBodyVulnerability >(mBodyVulnerability);
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
  return dot < static_cast< float >(cos(rstl::max_val(1.5707964f, 3.1415927f - mMouthDamageAngle)));
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
  return CVector3f::GetAngleDiff(direction, forward) < 0.17453292f;
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

void CBlogg::ApplyCollisionActorDamage(CStateManager& mgr, const TUniqueId& senderId,
                                       float multiplier) {
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
      SendScriptMsgs(kSS_InternalState00, mgr);
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

CEntity* REL_LoadBlogg(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
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

static void SetFuncPtrs() {
  static SBlogg_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &REL_LoadBlogg;
  SetSBlogg_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSBlogg_FuncPtrs(nullptr); }
