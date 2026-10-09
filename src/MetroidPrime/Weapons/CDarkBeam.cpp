#include "MetroidPrime/Weapons/CDarkBeam.hpp"

#include "MetroidPrime/Player/GunResNames.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/SFX/Weapons2.h"
#include "MetroidPrime/SFX/Weapons2_MP.h"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"

// Asset-name pointers defined in another TU (unsplit .sdata2).

static const ushort kFireSounds[2][2] = {
    {SFXsam_a_drkfire_00_oneshot, SFXsam_a_drkchfire_00_oneshot},
    {SFXsa2_a_drkfire_00_oneshot, SFXsa2_a_drkchfire_00_oneshot}};

CDarkBeam::CDarkBeam(TUniqueId playerId, const CVector3f& scale, int flags)
: CGunWeapon(kWT_Dark, playerId, scale, flags)
, mChargedProjectileId(kInvalidUniqueId)
, mEffectsLoaded(false)
, mInEndEffect(false) {}

CDarkBeam::~CDarkBeam() {}

void CDarkBeam::ReInitVariables() {
  mSmokeGenerator = nullptr;
  mChargeGenerator = nullptr;
  mEffectsLoaded = false;
  mInEndEffect = false;
  mChargedShotSound = CSfxHandle();
  mChargedProjectileId = kInvalidUniqueId;
  mEnabledSecondaryEffect = kSFT_None;
}

void CDarkBeam::PreRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) {}

void CDarkBeam::PostRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) {
  if (!mSmokeGenerator.null()) {
    mSmokeGenerator->Render();
  }
  if (mEnabledSecondaryEffect != kSFT_None && !mChargeGenerator.null()) {
    mChargeGenerator->Render();
  }
  CGunWeapon::PostRenderGunFx(mgr, xf);
}

void CDarkBeam::UpdateGunFx(bool shotSmoke, float dt, const CStateManager& mgr,
                            const CTransform4f& xf) {
  if (!mSmokeGenerator.null()) {
    CTransform4f locator =
        mSolidModelData->GetScaledLocatorTransform(rstl::string_l(CGunWeapon::skMuzzleLocator));
    mSmokeGenerator->SetTranslation(locator.GetTranslation());
    mSmokeGenerator->SetOrientation(locator.GetRotation());
    mSmokeGenerator->Update(dt);
  }
  if (!mChargeGenerator.null()) {
    if (mInEndEffect && mChargeGenerator->IsSystemDeletable()) {
      mEnabledSecondaryEffect = kSFT_None;
      mChargeGenerator = nullptr;
    }
    if (mEnabledSecondaryEffect != kSFT_None) {
      if (mInEndEffect) {
        mChargeGenerator->SetTranslation(xf.GetTranslation());
        mChargeGenerator->SetOrientation(xf.GetRotation());
      } else {
        mChargeGenerator->SetGlobalOrientAndTrans(xf);
      }
      mChargeGenerator->Update(dt);
    }
  }
  CGunWeapon::UpdateGunFx(shotSmoke, dt, mgr, xf);
}

void CDarkBeam::Update(float dt, CStateManager& mgr) {
  CGunWeapon::Update(dt, mgr);
  if (!mEffectsLoaded) {
    mEffectsLoaded =
        mSmokeEffect->TryCache() && mChargeEffect->TryCache() && mEndEffect->TryCache();
    if (mEffectsLoaded) {
      mSmokeGenerator = rs_new CElementGen(*mSmokeEffect);
      mSmokeGenerator->SetGlobalScale(mScale);
    }
  }
  if (mChargedProjectileId != kInvalidUniqueId && mChargedShotSound) {
    bool finished = true;
    if (const CEnergyProjectile* projectile =
            TCastToConstPtr< CEnergyProjectile >(mgr.GetObjectById(mChargedProjectileId))) {
      if (projectile->HasExploded()) {
        CSfxManager::SfxStop(mChargedShotSound);
      } else {
        finished = false;
      }
    }
    if (finished) {
      mChargedShotSound = CSfxHandle();
      mChargedProjectileId = kInvalidUniqueId;
    }
  }
}

void CDarkBeam::Fire(const TCachedToken< CWeaponDescription >& projectile, bool underwater,
                     float dt, CPlayerState::EChargeStage chargeState, const CTransform4f& xf,
                     CStateManager& mgr, TUniqueId homingTarget, uint projectileAttributes,
                     ushort soundId, TUniqueId* projectileId, CSfxHandle* soundHandle,
                     float chargeFactor1, float chargeFactor2) {
  ushort sfx;
  if (soundId == CSfxManager::kInternalInvalidSfxId) {
    sfx = kFireSounds[mgr.IsMultiplayer() ? 1 : 0][chargeState];
  } else {
    sfx = soundId;
  }
  const bool charged = chargeState == CPlayerState::kCS_Charged;
  if (!charged) {
    ActivateCharge(false, true);
  }
  CGunWeapon::Fire(projectile, underwater, dt, chargeState, xf, mgr, homingTarget,
                   projectileAttributes, sfx, charged ? &mChargedProjectileId : nullptr,
                   charged ? &mChargedShotSound : nullptr, chargeFactor1, 1.f);
}

void CDarkBeam::Load(CStateManager& mgr, bool subtypeBasePose) {
  CGunWeapon::Load(mgr, subtypeBasePose);
  mSmokeEffect->Lock();
  mChargeEffect->Lock();
  mEndEffect->Lock();
  mInEndEffect = false;
}

void CDarkBeam::Unload(CStateManager& mgr) {
  CGunWeapon::Unload(mgr);
  if (!mgr.IsMultiplayer()) {
    mEndEffect->Unlock();
    mChargeEffect->Unlock();
    mSmokeEffect->Unlock();
  }
  ReInitVariables();
}

void CDarkBeam::ReleaseResources(CStateManager& mgr) {
  CGunWeapon::ReleaseResources(mgr);
  if (!mgr.IsMultiplayer()) {
    mEndEffect->Unlock();
    mChargeEffect->Unlock();
    mSmokeEffect->Unlock();
  }
  mSmokeGenerator = nullptr;
  mChargeGenerator = nullptr;
  mEnabledSecondaryEffect = kSFT_None;
}

bool CDarkBeam::IsLoaded() const { return CGunWeapon::IsLoaded() && mEffectsLoaded; }

void CDarkBeam::EnableSecondaryFx(ESecondaryFxType type) {
  switch (type) {
  case kSFT_None:
  case kSFT_CancelCharge:
    if (mEnabledSecondaryEffect == kSFT_None) {
      return;
    }
    break;
  default:
    break;
  }

  switch (type) {
  case kSFT_None:
  case kSFT_ToCombo:
  case kSFT_CancelCharge:
    if (!mInEndEffect) {
      mChargeGenerator = rs_new CElementGen(*mEndEffect);
      mChargeGenerator->SetGlobalScale(mScale);
      mInEndEffect = true;
      mEnabledSecondaryEffect = kSFT_CancelCharge;
    }
    break;
  case kSFT_Charge:
    mChargeGenerator = rs_new CElementGen(*mChargeEffect);
    mChargeGenerator->SetGlobalScale(mScale);
    mEnabledSecondaryEffect = type;
    mInEndEffect = false;
    break;
  default:
    break;
  }
}

void CDarkBeam::EnableFx(bool enable) {
  if (!mSmokeGenerator.null()) {
    mSmokeGenerator->SetParticleEmission(enable);
  }
}

void CDarkBeam::InitializeResources(CStateManager& mgr) {
  if (!mResourcesAllocated) {
    CGunWeapon::InitializeResources(mgr);
    mSmokeEffect = gpSimplePool->GetObj(NWeaponRes::kIceSmoke);
    mChargeEffect = gpSimplePool->GetObj(NWeaponRes::kIce2nd1);
    mEndEffect = gpSimplePool->GetObj(NWeaponRes::kIce2nd2);
  }
}
