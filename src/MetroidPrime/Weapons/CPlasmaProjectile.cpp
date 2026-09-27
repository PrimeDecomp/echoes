#include "MetroidPrime/Weapons/CPlasmaProjectile.hpp"

#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Particles/CElectricDescription.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CBeamInfo.hpp"
#include "MetroidPrime/Weapons/CWeaponAssetInfo.hpp"

const int CPlasmaProjectile::kMaxPlasmaLights = 3;
const float CPlasmaProjectile::kInvMaxPlasmaLights = 1.f / CCast::ToReal32(kMaxPlasmaLights - 1);
static const CColor skCoreColor(1.f, 1.f, 1.f, 0.3f);

CPlasmaProjectile::CPlasmaProjectile(const TToken< CWeaponDescription >& description,
                                     const rstl::string& name, EWeaponType type,
                                     const CBeamInfo& beamInfo, const CTransform4f& xf,
                                     EMaterialTypes material, const CDamageInfo& damage,
                                     TUniqueId uid, TAreaId areaId, TUniqueId owner,
                                     const CWeaponAssetInfo& resources, bool drawOwnerFirst,
                                     uint attribs)
: CBeamProjectile(description, name, type, xf, beamInfo.GetLength(), beamInfo.GetRadius(),
                  beamInfo.GetTravelSpeed(), material, damage, uid, areaId, owner, attribs,
                  (beamInfo.GetBeamAttributes() & 0x200) != 0)
, mBeamAttributes(beamInfo.GetBeamAttributes())
, mLifeTime(beamInfo.GetLifeTime())
, mPulseSpeed(beamInfo.GetPulseSpeed())
, mShutdownTime(beamInfo.GetShutdownTime())
, mExpansionSpeed(beamInfo.GetExpansionSpeed())
, mMaxLength(beamInfo.GetLength() / 32.f)
, mCoreColor(skCoreColor)
, mInnerColor(beamInfo.GetInnerColor())
, mOuterColor(beamInfo.GetOuterColor())
, mPhazonDamage()
, mExpansionState(kES_Inactive)
, mInitialDamage(0.f)
, mBeamWidth(0.f)
, mLifeTimer(0.f)
, mExpansionT(0.f)
, mExpansion(0.f)
, mBeamAngle(0.f)
, mEnergyPulseStartY(0.f)
, mShutdownTimer(0.f)
, mContactPulseTimer(0.f)
, mEnergyPulseTimer(0.f)
, mPlayerEffectPulseTimer(0.f)
, mPlayerDamageDuration(0.f)
, mPlayerDamageTimer(0.f)
, mTexture(gpSimplePool->GetObj(SObjectTag('TXTR', beamInfo.GetTextureId())))
, mGlowTexture(gpSimplePool->GetObj(SObjectTag('TXTR', beamInfo.GetGlowTextureId())))
, mPulseFxDesc(gpSimplePool->GetObj(SObjectTag('PART', beamInfo.GetPulseFXId())))
, mContactFxDesc(beamInfo.GetContactFXId() != kInvalidAssetId
                     ? rstl::optional_object< TLockedToken< CGenDescription > >(
                           gpSimplePool->GetObj(SObjectTag('PART', beamInfo.GetContactFXId())))
                     : rstl::optional_object_null())
, mMuzzleFxDesc(beamInfo.GetMuzzleFXId() != kInvalidAssetId
                    ? rstl::optional_object< TLockedToken< CGenDescription > >(
                          gpSimplePool->GetObj(SObjectTag('PART', beamInfo.GetMuzzleFXId())))
                    : rstl::optional_object_null())
, mContactGen(mContactFxDesc ? rs_new CElementGen(*mContactFxDesc, CElementGen::kMOT_One) : nullptr)
, mPulseGen(rs_new CElementGen(mPulseFxDesc, CElementGen::kMOT_Normal))
, mWeaponGen()
, mMuzzleGen(mMuzzleFxDesc ? rs_new CElementGen(*mMuzzleFxDesc, CElementGen::kMOT_Normal) : nullptr)
, mMuzzleScale(1.f, 1.f, 1.f)
, mFreezeSteamTxtr(resources.GetAsset(0))
, mFreezeIceTxtr(resources.GetAsset(1))
, mVisorElectric(resources.GetAsset(2) != kInvalidAssetId
                     ? rstl::optional_object< TToken< CElectricDescription > >(
                           gpSimplePool->GetObj(SObjectTag('ELSC', resources.GetAsset(2))))
                     : rstl::optional_object_null())
, mVisorParticle(resources.GetAsset(3) != kInvalidAssetId
                     ? rstl::optional_object< TToken< CGenDescription > >(
                           gpSimplePool->GetObj(SObjectTag('PART', resources.GetAsset(3))))
                     : rstl::optional_object_null())
, mFreezeSfx(resources.GetAsset(4))
, mElectricSfx(resources.GetAsset(5))
, mSustainedDamagePlayerId(kInvalidUniqueId)
, x6a6_0_(false)
, mEnableEnergyPulse(true)
, mFiring(false)
, mTexturesLoaded(false)
, mDrawOwnerFirst(drawOwnerFirst)
, mInitialDamageEnabled(false)
, mInitialDamagePending(false) {
  mTexture.Lock();
  mGlowTexture.Lock();
  if (mContactGen.get()) {
    const float scale = beamInfo.GetContactFxScale();
    mContactGen->SetGlobalScale(CVector3f(scale, scale, scale));
    mContactGen->SetParticleEmission(false);
  }
  const float pulseScale = beamInfo.GetPulseFxScale();
  mPulseGen->SetGlobalScale(CVector3f(pulseScale, pulseScale, pulseScale));
  mPulseGen->SetParticleEmission(false);
  if (mMuzzleGen.get()) {
    mMuzzleGen->SetGlobalScale(CVector3f(pulseScale, pulseScale, pulseScale));
    mMuzzleGen->SetParticleEmission(false);
  }
}

float CPlasmaProjectile::UpdateBeamState(float dt, CStateManager& mgr) {
  switch (mExpansionState) {
  case kES_Attack:
    if (mExpansionT > 0.5f) {
      mExpansionState = kES_Sustain;
    } else {
      mExpansionT += dt * mExpansionSpeed;
    }
    break;
  case kES_Sustain:
    if (mBeamAttributes & 4) {
      if (mLifeTimer > mLifeTime) {
        mExpansionState = kES_Release;
      } else {
        mLifeTimer += dt;
      }
    }
    break;
  case kES_Release:
    mExpansionT += dt * mExpansionSpeed;
    if (mExpansionT > 1.f) {
      mExpansionT = 1.f;
      mExpansionState = kES_Done;
      mEnableEnergyPulse = false;
    }
    break;
  case kES_Done:
    mShutdownTimer += dt;
    if (mShutdownTimer > mShutdownTime &&
        (!mContactGen.get() || mContactGen->GetParticleCountAll() == 0)) {
      mExpansionState = kES_Inactive;
      ResetBeam(mgr, true);
    }
    break;
  default:
    break;
  }
  return -4.f * mExpansionT * (mExpansionT - 1.f);
}

void CPlasmaProjectile::MakeBillboardEffect(
    const rstl::optional_object< TToken< CGenDescription > >& particle,
    const rstl::optional_object< TToken< CElectricDescription > >& electric,
    const rstl::string& name, CStateManager& mgr, uint playerMask) {
  // TODO: create the HUD billboard effect with the affected player's visibility mask.
}

void CPlasmaProjectile::UpdatePlayerEffects(float dt, CStateManager& mgr) {
  // TODO: initial contact damage, per-player visor effects and sustained-damage ownership.
}

void CPlasmaProjectile::UpdateFx(const CTransform4f& xf, float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  mTexturesLoaded = mTexture.IsLoaded() && mGlowTexture.IsLoaded();
  CauseDamage(mExpansionState == kES_Attack || mExpansionState == kES_Sustain);
  CBeamProjectile::UpdateFx(xf, dt, mgr);
  UpdatePlayerEffects(dt, mgr);

  if (mBeamAttributes & 1) {
    rstl::reserved_vector< CVector3f, 8 >& cache = PointCache();
    for (int i = 7; i > 0; --i) {
      cache[i] = cache[i - 1];
    }
    cache[0] = GetCurrentPos();
  }
  // TODO: orient/update contact particles and the additional muzzle generator.
  const float expansion = UpdateBeamState(dt, mgr);
  UpdateEnergyPulse(dt);
  mBeamAngle += 720.f * dt;
  if (mBeamAngle > 360.f) {
    mBeamAngle = 0.f;
  }
  mBeamWidth = expansion * GetMaxRadius();
  mExpansion = expansion;
  mEnergyPulseStartY += dt * mPulseSpeed;
  if (mEnergyPulseStartY > 5.f) {
    mEnergyPulseStartY = 0.f;
  }
  UpdateLights(expansion, dt, mgr);
}

bool CPlasmaProjectile::CanRenderUnsorted(const CStateManager&) const { return false; }

void CPlasmaProjectile::AddToRenderer(const CStateManager& mgr) const {
  // TODO: enqueue optional contact/muzzle particles and the enabled pulse generator.
  EnsureRendered(mgr, GetBeamTransform().GetTranslation(), GetSortingBounds(mgr));
}

void CPlasmaProjectile::Render(const CStateManager& mgr) const {
  // TODO: draw the four independently enabled beam layers and motion blur.
}

void CPlasmaProjectile::Fire(const CTransform4f& xf, CStateManager& mgr, bool flag) {
  SetActive(true);
  SetLightsActive(true, mgr);
  mEnableEnergyPulse = true;
  mFiring = true;
  x6a6_0_ = flag;
  mExpansionState = kES_Attack;
  mInitialDamagePending = mInitialDamageEnabled;
  if (mBeamAttributes & 1) {
    rstl::reserved_vector< CVector3f, 8 >& cache = PointCache();
    for (int i = 0; i < cache.size(); ++i) {
      cache[i] = xf.GetTranslation();
    }
  }
}

void CPlasmaProjectile::ResetBeam(CStateManager& mgr, bool fullReset) {
  if (fullReset) {
    SetActive(false);
    SetLightsActive(false, mgr);
    mLifeTimer = 0.f;
    mExpansionT = 0.f;
    mBeamAngle = 0.f;
    mShutdownTimer = 0.f;
    mContactPulseTimer = 0.f;
    mEnergyPulseTimer = 0.f;
    mPlayerEffectPulseTimer = 0.f;
    mExpansionState = kES_Inactive;
  } else {
    mExpansionState = kES_Release;
  }
  mFiring = false;
  mPulseGen->SetParticleEmission(false);
  if (mContactGen.get()) {
    mContactGen->SetParticleEmission(false);
  }
  if (mMuzzleGen.get()) {
    mMuzzleGen->SetParticleEmission(false);
  }
}

void CPlasmaProjectile::RenderBeam(int subdivisions, float width, const CColor& color,
                                   int flags) const {
  // TODO: generate the radial beam strips using the selected texture and blend flags.
}

void CPlasmaProjectile::UpdateEnergyPulse(float dt) {
  // TODO: distribute pulse particles along the current collision-clipped beam length.
  mPulseGen->Update(dt);
}

void CPlasmaProjectile::RenderMotionBlur() const {
  // TODO: draw the cached beam endpoints with the fading outer color.
}

void CPlasmaProjectile::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  // TODO: register/unregister the weapon, create/delete lights and release player damage state.
  CGameProjectile::AcceptScriptMsg(mgr, msg);
}

void CPlasmaProjectile::SetLightsActive(bool active, CStateManager& mgr) {
  for (int i = 0; i < mLights.size(); ++i) {
    if (mLights[i] == kInvalidUniqueId) {
      continue;
    }
    if (CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(mLights[i]))) {
      light->SetActive(active);
    }
  }
}

void CPlasmaProjectile::CreatePlasmaLights(uint sourceId, const CLight& light, CStateManager& mgr) {
  // TODO: allocate/register three lights with this projectile as their parent.
}

void CPlasmaProjectile::DeletePlasmaLights(CStateManager& mgr) {
  // TODO: free each valid light through the state manager before clearing the list.
}

void CPlasmaProjectile::UpdateLights(float expansion, float dt, CStateManager& mgr) {
  // TODO: update the weapon light generator and distribute scaled lights along the beam.
}

void CPlasmaProjectile::SetInitialDamage(float damage) {
  mInitialDamage = damage;
  mInitialDamageEnabled = damage > 0.f;
}
