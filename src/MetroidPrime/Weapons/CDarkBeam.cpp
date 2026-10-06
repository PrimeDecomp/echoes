#include "MetroidPrime/Weapons/CDarkBeam.hpp"

#include "Kyoto/Particles/CElementGen.hpp"

CDarkBeam::CDarkBeam(TUniqueId playerId, const CVector3f& scale, int flags)
: CGunWeapon(kWT_Dark, playerId, scale, flags), mChargedProjectileId(kInvalidUniqueId) {}

CDarkBeam::~CDarkBeam() {}

void CDarkBeam::ReInitVariables() {}

void CDarkBeam::PostRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) {}

void CDarkBeam::UpdateGunFx(bool shotSmoke, float dt, const CStateManager& mgr,
                            const CTransform4f& xf) {}

void CDarkBeam::Update(float dt, CStateManager& mgr) {}

void CDarkBeam::Fire(const TToken< CWeaponDescription >& projectile, bool underwater,
                     float dt, CPlayerState::EChargeStage chargeState, const CTransform4f& xf,
                     CStateManager& mgr, TUniqueId homingTarget, uint projectileAttributes,
                     ushort soundId, TUniqueId* projectileId, CSfxHandle* soundHandle,
                     float chargeFactor1, float chargeFactor2) {}

void CDarkBeam::Load(CStateManager& mgr, bool subtypeBasePose) {}

void CDarkBeam::Unload(CStateManager& mgr) {}

void CDarkBeam::ReleaseResources(CStateManager& mgr) {}

bool CDarkBeam::IsLoaded() const {}

void CDarkBeam::EnableSecondaryFx(ESecondaryFxType type) {}

void CDarkBeam::InitializeResources(CStateManager& mgr) {}

void CDarkBeam::PreRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) {}

void CDarkBeam::EnableFx(bool enable) {}
