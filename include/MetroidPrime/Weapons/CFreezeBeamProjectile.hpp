#ifndef _CFREEZEBEAMPROJECTILE
#define _CFREEZEBEAMPROJECTILE

#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"

class CFreezeBeamProjectile : public CEnergyProjectile {
public:
  CFreezeBeamProjectile(const TToken< CWeaponDescription >& description, EWeaponType type,
                        const CTransform4f& xf, EMaterialTypes excludeMaterial,
                        const CDamageInfo& damage, TUniqueId uid, TAreaId areaId, TUniqueId owner,
                        float freezeDuration, float iceImpactBoundScale, ushort impactSfx,
                        CAssetId iceImpactParticleId, CAssetId steamTextureId);

  // CEntity
  ~CFreezeBeamProjectile() override;

  // CGameProjectile
  void ApplyDamageToOneActor(CStateManager& mgr, const CDamageInfo& damage, TUniqueId id,
                             const CVector3f& direction) override;

  // CEnergyProjectile
  bool Explode(const CVector3f& position, const CVector3f& normal,
               EWeaponCollisionResponseTypes type, CStateManager& mgr,
               const CDamageVulnerability& vulnerability, TUniqueId hitActor) override;

private:
  // Guessed names: meanings established by the freeze and ice-impact consumers.
  float mFreezeDuration;
  ushort mImpactSfx;
  CAssetId mIceImpactParticleId;
  CAssetId mSteamTextureId;
  rstl::optional_object< TLockedToken< CGenDescription > > mIceImpactParticle;
  float mIceImpactBoundScale;
};
CHECK_SIZEOF(CFreezeBeamProjectile, 0x590)

#endif // _CFREEZEBEAMPROJECTILE
