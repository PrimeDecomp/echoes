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
#include "MetroidPrime/TCastTo.hpp"
#include "REL/REL_Setup.h"

#include <math.h>

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"ShouldPatrol", static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::ShouldPatrol)},
    {"AnimOver", static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::AnimOver)},
    {"ShouldPrepareToAttack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::ShouldPrepareToAttack)},
    {"IsPlayerStunned",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::IsPlayerStunned)},
    {"InProjectileRange",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::InProjectileRange)},
    {"CollidedWithWall",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::CollidedWithWall)},
    {"CanBitePlayer", static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::CanBitePlayer)},
    {"InMeleeRange", static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::InMeleeRange)},
    {"InBiteRange", static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::InBiteRange)},
    {"CantMoveToPlayer",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::CantMoveToPlayer)},
    {"BallGrabbed", static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::BallGrabbed)},
    {"ShouldAbortBallGrab",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CBlogg::ShouldAbortBallGrab)},
};

static CPatterned::StateMachine::SCodeFunction skCodeFuncs[] = {
    {"ComputeTauntProbability",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CBlogg::ComputeTauntProbability)},
    {"EndMeleePursuit",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CBlogg::EndMeleePursuit)},
};

static const char* const skMouthLocatorName = "mouth_LCTR"; // Guessed name

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
, mPlayerPursuitTime(0.f)
, mBallPursuitTime(0.f)
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
, xbb0_(0)
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
