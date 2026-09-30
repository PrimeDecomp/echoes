#include "MetroidPrime/Weapons/CWeapon.hpp"

#include "MetroidPrime/CActorParameters.hpp"

CWeapon::CWeapon(TUniqueId uid, TAreaId areaId, bool active, TUniqueId owner, EWeaponType type,
                 const rstl::string& name, const CTransform4f& xf, const CMaterialFilter& filter,
                 const CMaterialList& materials, const CDamageInfo& damageInfo, int attribs,
                 const CModelData& modelData)
: CActor(uid, name, CEntityInfo(areaId, CEntity::mNullConnectionList, active), 0, xf, modelData,
         materials, CActorParameters(), kInvalidUniqueId)
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
    // TODO: reconstruct scaled damage with fresh hit metadata, not a copy of the original record.
  } else {
    mCurDamageInfo = mOrigDamageInfo;
  }
  CActor::Think(dt, mgr);
}

void CWeapon::FluidFXThink(EFluidState state, CScriptWater& water, CStateManager& mgr) {
  // TODO: weapon-dependent splash strength and fluid-plane manager effects.
}

void CWeapon::Render(const CStateManager& mgr) const {}

EWeaponCollisionResponseTypes CWeapon::GetCollisionResponseType(const CVector3f& position,
                                                                const CVector3f& direction,
                                                                const CWeaponMode& mode,
                                                                int attribs) const {
  return kWCR_Projectile;
}
