#include "MetroidPrime/Weapons/CDarkBeam.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"

static const ushort kFireSoundIds[2][2] = {
    {0x1fc9, 0x1fc8},
    {0x25ac, 0x25a6},
};

CDarkBeam::CDarkBeam(TUniqueId playerId, const CVector3f& scale, int flags)
: CGunWeapon(kWT_Dark, playerId, scale, flags), mChargedProjectileId(kInvalidUniqueId) {}

CDarkBeam::~CDarkBeam() {}

void CDarkBeam::ReInitVariables() {
  mSmokeGenerator = nullptr;
  mChargeGenerator = nullptr;
  mEffectsLoaded = false;
  mInEndEffect = false;
  mChargedShotSound.Clear();
  mChargedProjectileId = kInvalidUniqueId;
  mEnabledSecondaryEffect = kSFT_None;
}

void CDarkBeam::PreRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) {}

void CDarkBeam::PostRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) {
  if (mSmokeGenerator.get() != nullptr) {
    mSmokeGenerator->Render();
  }
  if (mEnabledSecondaryEffect != kSFT_None && mChargeGenerator.get() != nullptr) {
    mChargeGenerator->Render();
  }
  CGunWeapon::PostRenderGunFx(mgr, xf);
}

void CDarkBeam::UpdateGunFx(bool shotSmoke, float dt, const CStateManager& mgr,
                            const CTransform4f& xf) {
  if (mSmokeGenerator.get() != nullptr) {
    CTransform4f locator =
        mSolidModelData->GetScaledLocatorTransform(rstl::string_l(CGunWeapon::skMuzzleLocator));
    mSmokeGenerator->SetTranslation(locator.GetTranslation());
    mSmokeGenerator->SetOrientation(locator.GetRotation());
    mSmokeGenerator->Update(dt);
  }
  if (mChargeGenerator.get() != nullptr) {
    if (mInEndEffect && mChargeGenerator->IsSystemDeletable()) {
      mEnabledSecondaryEffect = kSFT_None;
      mChargeGenerator = nullptr;
    }
    if (mChargeGenerator.get() != nullptr && mEnabledSecondaryEffect != kSFT_None) {
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
        mSmokeEffect->IsLoaded() && mChargeEffect->IsLoaded() && mEndEffect->IsLoaded();
    if (mEffectsLoaded) {
      mSmokeGenerator = rs_new CElementGen(*mSmokeEffect);
      mSmokeGenerator->SetGlobalScale(mScale);
    }
  }

  if (mChargedProjectileId != kInvalidUniqueId && mChargedShotSound) {
    bool stop = true;
    CEnergyProjectile* projectile =
        TCastToPtr< CEnergyProjectile >(mgr.ObjectById(mChargedProjectileId));
    if (projectile != nullptr) {
      if (projectile->HasExploded()) {
        CSfxManager::SfxStop(mChargedShotSound);
      } else {
        stop = false;
      }
    }
    if (stop) {
      mChargedShotSound.Clear();
      mChargedProjectileId = kInvalidUniqueId;
    }
  }
}

void CDarkBeam::Fire(const TToken< CWeaponDescription >& projectile, bool underwater, float dt,
                     CPlayerState::EChargeStage chargeState, const CTransform4f& xf,
                     CStateManager& mgr, TUniqueId homingTarget, uint projectileAttributes,
                     ushort soundId, TUniqueId* projectileId, CSfxHandle* soundHandle,
                     float chargeFactor1, float chargeFactor2) {
  if (soundId == CSfxManager::kInternalInvalidSfxId) {
    soundId = kFireSoundIds[mgr.IsMultiplayer() ? 1 : 0][chargeState];
  }
  const bool charged = chargeState == CPlayerState::kCS_Charged;
  if (!charged) {
    ActivateCharge(false, true);
  }
  CGunWeapon::Fire(projectile, underwater, dt, chargeState, xf, mgr, homingTarget,
                   projectileAttributes, soundId, charged ? &mChargedProjectileId : nullptr,
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

void CDarkBeam::InitializeResources(CStateManager& mgr) {
  if (!mResourcesAllocated) {
    CGunWeapon::InitializeResources(mgr);
    mSmokeEffect = gpSimplePool->GetObj("IceSmoke");
    mChargeEffect = gpSimplePool->GetObj("Ice2nd_1");
    mEndEffect = gpSimplePool->GetObj("Ice2nd_2");
  }
}

void CDarkBeam::EnableFx(bool enable) {
  if (mSmokeGenerator.get() != nullptr) {
    mSmokeGenerator->SetParticleEmission(enable);
  }
}
