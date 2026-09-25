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

// Guessed names for TU-local state.
static float sBallCloseToCollisionDistance;
static rstl::reserved_vector< int, 64 > sWakeEffectForMaterial;

void CMorphBall::DeleteBallShadow() { mShadow = nullptr; }

void CMorphBall::CreateBallShadow() {
  if (!mShadow.get()) {
    mShadow = rs_new CMorphBallShadow(64, 64, gpSimplePool->GetObj("TXTR_BallFade"));
  }
}

// Scaffold, not a reconstructed implementation.
void CMorphBall::RenderToShadowTex(CStateManager& mgr) {
  // TODO: Gather shadow receivers and render CMorphBallShadow with the player texture.
}

// Scaffold, not a reconstructed implementation.
void CMorphBall::DrawBallShadow(CStateManager& mgr) {
  // TODO: Draw the projected ball shadow against the gathered world/actor receivers.
}

// Guessed name.
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
    mWakeEffects.push_back(rstl::auto_ptr< CDeferredParticleEffect >(
        rs_new CDeferredParticleEffect(gpSimplePool->GetObj(effects[i]), group)));
  }
}

// Guessed name.
void CMorphBall::ResetScrewAttackExitAnimationTimer() { mScrewAttackExitAnimationFrames = 5; }

// Guessed name.
int CMorphBall::GetScrewAttackGroundedFrames() const { return mScrewAttackGroundedFrames; }

bool CMorphBall::InScrewAttackMode() const {
  return mBallState == kBS_ScrewAttack || mBallState == kBS_ScrewAttackWallJump ||
         mBallState == kBS_ScrewAttackRecovery;
}

bool CMorphBall::IsProjectile() const { return mBallState == kBS_Projectile; }

bool CMorphBall::IsBoostShieldActive() const { return mTimeNotInBoost < 1.f; }

float CMorphBall::GetBoostChargeTimer() const { return mBoostChargeTime; }

float CMorphBall::GetTimeNotInBoost() const { return mTimeNotInBoost; }

bool CMorphBall::IsBoosting() const {
  return mBallState == kBS_Boost || mBallState == kBS_SpiderBoost;
}

// Guessed name; workspace type name is a cross-game hypothesis.
void CMorphBall::PointGenerator(const CSkinnedModel& model, const SSkinningWorkspace& workspace,
                                void* context) {
  if (context) {
    static_cast< CRainSplashGenerator* >(context)->GeneratePoints(model, workspace);
  }
}

// Scaffold, not a reconstructed implementation.
void CMorphBall::StartLandingSfx() {
  // TODO: Choose and play the landing sound from velocity and selected surface material.
}

void CMorphBall::StopSounds() {
  if (mRollSfx) {
    CSfxManager::SfxStop(mRollSfx);
    mRollSfx.Clear();
  }
  if (mSpiderSfx) {
    CSfxManager::SfxStop(mSpiderSfx);
    mSpiderSfx.Clear();
  }
  if (mDeathBallSfx) {
    CSfxManager::SfxStop(mDeathBallSfx);
    mDeathBallSfx.Clear();
  }
  if (mScrewAttackSfx) {
    CSfxManager::SfxStop(mScrewAttackSfx);
    mScrewAttackSfx.Clear();
  }
}

// Scaffold, not a reconstructed implementation.
void CMorphBall::StartScrewAttackSfx() {
  // TODO: Start the Screw Attack sound with the player's sound-channel settings.
}

// Scaffold, not a reconstructed implementation.
void CMorphBall::UpdateMorphBallSound(float dt, CStateManager& mgr) {
  // TODO: Maintain the roll, Spider, death-ball and Screw Attack emitters.
}

// Scaffold, not a reconstructed implementation.
void CMorphBall::SelectMorphBallSounds(const CMaterialList& material) {
  // TODO: Select roll/landing sound IDs for collision material and multiplayer mode.
}

void CMorphBall::TakeDamage(float damage) {
  if (damage <= 0.f) {
    mDamageEffect = 0.f;
    mDamageEffectDecaySpeed = 0.f;
    return;
  }

  if (damage >= 20.f) {
    mDamageEffectDecaySpeed = 0.25f;
  } else if (damage > 5.f) {
    mDamageEffectDecaySpeed = 1.f - 0.75f * ((damage - 5.f) / 15.f);
  } else {
    mDamageEffectDecaySpeed = 1.f;
  }
  mDamageEffect = 1.f;
}

CMorphBall::EBombJumpState CMorphBall::GetBombJumpState() const { return mBombJumpState; }

void CMorphBall::SetBallBoostState(EBallBoostState state) { mBoostState = state; }

CMorphBall::EBallBoostState CMorphBall::GetBallBoostState() const { return mBoostState; }

void CMorphBall::SetAsProjectile(bool projectile) {
  if (projectile) {
    mBallState = kBS_Projectile;
  } else if (mBallState == kBS_Projectile) {
    mBallState = kBS_Normal;
  }
}

void CMorphBall::TouchModel(const CStateManager& mgr) const {
  mBallModel->Touch(mgr, mBallModelShader);
  if (mPlayer.GetPlayerState()->HasPowerUp(CPlayerState::kIT_SpiderBall) &&
      mSpiderBallGlassModel.get()) {
    mSpiderBallGlassModel->Touch(mgr, mSpiderBallGlassModelShader);
  }
  mLowPolyBallModel->Touch(mgr, mLowPolyBallModelShader);
}

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

// Scaffold, not a reconstructed implementation.
void CMorphBall::LoadMorphBallModel() {
  // TODO: Select normal/spider/boost resources and glow colors for the three Echoes suits.
}

// Scaffold, not a reconstructed implementation.
void CMorphBall::FluidFXThink(CActor::EFluidState state, CScriptWater& water, CStateManager& mgr) {
  // TODO: Rate-limit water splashes using speed, fluid state and the fluid-plane manager.
}

bool CMorphBall::IsClimbable(const CCollisionInfo& collision) const {
  if (CMath::AbsF(collision.GetNormalLeft().GetZ()) < 0.7f) {
    const float height = GetBallPosition().GetZ() - collision.GetPoint().GetZ();
    return height > 0.1f && height < GetBallRadius() - 0.05f;
  }
  return false;
}

// The original body is genuinely empty.
void CMorphBall::Touch(CActor& actor, CStateManager& mgr) {}

// Scaffold, not a reconstructed implementation.
float CMorphBall::ComputeMaxSpeed() const {
  // TODO: Use the surface-restraint tweak or clamp the half-pipe velocity-derived limit.
  return 0.f;
}

void CMorphBall::SpinToSpeed(float speed, const CVector3f& direction, float dt) {
  const float angularSpeed = mPlayer.GetAngularVelocityWR().GetVector().Magnitude();
  mPlayer.ApplyTorqueWR(dt * (speed - angularSpeed) * direction);
}

void CMorphBall::ApplyGravity() {
  mPlayer.SetMomentumWR(CVector3f(0.f, 0.f, mPlayer.GetMass() * GetGravityAcceleration()));
}

// Scaffold, not a reconstructed implementation.
float CMorphBall::GetGravityAcceleration() const {
  // TODO: Select normal/water/Screw Attack/wall-jump gravity from CTweakBall.
  return 0.f;
}

// Scaffold, not a reconstructed implementation.
float CMorphBall::CalculateSurfaceFriction() const {
  // TODO: Use the surface-restraint tweak, attachment state and energy-drain count.
  return 0.f;
}

// Scaffold, not a reconstructed implementation.
void CMorphBall::ComputeLiftForces(const CVector3f& controlForce, const CVector3f& velocity,
                                   const CStateManager& mgr) {
  // TODO: Update the lift averages and apply the contact-dependent upward force.
}

CAABox CMorphBall::GetRenderBounds(const CStateManager& mgr) const {
  const CVector3f center = GetBallPosition();
  const CVector3f extent(2.f * mRadius, 2.f * mRadius, 2.f * mRadius);
  CAABox bounds(center - extent, center + extent);
  if (mSlowBlueTailSwooshGen->GetModulationColor().GetAlpha() != 0.f) {
    const rstl::optional_object< CAABox > trailBounds = mSlowBlueTailSwooshGen->GetBounds();
    if (trailBounds.valid()) {
      bounds.AccumulateBounds(trailBounds->GetMinPoint());
      bounds.AccumulateBounds(trailBounds->GetMaxPoint());
    }
  }
  return bounds;
}

// Scaffold, not a reconstructed implementation.
void CMorphBall::CollidedWith(const TUniqueId& id, const CCollisionInfoList& collisions,
                              CStateManager& mgr) {
  // TODO: Process contact materials/normals, boost damage, half-pipe and Screw Attack collisions.
}

// Scaffold, not a reconstructed implementation.
bool CMorphBall::BallCloseToCollision(const CStateManager& mgr, float distance,
                                      const CMaterialFilter& filter) const {
  // TODO: Test a swept sphere against the world's filtered collision geometry.
  return false;
}

void CMorphBall::DisableHalfPipeStatus() {
  SetIsInHalfPipeMode(false);
  SetIsInHalfPipeModeInAir(false);
  SetTouchedHalfPipeRecently(false);
  mTouchHalfPipeCooldown = 0.f;
  mDisableControlCooldown = 0.f;
  mPlayer.SetCollisionAccuracyModifier(5.f);
  mPrevHalfPipeNormal = CVector3f::Zero();
  mHalfPipeNormal = CVector3f::Zero();
}

void CMorphBall::SetTouchedHalfPipeRecently(bool touched) {
  mTouchedHalfPipeRecently = touched;
}

bool CMorphBall::GetTouchedHalfPipeRecently() const { return mTouchedHalfPipeRecently; }

void CMorphBall::SetIsInHalfPipeModeInAir(bool active) { mInHalfPipeModeInAir = active; }

bool CMorphBall::GetIsInHalfPipeModeInAir() const { return mInHalfPipeModeInAir; }

void CMorphBall::SetIsInHalfPipeMode(bool active) { mInHalfPipeMode = active; }

bool CMorphBall::GetIsInHalfPipeMode() const { return mInHalfPipeMode; }

// Scaffold, not a reconstructed implementation.
void CMorphBall::UpdateHalfPipeStatus(CStateManager& mgr, float dt) {
  // TODO: Expire half-pipe/contact cooldowns and adjust collision accuracy.
}

// Guessed name.
void CMorphBall::RenderScrewAttackJumpEffects() const {
  if (mScrewAttackJumpFlashGen.get()) {
    mScrewAttackJumpFlashGen->Render();
  }
  if (mScrewAttackWallJumpFlashGen.get()) {
    mScrewAttackWallJumpFlashGen->Render();
  }
}

// Scaffold, not a reconstructed implementation.
void CMorphBall::RenderDamageEffects(const CStateManager& mgr,
                                     const CTransform4f& transform) const {
  // TODO: Render the damage overlay using the ball's damage timer and glow color.
}

void CMorphBall::RenderIceBreakEffect(const CStateManager& mgr) const {
  if (mMorphBallIceBreakGen.get()) {
    mMorphBallIceBreakGen->Render();
  }
}

void CMorphBall::UpdateIceBreakEffect(float dt) {
  if (!mMorphBallIceBreakGen.get() && mMorphBallIceBreak.HasLock() &&
      mMorphBallIceBreak.IsLoaded()) {
    mMorphBallIceBreakGen = rs_new CElementGen(mMorphBallIceBreak);
    mMorphBallIceBreakGen->SetOrientation(mPlayer.GetTransform().GetRotation());
  }
  if (mMorphBallIceBreakGen.get()) {
    if (mMorphBallIceBreakGen->IsSystemDeletable()) {
      mMorphBallIceBreakGen = nullptr;
      mMorphBallIceBreak.Unlock();
    } else {
      mMorphBallIceBreakGen->SetGlobalTranslation(GetBallPosition());
      mMorphBallIceBreakGen->Update(dt);
    }
  }
}

void CMorphBall::ResetMorphBallIceBreak() {
  mMorphBallIceBreak.Lock();
  mMorphBallIceBreakGen = nullptr;
}

bool CMorphBall::IsMorphBallTransitionFlashValid() const {
  return mMorphBallTransitionFlashGen.get() != nullptr;
}

// Scaffold, not a reconstructed implementation.
void CMorphBall::RenderMorphBallTransitionFlash(const CStateManager& mgr) const {
  // TODO: Apply the suit-dependent transition color before rendering the generator.
}

void CMorphBall::UpdateMorphBallTransitionFlash(float dt) {
  if (!mMorphBallTransitionFlashGen.get() && mMorphBallTransitionFlash.HasLock() &&
      mMorphBallTransitionFlash.IsLoaded()) {
    mMorphBallTransitionFlashGen = rs_new CElementGen(mMorphBallTransitionFlash);
    mMorphBallTransitionFlashGen->SetOrientation(mPlayer.GetTransform().GetRotation());
  }
  if (mMorphBallTransitionFlashGen.get()) {
    if (mMorphBallTransitionFlashGen->IsSystemDeletable()) {
      mMorphBallTransitionFlashGen = nullptr;
      mMorphBallTransitionFlash.Unlock();
    } else {
      mMorphBallTransitionFlashGen->SetGlobalTranslation(GetBallPosition());
      mMorphBallTransitionFlashGen->Update(dt);
    }
  }
}

void CMorphBall::ResetMorphBallTransitionFlash() {
  mMorphBallTransitionFlash.Lock();
  mMorphBallTransitionFlashGen = nullptr;
}

// Scaffold, not a reconstructed implementation.
void CMorphBall::Render(const CStateManager& mgr, const CActorLights* lights) const {
  // TODO: Render the ball, glass, trails and Echoes Screw Attack/death-ball effects.
}

// Scaffold, not a reconstructed implementation.
void CMorphBall::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  // TODO: Prepare model animation, rain-splash point generation, actor lights and the world shadow.
}

// Scaffold, not a reconstructed implementation.
float CMorphBall::GetMinimumAlignmentSpeed() const {
  // TODO: Return zero in Spider mode; otherwise use the alignment-speed tweak.
  return 0.f;
}

void CMorphBall::DampLinearAndAngularVelocities(float linearDamping, float angularDamping,
                                                float dt) {
  const float frames = 60.f * dt;
  const float linearScale = pow(1.f - linearDamping, frames);
  mPlayer.SetVelocityWR(linearScale * mPlayer.GetVelocityWR());
  const float angularScale = pow(1.f - angularDamping, frames);
  mPlayer.SetAngularVelocityWR(mPlayer.GetAngularVelocityWR() * angularScale);
}

void CMorphBall::ApplyFriction(float friction) {
  CVector3f velocity = mPlayer.GetVelocityWR();
  if (velocity.Magnitude() <= friction) {
    velocity = CVector3f::Zero();
  } else {
    velocity = (velocity.Magnitude() - friction) * velocity.AsNormalized();
  }
  mPlayer.SetVelocityWR(velocity);
}

// Scaffold, not a reconstructed implementation.
bool CMorphBall::UpdateMarbleDynamics(CStateManager& mgr, float dt, const CVector3f& point) {
  // TODO: Apply marble alignment, rolling torque and contact-force response.
  return false;
}

// Scaffold, not a reconstructed implementation.
void CMorphBall::ApplyBoostBallDamage(CStateManager& mgr, TUniqueId id, const CDamageInfo& damage,
                                      float dt) {
  // TODO: Filter already-hit actors, scale damage and update the cooldown/history.
}

void CMorphBall::CancelBoosting() {
  mBoostChargeTime = 0.f;
  mBoostDrainTime = 0.f;
  if (mBallAnimationIndex == 1) {
    mBallAnimationIndex = 0;
    CSfxManager::SfxStop(mBoostChargeSfx);
    mBoostChargeSfx.Clear();
  }
}

void CMorphBall::LeaveBoosting() {
  if (IsBoosting()) {
    mBoostChargeTime = 0.f;
    mBallState = kBS_Normal;
  }
  mBoostDrainTime = 0.f;
}

// Scaffold, not a reconstructed implementation.
void CMorphBall::EnterBoosting(CStateManager& mgr, bool skipImpulse) {
  // TODO: Enter normal/spider boost, optionally apply the impulse, and reset damage history.
}

// Scaffold, not a reconstructed implementation.
void CMorphBall::ComputeBoostBallMovement(const CFinalInput& input, CStateManager& mgr, float dt) {
  // TODO: Handle charge, release, draining, Spider Boost direction and damage.
}

void CMorphBall::SetScrewAttackActive(bool active) { mForcedScrewJumpInput = active; }

// Scaffold, not a reconstructed implementation.
void CMorphBall::ApplyScrewAttackDamage(float dt, CStateManager& mgr) {
  // TODO: Build the swept contact list and apply Screw Attack damage.
}

// Scaffold, not a reconstructed implementation.
void CMorphBall::UpdateScrewAttackRecovery(float dt) {
  // TODO: Recover from recoil/collisions and request the player's exit animation.
}

// Scaffold, not a reconstructed implementation.
void CMorphBall::ComputeScrewAttackMovement(const CFinalInput& input, CStateManager& mgr,
                                            float dt) {
  // TODO: Handle jump/wall-jump input, speed/height limits and Screw Attack recovery.
}

// Scaffold, not a reconstructed implementation.
void CMorphBall::UpdateDeathBall(float dt, CStateManager& mgr) {
  // TODO: Update the multiplayer death-ball effects and per-object damage cooldowns.
}

// Scaffold, not a reconstructed implementation.
void CMorphBall::UpdateBallLight(float dt, CStateManager& mgr) {
  // TODO: Update the inner-glow light from the generator and ball lighting state.
}

// Scaffold, not a reconstructed implementation.
void CMorphBall::UpdateEffects(float dt, CStateManager& mgr) {
  // TODO: Update particle transforms, trail history, wake selection and light intensities.
}

void CMorphBall::StopParticleWakes() {
  mWallSparkGen->SetParticleEmission(false);
  for (int i = 0; i < mWakeEffects.size(); ++i) {
    mWakeEffects[i]->SetParticleEmission(false);
  }
}

void CMorphBall::LeaveMorphBallState(CStateManager& mgr) {
  LeaveBoosting();
  CancelBoosting();
  CSfxManager::SfxStop(mBoostChargeSfx);
  mBoostReleaseSfx.Clear();
  StopParticleWakes();
}

// Scaffold, not a reconstructed implementation.
void CMorphBall::EnterMorphBallState(CStateManager& mgr, EBallState state) {
  // TODO: Reset ball/spider/boost state, sounds, averages and lighting for the requested mode.
}

void CMorphBall::SetBallLightActive(CStateManager& mgr, bool active) {
  mBallLightActive = active;
}

void CMorphBall::DeleteLight(CStateManager& mgr) {
  if (mBallInnerGlowLight != kInvalidUniqueId) {
    mgr.DeleteObjectRequest(mBallInnerGlowLight);
    mBallInnerGlowLight = kInvalidUniqueId;
  }
}

// Scaffold, not a reconstructed implementation.
bool CMorphBall::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                 EUserEventType type) {
  // TODO: Handle event 0x1b during morphed Screw Attack recovery through CPlayer.
  return false;
}

// Scaffold, not a reconstructed implementation.
void CMorphBall::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  // TODO: Handle Echoes CScriptMsg creation/deletion of the inner-glow light.
}

// Scaffold, not a reconstructed implementation.
void CMorphBall::Update(float dt, CStateManager& mgr) {
  // TODO: Update effects, light, death-ball state, tire interpolation, damage decay, rain and
  // sound.
}

void CMorphBall::SwitchToTire() {
  mTireMode = true;
  mTireInterpolating = true;
  mBallTiltAngle = 0.f;
  mTireInterpolationSpeed = 1.f;
}

void CMorphBall::SwitchToMarble() {
  const CUnitVector3f axis(mPlayer.GetTransform().TransposeRotate(mPlayer.GetLookDir()));
  const CQuaternion rotation =
      CQuaternion::AxisAngle(axis, CRelAngle::FromRadians(mBallTiltAngle));
  mPlayer.SetTransform(mPlayer.GetTransform() * rotation.BuildTransform4f());
  mTireMode = false;
  mTireInterpolating = true;
  mTireInterpolationSpeed = -1.f;
}

// Scaffold, not a reconstructed implementation.
void CMorphBall::UpdateBallDynamics(CStateManager& mgr, float dt) {
  // TODO: Update contact orientation, tire/marble mode, damping and velocity history.
}

// Scaffold, not a reconstructed implementation.
float CMorphBall::BallTurnInput(const CFinalInput& input) const {
  // TODO: Use the player's Echoes control mapping: turn-left minus turn-right.
  return 0.f;
}

bool CMorphBall::CalculateBallContactInfo(CVector3f& normal, CVector3f& point) const {
  if (mCollisionInfos.GetCount() == 0) {
    return false;
  }
  normal = mCollisionInfos[0].GetNormalLeft();
  point = mCollisionInfos[0].GetPoint();
  return true;
}

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

CVector3f CMorphBall::GetBallPosition() const {
  return mPlayer.GetTranslation() + CVector3f(0.f, 0.f, mRadius);
}

CTransform4f CMorphBall::GetBallToWorld() const {
  return CTransform4f::Translate(GetBallPosition()) * mPlayer.GetTransform().GetRotation();
}

CTransform4f CMorphBall::GetSwooshToWorld() const {
  return CTransform4f::Translate(mPlayer.GetTranslation() +
                                 CVector3f(0.f, 0.f, GetBallRadius())) *
         mSurfaceToWorld.GetRotation() *
         CTransform4f::RotateY(CRelAngle::FromRadians(mBallTiltAngle));
}

// Scaffold, not a reconstructed implementation.
void CMorphBall::ComputeMarioMovement(const CFinalInput& input, CStateManager& mgr, float dt) {
  // TODO: Compute camera-relative control, friction, lift, torque and contact response.
}

// Scaffold, not a reconstructed implementation.
void CMorphBall::TransformSpiderBallState(const CQuaternion& rotation,
                                          const CVector3f& translation) {
  // TODO: Rotate the saved physics forces/normals and transform the track point about the ball.
}

// Scaffold, not a reconstructed implementation.
void CMorphBall::CreateSpiderBallParticles(CStateManager& mgr, const CVector3f& ballPosition,
                                           const CVector3f& trackPoint) {
  // TODO: Emit/update the magnet effect along the ball-to-track segment.
}

float CMorphBall::GetSpiderBallSwingControllerMovementScalar() const {
  if (mSwingControlTime < 1.2f) {
    return 1.f;
  }
  return rstl::max_val(0.f, (2.4f - mSwingControlTime) / 1.2f);
}

void CMorphBall::UpdateSpiderBallSwingControllerMovementTimer(float movement, float dt) {
  if (CMath::AbsF(movement) < 0.05f) {
    ResetSpiderBallSwingControllerMovementTimer();
  } else if (mSwingControlDirection == CMath::Sign(movement)) {
    mSwingControlTime += dt;
  } else {
    ResetSpiderBallSwingControllerMovementTimer();
    mSwingControlDirection = CMath::Sign(movement);
  }
}

void CMorphBall::ResetSpiderBallSwingControllerMovementTimer() {
  mSwingControlDirection = 0.f;
  mSwingControlTime = 0.f;
}

// Scaffold, not a reconstructed implementation.
float CMorphBall::GetSpiderBallControllerMovement(const CFinalInput& input) const {
  // TODO: Convert mapped movement axes to signed magnitude with the Echoes angle dead zones.
  return 0.f;
}

void CMorphBall::SetSpiderBallSwingingState(bool swinging) {
  if (mSpiderBallSwinging != swinging) {
    ResetSpiderBallSwingControllerMovementTimer();
    mSpiderSwingInAir = true;
  }
  mSpiderBallSwinging = swinging;
}

// Scaffold, not a reconstructed implementation.
bool CMorphBall::FindClosestSpiderBallWaypoint(
    CStateManager& mgr, const CVector3f& center, CVector3f& trackPoint,
    CVector3f& interpolatedDirection, CVector3f& direction, float& distance, CVector3f& normal,
    ESpiderSurfaceType& surfaceType, TUniqueId& surfaceId, CTransform4f& surfaceTransform) const {
  // TODO: Search waypoint tracks, scripted surfaces and collision surfaces; populate the outputs.
  return false;
}

// Scaffold, not a reconstructed implementation.
bool CMorphBall::CheckForSwitchToSpiderBallSwinging(CStateManager& mgr) const {
  // TODO: Check Spider surface kind, attachment geometry and the player's movement state.
  return false;
}

// Scaffold, not a reconstructed implementation.
void CMorphBall::ApplySpiderBallRollForces(const CFinalInput& input, CStateManager& mgr, float dt) {
  // TODO: Find the attachment, project controls and apply Spider roll/attraction forces.
}

void CMorphBall::ResetSpiderBallForces() {
  mNormalizedSpiderSurfaceForces = CVector2f(0.f, 0.f);
  mSpiderTrackForceMagnitude = 0.f;
  mSpiderViewControlMagnitude = 0.f;
  mSpiderForcesReset = true;
}

// Scaffold, not a reconstructed implementation.
CVector2f CMorphBall::CalculateSpiderBallAttractionSurfaceForces(const CFinalInput& input) const {
  // TODO: Build the mapped two-axis attraction input with movement gating.
  return CVector2f(0.f, 0.f);
}

// Scaffold, not a reconstructed implementation.
CVector3f CMorphBall::TransformSpiderBallForcesXZ(CVector2f& forces, CStateManager& mgr) const {
  // TODO: Transform the XZ force plane using the player's Echoes camera mode.
  return CVector3f::Zero();
}

// Scaffold, not a reconstructed implementation.
CVector3f CMorphBall::TransformSpiderBallForcesXY(CVector2f& forces, CStateManager& mgr) const {
  // TODO: Transform the XY force plane using the player's Echoes camera mode.
  return CVector3f::Zero();
}

// Scaffold, not a reconstructed implementation.
void CMorphBall::ApplySpiderBallSwingingForces(const CFinalInput& input, CStateManager& mgr,
                                               float dt) {
  // TODO: Apply the radial constraint, swing input and gravity about the Spider track.
}

void CMorphBall::UpdateSpiderBall(const CFinalInput& input, CStateManager& mgr, float dt) {
  SetSpiderBallSwingingState(CheckForSwitchToSpiderBallSwinging(mgr));
  if (mSpiderBallSwinging) {
    ApplySpiderBallSwingingForces(input, mgr, dt);
  } else {
    ApplySpiderBallRollForces(input, mgr, dt);
  }
}

void CMorphBall::SetDamageTimer(float time) { mDamageTimer = time; }

void CMorphBall::SetDisableSpiderBallTime(float time) { mDisableSpiderBallTime = time; }

// Scaffold, not a reconstructed implementation.
bool CMorphBall::IsMovementAllowed() const {
  // TODO: Check per-player free-look controls, morph transitions and the control cooldown.
  return false;
}

void CMorphBall::ComputeBallMovement(const CFinalInput& input, CStateManager& mgr, float dt) {
  switch (mBallState) {
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

// Scaffold, not a reconstructed implementation.
float CMorphBall::ForwardInput(const CFinalInput& input) const {
  // TODO: Use the player's Echoes control mapping: forward minus backward, gated by
  // IsMovementAllowed.
  return 0.f;
}

// Scaffold, not a reconstructed implementation.
float CMorphBall::GetBallTouchRadius() const {
  // TODO: Use CTweakBall::GetBallTouchRadius once the shared tweak interface is declared.
  return 0.f;
}

float CMorphBall::GetBallRadius() const { return mPlayer.GetTweakPlayer()->GetBallRadius(); }

// Ownership cleanup is supplied by the members' destructors.
CMorphBall::~CMorphBall() {}

// Material 59 has no established semantic name in this checkout.
CMorphBall::CMorphBall(CPlayer& player, float radius, bool multiplayer)
: mPlayer(player)
, mLoadedModelId(-1)
, mBallGlowColorIdx(0)
, mRadius(radius)
, mBoostControlForce(CVector3f::Zero())
, mControlForce(CVector3f::Zero())
, mTireMode(false)
, mTireLeanAngle(0.f)
, mBallTiltAngle(0.f)
, mCollisionSphere(
      CSphere(CVector3f(0.f, 0.f, radius), radius),
      CMaterialList(kMT_Player, kMT_Unknown59, kMT_GroundCollider, kMT_NoPlayerCollision))
, mBallModel(GetMorphBallModel(multiplayer ? "SamusMultiBallANCS" : "SamusBallCMDL", radius))
, mBallModelShader(0)
, mSpiderBallGlassModel(GetMorphBallModel("", radius))
, mSpiderBallGlassModelShader(0)
, mLowPolyBallModel(GetMorphBallModel("SamusBallLowPolyCMDL", radius))
, mLowPolyBallModelShader(0)
, mFrozenBallModel(GetMorphBallModel("SamusBallFrozenCMDL", radius))
, mLastWallCollisionFrame(-1)
, mLastFloorCollisionFrame(-1)
, mBallState(kBS_Normal)
, mPlayerToSpiderNormal(CVector3f::Zero())
, mSpiderPullMovement(1.f)
, mSpiderTrackPoint(CVector3f::Zero())
, mSpiderInterpBetweenPoints(CVector3f::Zero())
, mSpiderBetweenPoints(CVector3f::Zero())
, mLinearVelocityDamping(0.f)
, mAngularVelocityDamping(0.f)
, mSpiderNearby(false)
, mTouchingSpider(false)
, mSpiderBallSwinging(false)
, mSpiderSwingInAir(true)
, mSpiderSurfaceType(kSST_None)
, mSpiderSurfaceTransform(CTransform4f::Identity())
, mSpiderSurfacePivotAngle(0.f)
, mSpiderSurfacePivotTargetAngle(0.f)
, mRefPullVelocity(0.f)
, mPlayerToSpiderTrackDistance(0.f)
, mSwingControlDirection(0.f)
, mSwingControlTime(0.f)
, mNormalizedSpiderSurfaceForces(0.f, 0.f)
, mSpiderTrackForceMagnitude(0.f)
, mSpiderViewControlMagnitude(0.f)
, mDamageTimer(0.f)
, mSpiderForcesReset(false)
, mSurfaceToWorld(CTransform4f::Identity())
, mSlowBlueTailSwoosh(
      gpSimplePool->GetObj(multiplayer ? "SlowBlueTailSwoosh_MP" : "SlowBlueTailSwoosh"))
, mSlowBlueTailSwoosh2(
      gpSimplePool->GetObj(multiplayer ? "SlowBlueTailSwoosh2_MP" : "SlowBlueTailSwoosh2"))
, mJaggyTrail(gpSimplePool->GetObj(multiplayer ? "JaggyTrail_MP" : "JaggyTrail"))
, mSideSwoosh(gpSimplePool->GetObj("SideSwooshSide"))
, mWallSpark(gpSimplePool->GetObj("WallSpark"))
, mBallInnerGlow(gpSimplePool->GetObj("BallInnerGlow"))
, mSpiderBallMagnet(gpSimplePool->GetObj("SpiderBallMagnetEffect"))
, mBoostBallGlow(gpSimplePool->GetObj("BoostBallGlow"))
, mMorphBallTransitionFlash(gpSimplePool->GetObj("MorphBallTransitionFlash"))
, mMorphBallIceBreak(gpSimplePool->GetObj("Effect_MorphBallIceBreak"))
, mBoostEffect(gpSimplePool->GetObj("BoostEffect"))
, mDeathBallOuterShell(gpSimplePool->GetObj("DeathBallOuterShell"))
, mDeathBallSpikes(gpSimplePool->GetObj("DeathBallSpikes"))
, mScrewAttackJumpFlash(gpSimplePool->GetObj("ScrewAttackJumpFlash"))
, mSlowBlueTailSwooshGen(rs_new CParticleSwoosh(mSlowBlueTailSwoosh, 0))
, mSlowBlueTailSwooshGen2(rs_new CParticleSwoosh(mSlowBlueTailSwoosh, 0))
, mSlowBlueTailSwoosh2Gen(rs_new CParticleSwoosh(mSlowBlueTailSwoosh2, 0))
, mSlowBlueTailSwoosh2Gen2(rs_new CParticleSwoosh(mSlowBlueTailSwoosh2, 0))
, mJaggyTrailGen(rs_new CParticleSwoosh(mJaggyTrail, 0))
, mSideSwooshGen(multiplayer ? nullptr : rs_new CParticleSwoosh(mSideSwoosh, 0))
, mSideSwooshGen2(multiplayer ? nullptr : rs_new CParticleSwoosh(mSideSwoosh, 0))
, mWallSparkGen(rs_new CElementGen(mWallSpark))
, mBallInnerGlowGen(rs_new CElementGen(mBallInnerGlow))
, mSpiderBallMagnetGen(rs_new CElementGen(mSpiderBallMagnet))
, mBoostBallGlowGen(rs_new CElementGen(mBoostBallGlow))
, mBoostEffectGen(nullptr)
, mMorphBallTransitionFlashGen(nullptr)
, mMorphBallIceBreakGen(nullptr)
, mDeathBallOuterShellGen(nullptr)
, mDeathBallSpikesGen(nullptr)
, mScrewAttackJumpFlashGen(nullptr)
, mScrewAttackWallJumpFlashGen(nullptr)
, mWakeEffectIndex(-1)
, mBallInnerGlowLight(kInvalidUniqueId)
, mBallLightActive(false)
, mWorldShadow(rs_new CWorldShadow(16, 16, false))
, mActorLights(
      rs_new CActorLights(8, CVector3f::Zero(), 4, 4, 0.1f, false, false, false, false))
, mRainSplashGen(rs_new CRainSplashGenerator(mBallModel->GetScale(), 40, 2, 0.15f, 0.5f))
, mTireFactor(0.f)
, mMaxTireFactor(0.5f)
, mTireInterpolationSpeed(1.f)
, mTireInterpolating(false)
, mBoostOverLightFactor(0.f)
, mBoostLightFactor(0.f)
, mSpiderLightFactor(0.f)
, mBallOrientationAverage(CQuaternion::NoRotation())
, mBallPositionAverage(CVector3f::Zero())
, mLiftSpeedAverage(0.f)
, mLiftControlForceAverage(CVector3f::Zero())
, mFailsafeCounter(0)
, mVelocityBeforeFailsafe(CVector3f::Zero())
, mVelocityAfterFailsafe(CVector3f::Zero())
, mBoostEnabled(true)
, mTouchedFloorDuringBoost(false)
, mBoostChargeTime(0.f)
, mTimeNotInBoost(1000.f)
, x1028_(0.f)
, mBoostDrainTime(0.f)
, mBoostEffectTime(0.f)
, mBoostDamageScale(1.f)
, mDisableSpiderBallTime(0.f)
, mHasSpiderBoostDirection(false)
, mBoostTrailFadeTimer(0.f)
, mInHalfPipeMode(false)
, mInHalfPipeModeInAir(false)
, mTouchedHalfPipeRecently(false)
, mBallCloseToCollision(false)
, mCloseToCollisionTime(0.f)
, mTouchHalfPipeCooldown(0.f)
, mDisableControlCooldown(0.f)
, mTouchedHalfPipeRecentCooldown(0.f)
, mPrevHalfPipeNormal(CVector3f::Zero())
, mHalfPipeNormal(CVector3f::Zero())
, mBallAnimationIndex(0)
, mRollSfxId(0xffff)
, mLandSfxId(0xffff)
, mWallSparkFrameCountdown(1)
, mEndScrewAttackRequested(false)
, mTouchingWall(false)
, mPendingRecoil(false)
, mRecoiling(false)
, mWallJumpInputPending(false)
, mCollidedDuringRecovery(false)
, x18a8_30_(false)
, mForcedScrewJumpInput(false)
, mScrewAttackJumpCount(0)
, mWallJumpCount(0)
, mScrewAttackExitAnimationFrames(0)
, mScrewAttackGroundedFrames(0)
, mTimeSinceScrewAttackJump(0.f)
, mWallContactTime(0.f)
, mScrewAttackRecoveryCollisionTime(0.f)
, mWallNormal(CVector3f::Zero())
, mScrewAttackDirection(CVector3f::Zero())
, mBoostState(kBBS_BoostAvailable)
, mBombJumpState(kBJS_BombJumpAvailable)
, mDamageEffect(0.f)
, mDamageEffectDecaySpeed(0.f)
, mDamageTime(0.f)
, mMultiplayer(multiplayer)
, mShadow(nullptr) {
  mSpiderBallMagnetGen->SetParticleEmission(false);
  mSpiderBallMagnetGen->Update(double(1.f / 60.f));
  sBallCloseToCollisionDistance = GetBallRadius() + 0.2f;
  InitializeWakeEffects();
  mDeathBallDamageCooldowns.reserve(16);
  // TODO: recover the single-player material preparation calls (see research notes).
  mPlayer.SetCollisionAccuracyModifier(5.f);
}
