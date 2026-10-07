#include "MetroidPrime/Weapons/CWeapon.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CFluidPlaneManager.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"

#include "rstl/math.hpp"

CWeapon::CWeapon(TUniqueId uid, TAreaId areaId, bool active, TUniqueId owner, EWeaponType type,
                 const rstl::string& name, const CTransform4f& xf, const CMaterialFilter& filter,
                 const CMaterialList& materials, const CDamageInfo& damageInfo, int attribs,
                 const CModelData& modelData)
: CActor(uid, name, CEntityInfo(areaId, CEntity::NullConnectionList, active), 0, xf, modelData,
         materials, CActorParameters::None(), kInvalidUniqueId)
, mProjectileAttribs(attribs)
, mOwnerId(owner)
, mWeaponType(type)
, mFilter(filter)
, mOrigDamageInfo(damageInfo)
, mCurDamageInfo(damageInfo)
, mCurTime(0.f)
, mDamageFalloffSpeed(0.f)
, mDamageDuration(0.f)
, mInterferenceDuration(0.f) {
  // TODO: attribute 0x02000000 also sets the unresolved CActor flag x154_7_.
}

CWeapon::~CWeapon() {}

void CWeapon::SetDamageFalloffSpeed(float speed) {
  if (speed > 0.f) {
    mDamageFalloffSpeed = 1.f / speed;
  }
}

void CWeapon::Think(float dt, CStateManager& mgr) {
  mCurTime += dt;
  if (HasAttrib(kPA_DamageFalloff)) {
    const float scale = rstl::max_val(1.f - mCurTime * mDamageFalloffSpeed, 0.f);
    const float damage = scale * mOrigDamageInfo.GetDamage();
    const float radius = scale * mOrigDamageInfo.GetRadius();
    const float knockback = scale * mOrigDamageInfo.GetKnockBackPower();
    mCurDamageInfo = CDamageInfo(mOrigDamageInfo.GetWeaponMode(), damage, radius, knockback);
  } else {
    mCurDamageInfo = mOrigDamageInfo;
  }
  CActor::Think(dt, mgr);
}

void CWeapon::FluidFXThink(EFluidState state, CScriptWater& water, CStateManager& mgr) {
  bool doSplash = true;
  float magnitude = 0.f;
  switch (mWeaponType) {
  case kWT_Power:
  case kWT_Light:
  case kWT_Phazon:
    magnitude = 0.1f;
    break;
  case kWT_Dark:
    magnitude = 0.3f;
    break;
  case kWT_Annihilator:
    break;
  case kWT_Missile:
    magnitude = 0.5f;
    break;
  default:
    doSplash = false;
    break;
  }

  if (HasAttrib(kPA_ComboShot)) {
    if (state == kFS_InFluid) {
      doSplash = false;
    } else {
      magnitude += 0.5f;
    }
  }
  if (HasAttrib(kPA_Charged)) {
    magnitude += 0.25f;
  }
  magnitude = rstl::min_val(magnitude, 1.f);

  if (doSplash) {
    const CVector3f position(GetTranslation().GetX(), GetTranslation().GetY(),
                             water.GetTriggerBoundsWR().GetMaxPoint().GetZ());
    if (HasAttrib(kPA_ComboShot)) {
      doSplash = water.CanRippleAtPoint(position);
    } else if (state == kFS_InFluid) {
      doSplash = false;
    }

    if (doSplash) {
      const bool playSound = state == kFS_EnteredFluid || state == kFS_LeftFluid;
      mgr.GetFluidPlaneManager()->CreateSplash(GetUniqueId(), mgr, water, position, magnitude,
                                               playSound);
    }
  }
}

void CWeapon::Render(const CStateManager& mgr) const {}

EWeaponCollisionResponseTypes CWeapon::GetCollisionResponseType(const CVector3f& position,
                                                                const CVector3f& direction,
                                                                const CWeaponMode& mode,
                                                                int attribs) const {
  return kWCR_Projectile;
}
