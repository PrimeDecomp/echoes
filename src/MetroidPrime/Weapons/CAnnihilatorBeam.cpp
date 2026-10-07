#include "MetroidPrime/Weapons/CAnnihilatorBeam.hpp"

#include "Kyoto/Particles/CElementGen.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CCameraShakerManager.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Weapons/CAnnihilatorProjectile.hpp"
#include "MetroidPrime/Weapons/WeaponSound.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerGun.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "rstl/math.hpp"
#include "Kyoto/Particles/CRealElement.hpp"
#include "Kyoto/Particles/CVectorElement.hpp"
#include "Weapons/CWeaponDescription.hpp"

// Unnamed string pointer ("Plasma2nd_1") in another unit's sdata2.
extern const char* const lbl_8041D3AC;
// Unnamed shared muzzle offset duration (0.5f) in another unit's sdata2.
extern const float lbl_8041C620;

static const ushort kSoundIds[2][2] = {{0x1FCC, 0x1FDC}, {0x25BF, 0x25BA}};

CAnnihilatorBeam::CAnnihilatorBeam(TUniqueId playerId, const CVector3f& scale, int flags)
: CGunWeapon(kWT_Annihilator, playerId, scale, flags)
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
  if (mChargeGenerator.get() && mEnabledSecondaryEffect != kSFT_None) {
    mChargeGenerator->Render();
  }
  CGunWeapon::PostRenderGunFx(mgr, xf);
}

void CAnnihilatorBeam::UpdateGunFx(bool shotSmoke, float dt, const CStateManager& mgr,
                                   const CTransform4f& xf) {
  if (mChargeGenerator.get() && mEnabledSecondaryEffect != kSFT_None) {
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

void CAnnihilatorBeam::Fire(const TCachedToken< CWeaponDescription >& projectile, bool underwater,
                            float dt, CPlayerState::EChargeStage chargeState,
                            const CTransform4f& xf, CStateManager& mgr, TUniqueId homingTarget,
                            uint projectileAttributes, ushort soundId, TUniqueId* projectileId,
                            CSfxHandle* soundHandle, float chargeFactor1, float chargeFactor2) {
  ushort sfx;
  if (soundId == CSfxManager::kInternalInvalidSfxId) {
    sfx = kSoundIds[mgr.IsMultiplayer() ? 1 : 0][chargeState];
  } else {
    sfx = soundId;
  }
  CTransform4f shotXf = xf;
  shotXf.AddTranslation(-1.2f * shotXf.GetForward());
  CPlayer* player = GetPlayerFromAll(mgr);
  if (chargeState == CPlayerState::kCS_Normal) {
    if (mShotDelayTimer < 0.01f) {
      const TUniqueId target = homingTarget;
      ActivateCharge(false, true);
      FireProjectile(projectile, underwater, dt, chargeState, shotXf, mgr, target,
                     projectileAttributes, sfx, chargeFactor1, 1.f, 1.f);
      mShotDelayTimer += gpTweakPlayerGun->GetBeamInfo(CPlayerState::kBI_Annihilator).mCoolDown;
      mShotDelay = gpTweakPlayerGun->GetBeamInfo(CPlayerState::kBI_Annihilator).mCoolDown;
    }
  } else {
    CGunWeapon::Fire(projectile, underwater, dt, chargeState, xf, mgr, homingTarget,
                     projectileAttributes, sfx, projectileId, soundHandle, chargeFactor1, 1.f);
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

bool CAnnihilatorBeam::IsLoaded() const {
  bool ret = false;
  if (CGunWeapon::IsLoaded() && mEffectLoaded) {
    ret = true;
  }
  return ret;
}

void CAnnihilatorBeam::EnableSecondaryFx(ESecondaryFxType type) {
  switch (type) {
  case kSFT_CancelCharge:
    if (mEnabledSecondaryEffect != kSFT_None && mChargeGenerator.get() != nullptr) {
      mChargeGenerator->SetParticleEmission(false);
      break;
    }
    return;
  case kSFT_Charge:
    mChargeGenerator = rs_new CElementGen(TToken< CGenDescription >(*mChargeEffect),
                                          CElementGen::kMOT_Normal, CElementGen::kOSF_One);
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
    mChargeEffect = TCachedToken< CGenDescription >(gpSimplePool->GetObj(lbl_8041D3AC));
    TToken< CWeaponDescription > weapon(mWeapons[0]);
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

void CAnnihilatorBeam::FireProjectile(const TCachedToken< CWeaponDescription >& projectile,
                                      bool underwater, float dt,
                                      CPlayerState::EChargeStage chargeState,
                                      const CTransform4f& xf, CStateManager& mgr,
                                      TUniqueId homingTarget, uint projectileAttributes,
                                      ushort soundId, float damageFactor, float projectileScale,
                                      float projectileFactor) {
  CDamageInfo damage(GetDamageInfo(mgr, chargeState, damageFactor));
  CVector3f scale =
      (chargeState == CPlayerState::kCS_Normal && (projectileAttributes & 8) == 0 ? 1.f
                                                                                  : projectileScale) *
      CVector3f::One();
  const bool partialCharge =
      chargeState == CPlayerState::kCS_Normal ? false : !close_enough(damageFactor, 1.f);
  CPlayer* player = GetPlayerFromAll(mgr);
  uint chargeAttributes =
      (chargeState != CPlayerState::kCS_Normal ? CWeapon::kPA_Charged : CWeapon::kPA_None) |
      (partialCharge ? CWeapon::kPA_ParticleOPTS : CWeapon::kPA_None);
  chargeAttributes |= projectileAttributes;
  CEnergyProjectile* proj = rs_new CAnnihilatorProjectile(
      projectile, mWeaponType, xf, kMT_NoPlatformCollision, damage, mgr.AllocateUniqueId(),
      kInvalidAreaId, GetPlayerId(), homingTarget, projectileAttributes, underwater, scale,
      mProjectileSpeed, mProjectileTurnRate);
  if (proj) {
    mgr.AddObject(proj);
    if (chargeState != CPlayerState::kCS_Normal && damageFactor == 1.f) {
      proj->SetX4104(true);
    }
    proj->SetHomingTurnRateScale(projectileFactor);
    proj->InitializeMuzzleOffset(lbl_8041C620, mgr);
    proj->SetFluidList(player->GetCameraManager()->GetFirstPersonCamera()->GetFluidList());
    proj->Think(dt, mgr);
  }
  if (chargeState != CPlayerState::kCS_Normal && (chargeAttributes & 0x800000) == 0) {
    mEnableCharge = true;
    player->CameraManager()->CameraShakerManager()->AddCameraShaker(
        gpTweakPlayerGunSingle->GetRecoilCameraShakerData(), mgr, false, false);
  }
  if ((chargeAttributes & 0x1000000) == 0) {
    CAnimData& animData = *mSolidModelData->AnimationData();
    animData.EnableLooping(false);
    animData.SetAnimation(CAnimPlaybackParms(mShootAnimIds[chargeState], -1, 1.f, true), false);
  }
  if (soundId != CSfxManager::kInternalInvalidSfxId) {
    PlaySfxForPlayer(player, soundId, mSoundVolume, mgr.GetNextAreaId().Value(), underwater,
                     false);
  }
}
