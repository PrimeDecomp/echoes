#include "MetroidPrime/Weapons/CBeamProjectile.hpp"

void fn_80049ED8(CActor*, CStateManager&);

CBeamProjectile::CBeamProjectile(const TToken< CWeaponDescription >& description,
                                 const rstl::string& name, EWeaponType type, const CTransform4f& xf,
                                 float maxLength, float beamRadius, float travelSpeed,
                                 EMaterialTypes material, const CDamageInfo& damage, TUniqueId uid,
                                 TAreaId areaId, TUniqueId owner, uint attribs, bool growingBeam)
: CGameProjectile(false, description, name, type, xf, material, damage, uid, areaId, owner,
                  kInvalidUniqueId, attribs, false, CVector3f(1.f, 1.f, 1.f), CImpactVisorEffect())
, mMaxLength(maxLength)
, mInvMaxLength(1.f / mMaxLength)
, mBeamRadius(beamRadius)
, mDamageType(kDT_None)
, x428_(kInvalidUniqueId)
, mCollisionActorId(kInvalidUniqueId)
, mGrowingBeamLength(growingBeam ? 0.f : mMaxLength)
, mBeamLength(mMaxLength)
, mTravelSpeed(travelSpeed)
, mCollisionNormal(CVector3f::Up())
, mCollisionPoint(CVector3f::Zero())
, mXf(CTransform4f::Identity())
, mLocalBounds(CAABox::Identity())
, mWorldBounds(CAABox::Identity())
, x4b0_(CVector3f::Zero())
, mPointCache(CVector3f::Zero())
, mGrowingBeam(growingBeam)
, mEnableTouchDamage(false) {}

rstl::optional_object< CAABox > CBeamProjectile::GetTouchBounds() const {
  if (!GetActive() || !mEnableTouchDamage) {
    return rstl::optional_object_null();
  }
  const CVector3f allowance(0.1f, 0.1f, 0.1f);
  return CAABox(GetTranslation() - allowance, GetTranslation() + allowance);
}

void CBeamProjectile::PreRenderAllViewports(CStateManager& mgr) {
  const CAABox bounds = mLocalBounds.GetTransformedAABox(mXf);
  SetOtherBounds(bounds);
  SetRenderBounds(bounds);
  fn_80049ED8(this, mgr);
}

void CBeamProjectile::ResetBeam(CStateManager&, bool) {
  if (mGrowingBeam) {
    mGrowingBeamLength = 0.f;
  }
}

void CBeamProjectile::SetCollisionResultData(EDamageType type, CRayCastResult& result,
                                             TUniqueId id) {
  mDamageType = type;
  mBeamLength = result.GetTime();
  mCollisionPoint = result.GetPoint();
  mCollisionNormal = result.GetPlane().GetNormal();
  mCollisionActorId = type == kDT_Actor ? id : kInvalidUniqueId;
  SetTranslation(result.GetPoint());
}

void CBeamProjectile::UpdateFx(const CTransform4f& xf, float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  SetTransformAlt(xf.GetRotation());
  if (mGrowingBeam) {
    mGrowingBeamLength += mTravelSpeed * dt;
    if (mGrowingBeamLength > mMaxLength) {
      mGrowingBeamLength = mMaxLength;
    }
  }
  mBeamLength = mGrowingBeamLength;
  mDamageType = kDT_None;
  mPreviousPos = xf.GetTranslation();
  SetTranslation(mPreviousPos + mGrowingBeamLength * xf.GetForward().AsNormalized());
  mLocalBounds = CAABox(-mBeamRadius, 0.f, -mBeamRadius, mBeamRadius, mBeamLength, mBeamRadius);
  mWorldBounds = mLocalBounds.GetTransformedAABox(xf);

  // TODO: build the near list, raycast, clip the beam and apply actor/world damage.
  mXf = xf;
}

void CBeamProjectile::SetMaxLength(float length) {
  mMaxLength = length;
  mInvMaxLength = 1.f / mMaxLength;
  if (!mGrowingBeam) {
    mGrowingBeamLength = mMaxLength;
  }
}
