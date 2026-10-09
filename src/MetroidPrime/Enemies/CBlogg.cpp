#include "MetroidPrime/Enemies/CBlogg.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Enemies/CPatternedInfo.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrBlogg.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTeamAiMgr.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/TCastTo.hpp"
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
, xb24_(0.f)
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
