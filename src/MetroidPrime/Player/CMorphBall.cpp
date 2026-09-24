#include "MetroidPrime/Player/CMorphBall.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Particles/CDeferredParticleEffect.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CParticleSwoosh.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CRainSplashGenerator.hpp"
#include "MetroidPrime/CWorldShadow.hpp"
#include "MetroidPrime/Player/CMorphBallShadow.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"

#include "rstl/math.hpp"

// Structure-first reconstruction. TODO bodies below are scaffolds, not equivalent implementations.

// Guessed names for TU-local state (G2ME01 80419098 and 8040d568).
static float sBallCloseToCollisionDistance;
static rstl::reserved_vector< int, 64 > sWakeEffectForMaterial;

// 800c02a4
void CMorphBall::DeleteBallShadow() { x1908_shadow = nullptr; }

// 800c02e0
void CMorphBall::CreateBallShadow() {
  if (!x1908_shadow.get()) {
    x1908_shadow = rs_new CMorphBallShadow(64, 64, gpSimplePool->GetObj("TXTR_BallFade"));
  }
}

// 800c03dc -- Scaffold, not a reconstructed implementation.
void CMorphBall::RenderToShadowTex(CStateManager& mgr) {
  // TODO: Gather shadow receivers and render CMorphBallShadow with the player texture.
}

// 800c04d4 -- Scaffold, not a reconstructed implementation.
void CMorphBall::DrawBallShadow(CStateManager& mgr) {
  // TODO: Draw the projected ball shadow against the gathered world/actor receivers.
}

// 800c05cc -- Guessed name.
void CMorphBall::InitializeWakeEffects() {
  sWakeEffectForMaterial.resize(64, -1);
  sWakeEffectForMaterial[kMT_Phazon] = 0;
  sWakeEffectForMaterial[kMT_Dirt] = 2;
  sWakeEffectForMaterial[kMT_Organic] = 3;
  sWakeEffectForMaterial[kMT_Sand] = 4;
  const char* effects[] = {"PhazonWake",  "PhazonWakeOrange", "DirtWake",
                           "OrganicWake", "SandWake",         "RainWake"};
  const char* groups[] = {"PhazonWake_DGRP",  "PhazonWakeOrange_DGRP", "DirtWake_DGRP",
                          "OrganicWake_DGRP", "SandWake_DGRP",         "RainWake_DGRP"};
  for (int i = 0; i < 6; ++i) {
    const CDependencyGroupToken group(gpSimplePool->GetObj(groups[i]), *gpSimplePool);
    xe10_wakeEffects.push_back(rstl::auto_ptr< CDeferredParticleEffect >(
        rs_new CDeferredParticleEffect(gpSimplePool->GetObj(effects[i]), group)));
  }
}

// 800c08c0 -- Guessed name.
void CMorphBall::ResetScrewAttackExitAnimationTimer() { x18b4_screwAttackExitAnimationFrames = 5; }

// 800c08cc -- Guessed name.
int CMorphBall::GetScrewAttackGroundedFrames() const { return x18b8_screwAttackGroundedFrames; }

// 800c08d4
bool CMorphBall::InScrewAttackMode() const {
  return xc80_ballState == kBS_ScrewAttack || xc80_ballState == kBS_ScrewAttackWallJump ||
         xc80_ballState == kBS_ScrewAttackRecovery;
}

// 800c08fc
bool CMorphBall::IsProjectile() const { return xc80_ballState == kBS_Projectile; }

// 800c0910
bool CMorphBall::IsBoostShieldActive() const { return x1024_timeNotInBoost < 1.f; }

// 800c0928
float CMorphBall::GetBoostChargeTimer() const { return x1020_boostChargeTime; }

// 800c0930
float CMorphBall::GetTimeNotInBoost() const { return x1024_timeNotInBoost; }

// 800c0938
bool CMorphBall::IsBoosting() const {
  return xc80_ballState == kBS_Boost || xc80_ballState == kBS_SpiderBoost;
}

// 800c0958 -- Guessed name; workspace type name is a cross-game hypothesis.
void CMorphBall::PointGenerator(const CSkinnedModel& model, const SSkinningWorkspace& workspace,
                                void* context) {
  if (context) {
    static_cast< CRainSplashGenerator* >(context)->GeneratePoints(model, workspace);
  }
}

// 800c0994 -- Scaffold, not a reconstructed implementation.
void CMorphBall::StartLandingSfx() {
  // TODO: Choose and play the landing sound from velocity and selected surface material.
}

// 800c0a68
void CMorphBall::StopSounds() {
  if (x1890_rollSfx) {
    CSfxManager::SfxStop(x1890_rollSfx);
    x1890_rollSfx.Clear();
  }
  if (x1894_spiderSfx) {
    CSfxManager::SfxStop(x1894_spiderSfx);
    x1894_spiderSfx.Clear();
  }
  if (x1898_deathBallSfx) {
    CSfxManager::SfxStop(x1898_deathBallSfx);
    x1898_deathBallSfx.Clear();
  }
  if (x189c_screwAttackSfx) {
    CSfxManager::SfxStop(x189c_screwAttackSfx);
    x189c_screwAttackSfx.Clear();
  }
}

// 800c0b20 -- Scaffold, not a reconstructed implementation.
void CMorphBall::StartScrewAttackSfx() {
  // TODO: Start the Screw Attack sound with the player's sound-channel settings.
}

// 800c0ba4 -- Scaffold, not a reconstructed implementation.
void CMorphBall::UpdateMorphBallSound(float dt, CStateManager& mgr) {
  // TODO: Maintain the roll, Spider, death-ball and Screw Attack emitters.
}

// 800c0ff4 -- Scaffold, not a reconstructed implementation.
void CMorphBall::SelectMorphBallSounds(const CMaterialList& material) {
  // TODO: Select roll/landing sound IDs for collision material and multiplayer mode.
}

// 800c1170
void CMorphBall::TakeDamage(float damage) {
  if (damage <= 0.f) {
    x18f8_damageEffect = 0.f;
    x18fc_damageEffectDecaySpeed = 0.f;
    return;
  }

  if (damage >= 20.f) {
    x18fc_damageEffectDecaySpeed = 0.25f;
  } else if (damage > 5.f) {
    x18fc_damageEffectDecaySpeed = 1.f - 0.75f * ((damage - 5.f) / 15.f);
  } else {
    x18fc_damageEffectDecaySpeed = 1.f;
  }
  x18f8_damageEffect = 1.f;
}

// 800c11e8
CMorphBall::EBombJumpState CMorphBall::GetBombJumpState() const { return x18f4_bombJumpState; }

// 800c11f0
void CMorphBall::SetBallBoostState(EBallBoostState state) { x18f0_boostState = state; }

// 800c11f8
CMorphBall::EBallBoostState CMorphBall::GetBallBoostState() const { return x18f0_boostState; }

// 800c1200
void CMorphBall::SetAsProjectile(bool projectile) {
  if (projectile) {
    xc80_ballState = kBS_Projectile;
  } else if (xc80_ballState == kBS_Projectile) {
    xc80_ballState = kBS_Normal;
  }
}

// 800c122c
void CMorphBall::TouchModel(const CStateManager& mgr) const {
  x58_ballModel->Touch(mgr, x5c_ballModelShader);
  if (x0_player.GetPlayerState()->HasPowerUp(CPlayerState::kIT_SpiderBall) &&
      x60_spiderBallGlassModel.get()) {
    x60_spiderBallGlassModel->Touch(mgr, x64_spiderBallGlassModelShader);
  }
  x68_lowPolyBallModel->Touch(mgr, x6c_lowPolyBallModelShader);
}

// 800c12ac
CModelData* CMorphBall::GetMorphBallModel(const rstl::string& name, float radius) {
  if (name == rstl::string("")) {
    return nullptr;
  }
  const SObjectTag* tag = gpResourceFactory->GetResourceIdByName(name.data());
  const CVector3f scale(2.f * radius, 2.f * radius, 2.f * radius);
  if (tag->type == 'CMDL') {
    return rs_new CModelData(CStaticRes(tag->id, scale));
  }
  return rs_new CModelData(CAnimRes(tag->id, CAnimRes::kDefaultCharIdx, scale, 0, false));
}

// 800c13ec -- Scaffold, not a reconstructed implementation.
void CMorphBall::LoadMorphBallModel() {
  // TODO: Select normal/spider/boost resources and glow colors for the three Echoes suits.
}

// 800c1718 -- Scaffold, not a reconstructed implementation.
void CMorphBall::FluidFXThink(CActor::EFluidState state, CScriptWater& water, CStateManager& mgr) {
  // TODO: Rate-limit water splashes using speed, fluid state and the fluid-plane manager.
}

// 800c1864
bool CMorphBall::IsClimbable(const CCollisionInfo& collision) const {
  if (CMath::AbsF(collision.GetNormalLeft().GetZ()) < 0.7f) {
    const float height = GetBallPosition().GetZ() - collision.GetPoint().GetZ();
    return height > 0.1f && height < GetBallRadius() - 0.05f;
  }
  return false;
}

// 800c1908 -- The original body is genuinely empty.
void CMorphBall::Touch(CActor& actor, CStateManager& mgr) {}

// 800c190c -- Scaffold, not a reconstructed implementation.
float CMorphBall::ComputeMaxSpeed() const {
  // TODO: Use the surface-restraint tweak or clamp the half-pipe velocity-derived limit.
  return 0.f;
}

// 800c19a4
void CMorphBall::SpinToSpeed(float speed, const CVector3f& direction, float dt) {
  const float angularSpeed = x0_player.GetAngularVelocityWR().GetVector().Magnitude();
  x0_player.ApplyTorqueWR(dt * (speed - angularSpeed) * direction);
}

// 800c1a64
void CMorphBall::ApplyGravity() {
  x0_player.SetMomentumWR(CVector3f(0.f, 0.f, x0_player.GetMass() * GetGravityAcceleration()));
}

// 800c1ac0 -- Scaffold, not a reconstructed implementation.
float CMorphBall::GetGravityAcceleration() const {
  // TODO: Select normal/water/Screw Attack/wall-jump gravity from CTweakBall.
  return 0.f;
}

// 800c1b50 -- Scaffold, not a reconstructed implementation.
float CMorphBall::CalculateSurfaceFriction() const {
  // TODO: Use the surface-restraint tweak, attachment state and energy-drain count.
  return 0.f;
}

// 800c1be8 -- Scaffold, not a reconstructed implementation.
void CMorphBall::ComputeLiftForces(const CVector3f& controlForce, const CVector3f& velocity,
                                   const CStateManager& mgr) {
  // TODO: Update the lift averages and apply the contact-dependent upward force.
}

// 800c22f8
CAABox CMorphBall::GetRenderBounds(const CStateManager& mgr) const {
  const CVector3f center = GetBallPosition();
  const CVector3f extent(2.f * xc_radius, 2.f * xc_radius, 2.f * xc_radius);
  CAABox bounds(center - extent, center + extent);
  if (xdc8_slowBlueTailSwooshGen->GetModulationColor().GetAlpha() != 0.f) {
    const rstl::optional_object< CAABox > trailBounds = xdc8_slowBlueTailSwooshGen->GetBounds();
    if (trailBounds.valid()) {
      bounds.AccumulateBounds(trailBounds->GetMinPoint());
      bounds.AccumulateBounds(trailBounds->GetMaxPoint());
    }
  }
  return bounds;
}

// 800c2478 -- Scaffold, not a reconstructed implementation.
void CMorphBall::CollidedWith(const TUniqueId& id, const CCollisionInfoList& collisions,
                              CStateManager& mgr) {
  // TODO: Process contact materials/normals, boost damage, half-pipe and Screw Attack collisions.
}

// 800c3438 -- Scaffold, not a reconstructed implementation.
bool CMorphBall::BallCloseToCollision(const CStateManager& mgr, float distance,
                                      const CMaterialFilter& filter) const {
  // TODO: Test a swept sphere against the world's filtered collision geometry.
  return false;
}

// 800c36a0
void CMorphBall::DisableHalfPipeStatus() {
  SetIsInHalfPipeMode(false);
  SetIsInHalfPipeModeInAir(false);
  SetTouchedHalfPipeRecently(false);
  x185c_touchHalfPipeCooldown = 0.f;
  x1860_disableControlCooldown = 0.f;
  x0_player.SetCollisionAccuracyModifier(5.f);
  x1868_prevHalfPipeNormal = CVector3f::Zero();
  x1874_halfPipeNormal = CVector3f::Zero();
}

// 800c3734
void CMorphBall::SetTouchedHalfPipeRecently(bool touched) {
  x1854_26_touchedHalfPipeRecently = touched;
}

// 800c3744
bool CMorphBall::GetTouchedHalfPipeRecently() const { return x1854_26_touchedHalfPipeRecently; }

// 800c3750
void CMorphBall::SetIsInHalfPipeModeInAir(bool active) { x1854_25_inHalfPipeModeInAir = active; }

// 800c3760
bool CMorphBall::GetIsInHalfPipeModeInAir() const { return x1854_25_inHalfPipeModeInAir; }

// 800c376c
void CMorphBall::SetIsInHalfPipeMode(bool active) { x1854_24_inHalfPipeMode = active; }

// 800c377c
bool CMorphBall::GetIsInHalfPipeMode() const { return x1854_24_inHalfPipeMode; }

// 800c3788 -- Scaffold, not a reconstructed implementation.
void CMorphBall::UpdateHalfPipeStatus(CStateManager& mgr, float dt) {
  // TODO: Expire half-pipe/contact cooldowns and adjust collision accuracy.
}

// 800c3920 -- Guessed name.
void CMorphBall::RenderScrewAttackJumpEffects() const {
  if (xe08_screwAttackJumpFlashGen.get()) {
    xe08_screwAttackJumpFlashGen->Render();
  }
  if (xe0c_screwAttackWallJumpFlashGen.get()) {
    xe0c_screwAttackWallJumpFlashGen->Render();
  }
}

// 800c3980 -- Scaffold, not a reconstructed implementation.
void CMorphBall::RenderDamageEffects(const CStateManager& mgr,
                                     const CTransform4f& transform) const {
  // TODO: Render the damage overlay using the ball's damage timer and glow color.
}

// 800c3b78
void CMorphBall::RenderIceBreakEffect(const CStateManager& mgr) const {
  if (xdfc_morphBallIceBreakGen.get()) {
    xdfc_morphBallIceBreakGen->Render();
  }
}

// 800c3bb0
void CMorphBall::UpdateIceBreakEffect(float dt) {
  if (!xdfc_morphBallIceBreakGen.get() && xda0_morphBallIceBreak.HasLock() &&
      xda0_morphBallIceBreak.IsLoaded()) {
    xdfc_morphBallIceBreakGen = rs_new CElementGen(xda0_morphBallIceBreak);
    xdfc_morphBallIceBreakGen->SetOrientation(x0_player.GetTransform().GetRotation());
  }
  if (xdfc_morphBallIceBreakGen.get()) {
    if (xdfc_morphBallIceBreakGen->IsSystemDeletable()) {
      xdfc_morphBallIceBreakGen = nullptr;
      xda0_morphBallIceBreak.Unlock();
    } else {
      xdfc_morphBallIceBreakGen->SetGlobalTranslation(GetBallPosition());
      xdfc_morphBallIceBreakGen->Update(dt);
    }
  }
}

// 800c3d64
void CMorphBall::ResetMorphBallIceBreak() {
  xda0_morphBallIceBreak.Lock();
  xdfc_morphBallIceBreakGen = nullptr;
}

// 800c3dbc
bool CMorphBall::IsMorphBallTransitionFlashValid() const {
  return xdf8_morphBallTransitionFlashGen.get() != nullptr;
}

// 800c3dd0 -- Scaffold, not a reconstructed implementation.
void CMorphBall::RenderMorphBallTransitionFlash(const CStateManager& mgr) const {
  // TODO: Apply the suit-dependent transition color before rendering the generator.
}

// 800c3e60
void CMorphBall::UpdateMorphBallTransitionFlash(float dt) {
  if (!xdf8_morphBallTransitionFlashGen.get() && xd98_morphBallTransitionFlash.HasLock() &&
      xd98_morphBallTransitionFlash.IsLoaded()) {
    xdf8_morphBallTransitionFlashGen = rs_new CElementGen(xd98_morphBallTransitionFlash);
    xdf8_morphBallTransitionFlashGen->SetOrientation(x0_player.GetTransform().GetRotation());
  }
  if (xdf8_morphBallTransitionFlashGen.get()) {
    if (xdf8_morphBallTransitionFlashGen->IsSystemDeletable()) {
      xdf8_morphBallTransitionFlashGen = nullptr;
      xd98_morphBallTransitionFlash.Unlock();
    } else {
      xdf8_morphBallTransitionFlashGen->SetGlobalTranslation(GetBallPosition());
      xdf8_morphBallTransitionFlashGen->Update(dt);
    }
  }
}

// 800c4014
void CMorphBall::ResetMorphBallTransitionFlash() {
  xd98_morphBallTransitionFlash.Lock();
  xdf8_morphBallTransitionFlashGen = nullptr;
}

// 800c406c -- Scaffold, not a reconstructed implementation.
void CMorphBall::Render(const CStateManager& mgr, const CActorLights* lights) const {
  // TODO: Render the ball, glass, trails and Echoes Screw Attack/death-ball effects.
}

// 800c50b4 -- Scaffold, not a reconstructed implementation.
void CMorphBall::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  // TODO: Prepare model animation, rain-splash point generation, actor lights and the world shadow.
}

// 800c55dc -- Scaffold, not a reconstructed implementation.
float CMorphBall::GetMinimumAlignmentSpeed() const {
  // TODO: Return zero in Spider mode; otherwise use the alignment-speed tweak.
  return 0.f;
}

// 800c5614
void CMorphBall::DampLinearAndAngularVelocities(float linearDamping, float angularDamping,
                                                float dt) {
  const float frames = 60.f * dt;
  const float linearScale = pow(1.f - linearDamping, frames);
  x0_player.SetVelocityWR(linearScale * x0_player.GetVelocityWR());
  const float angularScale = pow(1.f - angularDamping, frames);
  x0_player.SetAngularVelocityWR(x0_player.GetAngularVelocityWR() * angularScale);
}

// 800c5714
void CMorphBall::ApplyFriction(float friction) {
  CVector3f velocity = x0_player.GetVelocityWR();
  if (velocity.Magnitude() <= friction) {
    velocity = CVector3f::Zero();
  } else {
    velocity = (velocity.Magnitude() - friction) * velocity.AsNormalized();
  }
  x0_player.SetVelocityWR(velocity);
}

// 800c57e4 -- Scaffold, not a reconstructed implementation.
bool CMorphBall::UpdateMarbleDynamics(CStateManager& mgr, float dt, const CVector3f& point) {
  // TODO: Apply marble alignment, rolling torque and contact-force response.
  return false;
}

// 800c5f70 -- Scaffold, not a reconstructed implementation.
void CMorphBall::ApplyBoostBallDamage(CStateManager& mgr, TUniqueId id, const CDamageInfo& damage,
                                      float dt) {
  // TODO: Filter already-hit actors, scale damage and update the cooldown/history.
}

// 800c65b8
void CMorphBall::CancelBoosting() {
  x1020_boostChargeTime = 0.f;
  x102c_boostDrainTime = 0.f;
  if (x1880_ballAnimationIndex == 1) {
    x1880_ballAnimationIndex = 0;
    CSfxManager::SfxStop(x1884_boostChargeSfx);
    x1884_boostChargeSfx.Clear();
  }
}

// 800c6618
void CMorphBall::LeaveBoosting() {
  if (IsBoosting()) {
    x1020_boostChargeTime = 0.f;
    xc80_ballState = kBS_Normal;
  }
  x102c_boostDrainTime = 0.f;
}

// 800c6664 -- Scaffold, not a reconstructed implementation.
void CMorphBall::EnterBoosting(CStateManager& mgr, bool skipImpulse) {
  // TODO: Enter normal/spider boost, optionally apply the impulse, and reset damage history.
}

// 800c6b78 -- Scaffold, not a reconstructed implementation.
void CMorphBall::ComputeBoostBallMovement(const CFinalInput& input, CStateManager& mgr, float dt) {
  // TODO: Handle charge, release, draining, Spider Boost direction and damage.
}

// 800c76ec
void CMorphBall::SetScrewAttackActive(bool active) { x18a8_31_forcedScrewJumpInput = active; }

// 800c76fc -- Scaffold, not a reconstructed implementation.
void CMorphBall::ApplyScrewAttackDamage(float dt, CStateManager& mgr) {
  // TODO: Build the swept contact list and apply Screw Attack damage.
}

// 800c7a88 -- Scaffold, not a reconstructed implementation.
void CMorphBall::UpdateScrewAttackRecovery(float dt) {
  // TODO: Recover from recoil/collisions and request the player's exit animation.
}

// 800c7fb8 -- Scaffold, not a reconstructed implementation.
void CMorphBall::ComputeScrewAttackMovement(const CFinalInput& input, CStateManager& mgr,
                                            float dt) {
  // TODO: Handle jump/wall-jump input, speed/height limits and Screw Attack recovery.
}

// 800c891c -- Scaffold, not a reconstructed implementation.
void CMorphBall::UpdateDeathBall(float dt, CStateManager& mgr) {
  // TODO: Update the multiplayer death-ball effects and per-object damage cooldowns.
}

// 800c8dc8 -- Scaffold, not a reconstructed implementation.
void CMorphBall::UpdateBallLight(float dt, CStateManager& mgr) {
  // TODO: Update the inner-glow light from the generator and ball lighting state.
}

// 800c93f8 -- Scaffold, not a reconstructed implementation.
void CMorphBall::UpdateEffects(float dt, CStateManager& mgr) {
  // TODO: Update particle transforms, trail history, wake selection and light intensities.
}

// 800ca560
void CMorphBall::StopParticleWakes() {
  xde4_wallSparkGen->SetParticleEmission(false);
  for (int i = 0; i < xe10_wakeEffects.size(); ++i) {
    xe10_wakeEffects[i]->SetParticleEmission(false);
  }
}

// 800ca5cc
void CMorphBall::LeaveMorphBallState(CStateManager& mgr) {
  LeaveBoosting();
  CancelBoosting();
  CSfxManager::SfxStop(x1884_boostChargeSfx);
  x1888_boostReleaseSfx.Clear();
  StopParticleWakes();
}

// 800ca620 -- Scaffold, not a reconstructed implementation.
void CMorphBall::EnterMorphBallState(CStateManager& mgr, EBallState state) {
  // TODO: Reset ball/spider/boost state, sounds, averages and lighting for the requested mode.
}

// 800ca754
void CMorphBall::SetBallLightActive(CStateManager& mgr, bool active) {
  xe4a_ballLightActive = active;
}

// 800ca75c
void CMorphBall::DeleteLight(CStateManager& mgr) {
  if (xe48_ballInnerGlowLight != kInvalidUniqueId) {
    mgr.DeleteObjectRequest(xe48_ballInnerGlowLight);
    xe48_ballInnerGlowLight = kInvalidUniqueId;
  }
}

// 800ca7b0 -- Scaffold, not a reconstructed implementation.
bool CMorphBall::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                 EUserEventType type) {
  // TODO: Handle event 0x1b during morphed Screw Attack recovery through CPlayer.
  return false;
}

// 800ca808 -- Scaffold, not a reconstructed implementation.
void CMorphBall::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  // TODO: Handle Echoes CScriptMsg creation/deletion of the inner-glow light.
}

// 800ca9a4 -- Scaffold, not a reconstructed implementation.
void CMorphBall::Update(float dt, CStateManager& mgr) {
  // TODO: Update effects, light, death-ball state, tire interpolation, damage decay, rain and
  // sound.
}

// 800cab68
void CMorphBall::SwitchToTire() {
  x28_tireMode = true;
  xe64_tireInterpolating = true;
  x30_ballTiltAngle = 0.f;
  xe60_tireInterpolationSpeed = 1.f;
}

// 800cab88
void CMorphBall::SwitchToMarble() {
  const CUnitVector3f axis(x0_player.GetTransform().TransposeRotate(x0_player.GetLookDir()));
  const CQuaternion rotation =
      CQuaternion::AxisAngle(axis, CRelAngle::FromRadians(x30_ballTiltAngle));
  x0_player.SetTransform(x0_player.GetTransform() * rotation.BuildTransform4f());
  x28_tireMode = false;
  xe64_tireInterpolating = true;
  xe60_tireInterpolationSpeed = -1.f;
}

// 800cac74 -- Scaffold, not a reconstructed implementation.
void CMorphBall::UpdateBallDynamics(CStateManager& mgr, float dt) {
  // TODO: Update contact orientation, tire/marble mode, damping and velocity history.
}

// 800cb50c -- Scaffold, not a reconstructed implementation.
float CMorphBall::BallTurnInput(const CFinalInput& input) const {
  // TODO: Use the player's Echoes control mapping: turn-left minus turn-right.
  return 0.f;
}

// 800cb59c
bool CMorphBall::CalculateBallContactInfo(CVector3f& normal, CVector3f& point) const {
  if (x74_collisionInfos.GetCount() == 0) {
    return false;
  }
  normal = x74_collisionInfos[0].GetNormalLeft();
  point = x74_collisionInfos[0].GetPoint();
  return true;
}

// 800cb5e8
CTransform4f CMorphBall::CalculateSurfaceToWorld(const CVector3f& normal, const CVector3f& point,
                                                 const CVector3f& direction) const {
  if (direction.CanBeNormalized()) {
    const CVector3f forward = direction.AsNormalized();
    CVector3f right = CVector3f::Cross(direction, normal);
    if (right.CanBeNormalized()) {
      right.Normalize();
      const CVector3f up = CVector3f::Cross(right, forward).AsNormalized();
      return CTransform4f::FromColumns(right, forward, up, point);
    }
  }
  return CTransform4f::Identity();
}

// 800cb730
CVector3f CMorphBall::GetBallPosition() const {
  return x0_player.GetTranslation() + CVector3f(0.f, 0.f, xc_radius);
}

// 800cb764
CTransform4f CMorphBall::GetBallToWorld() const {
  return CTransform4f::Translate(GetBallPosition()) * x0_player.GetTransform().GetRotation();
}

// 800cb7f4
CTransform4f CMorphBall::GetSwooshToWorld() const {
  return CTransform4f::Translate(x0_player.GetTranslation() +
                                 CVector3f(0.f, 0.f, GetBallRadius())) *
         xd28_surfaceToWorld.GetRotation() *
         CTransform4f::RotateY(CRelAngle::FromRadians(x30_ballTiltAngle));
}

// 800cb8b4 -- Scaffold, not a reconstructed implementation.
void CMorphBall::ComputeMarioMovement(const CFinalInput& input, CStateManager& mgr, float dt) {
  // TODO: Compute camera-relative control, friction, lift, torque and contact response.
}

// 800cc258 -- Scaffold, not a reconstructed implementation.
void CMorphBall::TransformSpiderBallState(const CQuaternion& rotation,
                                          const CVector3f& translation) {
  // TODO: Rotate the saved physics forces/normals and transform the track point about the ball.
}

// 800cc470 -- Scaffold, not a reconstructed implementation.
void CMorphBall::CreateSpiderBallParticles(CStateManager& mgr, const CVector3f& ballPosition,
                                           const CVector3f& trackPoint) {
  // TODO: Emit/update the magnet effect along the ball-to-track segment.
}

// 800cc67c
float CMorphBall::GetSpiderBallSwingControllerMovementScalar() const {
  if (xd0c_swingControlTime < 1.2f) {
    return 1.f;
  }
  return rstl::max_val(0.f, (2.4f - xd0c_swingControlTime) / 1.2f);
}

// 800cc6b4
void CMorphBall::UpdateSpiderBallSwingControllerMovementTimer(float movement, float dt) {
  if (CMath::AbsF(movement) < 0.05f) {
    ResetSpiderBallSwingControllerMovementTimer();
  } else if (xd08_swingControlDirection == CMath::Sign(movement)) {
    xd0c_swingControlTime += dt;
  } else {
    ResetSpiderBallSwingControllerMovementTimer();
    xd08_swingControlDirection = CMath::Sign(movement);
  }
}

// 800cc748
void CMorphBall::ResetSpiderBallSwingControllerMovementTimer() {
  xd08_swingControlDirection = 0.f;
  xd0c_swingControlTime = 0.f;
}

// 800cc758 -- Scaffold, not a reconstructed implementation.
float CMorphBall::GetSpiderBallControllerMovement(const CFinalInput& input) const {
  // TODO: Convert mapped movement axes to signed magnitude with the Echoes angle dead zones.
  return 0.f;
}

// 800cc89c
void CMorphBall::SetSpiderBallSwingingState(bool swinging) {
  if (xcc2_spiderBallSwinging != swinging) {
    ResetSpiderBallSwingControllerMovementTimer();
    xcc3_spiderSwingInAir = true;
  }
  xcc2_spiderBallSwinging = swinging;
}

// 800cc8f0 -- Scaffold, not a reconstructed implementation.
bool CMorphBall::FindClosestSpiderBallWaypoint(
    CStateManager& mgr, const CVector3f& center, CVector3f& trackPoint,
    CVector3f& interpolatedDirection, CVector3f& direction, float& distance, CVector3f& normal,
    ESpiderSurfaceType& surfaceType, TUniqueId& surfaceId, CTransform4f& surfaceTransform) const {
  // TODO: Search waypoint tracks, scripted surfaces and collision surfaces; populate the outputs.
  return false;
}

// 800cd550 -- Scaffold, not a reconstructed implementation.
bool CMorphBall::CheckForSwitchToSpiderBallSwinging(CStateManager& mgr) const {
  // TODO: Check Spider surface kind, attachment geometry and the player's movement state.
  return false;
}

// 800cd6c0 -- Scaffold, not a reconstructed implementation.
void CMorphBall::ApplySpiderBallRollForces(const CFinalInput& input, CStateManager& mgr, float dt) {
  // TODO: Find the attachment, project controls and apply Spider roll/attraction forces.
}

// 800ce0f0
void CMorphBall::ResetSpiderBallForces() {
  xd10_normalizedSpiderSurfaceForces = CVector2f(0.f, 0.f);
  xd18_spiderTrackForceMagnitude = 0.f;
  xd1c_spiderViewControlMagnitude = 0.f;
  xd24_spiderForcesReset = true;
}

// 800ce14c -- Scaffold, not a reconstructed implementation.
CVector2f CMorphBall::CalculateSpiderBallAttractionSurfaceForces(const CFinalInput& input) const {
  // TODO: Build the mapped two-axis attraction input with movement gating.
  return CVector2f(0.f, 0.f);
}

// 800ce258 -- Scaffold, not a reconstructed implementation.
CVector3f CMorphBall::TransformSpiderBallForcesXZ(CVector2f& forces, CStateManager& mgr) const {
  // TODO: Transform the XZ force plane using the player's Echoes camera mode.
  return CVector3f::Zero();
}

// 800ce300 -- Scaffold, not a reconstructed implementation.
CVector3f CMorphBall::TransformSpiderBallForcesXY(CVector2f& forces, CStateManager& mgr) const {
  // TODO: Transform the XY force plane using the player's Echoes camera mode.
  return CVector3f::Zero();
}

// 800ce3a8 -- Scaffold, not a reconstructed implementation.
void CMorphBall::ApplySpiderBallSwingingForces(const CFinalInput& input, CStateManager& mgr,
                                               float dt) {
  // TODO: Apply the radial constraint, swing input and gravity about the Spider track.
}

// 800ce728
void CMorphBall::UpdateSpiderBall(const CFinalInput& input, CStateManager& mgr, float dt) {
  SetSpiderBallSwingingState(CheckForSwitchToSpiderBallSwinging(mgr));
  if (xcc2_spiderBallSwinging) {
    ApplySpiderBallSwingingForces(input, mgr, dt);
  } else {
    ApplySpiderBallRollForces(input, mgr, dt);
  }
}

// 800ce7c0
void CMorphBall::SetDamageTimer(float time) { xd20_damageTimer = time; }

// 800ce7c8
void CMorphBall::SetDisableSpiderBallTime(float time) { x1038_disableSpiderBallTime = time; }

// 800ce7d0 -- Scaffold, not a reconstructed implementation.
bool CMorphBall::IsMovementAllowed() const {
  // TODO: Check per-player free-look controls, morph transitions and the control cooldown.
  return false;
}

// 800ce864
void CMorphBall::ComputeBallMovement(const CFinalInput& input, CStateManager& mgr, float dt) {
  switch (xc80_ballState) {
  case kBS_ScrewAttackRecovery:
    UpdateScrewAttackRecovery(dt);
    break;
  case kBS_ScrewAttack:
  case kBS_ScrewAttackWallJump:
    ComputeScrewAttackMovement(input, mgr, dt);
    break;
  case kBS_Normal:
  case kBS_Boost:
  case kBS_Spider:
  case kBS_SpiderBoost:
  case kBS_Projectile:
    ComputeBoostBallMovement(input, mgr, dt);
    ComputeMarioMovement(input, mgr, dt);
    break;
  }
}

// 800ce914 -- Scaffold, not a reconstructed implementation.
float CMorphBall::ForwardInput(const CFinalInput& input) const {
  // TODO: Use the player's Echoes control mapping: forward minus backward, gated by
  // IsMovementAllowed.
  return 0.f;
}

// 800ce9a4 -- Scaffold, not a reconstructed implementation.
float CMorphBall::GetBallTouchRadius() const {
  // TODO: Use CTweakBall::GetBallTouchRadius once the shared tweak interface is declared.
  return 0.f;
}

// 800ce9c8
float CMorphBall::GetBallRadius() const { return x0_player.GetTweakPlayer()->GetBallRadius(); }

// 800ce9f0 -- Ownership cleanup is supplied by the members' destructors.
CMorphBall::~CMorphBall() {}

// 800cf0b0 -- Material 59 has no established semantic name in this checkout.
CMorphBall::CMorphBall(CPlayer& player, float radius, bool multiplayer)
: x0_player(player)
, x4_loadedModelId(-1)
, x8_ballGlowColorIdx(0)
, xc_radius(radius)
, x10_boostControlForce(CVector3f::Zero())
, x1c_controlForce(CVector3f::Zero())
, x28_tireMode(false)
, x2c_tireLeanAngle(0.f)
, x30_ballTiltAngle(0.f)
, x38_collisionSphere(
      CSphere(CVector3f(0.f, 0.f, radius), radius),
      CMaterialList(kMT_Player, kMT_Unknown59, kMT_GroundCollider, kMT_NoPlayerCollision))
, x58_ballModel(GetMorphBallModel(multiplayer ? "SamusMultiBallANCS" : "SamusBallCMDL", radius))
, x5c_ballModelShader(0)
, x60_spiderBallGlassModel(GetMorphBallModel("", radius))
, x64_spiderBallGlassModelShader(0)
, x68_lowPolyBallModel(GetMorphBallModel("SamusBallLowPolyCMDL", radius))
, x6c_lowPolyBallModelShader(0)
, x70_frozenBallModel(GetMorphBallModel("SamusBallFrozenCMDL", radius))
, xc78_lastWallCollisionFrame(-1)
, xc7c_lastFloorCollisionFrame(-1)
, xc80_ballState(kBS_Normal)
, xc84_playerToSpiderNormal(CVector3f::Zero())
, xc90_spiderPullMovement(1.f)
, xc94_spiderTrackPoint(CVector3f::Zero())
, xca0_spiderInterpBetweenPoints(CVector3f::Zero())
, xcac_spiderBetweenPoints(CVector3f::Zero())
, xcb8_linearVelocityDamping(0.f)
, xcbc_angularVelocityDamping(0.f)
, xcc0_spiderNearby(false)
, xcc1_touchingSpider(false)
, xcc2_spiderBallSwinging(false)
, xcc3_spiderSwingInAir(true)
, xcc4_spiderSurfaceType(kSST_None)
, xcc8_spiderSurfaceTransform(CTransform4f::Identity())
, xcf8_spiderSurfacePivotAngle(0.f)
, xcfc_spiderSurfacePivotTargetAngle(0.f)
, xd00_refPullVelocity(0.f)
, xd04_playerToSpiderTrackDistance(0.f)
, xd08_swingControlDirection(0.f)
, xd0c_swingControlTime(0.f)
, xd10_normalizedSpiderSurfaceForces(0.f, 0.f)
, xd18_spiderTrackForceMagnitude(0.f)
, xd1c_spiderViewControlMagnitude(0.f)
, xd20_damageTimer(0.f)
, xd24_spiderForcesReset(false)
, xd28_surfaceToWorld(CTransform4f::Identity())
, xd58_slowBlueTailSwoosh(
      gpSimplePool->GetObj(multiplayer ? "SlowBlueTailSwoosh_MP" : "SlowBlueTailSwoosh"))
, xd60_slowBlueTailSwoosh2(
      gpSimplePool->GetObj(multiplayer ? "SlowBlueTailSwoosh2_MP" : "SlowBlueTailSwoosh2"))
, xd68_jaggyTrail(gpSimplePool->GetObj(multiplayer ? "JaggyTrail_MP" : "JaggyTrail"))
, xd70_sideSwoosh(gpSimplePool->GetObj("SideSwooshSide"))
, xd78_wallSpark(gpSimplePool->GetObj("WallSpark"))
, xd80_ballInnerGlow(gpSimplePool->GetObj("BallInnerGlow"))
, xd88_spiderBallMagnet(gpSimplePool->GetObj("SpiderBallMagnetEffect"))
, xd90_boostBallGlow(gpSimplePool->GetObj("BoostBallGlow"))
, xd98_morphBallTransitionFlash(gpSimplePool->GetObj("MorphBallTransitionFlash"))
, xda0_morphBallIceBreak(gpSimplePool->GetObj("Effect_MorphBallIceBreak"))
, xda8_boostEffect(gpSimplePool->GetObj("BoostEffect"))
, xdb0_deathBallOuterShell(gpSimplePool->GetObj("DeathBallOuterShell"))
, xdb8_deathBallSpikes(gpSimplePool->GetObj("DeathBallSpikes"))
, xdc0_screwAttackJumpFlash(gpSimplePool->GetObj("ScrewAttackJumpFlash"))
, xdc8_slowBlueTailSwooshGen(rs_new CParticleSwoosh(xd58_slowBlueTailSwoosh, 0))
, xdcc_slowBlueTailSwooshGen2(rs_new CParticleSwoosh(xd58_slowBlueTailSwoosh, 0))
, xdd0_slowBlueTailSwoosh2Gen(rs_new CParticleSwoosh(xd60_slowBlueTailSwoosh2, 0))
, xdd4_slowBlueTailSwoosh2Gen2(rs_new CParticleSwoosh(xd60_slowBlueTailSwoosh2, 0))
, xdd8_jaggyTrailGen(rs_new CParticleSwoosh(xd68_jaggyTrail, 0))
, xddc_sideSwooshGen(multiplayer ? nullptr : rs_new CParticleSwoosh(xd70_sideSwoosh, 0))
, xde0_sideSwooshGen2(multiplayer ? nullptr : rs_new CParticleSwoosh(xd70_sideSwoosh, 0))
, xde4_wallSparkGen(rs_new CElementGen(xd78_wallSpark))
, xde8_ballInnerGlowGen(rs_new CElementGen(xd80_ballInnerGlow))
, xdec_spiderBallMagnetGen(rs_new CElementGen(xd88_spiderBallMagnet))
, xdf0_boostBallGlowGen(rs_new CElementGen(xd90_boostBallGlow))
, xdf4_boostEffectGen(nullptr)
, xdf8_morphBallTransitionFlashGen(nullptr)
, xdfc_morphBallIceBreakGen(nullptr)
, xe00_deathBallOuterShellGen(nullptr)
, xe04_deathBallSpikesGen(nullptr)
, xe08_screwAttackJumpFlashGen(nullptr)
, xe0c_screwAttackWallJumpFlashGen(nullptr)
, xe44_wakeEffectIndex(-1)
, xe48_ballInnerGlowLight(kInvalidUniqueId)
, xe4a_ballLightActive(false)
, xe4c_worldShadow(rs_new CWorldShadow(16, 16, false))
, xe50_actorLights(
      rs_new CActorLights(8, CVector3f::Zero(), 4, 4, 0.1f, false, false, false, false))
, xe54_rainSplashGen(rs_new CRainSplashGenerator(x58_ballModel->GetScale(), 40, 2, 0.15f, 0.5f))
, xe58_tireFactor(0.f)
, xe5c_maxTireFactor(0.5f)
, xe60_tireInterpolationSpeed(1.f)
, xe64_tireInterpolating(false)
, xe68_boostOverLightFactor(0.f)
, xe6c_boostLightFactor(0.f)
, xe70_spiderLightFactor(0.f)
, xe74_ballOrientationAverage(CQuaternion::NoRotation())
, xec8_ballPositionAverage(CVector3f::Zero())
, xf08_liftSpeedAverage(0.f)
, xf48_liftControlForceAverage(CVector3f::Zero())
, x1000_failsafeCounter(0)
, x1004_velocityBeforeFailsafe(CVector3f::Zero())
, x1010_velocityAfterFailsafe(CVector3f::Zero())
, x101c_24_boostEnabled(true)
, x101c_25_touchedFloorDuringBoost(false)
, x1020_boostChargeTime(0.f)
, x1024_timeNotInBoost(1000.f)
, x1028_(0.f)
, x102c_boostDrainTime(0.f)
, x1030_boostEffectTime(0.f)
, x1034_boostDamageScale(1.f)
, x1038_disableSpiderBallTime(0.f)
, x1048_hasSpiderBoostDirection(false)
, x1850_boostTrailFadeTimer(0.f)
, x1854_24_inHalfPipeMode(false)
, x1854_25_inHalfPipeModeInAir(false)
, x1854_26_touchedHalfPipeRecently(false)
, x1854_27_ballCloseToCollision(false)
, x1858_closeToCollisionTime(0.f)
, x185c_touchHalfPipeCooldown(0.f)
, x1860_disableControlCooldown(0.f)
, x1864_touchedHalfPipeRecentCooldown(0.f)
, x1868_prevHalfPipeNormal(CVector3f::Zero())
, x1874_halfPipeNormal(CVector3f::Zero())
, x1880_ballAnimationIndex(0)
, x18a0_rollSfxId(0xffff)
, x18a2_landSfxId(0xffff)
, x18a4_wallSparkFrameCountdown(1)
, x18a8_24_endScrewAttackRequested(false)
, x18a8_25_touchingWall(false)
, x18a8_26_pendingRecoil(false)
, x18a8_27_recoiling(false)
, x18a8_28_wallJumpInputPending(false)
, x18a8_29_collidedDuringRecovery(false)
, x18a8_30_(false)
, x18a8_31_forcedScrewJumpInput(false)
, x18ac_screwAttackJumpCount(0)
, x18b0_wallJumpCount(0)
, x18b4_screwAttackExitAnimationFrames(0)
, x18b8_screwAttackGroundedFrames(0)
, x18bc_timeSinceScrewAttackJump(0.f)
, x18c0_wallContactTime(0.f)
, x18c4_screwAttackRecoveryCollisionTime(0.f)
, x18c8_wallNormal(CVector3f::Zero())
, x18d4_screwAttackDirection(CVector3f::Zero())
, x18f0_boostState(kBBS_BoostAvailable)
, x18f4_bombJumpState(kBJS_BombJumpAvailable)
, x18f8_damageEffect(0.f)
, x18fc_damageEffectDecaySpeed(0.f)
, x1900_damageTime(0.f)
, x1904_24_multiplayer(multiplayer)
, x1908_shadow(nullptr) {
  xdec_spiderBallMagnetGen->SetParticleEmission(false);
  xdec_spiderBallMagnetGen->Update(double(1.f / 60.f));
  sBallCloseToCollisionDistance = GetBallRadius() + 0.2f;
  InitializeWakeEffects();
  x18e0_deathBallDamageCooldowns.reserve(16);
  // TODO: recover the single-player material preparation calls at 800e4b20.
  x0_player.SetCollisionAccuracyModifier(5.f);
}
