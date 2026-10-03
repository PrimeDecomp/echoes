#include "MetroidPrime/Weapons/CTargetableProjectile.hpp"

CTargetableProjectile::CTargetableProjectile(
    const TToken< CWeaponDescription >& description, EWeaponType type, const CTransform4f& xf,
    EMaterialTypes excludeMaterial, const CDamageInfo& damage, const CDamageInfo& deflectedDamage,
    TUniqueId uid, TAreaId areaId, TUniqueId owner,
    const TToken< CWeaponDescription >& deflectedDescription, TUniqueId homingTarget, uint attribs,
    const CImpactVisorEffect& visorEffect, const CVector3f& scale)
: CEnergyProjectile(true, description, type, xf, excludeMaterial, damage, uid, areaId, owner,
                    homingTarget, attribs, false, scale, visorEffect, false, true, false, 1.f, 0.f,
                    0.f)
, mDeflectedWeaponDescription(deflectedDescription) {}

CTargetableProjectile::~CTargetableProjectile() {}

CEntity* CTargetableProjectile::TypesMatch(int typeId) const {}

void CTargetableProjectile::ResolveCollisionWithActor(const CRayCastResult& result, CActor& actor,
                                                      CStateManager& mgr) {}

CVector3f CTargetableProjectile::GetAimPosition(const CStateManager& mgr, float dt) const {}

bool CTargetableProjectile::Explode(const CVector3f& position, const CVector3f& normal,
                                    EWeaponCollisionResponseTypes type, CStateManager& mgr,
                                    const CDamageVulnerability& vulnerability, TUniqueId hitActor) {
}
