#ifndef _CPOWERBEAM
#define _CPOWERBEAM

#include "types.h"

#include "MetroidPrime/Weapons/CGunWeapon.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/single_ptr.hpp"

class CPowerBeam : public CGunWeapon {
public:
  CPowerBeam(TUniqueId playerId, const CVector3f& scale, int unk);

  // CGunWeapon
  ~CPowerBeam();

  void PreRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) override;
  void PostRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) override;
  void UpdateGunFx(bool shotSmoke, float dt, const CStateManager& mgr,
                   const CTransform4f& xf) override;
  void Fire(const TCachedToken< CWeaponDescription >& projectile, bool underwater, float dt,
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
  enum ESmokeState { kSS_Inactive, kSS_Active, kSS_Done };
  rstl::optional_object< TCachedToken< CGenDescription > > mShotSmoke;
  rstl::optional_object< TCachedToken< CGenDescription > > mPower2nd1;
  rstl::single_ptr< CElementGen > mShotSmokeGen;
  rstl::single_ptr< CElementGen > mPower2ndGen;
  float mSmokeTimer;
  ESmokeState mSmokeState;
  bool x244_24 : 1;
  bool mLoaded : 1;

  void ReInitVariables();
};
// CHECK_SIZEOF(CPowerBeam, 0x2a8)

#endif // _CPOWERBEAM
