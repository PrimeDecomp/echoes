#ifndef _CANNIHILATORBEAM
#define _CANNIHILATORBEAM

#include "MetroidPrime/Weapons/CGunWeapon.hpp"

// Guessed class name
class CAnnihilatorBeam : public CGunWeapon {
public:
  CAnnihilatorBeam(TUniqueId playerId, const CVector3f& scale, int flags);

  // CGunWeapon
  ~CAnnihilatorBeam() override;
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
  void InitializeResources(CStateManager& mgr) override; // Guessed name
  void Load(CStateManager& mgr, bool subtypeBasePose) override;
  void Unload(CStateManager& mgr) override;
  bool IsLoaded() const override;
  void ReleaseResources(CStateManager& mgr) override; // Guessed name

private:
  rstl::optional_object< TCachedToken< CGenDescription > > mChargeEffect; // Guessed name
  rstl::single_ptr< CElementGen > mChargeGenerator;                       // Guessed name
  float mShotDelayTimer;                                                  // Guessed name
  float mShotDelay;                                                       // Guessed name
  float mLightingResetDelayTimer;                                         // Guessed name
  float mProjectileSpeed;                                                 // Guessed name
  float mProjectileTurnRate;                                              // Guessed name
  TAreaId mLightingArea;                                                  // Guessed name
  bool mEffectLoaded : 1;                                                 // Guessed name
  bool mWorldLightingDimmed : 1;                                          // Guessed name

  void SetWorldLighting(CStateManager& mgr, TAreaId areaId, float speed,
                        float target);         // Guessed name
  void ResetWorldLighting(CStateManager& mgr); // Guessed name
  // Guessed name
  void FireProjectile(const TToken< CWeaponDescription >& projectile, bool underwater, float dt,
                      CPlayerState::EChargeStage chargeState, const CTransform4f& xf,
                      CStateManager& mgr, TUniqueId homingTarget, uint projectileAttributes,
                      ushort soundId, float damageFactor, float projectileScale,
                      float projectileFactor);

  void ReInitVariables(); // Guessed name
};
CHECK_SIZEOF(CAnnihilatorBeam, 0x2a4)

#endif // _CANNIHILATORBEAM
