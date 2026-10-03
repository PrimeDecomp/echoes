#include "MetroidPrime/Weapons/CFreezeBeamProjectile.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CIceImpact.hpp"

CFreezeBeamProjectile::CFreezeBeamProjectile(const TToken< CWeaponDescription >& description,
                                             EWeaponType type, const CTransform4f& xf,
                                             EMaterialTypes excludeMaterial,
                                             const CDamageInfo& damage, TUniqueId uid,
                                             TAreaId areaId, TUniqueId owner, float freezeDuration,
                                             float iceImpactBoundScale, ushort impactSfx,
                                             CAssetId iceImpactParticleId, CAssetId steamTextureId)
: CEnergyProjectile(true, description, type, xf, excludeMaterial, damage, uid, areaId, owner,
                    kInvalidUniqueId, 0, false, CVector3f::One(), CImpactVisorEffect(), false, true,
                    false, 1.f, 4.f, 4.f)
, mFreezeDuration(freezeDuration)
, mImpactSfx(impactSfx)
, mIceImpactParticleId(iceImpactParticleId)
, mSteamTextureId(steamTextureId)
, mIceImpactBoundScale(iceImpactBoundScale) {
  if (mIceImpactParticleId != kInvalidAssetId) {
    mIceImpactParticle = TLockedToken< CGenDescription >(
        gpSimplePool->GetObj(SObjectTag('PART', mIceImpactParticleId)));
  }
}

bool CFreezeBeamProjectile::Explode(const CVector3f& position, const CVector3f& normal,
                                    EWeaponCollisionResponseTypes type, CStateManager& mgr,
                                    const CDamageVulnerability& vulnerability, TUniqueId hitActor) {
  if (mIceImpactParticle.valid()) {
    CIceImpact* impact = rs_new CIceImpact(
        *mIceImpactParticle, mgr.AllocateUniqueId(), GetCurrentAreaId(), GetOwnerId(), true,
        rstl::string_l("Freeze Beam Ice Impact"), GetTransform(), 0, CVector3f(1.f, 1.f, 1.f),
        CColor(1.f, 1.f, 1.f, 1.f), mIceImpactBoundScale);
    mgr.AddObject(impact);
  }

  CSfxManager::AddEmitter(mImpactSfx, GetTranslation(), 127, GetCurrentAreaId().Value(), false,
                          false, CSfxManager::kMedPriority);
  return CEnergyProjectile::Explode(position, normal, type, mgr, vulnerability, hitActor);
}

void CFreezeBeamProjectile::ApplyDamageToOneActor(CStateManager& mgr, const CDamageInfo& damage,
                                                  TUniqueId id, const CVector3f& direction) {
  CGameProjectile::ApplyDamageToOneActor(mgr, damage, id, direction);
  if (CActor* actor = TCastToPtr< CActor >(mgr.GetObjectByIdFromListAll(id))) {
    if (CPlayer* player = TCastToPtr< CPlayer >(actor)) {
      player->Freeze(mFreezeDuration, mgr, mSteamTextureId, CSfxManager::kInternalInvalidSfxId,
                     kInvalidAssetId);
    }
  }
}

CFreezeBeamProjectile::~CFreezeBeamProjectile() {}
