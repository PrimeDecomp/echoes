#include "MetroidPrime/CDamageInfo.hpp"

#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"

static inline bool does_damage(const CWeaponTypeVulnerability& vulnerability) {
  return !close_enough(vulnerability.mDamageMultiplier, 0.f) &&
         vulnerability.mEffect != CWeaponTypeVulnerability::kE_Immune;
}

static inline bool check_hurts(const CWeaponTypeVulnerability& vulnerability) {
  bool hurts = true;
  if (!does_damage(vulnerability) && vulnerability.mEffect != CWeaponTypeVulnerability::kE_Immune) {
    hurts = false;
  }
  return hurts;
}

float CDamageInfo::GetDamage(const CDamageVulnerability& dVuln) const {
  if (dVuln.GetEffect(mWeaponMode) == CWeaponTypeVulnerability::kE_Immune) {
    return 0.f;
  }
  return GetVulnerableDamage(dVuln);
}

float CDamageInfo::GetVulnerableDamage(const CDamageVulnerability& dVuln) const {
  const CWeaponTypeVulnerability vulnerability = dVuln.GetVulnerability(mWeaponMode);
  return mDamage * vulnerability.mDamageMultiplier;
}

float CDamageInfo::GetRadiusDamage(const CDamageVulnerability& dVuln) const {
  if (dVuln.GetEffect(mWeaponMode) == CWeaponTypeVulnerability::kE_Immune) {
    return 0.f;
  }
  return GetVulnerableRadiusDamage(dVuln);
}

float CDamageInfo::GetVulnerableRadiusDamage(const CDamageVulnerability& dVuln) const {
  const CWeaponTypeVulnerability vulnerability = dVuln.GetVulnerability(mWeaponMode);
  return mRadiusDamageAmount * vulnerability.mDamageMultiplier;
}

float CDamageInfo::GetKnockBackPower(const CDamageVulnerability& dVuln, float distance) const {
  if (dVuln.GetEffect(mWeaponMode) == CWeaponTypeVulnerability::kE_Immune) {
    return 0.f;
  }
  return GetVulnerableKnockBackPower(dVuln, distance);
}

float CDamageInfo::GetVulnerableKnockBackPower(const CDamageVulnerability& dVuln,
                                               float distance) const {
  const CWeaponTypeVulnerability vulnerability = dVuln.GetVulnerability(mWeaponMode);
  if (!check_hurts(vulnerability)) {
    return 0.f;
  }

  const float radius = mDamageRadius;
  const bool hasFalloff = radius != 0.f && distance != 0.f;
  float falloff = hasFalloff ? (radius - distance) / radius : 1.f;
  if (vulnerability.mDamageMultiplier <= 0.5f) {
    falloff *= 2.f;
  }
  return falloff * mKnockbackPower;
}

CDamageInfo::CDamageInfo(const CDamageInfo& other, float dt)
: mWeaponMode(other.mWeaponMode)
, mDamage(other.mDamage * (60.f * dt))
, mRadiusDamageAmount(mDamage)
, mDamageRadius(other.mDamageRadius)
, mKnockbackPower(other.mKnockbackPower)
, mDamageSfxId(other.mDamageSfxId)
, mDamageLoopSfxId(other.mDamageLoopSfxId)
, mSamusVoiceSfxId(other.mSamusVoiceSfxId)
, mNoImmunity(true)
, mApplyRadiusDamage(other.mApplyRadiusDamage) {}

void CDamageInfo::SetDamageFromVulnerability(const CDamageVulnerability& dVuln, float damage) {
  const float multiplier = dVuln.GetVulnerability(mWeaponMode).mDamageMultiplier;
  if (close_enough(multiplier, 0.f)) {
    mDamage = 0.f;
  } else {
    mDamage = damage / multiplier;
  }
}
