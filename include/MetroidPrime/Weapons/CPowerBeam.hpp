#ifndef _CPOWERBEAM
#define _CPOWERBEAM

#include "types.h"

#include "MetroidPrime/Weapons/CGunWeapon.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/single_ptr.hpp"

class CPowerBeam : public CGunWeapon {
public:
  CPowerBeam(TUniqueId playerId, const CVector3f& scale, int unk);
  ~CPowerBeam();

  void PreRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) override;
  void PostRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) override;
  void UpdateGunFx(bool shotSmoke, float dt, const CStateManager& mgr, const CTransform4f& xf) override;
  void Fire(CToken& token, bool underwater, float dt, CPlayerState::EChargeStage chargeState,
                      const CTransform4f& xf, CStateManager& mgr, TUniqueId homingTarget,
                      int unk1, ushort unk2, TUniqueId id, CSfxHandle sfx, float chargeFactor1, float chargeFactor2) override;
  void EnableSecondaryFx(ESecondaryFxType type) override;
  void Update(float dt, CStateManager& mgr) override;
  void Load(CStateManager& mgr, bool subtypeBasePose) override;
  void Unload(CStateManager& mgr) override;
  bool IsLoaded() const override;

  void Unk9(CStateManager& mgr) override;
  void Unk11(CStateManager& mgr) override;

private:
  enum ESmokeState { kSS_Inactive, kSS_Active, kSS_Done };
  rstl::optional_object< TCachedToken<CGenDescription> > x21c_shotSmoke;
  rstl::optional_object< TCachedToken<CGenDescription> > x228_power2nd1;
  rstl::single_ptr<CElementGen> x234_shotSmokeGen;
  rstl::single_ptr<CElementGen> x238_power2ndGen;
  float x23c_smokeTimer;
  ESmokeState x240_smokeState;
  bool x244_24 : 1;
  bool x244_25_loaded : 1;

  void ReInitVariables();
};
// CHECK_SIZEOF(CPowerBeam, 0x2a8)

#endif // _CPOWERBEAM
