#include "MetroidPrime/Weapons/CBeamProjectile.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/TCastTo.hpp"

CBeamProjectile::CBeamProjectile(const TToken< CWeaponDescription >& description,
                                 const rstl::string& name, EWeaponType type, const CTransform4f& xf,
                                 float maxLength, float beamRadius, float travelSpeed,
                                 EMaterialTypes material, const CDamageInfo& damage, TUniqueId uid,
                                 TAreaId areaId, TUniqueId owner, uint attribs, bool growingBeam)
: CGameProjectile(false, description, name, type, xf, material, damage, uid, areaId, owner,
                  kInvalidUniqueId, attribs, false, CVector3f(1.f, 1.f, 1.f),
                  CImpactVisorEffect::None())
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
  const float allowance = 0.1f;
  const CVector3f& position = GetTranslation();
  return CAABox(position.GetX() - allowance, position.GetY() - allowance,
                position.GetZ() - allowance, position.GetX() + allowance,
                position.GetY() + allowance, position.GetZ() + allowance);
}

void CBeamProjectile::PreRenderAllViewports(CStateManager& mgr) {
  const CAABox& bounds = mLocalBounds.GetTransformedAABox(mXf);
  SetOtherBounds(bounds);
  SetRenderBounds(bounds);
  UpdatePortalSystemState(mgr);
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
  SetTransform(xf.GetRotation());
  if (mGrowingBeam) {
    mGrowingBeamLength += mTravelSpeed * dt;
    if (mGrowingBeamLength > mMaxLength) {
      mGrowingBeamLength = mMaxLength;
    }
  }
  mBeamLength = mGrowingBeamLength;
  mDamageType = kDT_None;
  const CVector3f origin = xf.GetTranslation();
  const CVector3f beamEnd =
      xf.GetTranslation() + mGrowingBeamLength * xf.GetForward().AsNormalized();
  mPreviousPos = origin;
  SetTranslation(beamEnd);

  mLocalBounds = CAABox(-mBeamRadius, 0.f, -mBeamRadius, mBeamRadius, mBeamLength, mBeamRadius);
  mWorldBounds = CAABox(CVector3f(-mBeamRadius, 0.f, -mBeamRadius),
                        CVector3f(mBeamRadius, mGrowingBeamLength, mBeamRadius))
                     .GetTransformedAABox(xf);

  TUniqueId collideId = kInvalidUniqueId;
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  mgr.BuildNearList(nearList, mWorldBounds,
                    CMaterialFilter::MakeExclude(CMaterialList(kMT_ProjectilePassthrough)), this);

  CRayCastResult res = RayCollisionCheckWithWorld(collideId, origin, beamEnd, mGrowingBeamLength,
                                                  nearList, mgr, kSGT_CollisionGeometry);

  if (TCastToPtr< CActor >(mgr.ObjectById(collideId))) {
    SetCollisionResultData(kDT_Actor, res, collideId);
    if (mEnableTouchDamage) {
      ApplyDamageToActors(mgr, CDamageInfo(mCurDamageInfo, dt));
    }
  } else if (res.IsValid()) {
    SetCollisionResultData(kDT_World, res, kInvalidUniqueId);
    if (mEnableTouchDamage) {
      mgr.ApplyDamageToWorld(GetOwnerId(), *this, res.GetPoint(), CDamageInfo(mCurDamageInfo, dt),
                             GetFilter());
    }
  } else {
    mCollisionPoint = xf * CVector3f(mBeamRadius, mBeamLength, mBeamRadius);
    SetTranslation(mCollisionPoint);
  }
  mXf = xf;
}

void CBeamProjectile::SetMaxLength(float length) {
  mMaxLength = length;
  mInvMaxLength = 1.f / mMaxLength;
  if (!mGrowingBeam) {
    mGrowingBeamLength = mMaxLength;
  }
}
