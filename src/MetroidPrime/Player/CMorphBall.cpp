#include "MetroidPrime/Player/CMorphBall.hpp"

#include "Collision/CCollidableAABox.hpp"
#include "Collision/CCollidableSphere.hpp"
#include "Collision/CCollisionPrimitive.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Collision/CollisionUtil.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Particles/CDeferredParticleEffect.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CParticleElectric.hpp"
#include "Kyoto/Particles/CParticleSwoosh.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CEnvFxManager.hpp"
#include "MetroidPrime/CFluidPlaneManager.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CRainSplashGenerator.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/CWorldShadow.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Player/CMorphBallShadow.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerBodyController.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDamageableTrigger.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpiderBallAttractionSurface.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpiderBallWaypoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakBall.hpp"
#include "MetroidPrime/Weapons/WeaponSound.hpp"
#include "WorldFormat/CCollisionCache.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerControls.hpp"

#include <string.h>

#include "rstl//algorithm.hpp"
#include "rstl/math.hpp"

// Structure-first reconstruction. TODO bodies below are scaffolds, not equivalent implementations.

const SMorphBallModelInfo CMorphBall::skBallCharacter[3] = {
    {"SamusBallCMDL", 0},
    {"SamusBallDarkCMDL", 0},
    {"SamusBallLightCMDL", 0},
};
const SMorphBallModelInfo CMorphBall::skBallLowPoly[3] = {
    {"SamusBallLowPolyCMDL", 0},
    {"SamusBallLowPolyCMDL", 0},
    {"SamusBallLowPolyCMDL", 0},
};
const SMorphBallModelInfo CMorphBall::skSpiderBallCharacter[3] = {
    {"SamusBallCMDL", 0},
    {"SamusSpiderBallDarkCMDL", 0},
    {"SamusBallLightCMDL", 0},
};
const SMorphBallModelInfo CMorphBall::skSpiderBallLowPoly[3] = {
    {"SamusSpiderBallLowPolyCMDL", 0},
    {"SamusSpiderBallLowPolyCMDL", 0},
    {"SamusSpiderBallLowPolyCMDL", 0},
};
const SMorphBallModelInfo CMorphBall::skBoostBallCharacter[3] = {
    {"SamusBallCMDL", 0},
    {"SamusBoostBallDarkCMDL", 0},
    {"SamusBallLightCMDL", 0},
};
const SMorphBallModelInfo CMorphBall::skBoostBallLowPoly[3] = {
    {"SamusSpiderBallLowPolyCMDL", 0},
    {"SamusSpiderBallLowPolyCMDL", 0},
    {"SamusSpiderBallLowPolyCMDL", 0},
};
const SMorphBallModelInfo CMorphBall::skSpiderBallGlass[3] = {
    {nullptr, 0},
    {"SamusSpiderBallDarkCapsCMDL", 0},
    {nullptr, 0},
};
const SMorphBallModelInfo CMorphBall::skFrozenBall[3] = {
    {"SamusBallFrozenCMDL", 0},
    {"SamusBallFrozenCMDL", 0},
    {"SamusBallFrozenCMDL", 0},
};
const uint CMorphBall::skBallGlowColorIdx[3] = {0, 1, 2};
const uint CMorphBall::skSpiderBallGlowColorIdx[3] = {0, 1, 2};
const uint CMorphBall::skBoostBallGlowColorIdx[3] = {0, 1, 2};

const CMorphBall::SColorRgb CMorphBall::skBallLightModulationColors[3] = {
    {102, 196, 255},
    {255, 128, 51},
    {255, 255, 204},
};

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
    mPlayer.ApplySubmergedPitchBend(CSfxManager::SfxStart(
        mLandSfxId, vol, mPlayer.GetSoundPan(CPlayer::kMSP_4), CSfxManager::kAllAreas, true));
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
  mScrewAttackSfx =
      CSfxManager::AddEmitter(mMultiplayer ? 0x25e2 : 0x2203, mPlayer.GetTranslation(),
                              mPlayer.GetCurrentAreaId().Value(), true, true);
}

void CMorphBall::UpdateMorphBallSound(float dt, CStateManager& mgr) {
  CVector3f velocity = mPlayer.GetVelocityWR();
  if (mBallState != kBS_Spider) {
    velocity.SetZ(0.f);
  }

  switch (mPlayer.GetPlayerMovementState()) {
  case NPlayer::kMS_OnGround:
  case NPlayer::kMS_FallingMorphed: {
    float speed = velocity.Magnitude();
    if (mBallState == kBS_Spider) {
      speed += 4.f * (dt * gpTweakBall->GetBallGravity());
    }

    bool rolling = false;
    if (!InScrewAttackMode() || mBallState == kBS_ScrewAttackRecovery) {
      rolling = true;
    }

    if (rolling && speed > 0.8f) {
      if (!mRollSfx) {
        if (mRollSfxId != 0xffff) {
          mRollSfx = AddEmitter(mPlayer, mRollSfxId, true, true, CSfxManager::kMedPriority, 0x7f,
                                0x14, 150.f, 1.f);
        }
        mPlayer.ApplySubmergedPitchBend(mRollSfx);
      }

      CSfxManager::PitchBend(mRollSfx,
                             CMath::Clamp(0, static_cast< int >(speed) * 500 + 0x2b4, 0x4000));
      const uchar vol = CCast::ToUint8(CMath::Clamp(64.f, 3.2f * speed + 64.f, 127.f));
      CSfxManager::UpdateEmitter(mRollSfx, mPlayer.GetTranslation(), CVector3f::Zero(), vol);
      break;
    }
  }
  default:
    if (mRollSfx) {
      CSfxManager::SfxStop(mRollSfx);
      mRollSfx.Clear();
    }
    break;
  }

  if (mBoostReleaseSfx) {
    if (!CSfxManager::IsPlaying(mBoostReleaseSfx) && !CSfxManager::IsQueued(mBoostReleaseSfx)) {
      mBoostReleaseSfx.Clear();
    } else {
      CSfxManager::UpdateEmitter(mBoostReleaseSfx, mPlayer.GetTranslation(), CVector3f::Zero(),
                                 0x7f);
    }
  }

  if (mBoostChargeSfx) {
    CSfxManager::UpdateEmitter(mBoostChargeSfx, mPlayer.GetTranslation(), CVector3f::Zero(), 0x7f);
  }

  if (mBallState == kBS_Spider) {
    if (!mSpiderSfx) {
      mSpiderSfx = AddEmitter(mPlayer, mgr.ReturnFirstIfSingleElseSecond(0x159, 0x2641), true, true,
                              0xc8, 0x7f, 0x14, 150.f, 1.f);
      mPlayer.ApplySubmergedPitchBend(mSpiderSfx);
    }
    CSfxManager::UpdateEmitter(mSpiderSfx, mPlayer.GetTranslation(), CVector3f::Zero(), 0x7f);
  } else if (mSpiderSfx) {
    CSfxManager::SfxStop(mSpiderSfx);
    mSpiderSfx.Clear();
  }

  if (mPlayer.GetPlayerState()->GetItemAmount(CPlayerState::kIT_DeathBall, true) != 0) {
    if (!mDeathBallSfx) {
      mDeathBallSfx = CSfxManager::AddEmitter(0x2611, mPlayer.GetTranslation(),
                                              mPlayer.GetCurrentAreaId().Value(), true, true, 0xc8);
      mPlayer.ApplySubmergedPitchBend(mDeathBallSfx);
    }
    CSfxManager::UpdateEmitter(mDeathBallSfx, mPlayer.GetTranslation(), CVector3f::Zero(), 0x7f);
  } else if (mDeathBallSfx) {
    CSfxManager::SfxStop(mDeathBallSfx);
    mDeathBallSfx.Clear();
  }
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

void CMorphBall::LoadMorphBallModel() {
  if (!mMultiplayer) {
    CPlayerState* playerState = mPlayer.GetPlayerState();
    const bool boostBall = playerState->HasPowerUp(CPlayerState::kIT_BoostBall);
    const bool spiderBall = playerState->HasPowerUp(CPlayerState::kIT_SpiderBall);
    const int modelIdx = playerState->GetCurrentSuitRaw();
    int loadModelId = modelIdx;
    if (spiderBall) {
      loadModelId = modelIdx + 3;
    } else if (boostBall) {
      loadModelId = modelIdx + 6;
    }

    if (mLoadedModelId == loadModelId) {
      return;
    }

    mLoadedModelId = loadModelId;
    if (spiderBall) {
      mBallModel =
          GetMorphBallModel(rstl::string_l(skSpiderBallCharacter[modelIdx].mName), mRadius);
      mBallModelShader = skSpiderBallCharacter[modelIdx].mShader;
      mLowPolyBallModel =
          GetMorphBallModel(rstl::string_l(skSpiderBallLowPoly[modelIdx].mName), mRadius);
      mLowPolyBallModelShader = skSpiderBallLowPoly[modelIdx].mShader;
      if (skSpiderBallGlass[modelIdx].mName != nullptr) {
        mSpiderBallGlassModel =
            GetMorphBallModel(rstl::string_l(skSpiderBallGlass[modelIdx].mName), mRadius);
        mSpiderBallGlassModelShader = skSpiderBallGlass[modelIdx].mShader;
      } else {
        mSpiderBallGlassModel = nullptr;
        mSpiderBallGlassModelShader = 0;
      }
      mBallGlowColorIdx = skSpiderBallGlowColorIdx[modelIdx];
    } else if (boostBall) {
      mBallModel = GetMorphBallModel(rstl::string_l(skBoostBallCharacter[modelIdx].mName), mRadius);
      mBallModelShader = skBoostBallCharacter[modelIdx].mShader;
      mLowPolyBallModel =
          GetMorphBallModel(rstl::string_l(skBoostBallLowPoly[modelIdx].mName), mRadius);
      mLowPolyBallModelShader = skBoostBallLowPoly[modelIdx].mShader;
      mBallGlowColorIdx = skBoostBallGlowColorIdx[modelIdx];
    } else {
      mBallModel = GetMorphBallModel(rstl::string_l(skBallCharacter[modelIdx].mName), mRadius);
      mBallModelShader = skBallCharacter[modelIdx].mShader;
      mLowPolyBallModel = GetMorphBallModel(rstl::string_l(skBallLowPoly[modelIdx].mName), mRadius);
      mLowPolyBallModelShader = skBallLowPoly[modelIdx].mShader;
      mBallGlowColorIdx = skBallGlowColorIdx[modelIdx];
    }
  }

  const float scale = 2.f * GetBallRadius();
  mBallModel->SetScale(CVector3f(scale, scale, scale));
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

          mPlayer.ApplyImpulseWR(
              CVector3f::Zero(),
              CAxisAngle::FromVector(CVector3f(-mSurfaceToWorld.Get00(), -mSurfaceToWorld.Get10(),
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

static EMaterialTypes CloseToCollisionMaterial1 = kMT_Player;    // Guessed name
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

void CMorphBall::SetTouchedHalfPipeRecently(bool touched) { mTouchedHalfPipeRecently = touched; }

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
        transform * CTransform4f::Translate(CVector3f(randX * translateMag, randY * translateMag,
                                                      randZ * translateMag));
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

void CMorphBall::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  if (1.f == mBoostLightFactor) {
    return;
  }

  CActorLights* lights = mPlayer.ActorLights();
  const bool lowDamage = mDamageEffect < 0.25f;
  lights->SetFindShadowLight(CWorldShadow::CanRender(mgr) && lowDamage);
  lights->SetShadowDynamicRangeThreshold(0.05f);
  lights->SetNeedsRelight(true);

  CCollidableSphere sphere = mCollisionSphere;
  sphere.SetSphere(CSphere(CVector3f::Zero(), sphere.GetSphere().GetRadius()));
  CAABox ballAABB = sphere.CalculateAABox(GetBallToWorld());

  int areaId = mPlayer.GetCurrentAreaId().Value();
  if (areaId != kInvalidAreaId.Value()) {
    const CWorld* world = mgr.GetWorld();
    if (world->GetAreaAlways(TAreaId(areaId)).IsLoaded()) {
      lights->BuildAreaLightList(mgr, world->GetAreaAlways(TAreaId(areaId)), ballAABB);
    }
  }

  lights->BuildDynamicLightList(mgr, ballAABB);

  if (mPlayer.ActorLights()->HasShadowLight()) {
    CCollidableSphere shadowSphere = mCollisionSphere;
    shadowSphere.SetSphere(CSphere(CVector3f::Zero(), shadowSphere.GetSphere().GetRadius()));

    const int shadowAreaId = mPlayer.GetCurrentAreaId().Value();
    const uint lightIndex = mPlayer.ActorLights()->GetShadowLightIndex();
    mWorldShadow->BuildLightShadowTexture(mgr, TAreaId(shadowAreaId), lightIndex,
                                          shadowSphere.CalculateAABox(GetBallToWorld()), false,
                                          false);
  } else {
    mWorldShadow->ResetBlur();
  }

  lights->SetAmbientColor(
      CColor::Lerp(lights->GetAmbientColor(), CColor::White(), mBoostLightFactor));
  *mActorLights = *lights;

  const float& lightFactor = rstl::max_val(mSpiderLightFactor, mBoostLightFactor);
  mActorLights->SetAmbientColor(
      CColor::Lerp(lights->GetAmbientColor(), CColor::White(), lightFactor));
}

float CMorphBall::GetMinimumAlignmentSpeed() const {
  if (mBallState == kBS_Spider) {
    return 0.f;
  }
  return gpTweakBall->GetMinimumAlignmentSpeed();
}

void CMorphBall::DampLinearAndAngularVelocities(float linearDamping, float angularDamping,
                                                float dt) {
  CVector3f velocity = mPlayer.GetVelocityWR();
  velocity *= pow(1.f - linearDamping, 60.f * dt);
  mPlayer.SetVelocityWR(velocity);

  CAxisAngle angularVelocity = mPlayer.GetAngularVelocityWR();
  const float damping = pow(1.f - angularDamping, 60.f * dt);
  angularVelocity *= damping;
  mPlayer.SetAngularVelocityWR(angularVelocity);
}

void CMorphBall::ApplyFriction(float friction) {
  CVector3f velocity = mPlayer.GetVelocityWR();
  if (friction < velocity.Magnitude()) {
    velocity = velocity.AsNormalized() * (velocity.Magnitude() - friction);
  } else {
    velocity = CVector3f::Zero();
  }
  mPlayer.SetVelocityWR(velocity);
}

bool CMorphBall::UpdateMarbleDynamics(CStateManager& mgr, float dt, const CVector3f& point) {
  bool aligned = false;
  bool continueForce = false;
  const float maxAcceleration =
      gpTweakBall->GetMaxBallTranslationAcceleration(mPlayer.GetSurfaceRestraint());

  if (mPlayer.GetVelocityWR().Magnitude() < 3.f &&
      mBoostControlForce.Magnitude() > 0.95f * maxAcceleration) {
    CVector3f momentum = mPlayer.GetMomentumWR();
    CVector3f localMomentum = mSurfaceToWorld.TransposeRotate(momentum);
    CVector3f localControlForce = mSurfaceToWorld.TransposeRotate(mBoostControlForce);
    localMomentum.SetZ(0.f);
    localControlForce.SetZ(0.f);
    if (localMomentum.CanBeNormalized() && localControlForce.CanBeNormalized()) {
      if (CVector3f::Dot(localMomentum.AsNormalized(), localControlForce.AsNormalized()) < -0.9f) {
        continueForce = true;
      }
    }
  }

  if (!continueForce) {
    const CVector3f velocity = mPlayer.GetVelocityWR();
    CVector3f ballToPoint =
        point - (mPlayer.GetTranslation() + CVector3f(0.f, 0.f, GetBallRadius()));

    const CVector3f addVelocity =
        CVector3f::Cross(mPlayer.GetAngularVelocityWR().GetVector(), ballToPoint);
    CVector3f slipVelocity = velocity - addVelocity;

    const float minLiftSpeed = mBallState == kBS_Spider ? -1.f : 0.4f;

    float liftSpeed = 0.f;
    if (mLiftSpeedAverage.size() > 3) {
      liftSpeed = *mLiftSpeedAverage.GetEntry(0);
      liftSpeed = rstl::min_val(*mLiftSpeedAverage.GetEntry(1), liftSpeed);
      liftSpeed = rstl::min_val(*mLiftSpeedAverage.GetEntry(2), liftSpeed);
    }

    if (slipVelocity.MagSquared() > 1.f && liftSpeed > minLiftSpeed) {
      if (slipVelocity.Magnitude() > M_PIF * 8.f) {
        slipVelocity = slipVelocity.AsNormalized() * M_PIF * 8.f;
      }

      CVector3f newVelocity = velocity + addVelocity;
      if (newVelocity.CanBeNormalized()) {
        bool useTireFactor = false;
        if (mTireMode && mBallState != kBS_Spider) {
          useTireFactor = true;
        }

        const float tireFactor = useTireFactor ? 0.25f : 1.f;

        const CVector3f& newVelocityDir = newVelocity.AsNormalized();
        const CVector3f torque =
            newVelocityDir *
            (slipVelocity.Magnitude() *
             -gpTweakBall->GetBallSlipFactor(mPlayer.GetSurfaceRestraint()) * tireFactor * 0.5f /
             GetBallRadius());
        const CVector3f worldTorque = CVector3f::Cross(ballToPoint.AsNormalized(), torque);
        mPlayer.ApplyTorqueWR(worldTorque);
      }
    }
  } else {
    const float spinSpeed = 25.f / GetBallRadius();
    CVector3f rotateAxis = CVector3f::Cross(mSurfaceToWorld.GetColumn(kDZ), mBoostControlForce);
    if (rotateAxis.CanBeNormalized()) {
      SpinToSpeed(spinSpeed, rotateAxis.AsNormalized(), 800.f);
    }
  }

  const float velocityMag = mPlayer.GetVelocityWR().Magnitude();
  if (velocityMag >= GetMinimumAlignmentSpeed()) {
    const CVector3f playerRight = mPlayer.GetTransform().GetColumn(kDX);
    CVector3f surfaceRight = mSurfaceToWorld.GetColumn(kDX);
    if (CVector3f::Dot(playerRight, surfaceRight) < 0.f) {
      surfaceRight = -surfaceRight;
    }

    CVector3f upVec = CVector3f::Cross(playerRight, surfaceRight);
    if (upVec.CanBeNormalized()) {
      if (!mTireMode) {
        const CVector3f alignmentImpulse = gpTweakBall->GetTireness() * upVec.AsNormalized();
        mPlayer.SetAngularImpulseWR(
            CAxisAngle::FromVector(mPlayer.GetAngularImpulseWR().GetVector() + alignmentImpulse));
      } else {
        CVector3f right(1.f, 0.f, 0.f);
        CVector3f localRight = GetBallToWorld().TransposeRotate(surfaceRight);
        CQuaternion rotation = CQuaternion::ShortestRotationArc(right, localRight);
        mPlayer.RotateInOneFrameOR(rotation, dt);
      }
    }

    const float alignmentMagnitude = GetIsInHalfPipeMode() ? 0.2f : 0.05f;
    if (upVec.Magnitude() < alignmentMagnitude) {
      aligned = true;
    }
  }

  return aligned;
}

static EMaterialTypes BoostDamageMaterial = kMT_Unknown59; // Guessed name

void CMorphBall::ApplyBoostBallDamage(CStateManager& mgr, TUniqueId id, const CDamageInfo& damage,
                                      float dt) {
  if (rstl::find(mBoostDamagedObjects.begin(), mBoostDamagedObjects.end(), id) !=
      mBoostDamagedObjects.end()) {
    return;
  }

  CDamageInfo boostDamage(damage);
  boostDamage.SetDamage(mBoostDamageScale * boostDamage.GetDamage());
  const bool boosting = IsBoosting();
  const bool hasCannonBall =
      mPlayer.GetPlayerState()->GetItemAmount(CPlayerState::kIT_CannonBall, true) != 0;

  if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(id))) {
    const TUniqueId playerId = mPlayer.GetUniqueId();
    if (CPhysicsActor* physAct = TCastToPtr< CPhysicsActor >(mgr.ObjectById(id))) {
      const CVector3f playerVelocity = mPlayer.GetVelocityWR();
      const CVector3f relVelocity = playerVelocity - physAct->GetVelocityWR();
      const float relSpeed = relVelocity.Magnitude();
      const float playerSpeed = playerVelocity.Magnitude();
      const CVector3f hitDir =
          close_enough(relSpeed, 0.f) ? CVector3f::Zero() : relVelocity * (1.f / relSpeed);

      bool canDamage = relSpeed > gpTweakBall->GetBoostBallMinRelativeSpeedForDamage();
      bool slowHit = false;
      if (relSpeed < gpTweakBall->GetBoostBallMinRelativeSpeedForDamage() && playerSpeed < 10.f) {
        slowHit = true;
      }

      canDamage |= slowHit;
      if (canDamage) {
        CPlayer* otherPlayer = TCastToPtr< CPlayer >(physAct);
        const float knockBackSpeed = gpTweakBall->GetBoostBallCollisionKnockBackSpeed();
        if (otherPlayer) {
          bool shielded = false;
          const bool otherMorphed =
              otherPlayer->GetMorphballTransitionState() == CPlayer::kMS_Morphed;
          if (otherPlayer->GetMorphBall()->IsBoostShieldActive() && !hasCannonBall) {
            shielded = true;
          }

          if (!otherMorphed || !shielded) {
            mgr.ApplyDamage(playerId, physAct->GetUniqueId(), playerId, boostDamage,
                            CMaterialFilter(), relVelocity);
            const bool otherHasCannonBall = otherPlayer->GetPlayerState()->GetItemAmount(
                                                CPlayerState::kIT_CannonBall, true) != 0;
            if (otherHasCannonBall) {
              mgr.ApplyDamage(physAct->GetUniqueId(), playerId, physAct->GetUniqueId(), boostDamage,
                              CMaterialFilter(), -relVelocity);
            }
          }

          if (otherMorphed && !hasCannonBall && playerSpeed > 10.f) {
            const float volumeFactor = CMath::Limit((playerSpeed - 10.f) / 30.f, 1.f);
            const int volume = static_cast< int >(volumeFactor * 107.f + 20.f);
            CSfxManager::SfxStart(0x468, volume, mPlayer.GetSoundPan(CPlayer::kMSP_4));
          }

          if (boosting) {
            if (otherMorphed) {
              const float hitSpeed = shielded
                                         ? knockBackSpeed
                                         : gpTweakBall->GetBoostBallHitPlayerBallKnockBackSpeed();
              otherPlayer->SetVelocityWR(hitSpeed * hitDir + CVector3f(0.f, 0.f, hitSpeed * 0.5f));
            } else {
              otherPlayer->SetVelocityWR(gpTweakBall->GetBoostBallHitPlayerFPKnockBackSpeed() *
                                         hitDir);
            }
            otherPlayer->SetMinimalAccelerationTimer(0.25f);
            mPlayer.SetVelocityWR(-knockBackSpeed * hitDir + CVector3f(0.f, 0.f, knockBackSpeed));
            CancelBoosting();
          }
        } else {
          mgr.ApplyDamage(playerId, physAct->GetUniqueId(), playerId, boostDamage,
                          CMaterialFilter(), relVelocity);
        }
      }
    } else {
      mgr.ApplyDamage(
          playerId, actor->GetUniqueId(), playerId, boostDamage,
          CMaterialFilter::MakeIncludeExclude(CMaterialList(BoostDamageMaterial), CMaterialList()),
          CVector3f::Zero());
    }
  }

  mBoostDamagedObjects.push_back(id);
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

void CMorphBall::EnterBoosting(CStateManager& mgr, bool skipImpulse) {
  if (mBallState == kBS_Spider) {
    mBallState = kBS_SpiderBoost;
    mSpiderBoostDirection = mPlayerToSpiderNormal;
    SetDamageTimer(0.05f);
    mBoostDamageScale = 1.f;
  } else {
    mBallState = kBS_Boost;
    mBoostDamageScale = 1.f;
  }

  if (mPlayer.GetX126c24()) {
    mPlayer.SetVelocityWR(mPlayer.GetVelocityWR() * 0.4f);
  }

  if (!skipImpulse) {
    const float boostChargeTime = mBoostChargeTime;
    float incSpeed = 0.f;
    if (boostChargeTime <= gpTweakBall->GetBoostBallChargeTimeTable(0)) {
      incSpeed = gpTweakBall->GetBoostBallIncrementalSpeedTable(0);
    } else if (boostChargeTime <= gpTweakBall->GetBoostBallChargeTimeTable(1)) {
      incSpeed = gpTweakBall->GetBoostBallIncrementalSpeedTable(1);
    } else if (boostChargeTime <= gpTweakBall->GetBoostBallChargeTimeTable(2)) {
      incSpeed = gpTweakBall->GetBoostBallIncrementalSpeedTable(2);
    }

    if (GetIsInHalfPipeMode()) {
      const float speedMul = mPlayer.GetVelocityWR().Magnitude() / 95.f;
      if (speedMul > 0.3f) {
        incSpeed = incSpeed - incSpeed * (speedMul - 0.3f);
      }
      incSpeed = rstl::max_val(0.f, incSpeed);
    } else {
      incSpeed *= 1.f - mPlayer.GetVelocityWR().Magnitude() / 50.f;
    }

    CVector3f lookDir;
    lookDir.SetX(mPlayer.GetLookDir().GetX());
    lookDir.SetY(mPlayer.GetLookDir().GetY());
    lookDir.SetZ(mPlayer.GetLookDir().GetZ());
    float lookMag2d = sqrt(lookDir.GetX() * lookDir.GetX() + lookDir.GetY() * lookDir.GetY());
    double lookAngle = atan2(lookDir.GetZ(), lookMag2d);
    float vertLookAngle = CMath::Rad2Rev(lookAngle) * 360.f;
    float lookMag2dZero = 0.f;
    if (CMath::AbsF(lookMag2d - lookMag2dZero) < 0.001f &&
        mPlayer.GetPlayerMovementState() == NPlayer::kMS_OnGround) {
      const CVector3f velocity = mPlayer.GetVelocityWR();
      const float velZ = velocity.GetZ();
      float velMag2d = sqrt(velocity.GetX() * velocity.GetX() + velocity.GetY() * velocity.GetY());
      float velMag2dZero = 0.f;
      if (CMath::AbsF(velMag2d - velMag2dZero) < 0.01f && CMath::AbsF(velZ) < 2.f) {
        const CGameCamera* camera = mPlayer.GetCameraManager()->GetCurrentCamera(mgr, true);
        lookDir.SetX(camera->GetTransform().Get01());
        lookDir.SetY(camera->GetTransform().Get11());
        lookDir.SetZ(camera->GetTransform().Get21());
        lookMag2d = sqrt(lookDir.GetX() * lookDir.GetX() + lookDir.GetY() * lookDir.GetY());
        lookAngle = atan2(lookDir.GetZ(), lookMag2d);
        vertLookAngle = CMath::Rad2Rev(lookAngle) * 360.f;
      }
    }

    float speedMul = 1.f;
    if (mBallState == kBS_SpiderBoost) {
      lookDir = mSpiderBoostDirection->AsNormalized();
      speedMul = gpTweakBall->GetSpiderBallBoostScalar();
    } else if (vertLookAngle > 40.f) {
      const float speedDamp = (vertLookAngle - 40.f) / 50.f;
      speedMul = 0.35f * speedDamp + (1.f - speedDamp);
    }

    if (mPlayer.CheckSubmerged() &&
        !mPlayer.GetPlayerState()->HasPowerUp(CPlayerState::kIT_GravityBoost)) {
      speedMul *= 0.5f;
    }

    mPlayer.ApplyImpulseWR(lookDir * (speedMul * incSpeed * mPlayer.GetMass()),
                           CAxisAngle::Identity());
  }

  mBoostDrainTime = 0.f;
  mBoostChargeTime = 0.f;
  mBoostEffectTime = 0.f;
  mTimeNotInBoost = 0.f;
  mBoostDamagedObjects.clear();
  mTouchedFloorDuringBoost = false;
  mBoostTrailFadeTimer = 1.f;

  mPlayer.SetTransform(CTransform4f(mSurfaceToWorld.BuildMatrix3f(), mPlayer.GetTranslation()));
  SwitchToTire();
  mBoostEffectGen = rs_new CElementGen(mBoostEffect);
}

extern const bool kBoostBallBreaksOrbit;                               // Guessed name
static EMaterialTypes BoostSphereMaterial = kMT_Unknown59;             // Guessed name
static EMaterialTypes BoostNearListMaterial1 = kMT_Unknown59;          // Guessed name
static EMaterialTypes BoostNearListMaterial2 = kMT_NonSolidDamageable; // Guessed name

void CMorphBall::ComputeBoostBallMovement(const CFinalInput& input, CStateManager& mgr, float dt) {
  if (!IsMovementAllowed()) {
    return;
  }

  mTimeNotInBoost += dt;
  CPlayerState* playerState = mPlayer.GetPlayerState();
  const bool hasCannonBall = playerState->GetItemAmount(CPlayerState::kIT_CannonBall, true) != 0;
  if (playerState->GetItemAmount(CPlayerState::kIT_BoostBall, true) == 0 &&
      playerState->GetItemAmount(CPlayerState::kIT_ActivateMorphballBoost, true) == 0 &&
      !hasCannonBall) {
    CancelBoosting();
    LeaveBoosting();
    return;
  }

  if ((!mBoostEnabled && !hasCannonBall) || mDisableSpiderBallTime > 0.f) {
    CancelBoosting();
    LeaveBoosting();
    return;
  }

  const CTransform4f ballToWorld = GetBallToWorld();
  const bool activateBoost =
      playerState->GetItemAmount(CPlayerState::kIT_ActivateMorphballBoost, true) != 0;
  if (activateBoost) {
    playerState->SetItemAmount(CPlayerState::kIT_ActivateMorphballBoost, 0);
  }

  if (!IsBoosting() || activateBoost) {
    if (mPlayer.fn_8022b7a8(input) && !activateBoost) {
      const bool canCharge = mTimeNotInBoost > gpTweakBall->GetBoostBallDrainTime();
      if (canCharge) {
        if (mBallAnimationIndex == 0) {
          mBallAnimationIndex = 1;
          mBoostChargeSfx = AddEmitter(mPlayer, mgr.ReturnFirstIfSingleElseSecond(0x90, 0x2628),
                                       true, true, 0xb4, 0x7f, 0x14, 150.f, 1.f);
        }

        mBoostChargeTime += dt;
        if (mBoostChargeTime > gpTweakBall->GetBoostBallMaxChargeTime()) {
          mBoostChargeTime = gpTweakBall->GetBoostBallMaxChargeTime();
        }
      } else {
        mBoostChargeTime = 0.f;
      }
    } else {
      if (mBallAnimationIndex == 1) {
        mBallAnimationIndex = 0;
        CSfxManager::RemoveEmitter(mBoostChargeSfx);
        mBoostChargeSfx = CSfxHandle();

        if (mBoostChargeTime >= gpTweakBall->GetBoostBallMinChargeTime()) {
          mBoostReleaseSfx =
              AddEmitter(mPlayer, mgr.ReturnFirstIfSingleElseSecond(0x8f, 0x2627), true, false,
                         CSfxManager::kMedPriority, 0x7f, 0x14, 150.f, 1.f);
        }
      }

      if (mBoostChargeTime >= gpTweakBall->GetBoostBallMinChargeTime() || activateBoost) {
        if (kBoostBallBreaksOrbit) {
          mPlayer.fn_8011eac4(CPlayer::kOR_BoostBall, mgr);
        }

        if (GetBallBoostState() == kBBS_BoostAvailable) {
          if (GetIsInHalfPipeMode() || mBallCloseToCollision || mBallState == kBS_SpiderBoost) {
            EnterBoosting(mgr, false);
          } else {
            const CVector3f surfaceY = mSurfaceToWorld.GetColumn(kDY);
            mPlayer.ApplyImpulseWR(CVector3f::Zero(), CAxisAngle::FromVector(10000.f * -surfaceY));
            CancelBoosting();
          }
        } else if (GetBallBoostState() == kBBS_BoostDisabled) {
          mPlayer.SetTransform(CTransform4f::LookAt(mPlayer.GetTransform().GetTranslation(),
                                                    mPlayer.GetTransform().GetTranslation() +
                                                        ballToWorld.GetColumn(kDY),
                                                    CVector3f::Up()));

          const CVector3f playerX = mPlayer.GetTransform().GetColumn(kDX);
          mPlayer.ApplyImpulseWR(CVector3f::Zero(), CAxisAngle::FromVector(10000.f * -playerX));
          CancelBoosting();
        }
      } else if (mBoostChargeTime > 0.f) {
        CancelBoosting();
      }
    }
  } else {
    mBoostDrainTime += dt;
    if (mBoostDrainTime > gpTweakBall->GetBoostBallDrainTime()) {
      LeaveBoosting();
    }

    if (!GetIsInHalfPipeMode() && mBallState != kBS_SpiderBoost && !mBallCloseToCollision) {
      if (mBoostDrainTime / gpTweakBall->GetBoostBallDrainTime() < 0.3f) {
        DampLinearAndAngularVelocities(0.5f, 0.01f, dt);
      }

      LeaveBoosting();
    }
  }

  if (!IsBoosting() && !hasCannonBall) {
    return;
  }

  mBoostDamagedObjects.clear();
  CDamageInfo damage =
      hasCannonBall ? gpTweakBall->GetCannonBallDamage() : gpTweakBall->GetBoostBallDamage();
  damage = damage.ApplyDoubleDamage(*mPlayer.GetPlayerState());

  const float speed = mPlayer.GetVelocityWR().Magnitude();
  const bool moving = speed > 1.1920929e-4f;
  const CVector3f motion = mPlayer.GetVelocityWR() * (1.f / speed);
  double distance = speed * dt;
  double hitDistance = distance;

  const CCollidableSphere sphere(CSphere(ballToWorld.GetTranslation(), damage.GetRadius()),
                                 CMaterialList(BoostSphereMaterial));
  const CAABox localBox = sphere.CalculateLocalAABox();
  CAABox box = localBox;
  if (moving) {
    const CVector3f delta = static_cast< float >(distance) * motion;
    box.AccumulateBounds(localBox.GetMinPoint() + delta);
    box.AccumulateBounds(localBox.GetMaxPoint() + delta);
  }

  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  mgr.BuildNearList(
      nearList, box,
      CMaterialFilter::MakeInclude(CMaterialList(BoostNearListMaterial1, BoostNearListMaterial2)),
      &mPlayer);

  TUniqueId hitId = kInvalidUniqueId;
  CCollisionInfo info;
  const bool collided =
      moving && CGameCollision::DetectDynamicCollisionMoving(
                    sphere, CTransform4f::Identity(), nearList, motion, hitId, info, distance, mgr);
  if (collided && hitId != kInvalidUniqueId) {
    ApplyBoostBallDamage(mgr, hitId, damage, dt);
    nearList.erase(rstl::find(nearList.begin(), nearList.end(), hitId));
  }

  for (rstl::reserved_vector< TUniqueId, 1024 >::iterator it = nearList.begin();
       it != nearList.end(); ++it) {
    const TUniqueId id = *it;
    CEntity* ent = mgr.ObjectById(id);
    if (CScriptDamageableTrigger* trigger = TCastToPtr< CScriptDamageableTrigger >(ent)) {
      const rstl::optional_object< CAABox > touchBounds = trigger->GetTouchBounds();
      if (touchBounds) {
        CVector3f normal = CVector3f::Up();
        CVector3f point = CVector3f::Zero();
        const bool intersects =
            CollisionUtil::AABoxSphereIntersection(*touchBounds, sphere.GetSphere()) ||
            (moving && CollisionUtil::MovingSphereAABox(sphere.GetSphere(), *touchBounds, motion,
                                                        hitDistance, point, normal));
        if (intersects && hitDistance <= distance) {
          ApplyBoostBallDamage(mgr, id, damage, dt);
        }
      }
    } else if (hasCannonBall) {
      if (CPhysicsActor* physAct = TCastToPtr< CPhysicsActor >(ent)) {
        if (CGameCollision::DetectDynamicCollisionBoolean(sphere, CTransform4f::Identity(),
                                                          *physAct)) {
          ApplyBoostBallDamage(mgr, id, damage, dt);
        }
      }
    }
  }
}

void CMorphBall::SetScrewAttackActive(bool active) { mForcedScrewJumpInput = active; }

static EMaterialTypes ScrewAttackIncludeMaterial1 =
    static_cast< EMaterialTypes >(34); // Guessed name
static EMaterialTypes ScrewAttackIncludeMaterial2 =
    static_cast< EMaterialTypes >(43);                             // Guessed name
static EMaterialTypes ScrewAttackIncludeMaterial3 = kMT_Unknown59; // Guessed name
static EMaterialTypes ScrewAttackIncludeMaterial4 =
    static_cast< EMaterialTypes >(50); // Guessed name
static EMaterialTypes ScrewAttackExcludeMaterial1 =
    static_cast< EMaterialTypes >(35); // Guessed name
static EMaterialTypes ScrewAttackExcludeMaterial2 =
    static_cast< EMaterialTypes >(45); // Guessed name

void CMorphBall::ApplyScrewAttackDamage(float dt, CStateManager& mgr) {
  CDamageInfo damage = gpTweakBall->GetScrewAttackDamage();
  const float radius = damage.GetRadius();
  damage.SetDamage(60.f * (dt * damage.GetDamage()));
  damage.SetRadiusDamage(60.f * (dt * damage.GetRadiusDamage()));
  damage.SetRadius(0.f);

  const CVector3f ballPos = GetBallPosition();
  const CAABox bounds(ballPos - CVector3f(radius, radius, radius),
                      ballPos + CVector3f(radius, radius, radius));
  const CSphere sphere(ballPos, radius);
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
      CMaterialList(ScrewAttackIncludeMaterial1, ScrewAttackIncludeMaterial2,
                    ScrewAttackIncludeMaterial3, ScrewAttackIncludeMaterial4),
      CMaterialList(ScrewAttackExcludeMaterial1, ScrewAttackExcludeMaterial2));
  mgr.BuildNearList(nearList, bounds, filter, &mPlayer);

  for (rstl::reserved_vector< TUniqueId, 1024 >::const_iterator it = nearList.begin();
       it != nearList.end(); ++it) {
    const TUniqueId uid = *it;
    bool applyDamage = true;
    if (const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(uid))) {
      if (player->GetMorphballTransitionState() == CPlayer::kMS_Morphed &&
          player->GetMorphBall()->InScrewAttackMode()) {
        applyDamage = false;
      }
    }

    const CActor* actor = static_cast< const CActor* >(mgr.GetObjectById(uid));
    const rstl::optional_object< CAABox > touchBounds = actor->GetTouchBounds();
    if (touchBounds.valid() && !CollisionUtil::SphereAABoxIntersection(sphere, *touchBounds)) {
      applyDamage = false;
    }

    if (applyDamage) {
      mgr.ApplyDamage(mPlayer.GetUniqueId(), uid, mPlayer.GetUniqueId(), damage, CMaterialFilter(),
                      CVector3f::Zero());
    }
  }
}

void CMorphBall::UpdateScrewAttackRecovery(float dt) {
  if (mPendingRecoil) {
    const CVector3f recoilVelocity = 12.f * mScrewAttackDirection + 24.f * CVector3f::Up();
    mPlayer.SetVelocityWR(recoilVelocity);
    mPlayer.CalculatePlayerMovementDirection(0.f, -recoilVelocity);
    mPendingRecoil = false;
    mRecoiling = true;
  }

  bool endRecovery = false;
  if (mCollidedDuringRecovery) {
    mTouchingWall = false;
    mCollidedDuringRecovery = false;
    if (x18a8_30_) {
      mScrewAttackRecoveryCollisionTime += 0.5f * dt;
      x18a8_30_ = false;
    } else {
      mScrewAttackRecoveryCollisionTime += dt;
    }
    if (mScrewAttackRecoveryCollisionTime > 0.4f) {
      endRecovery = true;
    }
  }

  const CVector3f velocity = mPlayer.GetVelocityWR();
  CVector3f lookDir = CVector3f::Forward();
  if (mScrewAttackDirection.CanBeNormalized()) {
    if (mRecoiling) {
      lookDir = -1.f * mScrewAttackDirection;
    } else {
      lookDir = mScrewAttackDirection;
    }

    if (!mRecoiling && mTouchingWall) {
      const CVector2f flatVelocity = CVector2f(velocity.GetX(), velocity.GetY());
      if (mPlayer.GetTranslation().GetZ() - mPlayer.GetLastSpaceJumpPosition().GetZ() > 0.f) {
        if (flatVelocity.MagSquared() < 64.f) {
          const CVector3f wallVelocity = velocity + 4.f * mScrewAttackDirection;
          mPlayer.SetVelocityWR(wallVelocity);
          mPlayer.CalculatePlayerMovementDirection(0.f, wallVelocity);
        }
      } else {
        const float damping = pow(0.05f, 60.f * dt);
        const CVector3f dampedVelocity =
            velocity - damping * CVector3f(flatVelocity.GetX(), flatVelocity.GetY(), 0.f);
        mPlayer.SetVelocityWR(dampedVelocity);
      }
    }
  } else {
    lookDir = mPlayer.GetMovementDirection();
  }

  if (mScrewAttackExitAnimationFrames != 0) {
    mPlayer.BodyController()->CommandMgr().DeliverCmd(CPBCJumpCmd(0, 4));
    --mScrewAttackExitAnimationFrames;
  }

  if (mScrewAttackSfx) {
    CSfxManager::SfxStop(mScrewAttackSfx);
    mScrewAttackSfx.Clear();
  }

  CTransform4f playerXf = mPlayer.GetTransform();
  playerXf.SetRotation(CTransform4f::LookAt(CVector3f::Zero(), lookDir, CVector3f::Up()));
  mPlayer.SetTransform(playerXf);

  if (mPlayer.GetPlayerMovementState() == NPlayer::kMS_OnGround) {
    ++mScrewAttackGroundedFrames;
    CVector2f flatVelocity(velocity.GetX(), velocity.GetY());
    const CVector3f dampedVelocity =
        velocity - static_cast< float >(pow(0.05f, 60.f * dt)) *
                       CVector3f(flatVelocity.GetX(), flatVelocity.GetY(), 0.f);
    mPlayer.SetVelocityWR(dampedVelocity);
  }

  if (endRecovery ||
      strcmp(mPlayer.BodyController()->GetBodyState().GetName(), "Locomotion") == 0) {
    mPlayer.fn_80184294(CPlayer::kMS_Morphed);
  }
}

void CMorphBall::ComputeScrewAttackMovement(const CFinalInput& input, CStateManager& mgr,
                                            float dt) {
  const float mass = mPlayer.GetMass();
  CPlayerBodyStateCmdMgr& cmdMgr = mPlayer.BodyController()->CommandMgr();
  const float initialDropLimit = gpTweakBall->GetScrewAttackInitialDropLimit();
  const float finalDropLimit = gpTweakBall->GetScrewAttackFinalDropLimit();
  const int dropLimitJumpCount = gpTweakBall->GetScrewAttackDropLimitJumpCount();
  const float dropLimit = initialDropLimit + (static_cast< float >(mScrewAttackJumpCount) /
                                              static_cast< float >(dropLimitJumpCount)) *
                                                 (finalDropLimit - initialDropLimit);
  const float drop = mPlayer.GetTranslation().GetZ() - mPlayer.GetLastSpaceJumpPosition().GetZ();

  bool jumpPressed = true;
  if (!mPlayer.JumpPressed(input) && !mForcedScrewJumpInput) {
    jumpPressed = false;
  }
  mForcedScrewJumpInput = false;

  bool startSfx = false;
  if (drop < 0.f && CMath::AbsF(drop) > dropLimit) {
    mEndScrewAttackRequested = true;
  }

  mTimeSinceScrewAttackJump += dt;
  if (mBallState == kBS_ScrewAttackWallJump && mTouchingWall && 0.f == mWallContactTime) {
    cmdMgr.DeliverCmd(CPBCMorphToScrewAttackCmd(5, 1));
    mPlayer.SetVelocityWR(CVector3f::Zero());
  }

  if (mBallState == kBS_ScrewAttackWallJump) {
    if (mWallJumpInputPending && mTimeSinceScrewAttackJump > 0.15f) {
      mEndScrewAttackRequested = true;
    }

    mScrewAttackDirection = mWallNormal;
    mWallContactTime += dt;
    const float wallTimeScale = mWallJumpCount != 0 ? 1.f : 1.5f;
    const float maxWallContactTime = wallTimeScale * gpTweakBall->GetScrewAttackWallJumpMaxTime();
    if (mTouchingWall && mWallContactTime > maxWallContactTime) {
      mEndScrewAttackRequested = true;
    }

    if (!mEndScrewAttackRequested && (jumpPressed || (mWallJumpInputPending && mTouchingWall))) {
      const float verticalVelocity = gpTweakBall->GetScrewAttackWallJumpVerticalVelocity();
      const float horizontalVelocity = gpTweakBall->GetScrewAttackWallJumpHorizontalVelocity();
      if (mTouchingWall) {
        mPlayer.SetLastSpaceJumpPosition(mPlayer.GetTranslation());
        startSfx = true;
        mTouchingWall = false;
        mWallJumpInputPending = false;
        CVector3f velocity = horizontalVelocity * mWallNormal;
        velocity += CVector3f(0.f, 0.f, verticalVelocity);
        mPlayer.SetVelocityWR(velocity);
        ++mScrewAttackJumpCount;
        ++mWallJumpCount;
        cmdMgr.DeliverCmd(CPBCMorphToScrewAttackCmd(4, 1));
      } else if (mWallJumpInputPending) {
        mEndScrewAttackRequested = true;
      } else {
        mWallJumpInputPending = true;
        mTimeSinceScrewAttackJump = 0.f;
      }
    }
  } else {
    const float verticalVelocity = gpTweakBall->GetScrewAttackVerticalJumpVelocity();
    const float jumpEnergy = mass * (0.5f * verticalVelocity * verticalVelocity);
    const float horizontalVelocity = gpTweakBall->GetScrewAttackHorizontalJumpVelocity();
    CVector3f moveDir = mPlayer.GetMovementDirection();
    mScrewAttackDirection = moveDir;
    if (jumpPressed && !mEndScrewAttackRequested && mScrewAttackJumpCount < 5) {
      mTimeSinceScrewAttackJump = 0.f;
      const float verticalSpeed = mPlayer.GetVelocityWR().GetZ();
      if (CMath::AbsF(drop) < dropLimit && verticalSpeed < 0.f) {
        const float potentialEnergy = mass * (drop * GetGravityAcceleration());
        const float jumpSpeed = CMath::SqrtF(2.f * (jumpEnergy + potentialEnergy) / mass);
        const CVector2f steering = CalculateSpiderBallAttractionSurfaceForces(input);
        if (CMath::AbsF(steering.GetX()) > 0.05f) {
          const float steeringAngle =
              -(gpTweakBall->GetScrewAttackMaxSteeringAngle().AsRadians() * steering.GetX());
          moveDir = CTransform4f::RotateZ(CRelAngle::FromRadians(steeringAngle)).Rotate(moveDir);
          CTransform4f playerXf = mPlayer.GetTransform();
          playerXf.RotateLocalZ(CRelAngle::FromRadians(steeringAngle));
          mPlayer.SetTransform(playerXf);
        }

        CVector3f velocity = horizontalVelocity * moveDir;
        velocity += CVector3f(0.f, 0.f, jumpSpeed);
        mPlayer.SetVelocityWR(velocity);
        ++mScrewAttackJumpCount;
        cmdMgr.DeliverCmd(CPBCLocomotionCmd(moveDir, moveDir));
        startSfx = true;
        if (!mScrewAttackJumpFlashGen.get()) {
          mScrewAttackJumpFlashGen = rs_new CElementGen(mScrewAttackJumpFlash);
          mScrewAttackJumpFlashGen->SetGlobalScale(mPlayer.GetModelData()->GetScale());
        } else {
          mScrewAttackWallJumpFlashGen = rs_new CElementGen(mScrewAttackJumpFlash);
          mScrewAttackWallJumpFlashGen->SetGlobalScale(mPlayer.GetModelData()->GetScale());
        }
      } else {
        mEndScrewAttackRequested = true;
      }
    }
  }

  if (mEndScrewAttackRequested) {
    mBallState = kBS_ScrewAttackRecovery;
    mWallJumpInputPending = false;
  } else {
    ApplyScrewAttackDamage(dt, mgr);
    if (startSfx) {
      StartScrewAttackSfx();
    }
    CSfxManager::UpdateEmitter(mScrewAttackSfx, mPlayer.GetTranslation(), CVector3f::Zero(), 0x7f);
  }

  if (mScrewAttackDirection.CanBeNormalized()) {
    CTransform4f playerXf = mPlayer.GetTransform();
    playerXf.SetRotation(
        CTransform4f::LookAt(CVector3f::Zero(), mScrewAttackDirection, CVector3f::Up()));
    mPlayer.SetTransform(playerXf);
  }
}

// Guessed name
struct SDeathBallCooldownFinder {
  SDeathBallCooldownFinder(const TUniqueId& id) : mId(id) {}
  bool operator()(const rstl::pair< TUniqueId, float >& cooldown) const {
    return cooldown.first == mId;
  }

  TUniqueId mId;
};

static EMaterialTypes DeathBallIncludeMaterial1 = static_cast< EMaterialTypes >(34); // Guessed name
static EMaterialTypes DeathBallIncludeMaterial2 = static_cast< EMaterialTypes >(43); // Guessed name
static EMaterialTypes DeathBallIncludeMaterial3 = kMT_Unknown59;                     // Guessed name
static EMaterialTypes DeathBallIncludeMaterial4 = static_cast< EMaterialTypes >(50); // Guessed name
static EMaterialTypes DeathBallExcludeMaterial1 = static_cast< EMaterialTypes >(35); // Guessed name
static EMaterialTypes DeathBallExcludeMaterial2 = static_cast< EMaterialTypes >(45); // Guessed name

void CMorphBall::UpdateDeathBall(float dt, CStateManager& mgr) {
  rstl::vector< rstl::pair< TUniqueId, float > >::iterator it = mDeathBallDamageCooldowns.begin();
  while (it != mDeathBallDamageCooldowns.end()) {
    it->second -= dt;
    if (it->second <= 0.f) {
      it = mDeathBallDamageCooldowns.erase(it);
    } else {
      ++it;
    }
  }

  if (mPlayer.GetPlayerState()->GetItemAmount(CPlayerState::kIT_DeathBall, true) == 0) {
    return;
  }

  CDamageInfo damage = gpTweakBall->GetDeathBallDamage();
  damage.SetDamage(60.f * (dt * damage.GetDamage()));
  damage.SetRadiusDamage(60.f * (dt * damage.GetRadiusDamage()));
  const float radius = damage.GetRadius();
  const CVector3f ballPos = GetBallPosition();
  const CAABox bounds(ballPos - CVector3f(radius, radius, radius),
                      ballPos + CVector3f(radius, radius, radius));
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
      CMaterialList(DeathBallIncludeMaterial1, DeathBallIncludeMaterial2, DeathBallIncludeMaterial3,
                    DeathBallIncludeMaterial4),
      CMaterialList(DeathBallExcludeMaterial1, DeathBallExcludeMaterial2));
  mgr.BuildNearList(nearList, bounds, filter, &mPlayer);

  for (rstl::reserved_vector< TUniqueId, 1024 >::const_iterator id = nearList.begin();
       id != nearList.end(); ++id) {
    const TUniqueId uid = *id;
    rstl::vector< rstl::pair< TUniqueId, float > >::iterator cooldown =
        rstl::find_if(mDeathBallDamageCooldowns.begin(), mDeathBallDamageCooldowns.end(),
                      SDeathBallCooldownFinder(uid));

    if (cooldown == mDeathBallDamageCooldowns.end()) {
      mgr.ApplyDamage(mPlayer.GetUniqueId(), uid, mPlayer.GetUniqueId(), damage, CMaterialFilter(),
                      CVector3f::Zero());
      if (mDeathBallDamageCooldowns.size() == mDeathBallDamageCooldowns.capacity()) {
        mDeathBallDamageCooldowns.reserve(mDeathBallDamageCooldowns.capacity() * 2);
      }
      mDeathBallDamageCooldowns.push_back_unsafe(
          rstl::pair< TUniqueId, float >(uid, gpTweakBall->GetDeathBallDamageDelay()));
    }
  }
}

void CMorphBall::UpdateBallLight(float dt, CStateManager& mgr) {
  if (mBallInnerGlowLight == kInvalidUniqueId) {
    return;
  }
  if (CGameLight* ballLight = TCastToPtr< CGameLight >(mgr.ObjectById(mBallInnerGlowLight))) {
    const CTransform4f swooshToWorld = GetSwooshToWorld();
    rstl::optional_object< CLight > light;
    if (IsMorphBallTransitionFlashValid() && mMorphBallTransitionFlashGen->SystemHasLight()) {
      light = mMorphBallTransitionFlashGen->GetLight();
    } else if (mBallInnerGlowGen.get() != nullptr && mBallInnerGlowGen->SystemHasLight()) {
      light = mBallInnerGlowGen->GetLight();
    }

    const bool lightSuitInDarkWorld =
        mPlayer.GetPlayerState()->GetItemAmount(CPlayerState::kIT_LightSuit, true) != 0 &&
        mgr.GetIsDarkWorld();

    if (light.valid() && mBallLightActive && (lightSuitInDarkWorld || !InScrewAttackMode()) &&
        mPlayer.GetSpawnedMorphballState() != CPlayer::kMS_Morphed) {
      if (mBallState == kBS_Spider) {
        ballLight->SetTranslation(swooshToWorld.GetTranslation() +
                                  mSurfaceToWorld.GetUp() * GetBallRadius());
      } else if (mPlayer.GetMorphballTransitionState() != CPlayer::kMS_Morphed) {
        ballLight->SetTranslation(mPlayer.GetTranslation() + CVector3f::Up() * GetBallRadius());
      } else {
        ballLight->SetTranslation(swooshToWorld.GetTranslation() +
                                  CVector3f::Up() * GetBallRadius());
      }

      CLight lightCopy(*light);
      const CColor& lightColor = lightCopy.GetColor();
      lightCopy.SetColor(CColor::Modulate(
          lightColor, GetBallGlowColor(skBallLightModulationColors[mBallGlowColorIdx])));

      if (!lightSuitInDarkWorld &&
          mPlayer.GetMorphballTransitionState() == CPlayer::kMS_Unmorphing) {
        const float t =
            rstl::min_val(rstl::max_val(mPlayer.GetMorphBallTransitionFactor() / 0.2f, 0.f), 1.f);
        lightCopy.SetColor(CColor::Lerp(lightColor, CColor::Black(), t));
      } else if (!lightSuitInDarkWorld &&
                 mPlayer.GetMorphballTransitionState() == CPlayer::kMS_Morphing) {
        const float t = rstl::min_val(
            rstl::max_val((mPlayer.GetMorphBallTransitionFactor() - 0.45f) / 0.35f, 0.f), 1.f);
        lightCopy.SetColor(CColor::Lerp(CColor::Black(), lightColor, t));
      } else if (mPlayer.GetPlayerState()->GetItemAmount(CPlayerState::kIT_Invisibility, true) !=
                 0) {
        lightCopy.SetColor(CColor(0.f, 0.f, 0.1f, 1.f));
      } else {
        lightCopy.SetColor(CColor::Lerp(lightColor, CColor::White(), mBoostLightFactor));
      }

      lightCopy.SetColor(CColor::Lerp(CColor::Black(), lightColor, mPlayer.GetDeathAlpha()));
      ballLight->SetLight(lightCopy);
      ballLight->SetActive(true);
    } else {
      ballLight->SetActive(false);
    }
  }
}

void CMorphBall::UpdateEffects(float dt, CStateManager& mgr) {
  const CTransform4f swooshToWorld = GetSwooshToWorld();
  const CPlayerState* playerState = mPlayer.GetPlayerState();
  const bool dead = mPlayer.GetDeathTime() > 0.f;

  if (!dead) {
    const CTransform4f ballToWorld = GetBallToWorld();
    const CVector3f ballPos = ballToWorld.GetTranslation();

    const CVector3f slowBlueOffset1 = swooshToWorld.Rotate(CVector3f(0.1f, 0.f, 0.f));
    mSlowBlueTailSwooshGen->SetTranslation(swooshToWorld.GetTranslation() + slowBlueOffset1);
    mSlowBlueTailSwooshGen->SetOrientation(swooshToWorld.GetRotation());
    mSlowBlueTailSwooshGen->SetWarmUp();
    mSlowBlueTailSwooshGen->Update(0.0);

    const CVector3f slowBlueOffset2 = swooshToWorld.Rotate(CVector3f(-0.1f, 0.f, 0.f));
    mSlowBlueTailSwooshGen2->SetTranslation(swooshToWorld.GetTranslation() + slowBlueOffset2);
    mSlowBlueTailSwooshGen2->SetOrientation(swooshToWorld.GetRotation());
    mSlowBlueTailSwooshGen2->SetWarmUp();
    mSlowBlueTailSwooshGen2->Update(0.0);

    const CVector3f slowBlueOffset3 = swooshToWorld.Rotate(CVector3f(0.f, 0.f, 0.65f));
    mSlowBlueTailSwoosh2Gen->SetTranslation(swooshToWorld.GetTranslation() + slowBlueOffset3);
    mSlowBlueTailSwoosh2Gen->SetOrientation(swooshToWorld.GetRotation());
    mSlowBlueTailSwoosh2Gen->SetWarmUp();
    mSlowBlueTailSwoosh2Gen->Update(0.0);

    const CVector3f slowBlueOffset4 = swooshToWorld.Rotate(CVector3f(0.f, 0.f, -0.65f));
    mSlowBlueTailSwoosh2Gen2->SetTranslation(swooshToWorld.GetTranslation() + slowBlueOffset4);
    mSlowBlueTailSwoosh2Gen2->SetOrientation(swooshToWorld.GetRotation());
    mSlowBlueTailSwoosh2Gen2->SetWarmUp();
    mSlowBlueTailSwoosh2Gen2->Update(0.0);

    mJaggyTrailGen->SetTranslation(swooshToWorld.GetTranslation());
    mJaggyTrailGen->SetOrientation(swooshToWorld.GetRotation());
    mJaggyTrailGen->SetWarmUp();
    mJaggyTrailGen->Update(0.0);

    const bool hasBoostBall = playerState->HasPowerUp(CPlayerState::kIT_BoostBall);
    if (!mgr.IsMultiplayer() && hasBoostBall && mLoadedModelId % 3 == 1) {
      mSideSwooshGen->SetTranslation(swooshToWorld.GetTranslation() +
                                     swooshToWorld.Rotate(CVector3f(-0.5859f, 0.f, 0.f)));
      mSideSwooshGen->SetOrientation(swooshToWorld.GetRotation());
      mSideSwooshGen->SetWarmUp();
      mSideSwooshGen->Update(0.0);

      mSideSwooshGen2->SetTranslation(swooshToWorld.GetTranslation() +
                                      swooshToWorld.Rotate(CVector3f(0.5859f, 0.f, 0.f)));
      mSideSwooshGen2->SetOrientation(swooshToWorld.GetRotation());
      mSideSwooshGen2->SetWarmUp();
      mSideSwooshGen2->Update(0.0);
    }

    mWallSparkGen->Update(dt);

    const bool emitRainWake = mPlayer.GetPlayerMovementState() == NPlayer::kMS_OnGround &&
                              mgr.GetWorld()->GetNeededEnvFx() == kEFX_Rain &&
                              mgr.GetEnvFxManager()->GetRainMagnitude() > 0.f &&
                              mgr.GetEnvFxManager()->IsSplashActive();
    if (emitRainWake) {
      mWakeEffects[5]->Load(true);
    }
    mWakeEffects[5]->SetParticleEmission(emitRainWake);
    if (emitRainWake) {
      const CTransform4f rainWakeXf =
          CTransform4f::LookAt(mPlayer.GetTranslation() + mPlayer.GetMovementDirection(),
                               mPlayer.GetTranslation(), CVector3f::Up());
      mWakeEffects[5]->SetOrientation(rainWakeXf);

      const float flatMoveSpeed = mPlayer.GetFlatMoveSpeed();
      const float ballMaxVelocity = mPlayer.GetBallMaxVelocity();
      const float rainDensity = 2.f * mgr.GetEnvFxManager()->GetRainMagnitude();
      const float rainGenRate = rainDensity * flatMoveSpeed / ballMaxVelocity;
      mWakeEffects[5]->SetGeneratorRate(rstl::min_val(rainGenRate, 1.f));
      mWakeEffects[5]->SetTranslation(mPlayer.GetTranslation());
    }

    for (int i = 0; i < 6; ++i) {
      if (i != mWakeEffectIndex && i != 5) {
        mWakeEffects[i]->SetParticleEmission(false);
      }
      mWakeEffects[i]->Update(dt);
    }

    if (static_cast< int >(mWallSparkFrameCountdown) > 0) {
      mWallSparkFrameCountdown -= 1;
      if (static_cast< int >(mWallSparkFrameCountdown) <= 0) {
        mWallSparkGen->SetParticleEmission(false);
      }
    }

    mBallInnerGlowGen->SetGlobalTranslation(swooshToWorld.GetTranslation());
    mBallInnerGlowGen->Update(dt);

    if (mBoostChargeTime == 0.f && mBoostDrainTime == 0.f) {
      const CColor clear(0);
      mBoostBallGlowGen->SetModulationColor(clear);
    } else {
      mBoostBallGlowGen->SetGlobalTranslation(swooshToWorld.GetTranslation());

      const float t = mBoostDrainTime == 0.f
                          ? mBoostChargeTime / gpTweakBall->GetBoostBallMaxChargeTime()
                          : 1.f - mBoostDrainTime / gpTweakBall->GetBoostBallDrainTime();

      CElementGen* boostBallGlowGen = mBoostBallGlowGen.get();
      boostBallGlowGen->SetModulationColor(
          CColor::Lerp(CColor(0.f, 0.f, 0.f, 1.f), CColor(1.f, 1.f, 0.4f, 1.f), t));
      mBoostBallGlowGen->Update(dt);
    }

    mSpiderBallMagnetGen->Update(dt);

    mBoostOverLightFactor -= 0.03f;
    mBoostOverLightFactor = rstl::max_val(0.f, mBoostOverLightFactor);
    if (mBoostOverLightFactor == 0.f) {
      mBoostLightFactor -= 0.04f;
      mBoostLightFactor = rstl::max_val(0.f, mBoostLightFactor);
    }

    if (IsBoosting()) {
      mBoostOverLightFactor = 1.f;
      mBoostLightFactor = 0.f;
    } else {
      mBoostLightFactor = rstl::max_val(
          mBoostLightFactor, mBoostChargeTime / gpTweakBall->GetBoostBallMaxChargeTime());
      mBoostLightFactor = rstl::min_val(1.f, mBoostLightFactor);
    }

    if (mBoostEffectGen.get()) {
      bool slowBoostEffect = false;
      float rate = 1.f;
      if (mPlayer.GetVelocityWR().MagSquared() > 100.f || mBoostEffectTime <= 0.02f) {
        const CTransform4f boostEffectXf =
            CTransform4f::LookAt(ballPos, ballPos + mPlayer.GetLookDir(), CVector3f::Up());
        mBoostEffectGen->SetOrientation(boostEffectXf.GetRotation());
      } else {
        slowBoostEffect = true;
      }
      mBoostEffectGen->SetTranslation(ballPos);

      if (slowBoostEffect) {
        rate = mBoostEffectGen->GetCurrentTime() < 1.7708333f ? 4.f : 2.f;
      }
      mBoostEffectGen->Update(dt * rate);

      mBoostEffectTime += dt;
      const bool boostEffectDone = !IsBoosting() && mBoostEffectTime > 1.5f;
      if (boostEffectDone || mBoostEffectGen->IsSystemDeletable()) {
        mBoostEffectGen = nullptr;
      }
    }

    const bool hasDeathBall = playerState->GetItemAmount(CPlayerState::kIT_DeathBall, true) != 0;
    if (hasDeathBall) {
      if (!mDeathBallOuterShellGen.get()) {
        mDeathBallOuterShellGen = rs_new CElementGen(mDeathBallOuterShell);
      }
      if (!mDeathBallSpikesGen.get()) {
        mDeathBallSpikesGen = rs_new CParticleElectric(mDeathBallSpikes);
      }
    }

    if (mDeathBallOuterShellGen.get()) {
      if (mDeathBallOuterShellGen->GetParticleCount() == 0 &&
          mDeathBallOuterShellGen->GetEmitterTime() != 0) {
        mDeathBallOuterShellGen = nullptr;
      } else {
        mDeathBallOuterShellGen->SetGlobalTranslation(ballPos);
        mDeathBallOuterShellGen->Update(dt);
        const float timeLeft = playerState->GetTimeLeft(CPlayerState::kIT_DeathBall);
        if (timeLeft < 0.5f && 0.f != timeLeft) {
          mDeathBallOuterShellGen->SetModulationColor(
              CColor::Lerp(CColor::Black(), CColor::White(), timeLeft / 0.5f));
        }
        if (!hasDeathBall) {
          mDeathBallOuterShellGen->SetGeneratorRate(0.f);
        }
      }
    }

    if (mDeathBallSpikesGen.get()) {
      const bool stopSpikes =
          !hasDeathBall ||
          (hasDeathBall && playerState->GetTimeLeft(CPlayerState::kIT_DeathBall) < 0.6f &&
           0.f != playerState->GetTimeLeft(CPlayerState::kIT_DeathBall));
      if (mDeathBallSpikesGen->GetParticleCount() == 0 &&
          mDeathBallSpikesGen->GetEmitterTime() != 0 && !stopSpikes) {
        mDeathBallSpikesGen = nullptr;
      } else {
        mDeathBallSpikesGen->SetGlobalTranslation(ballPos);
        mDeathBallSpikesGen->Update(dt);
        if (stopSpikes) {
          mDeathBallSpikesGen->SetGeneratorRate(0.f);
        }
      }
    }

    if (mScrewAttackJumpFlashGen.get()) {
      if (mScrewAttackJumpFlashGen->IsSystemDeletable()) {
        mScrewAttackJumpFlashGen = nullptr;
      } else {
        const CTransform4f& playerXf = mPlayer.GetTransform();
        mScrewAttackJumpFlashGen->Update(dt);
        mScrewAttackJumpFlashGen->SetGlobalTranslation(playerXf.GetTranslation());
        mScrewAttackJumpFlashGen->SetGlobalOrientation(playerXf.GetRotation());
      }
    }

    if (mScrewAttackWallJumpFlashGen.get()) {
      if (mScrewAttackWallJumpFlashGen->IsSystemDeletable()) {
        mScrewAttackWallJumpFlashGen = nullptr;
      } else {
        const CTransform4f& playerXf = mPlayer.GetTransform();
        mScrewAttackWallJumpFlashGen->Update(dt);
        mScrewAttackWallJumpFlashGen->SetGlobalTranslation(playerXf.GetTranslation());
        mScrewAttackWallJumpFlashGen->SetGlobalOrientation(playerXf.GetRotation());
      }
    }

    UpdateMorphBallTransitionFlash(dt);
    UpdateIceBreakEffect(dt);
  }

  if (!dead) {
    if (mBallState == kBS_Spider) {
      mSpiderLightFactor = rstl::min_val(1.f, mSpiderLightFactor + 0.25f);
    } else {
      mSpiderLightFactor = rstl::max_val(0.f, mSpiderLightFactor - 0.15f);
    }
  }
}

void CMorphBall::StopParticleWakes() {
  mWallSparkGen->SetParticleEmission(false);
  for (int i = 0; i < 6; ++i) {
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

void CMorphBall::SetBallLightActive(CStateManager& mgr, bool active) { mBallLightActive = active; }

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
      mgr.AddObject(rs_new CGameLight(
          mBallInnerGlowLight, kInvalidAreaId, false, rstl::string_l("BallLight"), GetBallToWorld(),
          mPlayer.GetUniqueId(), mBallInnerGlowGen->GetLight(), sourceId, 0, 0.f));
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
  const CQuaternion rotation = CQuaternion::AxisAngle(axis, CRelAngle::FromRadians(mBallTiltAngle));
  mPlayer.SetTransform(mPlayer.GetTransform() * rotation.BuildTransform4f());
  mTireMode = false;
  mTireInterpolating = true;
  mTireInterpolationSpeed = -1.f;
}

static EMaterialTypes BallCloseToCollisionMaterial = kMT_Unknown59; // Guessed name

void CMorphBall::UpdateBallDynamics(CStateManager& mgr, float dt) {
  CVector3f ballContactNormal(0.f, 0.f, 0.f);
  CVector3f ballContactPoint(0.f, 0.f, 0.f);
  CTransform4f ballToWorldXf(CTransform4f::Identity());

  const float angularDamping = pow(0.95f, 60.f * dt);
  mPlayer.SetAngularVelocityWR(
      CAxisAngle(angularDamping * mPlayer.GetAngularVelocityWR().GetVector()));

  const CMaterialFilter ballCloseFilter =
      CMaterialFilter::MakeInclude(CMaterialList(BallCloseToCollisionMaterial));
  mBallCloseToCollision = BallCloseToCollision(mgr, sBallCloseToCollisionDistance, ballCloseFilter);
  if (mBallCloseToCollision) {
    mCloseToCollisionTime += dt;
  } else {
    mCloseToCollisionTime = 0.f;
  }

  UpdateHalfPipeStatus(mgr, dt);

  mDisableControlCooldown -= dt;
  mDisableControlCooldown = rstl::max_val(mDisableControlCooldown, 0.f);
  mDamageTimer -= dt;
  mDamageTimer = rstl::max_val(mDamageTimer, 0.f);
  mDisableSpiderBallTime -= dt;
  mDisableSpiderBallTime = rstl::max_val(mDisableSpiderBallTime, 0.f);

  if (mBallState == kBS_Spider) {
    mSurfaceToWorld = CalculateSurfaceToWorld(mPlayerToSpiderNormal, mSpiderTrackPoint,
                                              mSpiderInterpBetweenPoints);
    mTireLeanAngle = 0.f;
    if (!mTireMode) {
      SwitchToTire();
    }
    mTireInterpolating = true;
    mTireInterpolationSpeed = -1.f;
    UpdateMarbleDynamics(mgr, dt, mSpiderTrackPoint);
  } else if (mPlayer.GetSurfaceRestraint() != CPlayer::kSR_Air) {
    if (CalculateBallContactInfo(ballContactNormal, ballContactPoint)) {
      mSurfaceToWorld =
          CalculateSurfaceToWorld(ballContactNormal, ballContactPoint, mPlayer.GetLookDir());

      const float ballSpeed = mPlayer.GetVelocityWR().Magnitude();
      if (ballSpeed < gpTweakBall->GetTireToMarbleThresholdSpeed() && mTireMode) {
        SwitchToMarble();
      }

      if (UpdateMarbleDynamics(mgr, dt, ballContactPoint) &&
          ballSpeed >= gpTweakBall->GetMarbleToTireThresholdSpeed() && !mTireMode) {
        SwitchToTire();
      }

      if (mTireMode) {
        const float accelRatio =
            mPlayer.GetTransform().TransposeRotate(mPlayer.GetForceWR()).GetX() /
            gpTweakBall->GetMaxBallTranslationAcceleration(mPlayer.GetSurfaceRestraint());
        mTireLeanAngle = accelRatio * gpTweakBall->GetMaxLeanAngle().AsRadians() *
                         gpTweakBall->GetForceToLeanGain();
        mTireLeanAngle = CMath::Limit(mTireLeanAngle, gpTweakBall->GetMaxLeanAngle().AsRadians());

        if (mPlayer.GetTransform().Get00() * mSurfaceToWorld.Get00() +
                mPlayer.GetTransform().Get10() * mSurfaceToWorld.Get10() +
                mPlayer.GetTransform().Get20() * mSurfaceToWorld.Get20() <
            0.f) {
          mTireLeanAngle = -mTireLeanAngle;
        }
      }
    }
  } else {
    mTireLeanAngle = 0.f;
  }

  if (!InScrewAttackMode()) {
    const float tiltAngleDelta = CMath::WrapPi(mTireLeanAngle - mBallTiltAngle);
    const float leanTracking = gpTweakBall->GetMaxLeanAngle().AsRadians() *
                               CMath::AbsF(tiltAngleDelta) * gpTweakBall->GetLeanTrackingGain();
    if (tiltAngleDelta > 0.05f) {
      mBallTiltAngle += leanTracking * dt;
    } else if (tiltAngleDelta < -0.05f) {
      mBallTiltAngle -= leanTracking * dt;
    } else {
      mBallTiltAngle = mTireLeanAngle;
    }
  } else {
    mPlayer.ClearAngularImpulses();
    mPlayer.SetAngularVelocityWR(CAxisAngle::Identity());
  }

  if (mBallState != kBS_Spider) {
    ApplyFriction(60.f * dt * CalculateSurfaceFriction());
  } else {
    DampLinearAndAngularVelocities(mLinearVelocityDamping, mAngularVelocityDamping, dt);
  }

  if (mBallState != kBS_Spider) {
    ApplyGravity();
  }

  mCollisionInfos.Clear();
  mBallOrientationAverage.AddValue(CQuaternion::FromMatrix(GetBallToWorld()));
  mBallPositionAverage.AddValue(GetBallPosition());
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
  CTransform4f ballToWorld =
      CTransform4f::Translate(mPlayer.GetTranslation() + CVector3f(0.f, 0.f, mRadius)) *
      mPlayer.GetTransform().GetRotation();
  return ballToWorld;
}

CTransform4f CMorphBall::GetSwooshToWorld() const {
  return CTransform4f::Translate(mPlayer.GetTranslation() + CVector3f(0.f, 0.f, GetBallRadius())) *
         mSurfaceToWorld.GetRotation() *
         CTransform4f::RotateY(CRelAngle::FromRadians(mBallTiltAngle));
}

void CMorphBall::ComputeMarioMovement(const CFinalInput& input, CStateManager& mgr, float dt) {
  mControlForce = CVector3f::Zero();
  mBoostControlForce = CVector3f::Zero();

  if (!IsMovementAllowed()) {
    return;
  }

  const float spiderPull =
      mPlayer.GetControlMapper().GetAnalogInput(CControlMapper::kC_SpiderBall, input);
  mSpiderPullMovement = spiderPull >= 0.5f ? 1.f : 0.f;

  if (mPlayer.GetPlayerState()->GetItemAmount(CPlayerState::kIT_SpiderBall, true) != 0 &&
      0.f != mSpiderPullMovement && !mDamageTimer) {
    const EBallState prevState = mBallState;
    if (mBallState != kBS_Spider) {
      mTouchingSpider = false;
      mBallState = kBS_Spider;
      mSpiderInterpBetweenPoints = mPlayer.GetTransform().GetColumn(kDZ);
      mSpiderBetweenPoints = mSpiderInterpBetweenPoints;
    }

    UpdateSpiderBall(input, mgr, dt);

    if (!mSpiderNearby) {
      if (prevState != kBS_Spider) {
        mBallState = prevState;
      } else {
        mBallState = kBS_Normal;
      }
      ResetSpiderBallForces();
    } else {
      if (prevState == kBS_Boost || prevState == kBS_SpiderBoost) {
        CancelBoosting();
        LeaveBoosting();
      }
      mBallState = kBS_Spider;
    }
  } else {
    if (!IsBoosting()) {
      mBallState = kBS_Normal;
    }
    ResetSpiderBallForces();
  }

  if (mBallState == kBS_Spider) {
    return;
  }

  const float forward = ForwardInput(input);
  const float turn = -BallTurnInput(input);
  const float maxSpeed = ComputeMaxSpeed();
  const float currentSpeed = mPlayer.GetVelocityWR().Magnitude();
  float forwardScale = 0.f;
  float turnScale = 0.f;
  float speedScale;
  float forwardAcc = 0.f;
  float turnAcc = 0.f;
  const CTransform4f controlXf =
      CTransform4f::LookAt(CVector3f::Zero(), mPlayer.GetControlDirFlat(), CVector3f::Up());
  const CVector3f controlFrameVel = controlXf.TransposeRotate(mPlayer.GetVelocityWR());

  if (CMath::AbsF(turn) > 0.1f) {
    const float controlTurn = turn * maxSpeed;
    const float controlTurnDelta = controlTurn - controlFrameVel.GetX();
    turnScale = CMath::Clamp(0.f, CMath::AbsF(controlTurnDelta) / maxSpeed, 1.f);
    float maxAccel;

    if (CMath::Sign(controlFrameVel.GetX()) != CMath::Sign(controlTurn) &&
        currentSpeed > 0.8f * maxSpeed) {
      maxAccel = gpTweakBall->GetBallForwardBrakingAcceleration(mPlayer.GetSurfaceRestraint());
    } else {
      maxAccel = gpTweakBall->GetMaxBallTranslationAcceleration(mPlayer.GetSurfaceRestraint());
    }

    if (controlTurnDelta < 0.f) {
      turnAcc = -maxAccel * turnScale;
    } else {
      turnAcc = maxAccel * turnScale;
    }
  }

  if (CMath::AbsF(forward) > 0.1f) {
    const float controlForward = forward * maxSpeed;
    const float controlForwardDelta = controlForward - controlFrameVel.GetY();
    forwardScale = CMath::Clamp(0.f, CMath::AbsF(controlForwardDelta) / maxSpeed, 1.f);
    float maxAccel;

    if (CMath::Sign(controlFrameVel.GetY()) != CMath::Sign(controlForward) &&
        currentSpeed > 0.8f * maxSpeed) {
      maxAccel = gpTweakBall->GetBallForwardBrakingAcceleration(mPlayer.GetSurfaceRestraint());
    } else {
      maxAccel = gpTweakBall->GetMaxBallTranslationAcceleration(mPlayer.GetSurfaceRestraint());
    }

    if (controlForwardDelta < 0.f) {
      forwardAcc = -maxAccel * forwardScale;
    } else {
      forwardAcc = maxAccel * forwardScale;
    }
  }

  if (0.f != forwardAcc || 0.f != turnAcc || IsBoosting() || GetIsInHalfPipeMode()) {
    const CVector3f forwardForce = controlXf.Rotate(CVector3f(0.f, forwardAcc, 0.f));
    const CVector3f turnForce = controlXf.Rotate(CVector3f(turnAcc, 0.f, 0.f));
    CVector3f controlForce = turnForce + forwardForce;
    mControlForce = controlForce;

    if (IsBoosting() && !GetIsInHalfPipeMode()) {
      const CVector3f controlLocalForce = mSurfaceToWorld.TransposeRotate(controlForce);
      CVector3f boostControlForce;
      boostControlForce = controlLocalForce;
      boostControlForce.SetY(0.f);
      boostControlForce.SetZ(0.f);
      controlForce = mSurfaceToWorld.Rotate(boostControlForce);
    }

    if (GetIsInHalfPipeMode()) {
      if (controlForce.Magnitude() > FLT_EPSILON) {
        if (GetIsInHalfPipeModeInAir() && currentSpeed <= 15.f) {
          const CVector3f halfPipeCol = mSurfaceToWorld.GetColumn(kDZ);
          const float halfPipeDot = CVector3f::Dot(controlForce, halfPipeCol);
          if (halfPipeDot / controlForce.Magnitude() < -0.85f) {
            DisableHalfPipeStatus();
            mDisableControlCooldown = 0.2f;

            const float impulseMag = -7.5f * mPlayer.GetMass();
            mPlayer.ApplyImpulseWR(impulseMag * halfPipeCol, CAxisAngle::Identity());
          }
        }

        if (GetIsInHalfPipeMode()) {
          const CVector3f halfPipeCol = mSurfaceToWorld.GetColumn(kDZ);
          const float halfPipeDot = CVector3f::Dot(controlForce, halfPipeCol);
          controlForce -= halfPipeDot * halfPipeCol;

          CVector3f controlLocalForce = mSurfaceToWorld.TransposeRotate(controlForce);
          CVector3f halfPipeForce = controlLocalForce;
          const float halfPipeXScale = 0.6f;
          const float halfPipeYScale = 1.4f * (IsBoosting() ? 0.f : 0.35f);
          halfPipeForce[kDX] *= halfPipeXScale;
          halfPipeForce[kDY] *= halfPipeYScale;
          controlForce = mSurfaceToWorld.Rotate(halfPipeForce);
          if (maxSpeed > 95.f) {
            mPlayer.SetVelocityWR(0.99f * mPlayer.GetVelocityWR());
          }
        }
      }
    }

    if (GetTouchedHalfPipeRecently()) {
      const float halfPipeDot = CVector3f::Dot(mPrevHalfPipeNormal, mHalfPipeNormal);
      if (halfPipeDot < 0.99f && halfPipeDot > 0.5f) {
        const CVector3f halfPipeRampAxis =
            CVector3f::Cross(mPrevHalfPipeNormal, mHalfPipeNormal).AsNormalized();
        CVector3f newVel = mPlayer.GetVelocityWR();
        const float rampVelDot = CVector3f::Dot(halfPipeRampAxis, newVel);
        newVel -= 0.15f * (rampVelDot * halfPipeRampAxis);
        mPlayer.SetVelocityWR(newVel);
      }
    }

    const float speedThreshold = 0.75f * maxSpeed;
    if (currentSpeed >= speedThreshold) {
      CVector3f currentVel = mPlayer.GetVelocityWR();
      const float velDot = CVector3f::Dot(controlForce, currentVel.AsNormalized());
      if (velDot > 0.f) {
        speedScale = (currentSpeed - speedThreshold) / (maxSpeed - speedThreshold);
        speedScale = CMath::Clamp(0.f, speedScale, 1.f);
        const CVector3f currentVelNorm = currentVel.AsNormalized();
        const float scaledVelDot = speedScale * velDot;
        controlForce -= scaledVelDot * currentVelNorm;
      }
    }

    mBoostControlForce = controlForce;
    mPlayer.ApplyForceWR(controlForce, CAxisAngle::Identity());
  }

  ComputeLiftForces(mControlForce, mPlayer.GetVelocityWR(), mgr);
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

static EMaterialTypes SpiderNearListExcludeMaterial1 = kMT_Character;           // Guessed name
static EMaterialTypes SpiderNearListExcludeMaterial2 = kMT_Player;              // Guessed name
static EMaterialTypes SpiderNearListExcludeMaterial3 = kMT_Projectile;          // Guessed name
static EMaterialTypes SpiderNearListExcludeMaterial4 = kMT_NoPlatformCollision; // Guessed name
static EMaterialTypes SpiderCollisionSurfaceMaterial =
    static_cast< EMaterialTypes >(61); // Guessed name

bool CMorphBall::FindClosestSpiderBallWaypoint(CStateManager& mgr, const CVector3f& ballCenter,
                                               CVector3f& closestPoint,
                                               CVector3f& interpDeltaBetweenPoints,
                                               CVector3f& deltaBetweenPoints, float& distance,
                                               CVector3f& normal, ESpiderSurfaceType& surfaceType,
                                               TUniqueId& surfaceId,
                                               CTransform4f& surfaceTransform) const {
  surfaceType = kSST_None;
  surfaceId = kInvalidUniqueId;
  float minDist = 2.1f;
  bool ret = false;
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  const CMaterialFilter nearFilter = CMaterialFilter::MakeExclude(
      CMaterialList(SpiderNearListExcludeMaterial1, SpiderNearListExcludeMaterial2,
                    SpiderNearListExcludeMaterial3, SpiderNearListExcludeMaterial4));
  const CAABox aabb(ballCenter - CVector3f(2.1f, 2.1f, 2.1f),
                    ballCenter + CVector3f(2.1f, 2.1f, 2.1f));
  mgr.BuildNearList(nearList, aabb, nearFilter, nullptr);

  for (rstl::reserved_vector< TUniqueId, 1024 >::iterator surfaceIt = nearList.begin();
       surfaceIt != nearList.end(); ++surfaceIt) {
    if (const CScriptSpiderBallAttractionSurface* const surface =
            TCastToConstPtr< CScriptSpiderBallAttractionSurface >(mgr.GetObjectById(*surfaceIt))) {
      const CVector3f surfaceNormal = surface->GetTransform().GetColumn(kDY).AsNormalized();
      CPlane plane(surface->GetTranslation(), CUnitVector3f(1.f * surfaceNormal));
      CVector3f point = CVector3f::Zero();

      if (CollisionUtil::RayPlaneIntersection(ballCenter + 2.1f * surfaceNormal,
                                              ballCenter - 2.1f * surfaceNormal, plane, point)) {
        const CVector3f halfScale = 0.5f * surface->GetScale();
        CTransform4f invScaleXf = CTransform4f::Scale(
            1.f / halfScale.GetX(), 1.f / halfScale.GetY(), 1.f / halfScale.GetZ());
        CVector3f clampedPoint = (invScaleXf * surface->GetTransform().GetQuickInverse()) * point;
        clampedPoint[kDX] = CMath::Clamp(-1.f, clampedPoint[kDX], 1.f);
        clampedPoint[kDZ] = CMath::Clamp(-1.f, clampedPoint[kDZ], 1.f);
        CTransform4f scaleXf =
            CTransform4f::Scale(halfScale.GetX(), halfScale.GetY(), halfScale.GetZ());
        CVector3f worldPoint = (surface->GetTransform() * scaleXf) * clampedPoint;
        const CVector3f finalDelta = worldPoint - ballCenter;
        const float finalMag = finalDelta.Magnitude();

        if (finalMag < minDist) {
          minDist = finalMag;
          closestPoint = worldPoint;
          distance = finalMag;
          normal = (-1.f / minDist) * finalDelta;
          surfaceType = kSST_ScriptedSurface;
          surfaceTransform = surface->GetTransform();
          ret = true;
        }
      }
    }
  }

  for (rstl::reserved_vector< TUniqueId, 1024 >::iterator waypointIt = nearList.begin();
       waypointIt != nearList.end(); ++waypointIt) {
    if (const CScriptSpiderBallWaypoint* waypoint =
            TCastToConstPtr< CScriptSpiderBallWaypoint >(mgr.GetObjectById(*waypointIt))) {
      const CScriptSpiderBallWaypoint* closestWp = nullptr;
      CVector3f worldPoint = CVector3f::Zero();
      CVector3f useInterpDeltaBetweenPoints = interpDeltaBetweenPoints;
      CVector3f useDeltaBetweenPoints = deltaBetweenPoints;
      waypoint->GetClosestPointAlongWaypoints(mgr, ballCenter, 2.1f, &closestWp, worldPoint,
                                              useDeltaBetweenPoints, 0.8f,
                                              useInterpDeltaBetweenPoints);

      if (closestWp != nullptr) {
        const CVector3f ballToPoint = worldPoint - ballCenter;
        const float ballToPointMag = ballToPoint.Magnitude();

        if (ballToPointMag < minDist) {
          minDist = ballToPointMag;
          closestPoint = worldPoint;
          interpDeltaBetweenPoints = useInterpDeltaBetweenPoints;
          deltaBetweenPoints = useDeltaBetweenPoints;
          distance = ballToPointMag;
          normal = (-1.f / minDist) * ballToPoint;
          surfaceType = kSST_Waypoint;
          ret = true;
        }
      }
    }
  }

  CCollisionCache cache(aabb, 1, 0, 0xffff);
  CGameCollision::BuildCollisionCache(mgr, cache, nearList,
                                      CGameCollision::kCUP_RemoveCachedNearListIds);
  const CMaterialFilter filter =
      CMaterialFilter::MakeInclude(CMaterialList(SpiderCollisionSurfaceMaterial));
  float minDistSq = minDist * minDist;
  CSphere sphere(ballCenter, 2.1f);
  CCollisionCacheIterator it(cache);
  while (!it.AtEnd()) {
    if (it.AtLeafStart()) {
      if (!CollisionUtil::AABoxSphereIntersection(*cache.GetLeafBounds(it), sphere)) {
        cache.SkipLeaf(it);
        continue;
      }
    }

    const SCachedCollisionSlot* slot = cache.NextTriangle(it);
    const CCollisionSurface& surface = slot->mTriangle.GetSurface();
    if (!filter.Passes(CMaterialList(surface.GetSurfaceFlags()))) {
      continue;
    }

    float baryU;
    float baryV;
    const float distSq = CollisionUtil::TriPointSqrDist_Float(
        ballCenter, surface.GetVert(0), surface.GetVert(1), surface.GetVert(2), &baryU, &baryV);
    if (distSq > minDistSq) {
      continue;
    }

    minDistSq = distSq;
    minDist = CMath::SqrtF(distSq);
    sphere = CSphere(ballCenter, minDist);
    const CVector3f point =
        CMath::BaryToWorld(surface.GetVert(0), surface.GetVert(1), surface.GetVert(2),
                           CVector3f(1.f - (baryU + baryV), baryV, baryU));
    closestPoint = point;
    distance = minDist;
    normal = (-1.f / minDist) * (point - ballCenter);
    surfaceType = kSST_CollisionSurface;
    const CVector3f& up =
        CMath::AbsF(CVector3f::Dot((point - ballCenter).AsNormalized(), CVector3f::Up())) < 0.9f
            ? static_cast< const CVector3f& >(CVector3f::Up())
            : mPlayer.GetControlDirFlat();
    surfaceTransform = CTransform4f::LookAt(ballCenter, point, up);
    surfaceId = TUniqueId(it.GetObjectId());
    ret = true;
  }

  return ret;
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

static float SpiderPlatformRiderDecayTime = 0.02f; // Guessed name

void CMorphBall::ApplySpiderBallRollForces(const CFinalInput& input, CStateManager& mgr, float dt) {
  CVector2f surfaceForces = CalculateSpiderBallAttractionSurfaceForces(input);
  CVector3f viewSurfaceForces = TransformSpiderBallForcesXZ(surfaceForces, mgr);
  const CTransform4f camXf = mPlayer.GetCameraManager()->GetCurrentCamera(mgr, true)->GetTransform();
  const CVector3f spiderDirNorm = mSpiderInterpBetweenPoints.AsNormalized();

  const float spiderUpDot = CMath::AbsF(CVector3f::Dot(spiderDirNorm, camXf.GetColumn(kDZ)));
  const float spiderForwardDot = CMath::AbsF(CVector3f::Dot(spiderDirNorm, camXf.GetColumn(kDY)));
  if (mPlayer.GetSpiderBallControlXY()) {
    if (spiderUpDot < 0.25f) {
      if (spiderForwardDot > 0.25f) {
        viewSurfaceForces = TransformSpiderBallForcesXY(surfaceForces, mgr);
      }
    }
  }

  const float forceMag = surfaceForces.Magnitude();
  CVector2f normSurfaceForces(0.f, 0.f);
  bool isSurface = false;
  if (mSpiderSurfaceType == kSST_CollisionSurface || mSpiderSurfaceType == kSST_ScriptedSurface) {
    isSurface = true;
  }
  float spiderTrackForceMag =
      isSurface ? forceMag : CVector3f::Dot(viewSurfaceForces, spiderDirNorm);

  bool forceApplied = true;
  bool moving;
  bool continueTrackForce = false;
  if (CMath::AbsF(forceMag) > 0.05f) {
    normSurfaceForces = surfaceForces.AsNormalized();
    if (!isSurface && CVector2f::Dot(normSurfaceForces, mNormalizedSpiderSurfaceForces) > 0.9f) {
      continueTrackForce = true;
      spiderTrackForceMag = forceMag * CMath::Sign(mSpiderTrackForceMagnitude);
    } else if (CMath::AbsF(spiderTrackForceMag) > 0.05f) {
      spiderTrackForceMag = forceMag * CMath::Sign(spiderTrackForceMag);
    } else {
      forceApplied = false;
    }
  } else {
    forceApplied = false;
  }

  if (!continueTrackForce) {
    mNormalizedSpiderSurfaceForces = normSurfaceForces;
    mSpiderTrackForceMagnitude = spiderTrackForceMag;
    mSpiderForcesReset = true;
  }

  if (!forceApplied) {
    spiderTrackForceMag = 0.f;
    ResetSpiderBallForces();
  }

  moving = true;
  if (!forceApplied) {
    if (!(mPlayer.GetVelocityWR().Magnitude() > 6.5f)) {
      moving = false;
    }
  }

  CVector3f moveDelta = CVector3f::Zero();
  if (mTouchingSpider && forceApplied) {
    if (isSurface) {
      moveDelta = 0.1f * viewSurfaceForces;
    } else {
      const float spiderTrackSign = CMath::Sign(spiderTrackForceMag);
      moveDelta = mSpiderBetweenPoints.AsNormalized() * 0.1f * spiderTrackSign;
    }
  }

  CVector3f ballPos = GetBallPosition() + moveDelta;
  bool lockToCurrentTrack = false;
  float distance = 0.f;
  if (!moving && mTouchingSpider && 1.f == mSpiderPullMovement && !mSpiderSwingInAir) {
    lockToCurrentTrack = true;
  }

  if (!lockToCurrentTrack) {
    mSpiderNearby = false;
    TUniqueId surfaceId = kInvalidUniqueId;
    if (FindClosestSpiderBallWaypoint(mgr, ballPos, mSpiderTrackPoint, mSpiderInterpBetweenPoints,
                                      mSpiderBetweenPoints, distance, mPlayerToSpiderNormal,
                                      mSpiderSurfaceType, surfaceId, mSpiderSurfaceTransform)) {
      mSpiderNearby = true;
      mSpiderSwingInAir = false;
      if (surfaceId != kInvalidUniqueId) {
        if (CScriptPlatform* platform = TCastToPtr< CScriptPlatform >(mgr.ObjectById(surfaceId))) {
          platform->AddRider(mPlayer.GetUniqueId(), mgr,
                             rstl::optional_object< float >(SpiderPlatformRiderDecayTime));
        }
      }

      if (CMath::AbsF(CVector3f::Dot(mSpiderInterpBetweenPoints, mPlayerToSpiderNormal)) >
              0.95f &&
          mSpiderSurfaceType == kSST_CollisionSurface) {
        mSpiderInterpBetweenPoints = mPlayer.GetTransform().GetColumn(kDY);
      }
    }
  } else {
    mPlayerToSpiderNormal = mSpiderTrackPoint - ballPos;
    distance = mPlayerToSpiderNormal.Magnitude();
    mPlayerToSpiderNormal *= 1.f / (-1.f * distance);
    mSpiderNearby = true;
  }

  if (mSpiderNearby) {
    if (distance < sBallCloseToCollisionDistance) {
      mTouchingSpider = true;
    }

    const float angVelDamp = 0.2f;
    if (mTouchingSpider == true) {
      if (moving) {
        if (!isSurface) {
          mLinearVelocityDamping = 0.4f;
          mAngularVelocityDamping = angVelDamp;

          const CVector3f spiderInterpNorm = mSpiderInterpBetweenPoints.AsNormalized();
          float viewControlMag = CVector3f::Dot(viewSurfaceForces, spiderInterpNorm);
          if (continueTrackForce && !mSpiderForcesReset) {
            viewControlMag = mSpiderViewControlMagnitude;
          } else {
            mSpiderViewControlMagnitude = viewControlMag;
            mSpiderForcesReset = false;
          }

          float spiderForceMag;
          if (CMath::AbsF(viewControlMag) > 0.1f) {
            const float spiderTrackSign = CMath::Sign(viewControlMag);
            spiderForceMag = spiderTrackSign * CMath::Clamp(-1.f, forceMag, 1.f);
          } else {
            spiderForceMag = 0.f;
            ResetSpiderBallForces();
          }

          if (distance > 1.05f) {
            spiderForceMag *= (1.05f - (distance - 1.05f)) / 1.05f;
          }

          mPlayer.ApplyForceWR(spiderForceMag * (mSpiderBetweenPoints.AsNormalized() * 90000.f),
                               CAxisAngle::Identity());
        } else {
          mLinearVelocityDamping = 0.3f;
          mAngularVelocityDamping = angVelDamp;

          const float surfaceXDot =
              CVector3f::Dot(mSpiderSurfaceTransform.GetColumn(kDX), viewSurfaceForces);
          const float surfaceZDot =
              CVector3f::Dot(mSpiderSurfaceTransform.GetColumn(kDZ), viewSurfaceForces);
          const float signedXForce =
              CMath::Sign(surfaceXDot) * CMath::AbsF(surfaceForces.GetX());
          const float signedZForce =
              CMath::Sign(surfaceZDot) * CMath::AbsF(surfaceForces.GetY());

          float surfaceXForce;
          if (CMath::AbsF(surfaceXDot) > 0.4f) {
            surfaceXForce = signedXForce;
          } else if (CMath::AbsF(surfaceXDot) > 0.1f) {
            surfaceXForce = (CMath::AbsF(surfaceXDot) / 0.3f) * signedXForce;
          } else {
            surfaceXForce = 0.f;
          }

          float surfaceZForce;
          if (CMath::AbsF(surfaceZDot) > 0.4f) {
            surfaceZForce = signedZForce;
          } else if (CMath::AbsF(surfaceZDot) > 0.1f) {
            surfaceZForce = (CMath::AbsF(surfaceZDot) / 0.3f) * signedZForce;
          } else {
            surfaceZForce = 0.f;
          }

          const CVector3f forceVec =
              45000.f * (surfaceXForce * mSpiderSurfaceTransform.GetColumn(kDX) +
                         surfaceZForce * mSpiderSurfaceTransform.GetColumn(kDZ));
          mPlayer.ApplyForceWR(forceVec, CAxisAngle::Identity());

          const rstl::pair< float, float > pivotForces(45000.f * surfaceXForce,
                                                       45000.f * surfaceZForce);
          float angle = mSpiderSurfacePivotTargetAngle;
          if (forceVec.MagSquared() > 0.f) {
            angle = atan2f(pivotForces.first, pivotForces.second);
            if (angle - mSpiderSurfacePivotAngle > M_PIF / 2.f) {
              angle -= M_PIF;
            } else if (mSpiderSurfacePivotAngle - angle > M_PIF / 2.f) {
              angle += M_PIF;
            }
            mSpiderSurfacePivotTargetAngle = angle;
          }

          const float pivotDelta = angle - mSpiderSurfacePivotAngle;
          const float absPivotDelta = CMath::AbsF(pivotDelta);
          const float pivotStep = rstl::min_val(0.2f, absPivotDelta);
          mSpiderSurfacePivotAngle = pivotStep * CMath::Sign(pivotDelta) + mSpiderSurfacePivotAngle;

          const CRelAngle pivotAngle = CRelAngle::FromRadians(mSpiderSurfacePivotAngle);
          const CTransform4f& rotateY = CTransform4f::RotateY(pivotAngle);
          mSpiderInterpBetweenPoints = mSpiderSurfaceTransform.Rotate(rotateY.GetColumn(kDZ));
        }
      }

      const float spiderPullForce = 8.f * (mPlayer.GetMass() * gpTweakBall->GetBallGravity());
      mPlayer.ApplyForceWR(CVector3f(0.f, 0.f, (1.f - mSpiderPullMovement) * spiderPullForce),
                           CAxisAngle::Identity());
    } else {
      mLinearVelocityDamping = 0.2f;
      mAngularVelocityDamping = angVelDamp;
    }

    mPlayer.SetMomentumWR(
        4.f * ((mPlayer.GetMass() * gpTweakBall->GetBallGravity()) * mPlayerToSpiderNormal));
  }
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
  const CTransform4f camXf =
      mPlayer.GetCameraManager()->GetCurrentCamera(mgr, true)->GetTransform();
  CVector3f ret = camXf.GetColumn(kDX) * forces.GetX() + camXf.GetColumn(kDY) * forces.GetY();
  return ret;
}

CVector3f CMorphBall::TransformSpiderBallForcesXZ(CVector2f& forces, CStateManager& mgr) const {
  const CTransform4f camXf =
      mPlayer.GetCameraManager()->GetCurrentCamera(mgr, true)->GetTransform();
  CVector3f ret = camXf.GetColumn(kDX) * forces.GetX() + camXf.GetColumn(kDZ) * forces.GetY();
  return ret;
}

void CMorphBall::ApplySpiderBallSwingingForces(const CFinalInput& input, CStateManager& mgr,
                                               float dt) {
  mLinearVelocityDamping = 0.04f;
  mAngularVelocityDamping = 0.99f;
  mPlayerToSpiderNormal = mSpiderTrackPoint - mPlayer.GetTranslation();

  const float playerToSpiderDist = mPlayerToSpiderNormal.Magnitude();
  mPlayerToSpiderNormal *= 1.f / (-1.f * playerToSpiderDist);

  const float movement = GetSpiderBallControllerMovement(input);
  UpdateSpiderBallSwingControllerMovementTimer(movement, dt);

  const float swingMovement = movement * GetSpiderBallSwingControllerMovementScalar();
  const float swingForce = 110000.f * playerToSpiderDist / 3.7f;
  const CVector3f swing = CVector3f::Cross(mPlayerToSpiderNormal, mSpiderBetweenPoints);
  mPlayer.ApplyForceWR(CVector3f::Cross(swing, mPlayerToSpiderNormal).AsNormalized() * swingForce *
                           swingMovement * 0.06f,
                       CAxisAngle::Identity());
  mPlayer.SetMomentumWR(CVector3f(0.f, 0.f, mPlayer.GetMass() * gpTweakBall->GetBallGravity()));
  mRefPullVelocity = (1.f - mSpiderPullMovement) * 3.7f + 1.4f;
  mPlayerToSpiderTrackDistance = playerToSpiderDist;

  CVector3f playerVel = mPlayer.GetVelocityWR();
  const float playerSpeed = playerVel.Magnitude();
  playerVel -= mPlayerToSpiderNormal * playerSpeed *
               CVector3f::Dot(mPlayerToSpiderNormal, playerVel.AsNormalized());

  float maxPullVel = 0.04f;
  if (1.f == mSpiderPullMovement && CMath::AbsF(mPlayerToSpiderNormal.GetZ()) > 0.8f) {
    maxPullVel = 0.3f;
  }

  const float pullDelta = mRefPullVelocity - playerToSpiderDist;
  const float signedMaxPull = maxPullVel * CMath::Sign(pullDelta);
  const float clampedPull = rstl::min_val(CMath::AbsF(signedMaxPull), CMath::AbsF(pullDelta));
  playerVel += mPlayerToSpiderNormal * (clampedPull * CMath::Sign(signedMaxPull) / dt);
  mPlayer.SetVelocityWR(playerVel);
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
, mActorLights(rs_new CActorLights(8, CVector3f::Zero(), 4, 4, 0.1f, false, false, false, false))
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
