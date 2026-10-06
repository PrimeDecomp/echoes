#include "MetroidPrime/Weapons/CAnnihilatorBeam.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CRealElement.hpp"
#include "Kyoto/Particles/CVectorElement.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CCameraShakerManager.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerGun.hpp"
#include "MetroidPrime/Weapons/CAnnihilatorProjectile.hpp"
#include "MetroidPrime/Weapons/WeaponSound.hpp"
#include "Weapons/CWeaponDescription.hpp"

static const ushort kFireSoundIds[2][2] = {
    {0x1fc9, 0x1fc8},
    {0x25ac, 0x25a6},
};

static const char* const kChargeEffectName = "Plasma2nd_1";

CAnnihilatorBeam::CAnnihilatorBeam(TUniqueId playerId, const CVector3f& scale, int flags)
: CGunWeapon(kWT_Annihilator, playerId, scale, flags)
, mChargeGenerator(nullptr)
, mShotDelayTimer(0.f)
, mShotDelay(0.f)
, mLightingResetDelayTimer(0.f)
, mProjectileSpeed(0.f)
, mProjectileTurnRate(0.f)
, mLightingArea(kInvalidAreaId)
, mEffectLoaded(false)
, mWorldLightingDimmed(false) {}

CAnnihilatorBeam::~CAnnihilatorBeam() {}

void CAnnihilatorBeam::ReInitVariables() {
  mChargeGenerator = nullptr;
  mEffectLoaded = false;
  mEnabledSecondaryEffect = kSFT_None;
}

void CAnnihilatorBeam::PostRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) {
  if (mChargeGenerator.get() != nullptr && mEnabledSecondaryEffect != kSFT_None) {
    mChargeGenerator->Render();
  }
  CGunWeapon::PostRenderGunFx(mgr, xf);
}

void CAnnihilatorBeam::UpdateGunFx(bool shotSmoke, float dt, const CStateManager& mgr,
                                   const CTransform4f& xf) {
  if (mChargeGenerator.get() != nullptr && mEnabledSecondaryEffect != kSFT_None) {
    if (mChargeGenerator->IsSystemDeletable()) {
      mEnabledSecondaryEffect = kSFT_None;
    }
    mChargeGenerator->SetTranslation(xf.GetTranslation());
    mChargeGenerator->SetOrientation(xf.GetRotation());
    mChargeGenerator->Update(dt);
  }
  CGunWeapon::UpdateGunFx(shotSmoke, dt, mgr, xf);
}

void CAnnihilatorBeam::Update(float dt, CStateManager& mgr) {
  CGunWeapon::Update(dt, mgr);
  mShotDelayTimer -= dt;
  mShotDelayTimer = rstl::max_val(0.f, mShotDelayTimer);
  mLightingResetDelayTimer -= dt;

  CPlayer* player = GetPlayer(mgr);
  const CPlayerState* playerState = player->GetPlayerState();
  if (!mgr.IsMultiplayer()) {
    if (playerState->GetChargeBeamFactor() > 0.5f) {
      SetWorldLighting(mgr, player->GetCurrentAreaId(), 0.2f, 0.8f);
    } else if (mLightingResetDelayTimer < 0.f && mWorldLightingDimmed) {
      SetWorldLighting(mgr, player->GetCurrentAreaId(), 2.f, 1.f);
    }
  }

  if (!IsLoaded() && CGunWeapon::IsLoaded() && !mEffectLoaded) {
    mEffectLoaded = mChargeEffect->IsLoaded();
  }
}

void CAnnihilatorBeam::Fire(const TToken< CWeaponDescription >& projectile, bool underwater,
                            float dt, CPlayerState::EChargeStage chargeState,
                            const CTransform4f& xf, CStateManager& mgr, TUniqueId homingTarget,
                            uint projectileAttributes, ushort soundId, TUniqueId* projectileId,
                            CSfxHandle* soundHandle, float chargeFactor1, float chargeFactor2) {
  if (soundId == CSfxManager::kInternalInvalidSfxId) {
    soundId = kFireSoundIds[mgr.IsMultiplayer() ? 1 : 0][chargeState];
  }

  CTransform4f shotXf = xf;
  shotXf.SetTranslation(shotXf.GetTranslation() + shotXf.GetColumn(kDY) * -1.2f);
  CPlayer* player = GetPlayerFromAll(mgr);
  if (chargeState == CPlayerState::kCS_Normal) {
    if (mShotDelayTimer < 0.01f) {
      ActivateCharge(false, true);
      FireProjectile(projectile, underwater, dt, chargeState, shotXf, mgr, homingTarget,
                     projectileAttributes, soundId, chargeFactor1, 1.f, 1.f);
      mShotDelayTimer += gpTweakPlayerGun->GetBeamInfo(CPlayerState::kBI_Annihilator).mCoolDown;
      mShotDelay = gpTweakPlayerGun->GetBeamInfo(CPlayerState::kBI_Annihilator).mCoolDown;
    }
  } else {
    CGunWeapon::Fire(projectile, underwater, dt, chargeState, xf, mgr, homingTarget,
                     projectileAttributes, soundId, projectileId, soundHandle, chargeFactor1, 1.f);
    mLightingResetDelayTimer = 0.65f;
    if (!mgr.IsMultiplayer()) {
      SetWorldLighting(mgr, player->GetCurrentAreaId(), 8.f, 0.7f);
    }
  }
}

void CAnnihilatorBeam::Load(CStateManager& mgr, bool subtypeBasePose) {
  CGunWeapon::Load(mgr, subtypeBasePose);
  mChargeEffect->Lock();
}

void CAnnihilatorBeam::Unload(CStateManager& mgr) {
  CGunWeapon::Unload(mgr);
  if (!mgr.IsMultiplayer()) {
    mChargeEffect->Unlock();
  }
  ResetWorldLighting(mgr);
  ReInitVariables();
}

void CAnnihilatorBeam::ReleaseResources(CStateManager& mgr) {
  CGunWeapon::ReleaseResources(mgr);
  if (!mgr.IsMultiplayer()) {
    mChargeEffect->Unlock();
  }
  ResetWorldLighting(mgr);
  mChargeGenerator = nullptr;
  mEnabledSecondaryEffect = kSFT_None;
}

bool CAnnihilatorBeam::IsLoaded() const { return CGunWeapon::IsLoaded() && mEffectLoaded; }

void CAnnihilatorBeam::EnableSecondaryFx(ESecondaryFxType type) {
  switch (type) {
  case kSFT_CancelCharge:
    if (mEnabledSecondaryEffect == kSFT_None || mChargeGenerator.get() == nullptr) {
      return;
    }
    mChargeGenerator->SetParticleEmission(false);
    break;
  case kSFT_Charge:
    mChargeGenerator = rs_new CElementGen(*mChargeEffect);
    mChargeGenerator->SetGlobalScale(mScale);
    break;
  default:
    break;
  }
  mEnabledSecondaryEffect = type;
}

void CAnnihilatorBeam::InitializeResources(CStateManager& mgr) {
  if (!mResourcesAllocated) {
    CGunWeapon::InitializeResources(mgr);
    mChargeEffect = gpSimplePool->GetObj(kChargeEffectName);

    TToken< CWeaponDescription > weapon = mWeapons[0];
    CVector3f velocity = 1.3f * CVector3f::Forward();
    weapon->mIVEC->GetValue(0, velocity);
    mProjectileSpeed = velocity.Magnitude();
    mProjectileTurnRate = 360.f;
    weapon->mTRAT->GetValue(0, mProjectileTurnRate);
    mProjectileTurnRate /= 60.f;
  }
}

void CAnnihilatorBeam::SetWorldLighting(CStateManager& mgr, TAreaId areaId, float speed,
                                        float target) {
  if (mWorldLightingDimmed && mLightingArea != areaId && mLightingArea != kInvalidAreaId) {
    CGameArea* area = mgr.World()->Area(mLightingArea);
    if (area->IsLoaded()) {
      area->SetWeaponWorldLighting(2.f, 1.f);
    }
  }
  mLightingArea = areaId;
  mWorldLightingDimmed = target != 1.f;
  if (mLightingArea != kInvalidAreaId) {
    CGameArea* area = mgr.World()->Area(mLightingArea);
    if (area->IsLoaded()) {
      area->SetWeaponWorldLighting(speed, target);
    }
  }
}

void CAnnihilatorBeam::ResetWorldLighting(CStateManager& mgr) {
  if (mWorldLightingDimmed && !mgr.IsMultiplayer()) {
    SetWorldLighting(mgr, GetPlayer(mgr)->GetCurrentAreaId(), 2.f, 1.f);
  }
}

void CAnnihilatorBeam::FireProjectile(const TToken< CWeaponDescription >& projectile,
                                      bool underwater, float dt,
                                      CPlayerState::EChargeStage chargeState,
                                      const CTransform4f& xf, CStateManager& mgr,
                                      TUniqueId homingTarget, uint projectileAttributes,
                                      ushort soundId, float damageFactor, float projectileScale,
                                      float projectileFactor) {
  CDamageInfo damage(GetDamageInfo(mgr, chargeState, damageFactor));
  CVector3f scale = (chargeState == CPlayerState::kCS_Normal && (projectileAttributes & 8) == 0
                         ? 1.f
                         : projectileScale) *
                    CVector3f::One();
  const bool partialCharge =
      chargeState == CPlayerState::kCS_Normal ? false : !close_enough(damageFactor, 1.f);
  CPlayer* player = GetPlayerFromAll(mgr);
  const uint chargeAttributes =
      (partialCharge ? CWeapon::kPA_ParticleOPTS : 0) |
      (chargeState != CPlayerState::kCS_Normal ? CWeapon::kPA_Charged : 0) | projectileAttributes;
  CAnnihilatorProjectile* proj = rs_new CAnnihilatorProjectile(
      projectile, mWeaponType, xf, kMT_NoPlatformCollision, damage, mgr.AllocateUniqueId(),
      kInvalidAreaId, mPlayerId, homingTarget, projectileAttributes, underwater, scale,
      mProjectileSpeed, mProjectileTurnRate);
  if (proj) {
    mgr.AddObject(proj);
    if (chargeState != CPlayerState::kCS_Normal && damageFactor == 1.f) {
      proj->SetX4104(true);
    }
    proj->SetHomingTurnRateScale(projectileFactor);
    proj->InitializeMuzzleOffset(0.5f, mgr);
    proj->SetFluidList(player->GetCameraManager()->GetFirstPersonCamera()->GetFluidList());
    proj->Think(dt, mgr);
  }
  if (chargeState != CPlayerState::kCS_Normal && (chargeAttributes & 0x800000) == 0) {
    mEnableCharge = true;
    const CCameraShakerData shaker = gpTweakPlayerGunSingle->GetRecoilCameraShakerData();
    player->CameraManager()->CameraShakerManager()->AddCameraShaker(shaker, mgr, false, false);
  }
  if ((chargeAttributes & 0x1000000) == 0) {
    CAnimData& animData = *mSolidModelData->AnimationData();
    animData.EnableLooping(false);
    animData.SetAnimation(CAnimPlaybackParms(mShootAnimIds[chargeState], -1, 1.f, true), false);
  }
  if (soundId != CSfxManager::kInternalInvalidSfxId) {
    PlaySfxForPlayer(player, soundId, mSoundVolume, mgr.GetNextAreaId().Value(), underwater, false);
  }
}
