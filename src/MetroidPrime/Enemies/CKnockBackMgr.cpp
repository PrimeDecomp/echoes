#include "MetroidPrime/Enemies/CKnockBackMgr.hpp"

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CKnockBackInfo.hpp"
#include "MetroidPrime/CStateManager.hpp"

#include <string.h>

// Guessed names for the reconstructed RULE reaction records.
const CKnockBackMgr::SReactionParameters CKnockBackMgr::skDefaultParameters = {kAR_None, kFU_None,
                                                                               0.f, 0.f, 0};
const CKnockBackMgr::EKnockBackWeaponType CKnockBackMgr::skWeaponTypes[kWT_Max] = {
    kKBWT_Power,     kKBWT_Dark,    kKBWT_Light,        kKBWT_Annihilator,  kKBWT_Bomb,
    kKBWT_PowerBomb, kKBWT_Missile, kKBWT_BoostBall,    kKBWT_CannonBall,   kKBWT_ScrewAttack,
    kKBWT_Phazon,    kKBWT_AI,      kKBWT_PoisonWater1, kKBWT_PoisonWater2, kKBWT_Lava,
    kKBWT_Heat,      kKBWT_Unused,  kKBWT_AreaDark,     kKBWT_AreaLight,    kKBWT_UnknownSource,
    kKBWT_SafeZone};

CKnockBackInfo::CKnockBackInfo(const CVector3f& direction, TUniqueId source, TUniqueId owner,
                               const CDamageInfo& damage, bool direct)
: mDirection(direction), mSourceId(source), mOwnerId(owner), mDamageInfo(damage), mDirect(direct) {}

CKnockBackMgr::CKnockBackMgr(CAssetId rules)
: CRuleSetEvaluator(rules)
, mActiveParameters(skDefaultParameters)
, mDeferredParameters(skDefaultParameters)
, mDeferredRemainingTime(0.f)
, mMinimumReaction(kAR_None)
, mMaximumReaction(kAR_Fall)
, x44_(0.f)
, x48_(0)
, x4c_(0)
, x50_(0)
, x54_(0)
, mCharacterState(kCS_Invalid)
, mWeaponType(kKBWT_Invalid)
, mAvailableReactions(0)
, mEnableSlow(true)
, mEnableFreeze(true)
, mEnableShock(false)
, mEnableBurn(true)
, mEnableBurnDeath(true)
, mEnableExplodeDeath(true)
, mEnableLaggedBurnDeath(true)
, x61_31_(true)
, mLocomotionDuringElectrocution(false)
, mIsMultiplayer(false) {
  for (int i = kAR_None; i <= kAR_Fall; ++i) {
    EnableAnimReaction(static_cast< EAnimReaction >(i), true);
  }
}

void CKnockBackMgr::EnableAnimReaction(EAnimReaction reaction, bool enabled) {
  const uchar mask = 1 << reaction;
  if (enabled) {
    mAvailableReactions |= mask;
  } else {
    mAvailableReactions &= ~mask;
  }
}

void CKnockBackMgr::EnableAllAnimReactions(bool enabled) {
  for (int i = kAR_None; i <= kAR_Fall; ++i) {
    EnableAnimReaction(static_cast< EAnimReaction >(i), enabled);
  }
}

bool CKnockBackMgr::IsAnimReactionEnabled(EAnimReaction reaction) const {
  const uchar mask = 1 << reaction;
  return (mAvailableReactions & mask) != 0;
}

void CKnockBackMgr::SetAnimReactionRange(EAnimReaction minimum, EAnimReaction maximum) {
  mMinimumReaction = minimum;
  mMaximumReaction = maximum;
}

void CKnockBackMgr::KnockBack(CStateManager& mgr, CActor& actor, const CKnockBackInfo& info) {
  mIsMultiplayer = mgr.IsMultiplayer();
  SelectDamageState(actor, info);
  const CVector3f direction = GetKnockBackDirection(info.GetDirection(), actor);
  DoKnockBackAnimation(
      direction, mgr, actor,
      info.GetDamageInfo().GetKnockBackPower(*actor.GetDamageVulnerability(), 0.f));
  ApplyFollowUp(actor, mgr, info.GetSourceId(), info.GetOwnerId());
  ApplyKnockBackEffects(actor, mgr, info);
}

void CKnockBackMgr::Update(float dt, CStateManager& mgr, CActor& actor) {
  if (mDeferredRemainingTime > 0.f) {
    mDeferredRemainingTime -= dt;
    if (mDeferredRemainingTime <= 0.f) {
      mActiveParameters = mDeferredParameters;
      ApplyFollowUp(actor, mgr, kInvalidUniqueId, kInvalidUniqueId);
      mDeferredParameters.mFollowUp = kFU_None;
      mDeferredParameters.mFollowUpDuration = 0.f;
      mDeferredRemainingTime = 0.f;
    }
  }
}

float CKnockBackMgr::CalculateExtraHurlVelocity(CStateManager& mgr, float magnitude,
                                                float resistance) const {
  float velocity = 0.f;
  if (magnitude > resistance) {
    const float randomFactor = 1.1f - 0.2f * mgr.Random()->Float();
    velocity = 2.f * randomFactor * (magnitude - resistance);
  }
  return velocity;
}

CKnockBackMgr::EKnockBackWeaponType CKnockBackMgr::GetKnockBackWeaponType(const CDamageInfo& info,
                                                                          EWeaponType weapon,
                                                                          bool direct) const {
  const CWeaponMode& mode = info.GetWeaponMode();
  EKnockBackWeaponType type = kKBWT_Invalid;
  switch (weapon) {
  case kWT_Power:
    if (mode.IsCharged()) {
      type = kKBWT_PowerCharged;
    } else if (mode.IsComboed()) {
      type = kKBWT_PowerCombo;
    } else {
      type = kKBWT_Power;
    }
    break;
  case kWT_Dark:
    if (mode.IsCharged()) {
      type = direct ? kKBWT_DarkChargedDirect : kKBWT_DarkChargedIndirect;
    } else if (mode.IsComboed()) {
      type = kKBWT_DarkCombo;
    } else {
      type = kKBWT_Dark;
    }
    break;
  case kWT_Light:
    if (mode.IsCharged()) {
      type = kKBWT_LightCharged;
    } else if (mode.IsComboed()) {
      type = info.NoImmunity() ? kKBWT_LightMissile : kKBWT_LightCombo;
    } else {
      type = kKBWT_Light;
    }
    break;
  case kWT_Annihilator:
    if (mode.IsCharged()) {
      if (info.ShouldApplyRadiusDamage()) {
        type = kKBWT_AnnihilatorChargedEffect;
      } else {
        type = kKBWT_AnnihilatorCharged;
      }
    } else if (mode.IsComboed()) {
      type = kKBWT_AnnihilatorCombo;
    } else if (info.GetDamage() == 0.f && info.GetRadiusDamage() == 0.f &&
               info.GetRadius() == 0.f && info.GetKnockBackPower() == 0.f) {
      type = kKBWT_AnnihilatorNoDamage;
    } else {
      type = kKBWT_Annihilator;
    }
    break;
  default:
    if (weapon >= kWT_Power && weapon <= kWT_SafeZone) {
      type = skWeaponTypes[weapon];
    }
    break;
  }
  return type;
}

CVector3f CKnockBackMgr::GetKnockBackDirection(const CVector3f& direction,
                                               const CActor& actor) const {
  CVector3f result(direction.ToVec2f(), 0.f);
  if (!result.IsMagnitudeSafe()) {
    result = -actor.GetTransform().GetForward();
  }
  return result;
}

void CKnockBackMgr::SelectDamageState(const CActor& actor, const CKnockBackInfo& info) {
  memcpy(&mActiveParameters, &skDefaultParameters, sizeof(mActiveParameters));
  const CDamageInfo& damage = info.GetDamageInfo();
  mWeaponType = GetKnockBackWeaponType(damage, static_cast< EWeaponType >(damage.GetWeaponMode1()),
                                       info.IsDirect());
  if (mWeaponType != kKBWT_Invalid) {
    mCharacterState = GetCharacterState(actor);
    mActiveParameters = skDefaultParameters;
    EvaluateRules();
    ValidateState(actor);
    switch (mActiveParameters.mFollowUp) {
    case kFU_FreezeBurn:
      mActiveParameters.mFollowUp = kFU_Freeze;
      mActiveParameters.mFollowUpDuration *= 0.5f;
      DeferFollowUp(mActiveParameters.mFollowUpDuration, kFU_Burn,
                    mActiveParameters.mFollowUpDuration);
      break;
    case kFU_FreezeDisintegration:
      mActiveParameters.mFollowUp = kFU_Freeze;
      mActiveParameters.mFollowUpDuration *= 0.5f;
      DeferFollowUp(mActiveParameters.mFollowUpDuration, kFU_LaggedBurnDeath,
                    mActiveParameters.mFollowUpDuration);
      break;
    default:
      break;
    }
  }
}

void CKnockBackMgr::ValidateState(const CActor& actor) {
  if (mActiveParameters.mReaction < mMinimumReaction) {
    mActiveParameters.mReaction = mMinimumReaction;
  } else if (mActiveParameters.mReaction > mMaximumReaction) {
    mActiveParameters.mReaction = mMaximumReaction;
  }

  EAnimReaction reaction = kAR_Invalid;
  if (IsAlive(actor)) {
    if (HasAnimReaction(actor, kAR_Hurled) && IsAnimReactionEnabled(kAR_Hurled) &&
        mActiveParameters.mReaction >= kAR_Hurled) {
      reaction = kAR_Hurled;
    } else if (HasAnimReaction(actor, kAR_KnockBack) && IsAnimReactionEnabled(kAR_KnockBack) &&
               mActiveParameters.mReaction >= kAR_KnockBack) {
      reaction = kAR_KnockBack;
    } else if (HasAnimReaction(actor, kAR_Flinch) && IsAnimReactionEnabled(kAR_Flinch) &&
               mActiveParameters.mReaction >= kAR_Flinch) {
      reaction = kAR_Flinch;
    }
  } else if (HasAnimReaction(actor, kAR_Fall) && IsAnimReactionEnabled(kAR_Fall) &&
             (mActiveParameters.mReaction >= kAR_Fall ||
              (!HasAnimReaction(actor, kAR_Hurled) && mActiveParameters.mReaction >= kAR_Hurled))) {
    reaction = kAR_Fall;
  } else if (HasAnimReaction(actor, kAR_Hurled) && IsAnimReactionEnabled(kAR_Hurled) &&
             mActiveParameters.mReaction >= kAR_Hurled) {
    reaction = kAR_Hurled;
  }
  mActiveParameters.mReaction = reaction != kAR_Invalid ? reaction : kAR_None;

  if (!mEnableFreeze) {
    if (mActiveParameters.mFollowUp == kFU_FreezeBurn) {
      mActiveParameters.mFollowUp = kFU_Burn;
    } else if (mActiveParameters.mFollowUp == kFU_FreezeDisintegration) {
      mActiveParameters.mFollowUp = kFU_LaggedBurnDeath;
    }
  }

  bool disableFollowUp = false;
  switch (mActiveParameters.mFollowUp) {
  case kFU_Slow:
    disableFollowUp = !mEnableSlow;
    break;
  case kFU_Freeze:
    disableFollowUp = !mEnableFreeze;
    break;
  case kFU_Shock:
    disableFollowUp = !mEnableShock;
    break;
  case kFU_Burn:
    disableFollowUp = !mEnableBurn;
    break;
  case kFU_BurnPhase:
    disableFollowUp = !mEnableBurn;
    break;
  case kFU_ExplodeDeath:
    disableFollowUp = !mEnableExplodeDeath;
    break;
  case kFU_IceDeath:
    disableFollowUp = !mEnableExplodeDeath;
    break;
  case kFU_BurnDeath:
    disableFollowUp = !mEnableBurnDeath;
    break;
  case kFU_LaggedBurnDeath:
    disableFollowUp = !mEnableLaggedBurnDeath;
    break;
  case kFU_BlackDeath:
    disableFollowUp = !mEnableLaggedBurnDeath;
    break;
  case kFU_ImmediateDisintegration:
    disableFollowUp = !mEnableLaggedBurnDeath;
    break;
  case kFU_FreezeBurn:
    disableFollowUp = !mEnableFreeze;
    break;
  case kFU_FreezeDisintegration:
    disableFollowUp = !mEnableFreeze;
    break;
  default:
    break;
  }
  if (disableFollowUp) {
    mActiveParameters.mFollowUp = kFU_None;
    mActiveParameters.mFollowUpDuration = 0.f;
    mActiveParameters.mSecondaryDuration = 0.f;
  }
}

void CKnockBackMgr::DeferFollowUp(float delay, EFollowUp followUp, float duration) {
  mDeferredParameters.mReaction = kAR_None;
  mDeferredParameters.mFollowUp = followUp;
  mDeferredParameters.mFollowUpDuration = duration;
  mDeferredParameters.mFlags = 0;
  mDeferredRemainingTime = delay;
}

CRuleValue CKnockBackMgr::GetConditionValue(FourCC condition) const {
  switch (condition) {
  case 'ALIV':
    return CRuleValue(mCharacterState == kCS_Alive);
  case 'DEAD':
    return CRuleValue(mCharacterState == kCS_Dead);
  case 'ISBL':
    return CRuleValue(IsBall());
  case 'IFZN':
    return CRuleValue(WasFrozen());
  case 'NGND':
    return CRuleValue(WasOnGround());
  case 'FGND':
    return CRuleValue(!WasOnGround());
  case 'IMPL':
    return CRuleValue(mIsMultiplayer);
  case 'PWBM':
    return CRuleValue(mWeaponType == kKBWT_Power);
  case 'PWCH':
    return CRuleValue(mWeaponType == kKBWT_PowerCharged);
  case 'PWCB':
    return CRuleValue(mWeaponType == kKBWT_PowerCombo);
  case 'DKBM':
    return CRuleValue(mWeaponType == kKBWT_Dark);
  case 'DKC1':
    return CRuleValue(mWeaponType == kKBWT_DarkChargedDirect);
  case 'DKC2':
    return CRuleValue(mWeaponType == kKBWT_DarkChargedIndirect);
  case 'DKCB':
    return CRuleValue(mWeaponType == kKBWT_DarkCombo);
  case 'LTBM':
    return CRuleValue(mWeaponType == kKBWT_Light);
  case 'LTC1':
    return CRuleValue(mWeaponType == kKBWT_LightCharged);
  case 'LTMD':
    return CRuleValue(mWeaponType == kKBWT_LightCombo);
  case 'LTMS':
    return CRuleValue(mWeaponType == kKBWT_LightMissile);
  case 'ANBM':
    return CRuleValue(mWeaponType == kKBWT_Annihilator);
  case 'ANBN':
    return CRuleValue(mWeaponType == kKBWT_AnnihilatorNoDamage);
  case 'ANCH':
    return CRuleValue(mWeaponType == kKBWT_AnnihilatorCharged);
  case 'ANCB':
    return CRuleValue(mWeaponType == kKBWT_AnnihilatorCombo);
  case 'ANCE':
    return CRuleValue(mWeaponType == kKBWT_AnnihilatorChargedEffect);
  case 'MISL':
    return CRuleValue(mWeaponType == kKBWT_Missile);
  case 'BOMB':
    return CRuleValue(mWeaponType == kKBWT_Bomb);
  case 'BOST':
    return CRuleValue(mWeaponType == kKBWT_BoostBall);
  case 'CANB':
    return CRuleValue(mWeaponType == kKBWT_CannonBall);
  case 'XSTP':
    return CRuleValue(mWeaponType == kKBWT_UnknownSource);
  case 'AIDG':
    return CRuleValue(mWeaponType == kKBWT_AI);
  case 'IADD':
    return CRuleValue(mWeaponType == kKBWT_AreaDark);
  case 'IADL':
    return CRuleValue(mWeaponType == kKBWT_AreaLight);
  case 'PBNL':
    return CRuleValue(mWeaponType == kKBWT_PowerBomb);
  default:
    return CRuleValue(false);
  }
}

int CKnockBackMgr::ExecuteAction(const CRuleAction& action) {
  switch (action.GetId()) {
  case 'FLCH':
    mActiveParameters.mReaction = kAR_Flinch;
    break;
  case 'HHIT':
    mActiveParameters.mReaction = kAR_KnockBack;
    break;
  case 'HURL':
    mActiveParameters.mReaction = kAR_Hurled;
    break;
  case 'DIMP':
    mActiveParameters.mReaction = kAR_Fall;
    break;
  case 'FRZN':
    mActiveParameters.mFollowUp = kFU_Freeze;
    mActiveParameters.mFollowUpDuration = action.GetProperty(0).GetFloat();
    break;
  case 'ELEC':
    mActiveParameters.mFollowUp = kFU_Shock;
    mActiveParameters.mFollowUpDuration = action.GetProperty(0).GetFloat();
    break;
  case 'BURN':
    mActiveParameters.mFollowUp = kFU_Burn;
    mActiveParameters.mFollowUpDuration = action.GetProperty(0).GetFloat();
    break;
  case 'DETH':
    mActiveParameters.mFollowUp = kFU_Death;
    break;
  case 'GIBB':
    mActiveParameters.mFollowUp = kFU_ExplodeDeath;
    break;
  case 'ICGB':
    mActiveParameters.mFollowUp = kFU_IceDeath;
    break;
  case 'ASHH':
    mActiveParameters.mFollowUp = kFU_BurnDeath;
    break;
  case 'DSGT':
    mActiveParameters.mFollowUp = kFU_LaggedBurnDeath;
    break;
  case 'UNFZ':
    mActiveParameters.mFlags |= 1;
    break;
  case 'STFR':
    mActiveParameters.mFlags |= 2;
    break;
  case 'STEC':
    mActiveParameters.mFlags |= 4;
    break;
  case 'BRKL':
    mActiveParameters.mFlags |= 8;
    break;
  case 'NPKB':
    mActiveParameters.mFlags |= 16;
    break;
  case 'STSL':
    mActiveParameters.mFlags |= 32;
    break;
  case 'IRTN':
    mActiveParameters.mFlags |= 64;
    break;
  case 'SLOW':
    mActiveParameters.mFollowUp = kFU_Slow;
    mActiveParameters.mFollowUpDuration = action.GetProperty(0).GetFloat();
    mActiveParameters.mSecondaryDuration = action.GetProperty(1).GetFloat();
    break;
  case 'BRNP':
    mActiveParameters.mFollowUp = kFU_BurnPhase;
    mActiveParameters.mFollowUpDuration = action.GetProperty(0).GetFloat();
    mActiveParameters.mSecondaryDuration = action.GetProperty(1).GetFloat();
    break;
  case 'BLKD':
    mActiveParameters.mFollowUp = kFU_BlackDeath;
    break;
  case 'IMEX':
    mActiveParameters.mFollowUp = kFU_ImmediateExplosion;
    mActiveParameters.mFollowUpDuration = action.GetProperty(0).GetFloat();
    break;
  case 'IMDS':
    mActiveParameters.mFollowUp = kFU_ImmediateDisintegration;
    break;
  default:
    break;
  }
  return false;
}
