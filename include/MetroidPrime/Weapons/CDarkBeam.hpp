#ifndef _CDARKBEAM
#define _CDARKBEAM

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "MetroidPrime/Weapons/CGunWeapon.hpp"

// Guessed class name
class CDarkBeam : public CGunWeapon {
public:
  CDarkBeam(TUniqueId playerId, const CVector3f& scale, int flags);

  // CGunWeapon
  ~CDarkBeam() override;
  void PreRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) override;
  void PostRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) override;
  void UpdateGunFx(bool shotSmoke, float dt, const CStateManager& mgr,
                   const CTransform4f& xf) override;
  void Fire(const TCachedToken< CWeaponDescription >& projectile, bool underwater, float dt,
            CPlayerState::EChargeStage chargeState, const CTransform4f& xf, CStateManager& mgr,
            TUniqueId homingTarget, uint projectileAttributes, ushort soundId,
            TUniqueId* projectileId, CSfxHandle* soundHandle, float chargeFactor1,
            float chargeFactor2) override;
  void EnableFx(bool enable) override;
  void EnableSecondaryFx(ESecondaryFxType type) override;
  void Update(float dt, CStateManager& mgr) override;
  void InitializeResources(CStateManager& mgr) override; // Guessed name
  void Load(CStateManager& mgr, bool subtypeBasePose) override;
  void Unload(CStateManager& mgr) override;
  bool IsLoaded() const override;
  void ReleaseResources(CStateManager& mgr) override; // Guessed name

private:
  rstl::optional_object< TCachedToken< CGenDescription > > mSmokeEffect;  // Guessed name
  rstl::optional_object< TCachedToken< CGenDescription > > mChargeEffect; // Guessed name
  rstl::optional_object< TCachedToken< CGenDescription > > mEndEffect;    // Guessed name
  rstl::single_ptr< CElementGen > mSmokeGenerator;                        // Guessed name
  rstl::single_ptr< CElementGen > mChargeGenerator;                       // Guessed name
  CSfxHandle mChargedShotSound;                                           // Guessed name
  TUniqueId mChargedProjectileId;                                         // Guessed name
  bool mEffectsLoaded : 1;                                                // Guessed name
  bool mInEndEffect : 1;                                                  // Guessed name

  void ReInitVariables(); // Guessed name
};
CHECK_SIZEOF(CDarkBeam, 0x2b4)

#endif // _CDARKBEAM
