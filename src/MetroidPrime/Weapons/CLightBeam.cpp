#include "MetroidPrime/Weapons/CLightBeam.hpp"

#include "Kyoto/Particles/CElementGen.hpp"

CLightBeam::CLightBeam(TUniqueId playerId, const CVector3f& scale, int flags)
: CGunWeapon(kWT_Light, playerId, scale, flags) {}

CLightBeam::~CLightBeam() {}

void CLightBeam::ReInitVariables() {}

void CLightBeam::PostRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) {}

void CLightBeam::UpdateGunFx(bool shotSmoke, float dt, const CStateManager& mgr,
                             const CTransform4f& xf) {}

void CLightBeam::Update(float dt, CStateManager& mgr) {}

void CLightBeam::Fire(const TCachedToken< CWeaponDescription >& projectile, bool underwater,
                      float dt, CPlayerState::EChargeStage chargeState, const CTransform4f& xf,
                      CStateManager& mgr, TUniqueId homingTarget, uint projectileAttributes,
                      ushort soundId, TUniqueId* projectileId, CSfxHandle* soundHandle,
                      float chargeFactor1, float chargeFactor2) {}

void CLightBeam::Load(CStateManager& mgr, bool subtypeBasePose) {}

void CLightBeam::Unload(CStateManager& mgr) {}

void CLightBeam::ReleaseResources(CStateManager& mgr) {}

bool CLightBeam::IsLoaded() const {}

void CLightBeam::EnableSecondaryFx(ESecondaryFxType type) {}

void CLightBeam::InitializeResources(CStateManager& mgr) {}
