#include "MetroidPrime/Enemies/CAiKnockBackMgr.hpp"

#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CKnockBackInfo.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"

const float CAiKnockBackMgr::skImpulseDurations[2] = {0.1f, 0.3f};
const pas::EAnimationState CAiKnockBackMgr::skReactionStates[5] = {
    pas::kAS_Invalid, pas::kAS_AdditiveFlinch, pas::kAS_KnockBack, pas::kAS_Hurled, pas::kAS_Fall};

CAiKnockBackMgr::CAiKnockBackMgr(CAssetId rules)
: CKnockBackMgr(rules)
, mSeverity(pas::kS_One)
, mFlinchType(1)
, mFlinchRemainingTime(0.f)
, mPhysicsKnockBackType(kPKBT_Constant)
, mImpulseDirection(CVector3f::Zero())
, mImpulseMagnitude(0.f)
, mImpulseRemainingTime(0.f)
, mAdditiveFlinchWeight(1.f)
, mPhysicsImpulseMagnitude(2.f)
, mKnockBackPhysicsEnabled(true)
, mHurlVelocityEnabled(true)
, mWasFrozen(false)
, mWasOnGround(true) {}

void CAiKnockBackMgr::Update(float dt, CStateManager& mgr, CActor& actor) {
  CKnockBackMgr::Update(dt, mgr, actor);
  ApplyImpulse(dt, static_cast< CPhysicsActor& >(actor));
  if (CPatterned* patterned = TCastToPtr< CPatterned >(&actor)) {
    mFlinchRemainingTime -= dt;
    if (mLocomotionDuringElectrocution && patterned->GetBodyController()->IsElectrocuting()) {
      patterned->BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_Locomotion));
    }
  }
}

void CAiKnockBackMgr::SetPhysicsKnockBackType(EPhysicsKnockBackType type) {
  mPhysicsKnockBackType = type;
}

void CAiKnockBackMgr::EnableKnockBackPhysics(bool enabled) {
  mKnockBackPhysicsEnabled = enabled;
  if (!mKnockBackPhysicsEnabled) {
    mImpulseMagnitude = 0.f;
    mImpulseRemainingTime = 0.f;
  }
}

bool CAiKnockBackMgr::IsAlive(const CActor& actor) const {
  if (const CPatterned* patterned = TCastToConstPtr< CPatterned >(&actor)) {
    return patterned->GetAlive();
  }
  return false;
}

CKnockBackMgr::ECharacterState CAiKnockBackMgr::GetCharacterState(const CActor& actor) const {
  if (const CPatterned* patterned = TCastToConstPtr< CPatterned >(&actor)) {
    return patterned->GetAlive() ? kCS_Alive : kCS_Dead;
  }
  return kCS_Invalid;
}

bool CAiKnockBackMgr::HasAnimReaction(const CActor& actor, EAnimReaction reaction) const {
  if (const CPatterned* patterned = TCastToConstPtr< CPatterned >(&actor)) {
    return patterned->GetBodyController()->HasBodyState(skReactionStates[reaction]);
  }
  return false;
}

void CAiKnockBackMgr::DoKnockBackAnimation(const CVector3f& direction, CStateManager& mgr,
                                           CActor& actor, float magnitude) {
  CPatterned* patterned = TCastToPtr< CPatterned >(&actor);
  if (!patterned) {
    return;
  }

  CBodyController& body = *patterned->BodyController();
  switch (mActiveParameters.mReaction) {
  case kAR_Hurled: {
    float hurlVelocity = 5.f;
    if (const CHealthInfo* health = patterned->GetHealthInfo()) {
      hurlVelocity += CalculateExtraHurlVelocity(mgr, magnitude, health->GetKnockBackResistance());
    }
    hurlVelocity = CMath::SqrtF(0.5f * hurlVelocity * patterned->GetGravityConstant());
    const CVector3f upwardDirection = direction + direction.Magnitude() * CVector3f::Up();
    if (upwardDirection.CanBeNormalized()) {
      body.CommandMgr().DeliverCmd(CBCHurledCmd(
          -direction, mHurlVelocityEnabled ? hurlVelocity * upwardDirection.AsNormalized()
                                           : CVector3f::Zero()));
      patterned->SetMomentumWR(
          CVector3f(0.f, 0.f, -patterned->GetMass() * patterned->GetGravityConstant()));
    }
    break;
  }
  case kAR_Fall:
    body.CommandMgr().DeliverCmd(CBCKnockDownCmd(
        -direction, mSeverity, (mActiveParameters.mFlags & kRF_SkipFallRotation) != 0));
    break;
  case kAR_KnockBack:
    body.CommandMgr().DeliverCmd(CBCKnockBackCmd(-direction, mSeverity));
    break;
  case kAR_Flinch: {
    const CPASDatabase& database = body.GetPASDatabase();
    const CVector3f localDirection = actor.GetTransform().TransposeRotate(direction);
    const float angle = CMath::ClampRadians(atan2(localDirection.GetY(), localDirection.GetX()));
    const CPASAnimParmData directionalParms(
        pas::kAS_AdditiveDirectionalReaction,
        CPASAnimParm::FromReal32(CRelAngle::FromRadians(angle).AsDegrees()),
        CPASAnimParm::FromEnum(mFlinchType));
    rstl::pair< float, int > best = database.FindBestAnimation(directionalParms, *mgr.Random(), -1);
    if (best.first <= 0.f) {
      const CPASAnimParmData parms(pas::kAS_AdditiveFlinch);
      best = database.FindBestAnimation(parms, *mgr.Random(), -1);
    }
    if (best.first > 0.f) {
      patterned->AnimationData()->AddAdditiveAnimation(best.second, GetAdditiveFlinchWeight(),
                                                       false, true);
      mFlinchRemainingTime = rstl::max_val(
          mFlinchRemainingTime, patterned->AnimationData()->GetAnimationDuration(best.second));
    }
    break;
  }
  default:
    break;
  }
}

void CAiKnockBackMgr::ApplyFollowUp(CActor& actor, CStateManager& mgr, TUniqueId source,
                                    TUniqueId owner) {
  if (CPatterned* patterned = TCastToPtr< CPatterned >(&actor)) {
    const CAiKnockBackMgr& controller = patterned->GetKnockBackController();
    patterned->ApplyKnockBackFollowUp(mgr, -patterned->GetTransform().GetForward(),
                                      controller.GetFollowUp(), controller.GetFollowUpDuration(),
                                      controller.GetSecondaryDuration(), source, owner);
  }
}

void CAiKnockBackMgr::ApplyKnockBackEffects(CActor& actor, CStateManager& mgr,
                                            const CKnockBackInfo& info) {
  if (CPatterned* patterned = TCastToPtr< CPatterned >(&actor)) {
    if (patterned->GetBodyController()->IsFrozen() &&
        (mActiveParameters.mFlags & kRF_BreakFreeze)) {
      patterned->BodyController()->FrozenBreakout();
    }
    if (patterned->GetBodyController()->IsOnFire() && (mActiveParameters.mFlags & kRF_DouseFire)) {
      patterned->BodyController()->DouseFlames();
    }
    if (patterned->GetBodyController()->IsElectrocuting() &&
        (mActiveParameters.mFlags & kRF_StopElectrocution)) {
      patterned->BodyController()->DouseElectrocuting();
    }
    if (patterned->GetBodyController()->GetTimeScale() > 0.f &&
        (mActiveParameters.mFlags & kRF_ResetTimeScale)) {
      patterned->BodyController()->SetTimeScale(1.f);
    }

    if (mKnockBackPhysicsEnabled && !patterned->GetBodyController()->IsFrozen() &&
        !(mActiveParameters.mFlags & kRF_DisablePhysics)) {
      if (info.GetDamageInfo().GetKnockBackPower(*actor.GetDamageVulnerability(), 0.f) > 0.f) {
        const CVector3f direction = GetKnockBackDirection(info.GetDirection(), actor);
        ResetKnockBackImpulse(actor, direction, mPhysicsImpulseMagnitude);
      }
    }
  }
}

void CAiKnockBackMgr::KnockBack(CStateManager& mgr, CActor& actor, const CKnockBackInfo& info) {
  mWasFrozen = false;
  if (CPatterned* patterned = TCastToPtr< CPatterned >(actor)) {
    mWasFrozen = patterned->GetBodyController()->IsFrozen();
    mWasOnGround = patterned->IsOnGround();
  }
  CKnockBackMgr::KnockBack(mgr, actor, info);
}

void CAiKnockBackMgr::ResetKnockBackImpulse(CActor& actor, const CVector3f& direction,
                                            float magnitude) {
  mImpulseDirection =
      direction.CanBeNormalized() ? direction.AsNormalized() : -actor.GetTransform().GetForward();
  if (mImpulseRemainingTime <= 0.f) {
    mImpulseMagnitude = magnitude;
  } else {
    mImpulseMagnitude +=
        magnitude * (1.f - mImpulseRemainingTime / skImpulseDurations[mPhysicsKnockBackType]);
  }
  mImpulseRemainingTime = skImpulseDurations[mPhysicsKnockBackType];
}

void CAiKnockBackMgr::ApplyImpulse(float dt, CPhysicsActor& actor) {
  mImpulseRemainingTime = rstl::max_val(mImpulseRemainingTime - dt, 0.f);
  if (!actor.GetMaterialList().HasMaterial(kMT_Immovable) && mImpulseRemainingTime > 0.f) {
    float remainingFactor = 1.f;
    switch (mPhysicsKnockBackType) {
    case kPKBT_Decaying:
      remainingFactor = mImpulseRemainingTime / skImpulseDurations[mPhysicsKnockBackType];
      break;
    default:
      break;
    }

    const CVector3f velocity = remainingFactor * mImpulseMagnitude * mImpulseDirection;
    const CVector3f displacement = dt * velocity;
    const CVector3f movement = (1.f / skImpulseDurations[mPhysicsKnockBackType]) * displacement;
    actor.MoveInOneFrameOR(actor.GetTransform().TransposeRotate(movement), dt);
  }
}

float CAiKnockBackMgr::GetAdditiveFlinchWeight() const { return mAdditiveFlinchWeight; }

void CAiKnockBackMgr::SetAdditiveFlinchWeight(float weight) { mAdditiveFlinchWeight = weight; }

bool CAiKnockBackMgr::WasOnGround() const { return mWasOnGround; }

bool CAiKnockBackMgr::WasFrozen() const { return mWasFrozen; }

bool CAiKnockBackMgr::IsBall() const { return false; }
