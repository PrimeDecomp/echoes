#include "MetroidPrime/Weapons/CBloggProjectile.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TCastTo.hpp"

CBloggProjectile::CBloggProjectile(bool active, const TToken< CWeaponDescription >& description,
                                   EWeaponType type, const CTransform4f& xf,
                                   EMaterialTypes excludeMaterial, const CDamageInfo& damage,
                                   TUniqueId uid, TAreaId areaId, TUniqueId owner,
                                   TUniqueId homingTarget, uint attribs, bool underwater,
                                   const CVector3f& scale, const CImpactVisorEffect& visorEffect,
                                   bool unused, bool playImpactSound, float value)
: CEnergyProjectile(active, description, type, xf, excludeMaterial, damage, uid, areaId, owner,
                    homingTarget, attribs, underwater, scale, visorEffect, unused, playImpactSound,
                    false, 1.f, 4.f, 4.f)
, x568_(value) {}

void CBloggProjectile::ApplyDamageToOneActor(CStateManager& mgr, const CDamageInfo& damageInfo,
                                             TUniqueId id, const CVector3f& direction) {
  CGameProjectile::ApplyDamageToOneActor(mgr, damageInfo, id, direction);
  if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(id))) {
    // The native code casts to CPlayer but does nothing with the result.
    CPlayer* player = TCastToPtr< CPlayer >(actor);
  }
}

CBloggProjectile::~CBloggProjectile() {}
