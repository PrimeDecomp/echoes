#include "MetroidPrime/Player/CMorphBall.hpp"

#include "Collision/CCollidableAABox.hpp"
#include "Collision/CCollidableSphere.hpp"
#include "Collision/CCollisionPrimitive.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Particles/CDeferredParticleEffect.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CParticleSwoosh.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CFluidPlaneManager.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CRainSplashGenerator.hpp"
#include "MetroidPrime/CWorldShadow.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Player/CMorphBallShadow.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakBall.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerControls.hpp"

#include "rstl/math.hpp"

// Structure-first reconstruction. TODO bodies below are scaffolds, not equivalent implementations.

const CMorphBall::SColorRgb CMorphBall::skBallHullGlowColors[3] = {
    {102, 196, 255},
    {255, 128, 51},
    {255, 204, 0},
};

// Roll sounds by material; the second set is used in multiplayer.
static const ushort skBallRollSfx[2][26] = {
    {
        0xffff, 0x1c10, 0xa5, 0x741, 0x1d48, 0xffff, 0xa4, 0x1c18,
        0x4cd, 0x1d8f, 0x1d90, 0x1d49, 0x1c6c, 0xffff, 0x1c6d, 0x1c6e,
        0xffff, 0x1ad0, 0x1d91, 0x1d92, 0xffff, 0xffff, 0x4cb, 0x621,
        0xffff, 0x1d5e,
    },
    {
        0xffff, 0x263b, 0x2633, 0x262f, 0x2631, 0xffff, 0x2630, 0x2636,
        0x262c, 0x262b, 0x262d, 0x263a, 0x262e, 0xffff, 0x2637, 0x263e,
        0xffff, 0x2639, 0x2634, 0x263d, 0xffff, 0xffff, 0x263f, 0x2635,
        0xffff, 0x2638,
    },
};

// Landing sounds by material; the second set is used in multiplayer.
static const ushort skBallLandSfx[2][26] = {
    {
        0xffff, 0x8e, 0xa9, 0x73f, 0x1d42, 0xffff, 0xa7, 0x1c17,
        0x4d0, 0x1c5b, 0x1c5c, 0x1d43, 0x1c69, 0xffff, 0x1c6a, 0x1c6b,
        0xffff, 0x4df, 0x1d36, 0x1d37, 0xffff, 0xffff, 0x4ca, 0x620,
        0xffff, 0x1d5b,
    },
    {
        0xffff, 0x26e4, 0x26e1, 0x26df, 0x2721, 0xffff, 0x26e0, 0x2701,
        0x26de, 0x270e, 0x270f, 0x2722, 0x2710, 0xffff, 0x2711, 0x2712,
        0xffff, 0x26e3, 0x1d36, 0x1d37, 0xffff, 0xffff, 0x26e5, 0x26e2,
        0xffff, 0x2729,
    },
};

inline CColor CMorphBall::GetBallGlowColor(const SColorRgb& color) {
  return CColor(color.mR, color.mG, color.mB, 0xff);
}

// Guessed names for TU-local state.
static float sBallCloseToCollisionDistance;
static rstl::reserved_vector< int, 64 > sWakeEffectForMaterial;

void CMorphBall::DeleteBallShadow() { mShadow = nullptr; }

void CMorphBall::CreateBallShadow() {
  if (!mShadow.get()) {
    mShadow = rs_new CMorphBallShadow(64, 64, gpSimplePool->GetObj("TXTR_BallFade"));
  }
}

void CMorphBall::RenderToShadowTex(CStateManager& mgr) {
  if (mShadow.get() == nullptr) {
    return;
  }

  const float ballRadius = mRadius;
  const CVector3f center =
      mPlayer.GetTranslation() + mPlayer.GetPrimitiveOffset() + CVector3f(0.f, 0.f, ballRadius);
  const float extent = 1.5f * mRadius;
  const CAABox aabb(CVector3f(center.GetX() - extent, center.GetY() - extent, center.GetZ() - 10.f),
                    CVector3f(center.GetX() + extent, center.GetY() + extent, center.GetZ()));
  mShadow->RenderIdBuffer(aabb, mgr, mPlayer);
}

void CMorphBall::DrawBallShadow(CStateManager& mgr) {
  if (mShadow.get() != nullptr) {
    float alpha = 1.f;
    switch (mPlayer.GetMorphballTransitionState()) {
    case CPlayer::kMS_Morphed:
      alpha = 1.f;
      break;
    case CPlayer::kMS_Unmorphed:
      return;
    case CPlayer::kMS_Unmorphing: {
      const float t = mPlayer.GetMorphBallTransitionFactor();
      alpha = 1.f - t;
      break;
    }
    case CPlayer::kMS_Morphing: {
      const float t = mPlayer.GetMorphBallTransitionFactor();
      alpha = t;
      break;
    }
    }

    mShadow->Render(mgr, alpha, *mgr.GetShadowTex());
  }
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

void CMorphBall::StartLandingSfx() {
  if (mPlayer.GetVelocityWR().GetZ() < -5.f && mLandSfxId != 0xffff) {
    const uchar vol =
        CCast::ToUint8(CMath::Clamp(95.f, 1.6f * mPlayer.GetLastVelocity().GetZ() + 95.f, 127.f));
    CSfxHandle handle = CSfxManager::SfxStart(
        mLandSfxId, vol, mPlayer.GetSoundPan(CPlayer::kMSP_4), CSfxManager::kAllAreas, true);
    mPlayer.ApplySubmergedPitchBend(handle);
  }
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

void CMorphBall::StartScrewAttackSfx() {
  if (mScrewAttackSfx) {
    CSfxManager::SfxStop(mScrewAttackSfx);
  }
  mScrewAttackSfx = CSfxManager::AddEmitter(mMultiplayer ? 0x25e2 : 0x2203, mPlayer.GetTranslation(),
                                            mPlayer.GetCurrentAreaId().Value(), true, true);
}

// Scaffold, not a reconstructed implementation.
void CMorphBall::UpdateMorphBallSound(float dt, CStateManager& mgr) {
  // TODO: Maintain the roll, Spider, death-ball and Screw Attack emitters.
}

void CMorphBall::SelectMorphBallSounds(const CMaterialList& material) {
  const int sfxSet = mMultiplayer ? 1 : 0;
  short rollSfx;
  if (!InScrewAttackMode()) {
    if (mPlayer.GetSelectFluidBallSound()) {
      if (mMultiplayer) {
        rollSfx = 0x263c;
      } else {
        rollSfx = 0x94;
      }
    } else {
      rollSfx = CPlayer::SfxIdFromMaterial(material, skBallRollSfx[sfxSet], 26, 0xffff);
    }
  } else {
    rollSfx = 0x25a;
  }
  mPlayer.SetSelectFluidBallSound(false);

  if (rollSfx != 0xffff) {
    if (mRollSfxId != rollSfx && mRollSfx) {
      CSfxManager::SfxStop(mRollSfx);
      mRollSfx.Clear();
    }
    mRollSfxId = rollSfx;
  }

  if (!InScrewAttackMode()) {
    mLandSfxId = CPlayer::SfxIdFromMaterial(material, skBallLandSfx[sfxSet], 26, 0xffff);
  } else {
    mLandSfxId =
        CPlayer::SfxIdFromMaterial(material, CPlayer::skPlayerLandSfxHard[sfxSet], 26, 0xffff);
  }
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

void CMorphBall::FluidFXThink(CActor::EFluidState state, CScriptWater& water, CStateManager& mgr) {
  const float speed = mPlayer.GetVelocityWR().Magnitude();
  const CVector3f splashPos(mPlayer.GetTranslation().GetX(), mPlayer.GetTranslation().GetY(),
                            water.GetTriggerBoundsWR().GetMaxPoint().GetZ());

  if (speed >= 8.f) {
    const float maxVel = mPlayer.GetBallMaxVelocity();
    if (mgr.GetFluidPlaneManager()->GetLastSplashDeltaTime(mPlayer.GetUniqueId()) >=
        0.1f * ((maxVel - speed) / (maxVel - 8.f))) {
      mgr.GetFluidPlaneManager()->CreateSplash(mPlayer.GetUniqueId(), mgr, water, splashPos, 0.f,
                                               state == CActor::kFS_EnteredFluid);
    }
  }

  const CVector2f flatVelocity(mPlayer.GetVelocityWR().GetX(), mPlayer.GetVelocityWR().GetY());
  const float flatMoveSpeed = flatVelocity.Magnitude();
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

float CMorphBall::ComputeMaxSpeed() const {
  float maxSpeed;
  if (GetIsInHalfPipeMode()) {
    maxSpeed = rstl::max_val(1.5f * mPlayer.GetVelocityWR().Magnitude(), 0.01f);
    maxSpeed = rstl::min_val(maxSpeed, 95.f);
  } else {
    maxSpeed = gpTweakBall->GetBallTranslationMaxSpeed(mPlayer.GetSurfaceRestraint());
  }
  return maxSpeed;
}

void CMorphBall::SpinToSpeed(float speed, const CVector3f& direction, float dt) {
  const float angularSpeed = mPlayer.GetAngularVelocityWR().GetVector().Magnitude();
  mPlayer.ApplyTorqueWR(dt * (speed - angularSpeed) * direction);
}

void CMorphBall::ApplyGravity() {
  mPlayer.SetMomentumWR(CVector3f(0.f, 0.f, mPlayer.GetMass() * GetGravityAcceleration()));
}

float CMorphBall::GetGravityAcceleration() const {
  if (mPlayer.CheckSubmerged() &&
      !mPlayer.GetPlayerState()->HasPowerUp(CPlayerState::kIT_GravityBoost)) {
    return gpTweakBall->GetBallWaterGravity();
  }
  if (mBallState == kBS_ScrewAttack) {
    return gpTweakBall->GetScrewAttackGravity();
  }
  if (mBallState == kBS_ScrewAttackWallJump) {
    return gpTweakBall->GetScrewAttackWallJumpGravity();
  }
  return gpTweakBall->GetBallGravity();
}

float CMorphBall::CalculateSurfaceFriction() const {
  float friction = gpTweakBall->GetBallTranslationFriction(mPlayer.GetSurfaceRestraint());
  if (mPlayer.GetAttachedActorId() != kInvalidUniqueId) {
    friction *= 2.f;
  }

  const int drainSourceCount = mPlayer.GetEnergyDrain().GetEnergyDrainSources().size();
  if (drainSourceCount > 0) {
    friction *= 1.5f * drainSourceCount;
  }

  return friction;
}

static EMaterialTypes LiftBoundsMaterial = kMT_Unknown59; // Guessed name
static EMaterialTypes LiftRayMaterial = kMT_Unknown59;    // Guessed name

void CMorphBall::ComputeLiftForces(const CVector3f& controlForce, const CVector3f& velocity,
                                   const CStateManager& mgr) {
  const float liftSpeed = velocity.Magnitude();
  mLiftSpeedAverage.AddValue(liftSpeed);
  mLiftControlForceAverage.AddValue(controlForce);

  const CVector3f avgControlForce = mLiftControlForceAverage.GetAverage().data();
  const float avgControlForceMag = avgControlForce.Magnitude();
  if (avgControlForceMag > 12000.f) {
    const float avgLiftSpeed = mLiftSpeedAverage.GetAverage().data();
    if (avgLiftSpeed < 4.f) {
      const CTransform4f primitiveXf = mPlayer.GetPrimitiveTransform();
      const CAABox primitiveBounds = mPlayer.GetCollisionPrimitive()->CalculateAABox(primitiveXf);
      const CVector3f liftBoundsOffset(0.1f, 0.1f, -0.05f);
      const CAABox liftBounds(primitiveBounds.GetMinPoint() - liftBoundsOffset,
                              primitiveBounds.GetMaxPoint() + liftBoundsOffset);
      if (CGameCollision::DetectStaticCollisionBoolean(
              mgr, CCollidableAABox(liftBounds, CMaterialList(LiftBoundsMaterial)),
              CTransform4f::Identity(), CMaterialFilter::skPassEverything)) {
        const CVector3f liftPos =
            primitiveXf.GetTranslation() + CVector3f(0.f, 0.f, 1.75f * GetBallRadius());
        const CVector3f liftDir = avgControlForce * (1.f / avgControlForceMag);
        const CMaterialFilter rayFilter =
            CMaterialFilter::MakeInclude(CMaterialList(LiftRayMaterial));
        const CRayCastResult result = mgr.RayStaticIntersection(liftPos, liftDir, 1.4f, rayFilter);
        if (!result.IsValid()) {
          const float liftScale = 1.f - rstl::max_val(0.f, avgLiftSpeed - 3.f);
          mPlayer.ApplyForceWR(CVector3f(0.f, 0.f, liftScale * 40000.f), CAxisAngle::Identity());

          mPlayer.ApplyImpulseWR(CVector3f::Zero(),
                                 CAxisAngle::FromVector(CVector3f(-mSurfaceToWorld.Get00(),
                                                                  -mSurfaceToWorld.Get10(),
                                                                  -mSurfaceToWorld.Get20()) *
                                                        1000.f * liftScale));
        }
      }
    }
  }
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

static EMaterialTypes CloseToCollisionMaterial1 = kMT_Player; // Guessed name
static EMaterialTypes CloseToCollisionMaterial2 = kMT_Unknown59; // Guessed name

bool CMorphBall::BallCloseToCollision(const CStateManager& mgr, float distance,
                                      const CMaterialFilter& filter) const {
  const CCollidableSphere prim(
      CSphere(mPlayer.GetTranslation() + CVector3f(0.f, 0.f, GetBallRadius()), distance),
      CMaterialList(CloseToCollisionMaterial1, CloseToCollisionMaterial2));
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  mgr.BuildColliderList(nearList, mPlayer, prim.CalculateLocalAABox());

  if (CGameCollision::DetectStaticCollisionBoolean(mgr, prim, CTransform4f::Identity(), filter)) {
    return true;
  }

  for (rstl::reserved_vector< TUniqueId, 1024 >::const_iterator id = nearList.begin();
       id != nearList.end(); ++id) {
    if (const CPhysicsActor* const actor =
            TCastToConstPtr< CPhysicsActor >(mgr.GetObjectById(*id))) {
      if (CCollisionPrimitive::CollideBoolean(
              CInternalCollisionStructure::CPrimDesc(prim, filter, CTransform4f::Identity()),
              CInternalCollisionStructure::CPrimDesc(*actor->GetCollisionPrimitive(),
                                                     CMaterialFilter::GetPassEverything(),
                                                     actor->GetPrimitiveTransform()))) {
        return true;
      }
    }
  }

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

void CMorphBall::UpdateHalfPipeStatus(CStateManager& mgr, float dt) {
  mTouchHalfPipeCooldown -= dt;
  mTouchHalfPipeCooldown = rstl::max_val(0.f, mTouchHalfPipeCooldown);
  mTouchedHalfPipeRecentCooldown -= dt;
  mTouchedHalfPipeRecentCooldown = rstl::max_val(0.f, mTouchedHalfPipeRecentCooldown);

  if (mTouchHalfPipeCooldown > 0.f) {
    const float avg = *mLiftSpeedAverage.GetAverage();
    if (avg > 25.f || (GetIsInHalfPipeMode() && avg > 4.5f)) {
      SetIsInHalfPipeMode(true);
      SetIsInHalfPipeModeInAir(!mBallCloseToCollision);
      SetTouchedHalfPipeRecently(mTouchedHalfPipeRecentCooldown > 0.f);
      if (GetIsInHalfPipeModeInAir()) {
        mPrevHalfPipeNormal = CVector3f::Zero();
        mHalfPipeNormal = CVector3f::Zero();
      }
    } else {
      DisableHalfPipeStatus();
    }
  } else {
    DisableHalfPipeStatus();
  }

  if (GetIsInHalfPipeMode()) {
    mPlayer.SetCollisionAccuracyModifier(20.f);
  } else {
    mPlayer.SetCollisionAccuracyModifier(5.f);
  }
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

void CMorphBall::RenderDamageEffects(const CStateManager& mgr,
                                     const CTransform4f& transform) const {
  CRandom16 rand(99);
  const float alpha = mPlayer.GetDeathAlpha();
  const float colorComponent = 0.1f * mDamageEffect * alpha;
  const CColor color(0.25f * mDamageEffect * alpha, colorComponent, colorComponent, 1.f);
  const CModelFlags flags = CModelFlags::Additive(color).DepthCompareUpdate(true, false);

  for (int i = 0; i < 5; ++i) {
    const float randX = rand.Float();
    const float randY = rand.Float();
    const float randZ = rand.Float();
    const float randomPhase = M_PIF * rand.Float();
    const float phase = 30.f * mDamageTime + randomPhase;
    const float translateMag = mDamageEffect * CMath::FastSinR(phase) * 0.15f;
    CTransform4f modelXf =
        transform * CTransform4f::Translate(
                        CVector3f(randX * translateMag, randY * translateMag, randZ * translateMag));
    mBallModel->RenderSolid(CModelData::kWM_Normal, modelXf, false, flags);
  }
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

void CMorphBall::RenderMorphBallTransitionFlash(const CStateManager& mgr) const {
  if (mMorphBallTransitionFlashGen.get() != nullptr) {
    mMorphBallTransitionFlashGen->SetModulationColor(
        GetBallGlowColor(skBallHullGlowColors[mBallGlowColorIdx]));
    mMorphBallTransitionFlashGen->Render();
  }
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

float CMorphBall::GetMinimumAlignmentSpeed() const {
  if (mBallState == kBS_Spider) {
    return 0.f;
  }
  return gpTweakBall->GetMinimumAlignmentSpeed();
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

void CMorphBall::EnterMorphBallState(CStateManager& mgr, EBallState state) {
  mBallState = state;
  mTireFactor = 0.f;
  mBoostTrailFadeTimer = 0.f;
  UpdateEffects(0.f, mgr);
  mBallAnimationIndex = 0;
  StopParticleWakes();
  StopSounds();
  mBoostOverLightFactor = 0.f;
  mBoostLightFactor = 0.f;
  mSpiderLightFactor = 0.f;
  DisableHalfPipeStatus();
  mBallTiltAngle = 0.f;
  mTireLeanAngle = 0.f;
  mScrewAttackJumpCount = 0;
  mWallJumpCount = 0;
  mScrewAttackGroundedFrames = 0;
  mScrewAttackExitAnimationFrames = 0;
  mScrewAttackRecoveryCollisionTime = 0.f;
  mPendingRecoil = false;
  mRecoiling = false;
  mEndScrewAttackRequested = false;
  mWallJumpInputPending = false;
  mCollidedDuringRecovery = false;
  x18a8_30_ = false;
  mTouchedFloorDuringBoost = false;
  mTireMode = false;
  mPlayer.GetPlayerState()->SetItemAmount(CPlayerState::kIT_ActivateMorphballBoost, 0);
  if (mBallState == kBS_ScrewAttack) {
    mScrewAttackSfx = CSfxManager::AddEmitter(0x4a1, mPlayer.GetTranslation(),
                                              mPlayer.GetCurrentAreaId().Value(), true, true);
  }
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

bool CMorphBall::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                 EUserEventType type) {
  if (type == kUE_EventStart && mPlayer.GetMorphballTransitionState() == CPlayer::kMS_Morphed &&
      mBallState == kBS_ScrewAttackRecovery) {
    mPlayer.fn_80184294(CPlayer::kMS_Morphed);
    return true;
  }
  return false;
}

void CMorphBall::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_XCRT:
    if (mBallInnerGlowGen.get() && mBallInnerGlowGen->SystemHasLight()) {
      mBallInnerGlowLight = mgr.AllocateUniqueId();
      const uint sourceId =
          mBallInnerGlow.GetTag().id + mgr.MaskUIdNumPlayers(mPlayer.GetUniqueId());
      mgr.AddObject(rs_new CGameLight(mBallInnerGlowLight, kInvalidAreaId, false,
                                      rstl::string_l("BallLight"), GetBallToWorld(),
                                      mPlayer.GetUniqueId(), mBallInnerGlowGen->GetLight(),
                                      sourceId, 0, 0.f));
    }
    break;
  case kSM_XDelete:
    DeleteLight(mgr);
    break;
  }
}

void CMorphBall::Update(float dt, CStateManager& mgr) {
  if (mBallState == kBS_Spider) {
    CreateSpiderBallParticles(mgr, GetBallPosition(), mSpiderTrackPoint);
  }

  if (mMultiplayer) {
    mBallModelShader = mPlayer.GetCurrentBeam();
  }

  UpdateEffects(dt, mgr);
  UpdateBallLight(dt, mgr);

  if (mPlayer.GetDeathTime() <= 0.f) {
    UpdateDeathBall(dt, mgr);
  }

  if (mDamageEffect > 0.f) {
    mDamageEffect -= mDamageEffectDecaySpeed * dt;
    if (mDamageEffect <= 0.f) {
      mDamageEffect = 0.f;
      mDamageEffectDecaySpeed = 0.f;
      mDamageTime = 0.f;
    } else {
      mDamageTime += dt;
    }
  }

  if (mTireInterpolating) {
    mTireFactor += mTireInterpolationSpeed * dt;
    if (mTireFactor < 0.f) {
      mTireInterpolating = false;
      mTireFactor = 0.f;
    } else if (mTireFactor > mMaxTireFactor) {
      mTireInterpolating = false;
      mTireFactor = mMaxTireFactor;
    }
  }

  mBoostTrailFadeTimer -= dt;
  mBoostTrailFadeTimer = rstl::max_val(0.f, mBoostTrailFadeTimer);

  if (mRainSplashGen.get() != nullptr) {
    mRainSplashGen->Update(dt, mgr);
  }

  UpdateMorphBallSound(dt, mgr);
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

float CMorphBall::BallTurnInput(const CFinalInput& input) const {
  if (!IsMovementAllowed()) {
    return 0.f;
  }

  const float turnLeftInput =
      mPlayer.GetControlMapper().GetAnalogInput(CControlMapper::kC_TurnLeft, input);
  const float turnRightInput =
      mPlayer.GetControlMapper().GetAnalogInput(CControlMapper::kC_TurnRight, input);

  return turnLeftInput - turnRightInput;
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

void CMorphBall::TransformSpiderBallState(const CQuaternion& rotation,
                                          const CVector3f& translation) {
  if (mBallState != kBS_Spider) {
    return;
  }

  mSpiderSwingInAir = true;
  CPhysicsState state = mPlayer.GetPhysicsState();
  const CTransform4f rotationXf = rotation.BuildTransform4f();
  state.SetConstantForceWR(rotationXf.Rotate(state.GetConstantForceWR()));
  state.SetForceWR(rotationXf.Rotate(state.GetForceWR()));
  mPlayer.SetPhysicsState(state);
  mSpiderInterpBetweenPoints = rotationXf.Rotate(mSpiderInterpBetweenPoints);
  mPlayerToSpiderNormal = rotationXf.Rotate(mPlayerToSpiderNormal);

  const CTransform4f deltaXf = CTransform4f::Translate(translation) * rotation.BuildTransform4f();
  const CTransform4f ballToWorld = GetBallToWorld();
  const CTransform4f worldToBall = ballToWorld.GetQuickInverse();
  const CTransform4f xf = ballToWorld * deltaXf * worldToBall;
  mSpiderTrackPoint = xf * mSpiderTrackPoint;
}

void CMorphBall::CreateSpiderBallParticles(CStateManager& mgr, const CVector3f& ballPos,
                                           const CVector3f& trackPoint) {
  mSpiderBallMagnetGen->SetParticleEmission(true);

  CVector3f ballToTrack = trackPoint - ballPos;
  const float ballToTrackMag = ballToTrack.Magnitude();
  const int subCount =
      static_cast< int >(ballToTrackMag / (mgr.IsMultiplayer() ? 0.5f : 0.2f) + 1.f);
  const float scale = 1.f / static_cast< float >(subCount);
  ballToTrack *= scale;
  int count = static_cast< int >(8.f * (ballToTrackMag / 2.1f));

  while (count >= 0) {
    CVector3f translation = ballPos;
    for (int i = 0; i < subCount; ++i) {
      mSpiderBallMagnetGen->SetTranslation(translation);
      mSpiderBallMagnetGen->ForceParticleCreation(1);
      translation += ballToTrack;
    }
    --count;
  }

  mSpiderBallMagnetGen->SetParticleEmission(false);
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

float CMorphBall::GetSpiderBallControllerMovement(const CFinalInput& input) const {
  if (!IsMovementAllowed()) {
    return 0.f;
  }

  const float forward =
      mPlayer.GetControlMapper().GetAnalogInput(CControlMapper::kC_Forward, input) -
      mPlayer.GetControlMapper().GetAnalogInput(CControlMapper::kC_Backward, input);
  const float turn =
      mPlayer.GetControlMapper().GetAnalogInput(CControlMapper::kC_TurnRight, input) -
      mPlayer.GetControlMapper().GetAnalogInput(CControlMapper::kC_TurnLeft, input);
  const double angleTemp = atan2(forward, turn);
  const float angle = (180.f / M_PIF) * static_cast< float >(angleTemp);
  const float hyp = CMath::SqrtF(forward * forward + turn * turn);

  if (angle > -35.f && angle < 125.f) {
    return hyp;
  }

  if (angle < -55.f || angle > 145.f) {
    return -hyp;
  }

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

bool CMorphBall::CheckForSwitchToSpiderBallSwinging(CStateManager& mgr) const {
  if (!mTouchingSpider) {
    return false;
  }

  if (1.f == mSpiderPullMovement) {
    if (mSpiderBallSwinging) {
      CVector3f closestPoint = CVector3f::Zero();
      CVector3f interpDeltaBetweenPoints = CVector3f::Zero();
      CVector3f deltaBetweenPoints = CVector3f::Zero();
      float distance = 0.f;
      CVector3f normal = CVector3f::Zero();
      CTransform4f surfaceTransform(CTransform4f::Identity());
      ESpiderSurfaceType surfaceType;
      TUniqueId surfaceId = kInvalidUniqueId;
      if (FindClosestSpiderBallWaypoint(mgr, GetBallPosition(), closestPoint,
                                        interpDeltaBetweenPoints, deltaBetweenPoints, distance,
                                        normal, surfaceType, surfaceId, surfaceTransform)) {
        if (distance < 2.1f) {
          return false;
        }
      }

      return true;
    }

    return false;
  }

  if (mSpiderBallSwinging) {
    return true;
  }

  return CMath::AbsF(mPlayerToSpiderNormal.GetZ()) > 0.9f;
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

CVector2f CMorphBall::CalculateSpiderBallAttractionSurfaceForces(const CFinalInput& input) const {
  if (!IsMovementAllowed()) {
    return CVector2f::Zero();
  }

  const float forwardBack =
      mPlayer.GetControlMapper().GetAnalogInput(CControlMapper::kC_Forward, input) -
      mPlayer.GetControlMapper().GetAnalogInput(CControlMapper::kC_Backward, input);
  const float rightLeft =
      mPlayer.GetControlMapper().GetAnalogInput(CControlMapper::kC_TurnRight, input) -
      mPlayer.GetControlMapper().GetAnalogInput(CControlMapper::kC_TurnLeft, input);
  return CVector2f(rightLeft, forwardBack);
}

CVector3f CMorphBall::TransformSpiderBallForcesXY(CVector2f& forces, CStateManager& mgr) const {
  const CTransform4f camXf = mPlayer.GetCameraManager()->GetCurrentCamera(mgr, true)->GetTransform();
  CVector3f ret = camXf.GetColumn(kDX) * forces.GetX() + camXf.GetColumn(kDY) * forces.GetY();
  return ret;
}

CVector3f CMorphBall::TransformSpiderBallForcesXZ(CVector2f& forces, CStateManager& mgr) const {
  const CTransform4f camXf = mPlayer.GetCameraManager()->GetCurrentCamera(mgr, true)->GetTransform();
  CVector3f ret = camXf.GetColumn(kDX) * forces.GetX() + camXf.GetColumn(kDZ) * forces.GetY();
  return ret;
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

bool CMorphBall::IsMovementAllowed() const {
  if (!mPlayer.GetTweakPlayerControls()->GetMoveDuringFreeLook() &&
      (mPlayer.IsInFreeLook() || mPlayer.IsLookButtonHeld())) {
    return false;
  }

  if (mPlayer.IsMorphBallTransitioning()) {
    return false;
  }

  return !(mDisableControlCooldown > 0.f);
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

float CMorphBall::ForwardInput(const CFinalInput& input) const {
  if (!IsMovementAllowed()) {
    return 0.f;
  }

  const float forwardInput =
      mPlayer.GetControlMapper().GetAnalogInput(CControlMapper::kC_Forward, input);
  const float backwardInput =
      mPlayer.GetControlMapper().GetAnalogInput(CControlMapper::kC_Backward, input);

  return forwardInput - backwardInput;
}

float CMorphBall::GetBallTouchRadius() const { return gpTweakBall->GetBallTouchRadius(); }

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
