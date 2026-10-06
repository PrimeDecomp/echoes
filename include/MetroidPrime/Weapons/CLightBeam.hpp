#ifndef _CLIGHTBEAM
#define _CLIGHTBEAM

#include "MetroidPrime/Weapons/CGunWeapon.hpp"

// Guessed class name
class CLightBeam : public CGunWeapon {
public:
  CLightBeam(TUniqueId playerId, const CVector3f& scale, int flags);

  // CGunWeapon
  ~CLightBeam() override;
  void PostRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) override;
  void UpdateGunFx(bool shotSmoke, float dt, const CStateManager& mgr,
                   const CTransform4f& xf) override;
  void Fire(const TToken< CWeaponDescription >& projectile, bool underwater, float dt,
            CPlayerState::EChargeStage chargeState, const CTransform4f& xf, CStateManager& mgr,
            TUniqueId homingTarget, uint projectileAttributes, ushort soundId,
            TUniqueId* projectileId, CSfxHandle* soundHandle, float chargeFactor1,
            float chargeFactor2) override;
  void EnableSecondaryFx(ESecondaryFxType type) override;
  void Update(float dt, CStateManager& mgr) override;
  void InitializeResources(CStateManager& mgr) override;
  void Load(CStateManager& mgr, bool subtypeBasePose) override;
  void Unload(CStateManager& mgr) override;
  bool IsLoaded() const override;
  void ReleaseResources(CStateManager& mgr) override;

private:
  rstl::optional_object< TToken< CWeaponDescription > > mChargedProjectiles[3]; // Guessed name
  rstl::optional_object< TCachedToken< CGenDescription > > mSecondaryEffect; // Guessed name
  rstl::single_ptr< CElementGen > mSecondaryGenerator; // Guessed name

  void ReInitVariables(); // Guessed name
};
CHECK_SIZEOF(CLightBeam, 0x2ac)

#endif // _CLIGHTBEAM
