#ifndef _CDAMAGEINFO
#define _CDAMAGEINFO

#include "MetroidPrime/CStateManager.hpp"
#include "types.h"

#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Weapons/WeaponTypes.hpp"

class CDamageVulnerability;

class CDamageInfo {
public:
  // CDamageInfo()
  // : x0_weaponMode(CWeaponMode::Invalid())
  // , x8_damage(0.f)
  // , xc_radiusDamage(0.f)
  // , x10_radius(0.f)
  // , x14_knockback(0.f)
  // , x18_24_noImmunity(false) {}

  // CDamageInfo(const CWeaponMode& mode, const float damage, const float radius,
  //             const float knockback, const bool noImmunity = false)
  // : x0_weaponMode(mode)
  // , x8_damage(damage)
  // , xc_radiusDamage(x8_damage)
  // , x10_radius(radius)
  // , x14_knockback(knockback)
  // , x18_24_noImmunity(noImmunity) {}

  CDamageInfo(CInputStream& in);
  CDamageInfo(const CDamageInfo&, float);

  CDamageInfo ApplyDoubleDamage(const CPlayerState& state) const;

  ushort GetWeaponMode1() const { return m_weaponMode1; }
  ushort GetWeaponMode2() const { return m_weaponMode2; }
  // void SetWeaponMode(const CWeaponMode& mode) { x0_weaponMode = mode; }
  float GetRadius() const { return m_damageRadius; }
  void SetRadius(float r) { m_damageRadius = r; }
  float GetKnockBackPower() const { return m_knockbackPower; }
  void SetKnockBackPower(float k) { m_knockbackPower = k; }
  float GetDamage() const { return m_damage; }
  void SetDamage(float d) { m_damage = d; }
  bool HasNoDamage() const { return m_damage <= 0.0f; }
  float GetDamage(const CDamageVulnerability& dVuln) const;
  float GetRadiusDamage() const { return m_radiusDamageAmount; }
  void SetRadiusDamage(float r) { m_radiusDamageAmount = r; }
  float GetRadiusDamage(const CDamageVulnerability& dVuln) const;
  bool NoImmunity() const { return m_noImmunity; }
  void SetNoImmunity(bool b) { m_noImmunity = b; }
  void MultiplyDamage(const float m) {
    m_damage = m * m_damage;
    m_radiusDamageAmount = m * m_radiusDamageAmount;
    m_knockbackPower = m * m_knockbackPower;
  }
  void MultiplyDamageAndRadius(float m) {
    m_damage *= m;
    m_radiusDamageAmount *= m;
    m_damageRadius *= m;
    m_knockbackPower *= m;
  }

private:
  ushort m_weaponMode1;
  ushort m_weaponMode2;
  float m_damage;
  float m_radiusDamageAmount;
  float m_damageRadius;
  float m_knockbackPower;
  short m_x14;
  short m_x16;
  short m_x18;
  bool m_noImmunity;
};

#endif // _CDAMAGEINFO
