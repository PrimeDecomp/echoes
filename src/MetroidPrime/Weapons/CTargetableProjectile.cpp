#include "MetroidPrime/Weapons/CTargetableProjectile.hpp"

#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TCastTo.hpp"

CTargetableProjectile::CTargetableProjectile(
    const TToken< CWeaponDescription >& description, EWeaponType type, const CTransform4f& xf,
    EMaterialTypes excludeMaterial, const CDamageInfo& damage, const CDamageInfo& deflectedDamage,
    TUniqueId uid, TAreaId areaId, TUniqueId owner,
    const TToken< CWeaponDescription >& deflectedDescription, TUniqueId homingTarget, uint attribs,
    const CImpactVisorEffect& visorEffect, const CVector3f& scale)
: CEnergyProjectile(true, description, type, xf, excludeMaterial, damage, uid, areaId, owner,
                    homingTarget,
                    attribs | kPA_PartialCharge | ::kPA_PlasmaProjectile | kPA_BigProjectile, false,
                    scale, visorEffect, false, true, false, 1.f, 4.f, 4.f)
, mDeflectedWeaponDescription(deflectedDescription)
, mDeflectedDamage(deflectedDamage)
, mDeflectToOwner(true) {
  MaterialList().Add(kMT_Target);
  MaterialList().Add(kMT_Orbit);
  MaterialList().Add(kMT_Unknown46);

  const CMaterialFilter& oldFilter = GetMaterialFilter();
  CMaterialList excluded = oldFilter.GetExcludeList();
  excluded.Add(kMT_Unknown46);
  SetMaterialFilter(CMaterialFilter(oldFilter.GetIncludeList(), excluded, oldFilter.GetType()));
}

CTargetableProjectile::~CTargetableProjectile() {}

void CTargetableProjectile::ResolveCollisionWithActor(const CRayCastResult& result, CActor& actor,
                                                      CStateManager& mgr) {
  if (TCastToPtr< CTargetableProjectile >(actor) != nullptr) {
    return;
  }

  const CVector3f aimPosition = GetAimPosition(mgr, 0.1f);
  CTransform4f xf = CTransform4f::LookAt(GetTranslation(), aimPosition, CVector3f::Up());
  xf.SetTranslation(GetTranslation());
  SetTransform(xf);
  CEnergyProjectile::ResolveCollisionWithActor(result, actor, mgr);
}

CVector3f CTargetableProjectile::GetAimPosition(const CStateManager& mgr, float dt) const {
  static float tickRecip = 1.f / CProjectileWeapon::GetTickTime();
  const CVector3f translation = GetTranslation();
  const CVector3f velocity = tickRecip * mProjectile.GetVelocity();
  const CVector3f gravity = tickRecip * mProjectile.GetGravity();
  return dt * (dt * (gravity * 0.5f)) + dt * velocity + translation;
}

bool CTargetableProjectile::Explode(const CVector3f& position, const CVector3f& normal,
                                    EWeaponCollisionResponseTypes type, CStateManager& mgr,
                                    const CDamageVulnerability& vulnerability, TUniqueId hitActor) {
  const bool exploded =
      CEnergyProjectile::Explode(position, normal, type, mgr, vulnerability, hitActor);
  if (!GetWeaponActive() && mDeflectToOwner) {
    const TUniqueId projectileOwner = mHitProjectileOwner;
    CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(projectileOwner));
    if (player != nullptr) {
      const CActor* const actor = TCastToConstPtr< CActor >(mgr.GetObjectById(GetOwnerId()));
      if (actor != nullptr) {
        const TUniqueId uid = mgr.AllocateUniqueId();
        const CVector3f aimPosition = actor->GetAimPosition(mgr, 0.f);
        CEnergyProjectile* projectile = rs_new CEnergyProjectile(
            true, mDeflectedWeaponDescription, GetType(),
            CTransform4f::LookAt(mProjectile.GetTranslation(), aimPosition, CVector3f::Up()),
            kMT_Player, mDeflectedDamage, uid, GetCurrentAreaId(), projectileOwner, GetOwnerId(), 0,
            false, CVector3f::One(), CImpactVisorEffect::None(), false, true, false, 1.f, 4.f, 4.f);
        mgr.AddObject(*projectile);
        projectile->AddMaterial(kMT_Orbit, mgr);
        player->SetAimTarget(uid);
        player->SetOrbitTargetId(uid, mgr);
        mHitProjectileOwner = kInvalidUniqueId;
      }
    }
  }
  return exploded;
}
