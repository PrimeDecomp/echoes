#ifndef _CBLOGGPROJECTILE
#define _CBLOGGPROJECTILE

#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"

// Guessed class name.
class CBloggProjectile : public CEnergyProjectile {
public:
  CBloggProjectile(bool active, const TToken< CWeaponDescription >& description, EWeaponType type,
                   const CTransform4f& xf, EMaterialTypes excludeMaterial,
                   const CDamageInfo& damage, TUniqueId uid, TAreaId areaId, TUniqueId owner,
                   TUniqueId homingTarget, uint attribs, bool underwater, const CVector3f& scale,
                   const CImpactVisorEffect& visorEffect, bool unused, bool playImpactSound,
                   float value);

  // CEntity
  ~CBloggProjectile() override;

  // CGameProjectile
  void ApplyDamageToOneActor(CStateManager& mgr, const CDamageInfo& damageInfo, TUniqueId id,
                             const CVector3f& direction) override;

private:
  float x568_; // Unknown meaning.
};
CHECK_SIZEOF(CBloggProjectile, 0x570)

#endif // _CBLOGGPROJECTILE
