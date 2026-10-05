#include "MetroidPrime/Weapons/CElectricBeamProjectile.hpp"

#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CParticleElectric.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "rstl/math.hpp"

CElectricBeamProjectile::CElectricBeamProjectile(const TToken< CWeaponDescription >& description,
                                                 EWeaponType type,
                                                 const CElectricBeamInfo& beamInfo,
                                                 const CTransform4f& xf, EMaterialTypes material,
                                                 const CDamageInfo& damage, TUniqueId uid,
                                                 TAreaId areaId, TUniqueId owner, uint attribs)
: CBeamProjectile(description, rstl::string_l("ElectricBeamProjectile"), type, xf,
                  beamInfo.GetLength(), beamInfo.GetRadius(), beamInfo.GetTravelSpeed(), material,
                  damage, uid, areaId, owner, attribs, false)
, mElectric(rs_new CParticleElectric(beamInfo.GetElectricDescription()))
, mGenDescription(beamInfo.GetParticleId() != kInvalidAssetId
                      ? rstl::optional_object< TLockedToken< CGenDescription > >(
                            gpSimplePool->GetObj(SObjectTag('PART', beamInfo.GetParticleId())))
                      : rstl::optional_object_null())
, mElementGen(mGenDescription ? rs_new CElementGen(*mGenDescription, CElementGen::kMOT_Normal,
                                                   CElementGen::kOSF_One)
                              : nullptr)
, mFadeSpeed(beamInfo.GetFadeSpeed())
, mDamageTimer(0.f)
, mDamageInterval(beamInfo.GetDamageInterval())
, mFiring(false) {
  if (mElementGen.get()) {
    mElementGen->SetParticleEmission(false);
  }
  mElectric->SetParticleEmission(false);
}

void CElectricBeamProjectile::UpdateFx(const CTransform4f& xf, float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  if (mDamageTimer <= 0.f) {
    CauseDamage(true);
  }

  if (GetDamageType() == kDT_Actor) {
    mDamageTimer = mDamageInterval;
    CauseDamage(false);
  }

  mDamageTimer -= dt;
  if (!close_enough(mFadeSpeed, 0.f)) {
    const float direction = mFiring ? 1.f : -1.f;
    mIntensity = rstl::min_val(1.f, dt * (direction / mFadeSpeed) + mIntensity);
    if (mIntensity < 0.f) {
      ResetBeam(mgr, true);
    }
  } else {
    mIntensity = 1.f;
  }

  CBeamProjectile::UpdateFx(xf, dt, mgr);

  if (mElementGen.get()) {
    mElementGen->SetModulationColor(CColor::Lerp(CColor::Black(), CColor::White(), mIntensity));
    const bool hasDamage = GetDamageType() != kDT_None;
    if (hasDamage) {
      mElementGen->SetGlobalOrientation(
          CTransform4f::LookAt(CVector3f::Zero(), GetSurfaceNormal(), CVector3f::Up()));
      mElementGen->SetGlobalTranslation(GetCurrentPos() + 0.001f * GetSurfaceNormal());
    }
    mElementGen->SetParticleEmission(hasDamage);
    mElementGen->Update(dt);
  }

  mElectric->SetModulationColor(CColor::Lerp(CColor::Black(), CColor::White(), mIntensity));
  mElectric->SetParticleEmission(true);
  CVector3f dir = GetCurrentPos() - GetBeamTransform().GetTranslation();
  if (dir.CanBeNormalized()) {
    dir.Normalize();
  }
  mElectric->SetOverrideIPos(GetBeamTransform().GetTranslation());
  mElectric->SetOverrideIVel(dir);
  mElectric->SetOverrideFPos(GetCurrentPos());
  mElectric->SetOverrideFVel(-dir);
  mElectric->Update(dt);
}

void CElectricBeamProjectile::AddToRenderer(const CStateManager& mgr) const {
  if (GetActive()) {
    if (mElementGen.get()) {
      gpRender->AddParticleGen(*mElementGen);
    }
    gpRender->AddParticleGen(*mElectric);
  }
}

void CElectricBeamProjectile::Fire(const CTransform4f& xf, CStateManager& mgr, bool flag) {
  mFiring = true;
  SetActive(true);
  mIntensity = 0.f;
}

void CElectricBeamProjectile::ResetBeam(CStateManager& mgr, bool fullReset) {
  if (fullReset) {
    mIntensity = 0.f;
    SetActive(false);
    if (mElementGen.get()) {
      mElementGen->SetParticleEmission(false);
    }
    mElectric->SetParticleEmission(false);
    CBeamProjectile::ResetBeam(mgr, true);
  } else {
    mFiring = false;
  }
}

void CElectricBeamProjectile::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Create:
    mgr.AddWeaponId(GetOwnerId(), GetType());
    CauseDamage(true);
    break;
  case kSM_Delete:
    DeleteProjectileLight(mgr);
    break;
  default:
    break;
  }
  CGameProjectile::AcceptScriptMsg(mgr, msg);
}
