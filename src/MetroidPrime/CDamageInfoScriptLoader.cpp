#include "MetroidPrime/CDamageInfo.hpp"

#include "MetroidPrime/ScriptLoader/Structs/SLdrTDamageInfo.hpp"

CDamageInfo LdrToDamageInfo(const SLdrTDamageInfo& data, bool charged, bool comboed,
                            bool noImmunity, bool applyRadiusDamage) {
  const float damage = data.damageAmount;
  const float radiusDamage = data.radiusDamageAmount;
  const float radius = data.damageRadius;
  const float knockback = data.knockBackPower;
  const CWeaponMode mode = CWeaponMode(static_cast< EWeaponType >(data.weaponType), charged,
                                       charged ? !charged : comboed);
  CDamageInfo result(mode, damage, radius, knockback, noImmunity, applyRadiusDamage);
  result.SetRadiusDamage(radiusDamage);
  return result;
}
