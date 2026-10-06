#ifndef _CDAMAGEINFO
#define _CDAMAGEINFO

#include "MetroidPrime/CStateManager.hpp"
#include "types.h"

#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Weapons/WeaponTypes.hpp"

class CDamageVulnerability;
struct SLdrTDamageInfo;

class CDamageInfo {
public:
  CDamageInfo()
  : mWeaponMode()
  , mDamage(0.f)
  , mRadiusDamageAmount(mDamage)
  , mDamageRadius(0.f)
  , mKnockbackPower(0.f)
  , mDamageSfxId(0xffff)
  , mDamageLoopSfxId(0xffff)
  , mSamusVoiceSfxId(0xffff)
  , mNoImmunity(false)
  , x1a_25_(false) {}

  // The x1a_25_ initializer is inferred from the two known argument patterns.
  CDamageInfo(const CWeaponMode& mode, float damage, float radius, float knockback,
              bool noImmunity = false)
  : mWeaponMode(mode)
  , mDamage(damage)
  , mRadiusDamageAmount(damage)
  , mDamageRadius(radius)
  , mKnockbackPower(knockback)
  , mDamageSfxId(0xffff)
  , mDamageLoopSfxId(0xffff)
  , mSamusVoiceSfxId(0xffff)
  , mNoImmunity(noImmunity)
  , x1a_25_(!noImmunity) {}

  CDamageInfo(CInputStream& in);
  CDamageInfo(const SLdrTDamageInfo& data, bool charged = false, bool comboed = false,
              bool noImmunity = false, bool flag = false);
  CDamageInfo(const CDamageInfo&, float);
  void SetDamageFromVulnerability(const CDamageVulnerability& dVuln, float damage);

  CDamageInfo ApplyDoubleDamage(const CPlayerState& state) const;

  ushort GetWeaponMode1() const { return mWeaponMode.GetRawType(); }
  const CWeaponMode& GetWeaponMode() const { return mWeaponMode; }
  ushort GetDamageSfxId() const { return mDamageSfxId; }
  ushort GetDamageLoopSfxId() const { return mDamageLoopSfxId; }
  ushort GetSamusVoiceSfxId() const { return mSamusVoiceSfxId; }

  void SetWeaponMode(const CWeaponMode& mode) { mWeaponMode = mode; }

  float GetRadius() const { return mDamageRadius; }
  void SetRadius(float r) { mDamageRadius = r; }
  float GetKnockBackPower() const { return mKnockbackPower; }
  float GetKnockBackPower(const CDamageVulnerability& vulnerability, float distance) const;
  bool GetX1a25() const { return x1a_25_; }
  void SetX1a25(bool flag) { x1a_25_ = flag; }
  void SetKnockBackPower(float k) { mKnockbackPower = k; }
  float GetDamage() const { return mDamage; }
  void SetDamage(float d) { mDamage = d; }
  bool HasNoDamage() const { return mDamage <= 0.0f; }
  float GetDamage(const CDamageVulnerability& dVuln) const;
  float GetRadiusDamage() const { return mRadiusDamageAmount; }
  void SetRadiusDamage(float r) { mRadiusDamageAmount = r; }
  float GetRadiusDamage(const CDamageVulnerability& dVuln) const;
  bool NoImmunity() const { return mNoImmunity; }
  void SetNoImmunity(bool b) { mNoImmunity = b; }
  void MultiplyDamage(const float m) {
    mDamage = m * mDamage;
    mRadiusDamageAmount = m * mRadiusDamageAmount;
    mKnockbackPower = m * mKnockbackPower;
  }
  void MultiplyDamageAndRadius(float m) {
    mDamage *= m;
    mRadiusDamageAmount *= m;
    mDamageRadius *= m;
    mKnockbackPower *= m;
  }

private:
  float GetVulnerableDamage(const CDamageVulnerability& dVuln) const;
  float GetVulnerableRadiusDamage(const CDamageVulnerability& dVuln) const;
  float GetVulnerableKnockBackPower(const CDamageVulnerability& dVuln, float distance) const;

  CWeaponMode mWeaponMode;
  float mDamage;
  float mRadiusDamageAmount;
  float mDamageRadius;
  float mKnockbackPower;
  ushort mDamageSfxId;
  ushort mDamageLoopSfxId;
  ushort mSamusVoiceSfxId;
  bool mNoImmunity : 1;
  bool x1a_25_ : 1;
};
CHECK_SIZEOF(CDamageInfo, 0x1c)

namespace NGunUtils {
CDamageInfo DifficultyModifyDamageInfo(const CDamageInfo& damage);
}

#endif // _CDAMAGEINFO
