#ifndef _CTARGETABLEPROJECTILE
#define _CTARGETABLEPROJECTILE

#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"

// Reconstructed GC declarations; the constructor signature is corroborated by
// the original R3ME01 MP2 export. See research/CTargetableProjectile-G2ME01.md.
class CTargetableProjectile : public CEnergyProjectile {
public:
  CTargetableProjectile(const TToken< CWeaponDescription >& description, EWeaponType type,
                        const CTransform4f& xf, EMaterialTypes excludeMaterial,
                        const CDamageInfo& damage, const CDamageInfo& deflectedDamage,
                        TUniqueId uid, TAreaId areaId, TUniqueId owner,
                        const TToken< CWeaponDescription >& deflectedDescription,
                        TUniqueId homingTarget, uint attribs, const CImpactVisorEffect& visorEffect,
                        const CVector3f& scale);

  // CEntity
  ~CTargetableProjectile() override;
  CEntity* TypesMatch(int typeId) const override;

  // CActor
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;

  // CGameProjectile
  void ResolveCollisionWithActor(const CRayCastResult& result, CActor& actor,
                                 CStateManager& mgr) override;

  // CEnergyProjectile
  bool Explode(const CVector3f& position, const CVector3f& normal,
               EWeaponCollisionResponseTypes type, CStateManager& mgr,
               const CDamageVulnerability& vulnerability, TUniqueId hitActor) override;

private:
  // Guessed descriptive names: used to create a projectile back toward the owner.
  TToken< CWeaponDescription > mDeflectedWeaponDescription;
  CDamageInfo mDeflectedDamage;
  bool mDeflectToOwner : 1;
};
CHECK_SIZEOF(CTargetableProjectile, 0x590)

#endif // _CTARGETABLEPROJECTILE
