#include "MetroidPrime/Weapons/CAnnihilatorBeam.hpp"

#include "Kyoto/Particles/CElementGen.hpp"

CAnnihilatorBeam::CAnnihilatorBeam(TUniqueId playerId, const CVector3f& scale, int flags)
: CGunWeapon(kWT_Annihilator, playerId, scale, flags) {}

CAnnihilatorBeam::~CAnnihilatorBeam() {}

void CAnnihilatorBeam::ReInitVariables() {}

void CAnnihilatorBeam::PostRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) {}

void CAnnihilatorBeam::UpdateGunFx(bool shotSmoke, float dt, const CStateManager& mgr,
                                   const CTransform4f& xf) {}

void CAnnihilatorBeam::Update(float dt, CStateManager& mgr) {}

void CAnnihilatorBeam::Fire(const TToken< CWeaponDescription >& projectile, bool underwater,
                            float dt, CPlayerState::EChargeStage chargeState,
                            const CTransform4f& xf, CStateManager& mgr, TUniqueId homingTarget,
                            uint projectileAttributes, ushort soundId, TUniqueId* projectileId,
                            CSfxHandle* soundHandle, float chargeFactor1, float chargeFactor2) {}

void CAnnihilatorBeam::Load(CStateManager& mgr, bool subtypeBasePose) {}

void CAnnihilatorBeam::Unload(CStateManager& mgr) {}

void CAnnihilatorBeam::ReleaseResources(CStateManager& mgr) {}

bool CAnnihilatorBeam::IsLoaded() const {}

void CAnnihilatorBeam::EnableSecondaryFx(ESecondaryFxType type) {}

void CAnnihilatorBeam::InitializeResources(CStateManager& mgr) {}

void CAnnihilatorBeam::SetWorldLighting(CStateManager& mgr, TAreaId areaId, float speed,
                                        float target) {}

void CAnnihilatorBeam::ResetWorldLighting(CStateManager& mgr) {}

void CAnnihilatorBeam::FireProjectile(const TCachedToken< CWeaponDescription >& projectile,
                                      bool underwater, float dt,
                                      CPlayerState::EChargeStage chargeState,
                                      const CTransform4f& xf, CStateManager& mgr,
                                      TUniqueId homingTarget, uint projectileAttributes,
                                      ushort soundId, float damageFactor, float projectileScale,
                                      float projectileFactor) {}
