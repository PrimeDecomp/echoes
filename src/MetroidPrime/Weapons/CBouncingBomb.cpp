#include "MetroidPrime/Weapons/CBouncingBomb.hpp"

#include "Collision/CRayCastResult.hpp"
#include "Collision/CollisionUtil.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CSphere.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"

CBouncingBomb::CBouncingBomb(TToken< CGenDescription > particle,
                             TToken< CGenDescription > explosion, TUniqueId uid, TAreaId areaId,
                             TUniqueId ownerId, float fuseTime, float touchRadius, EWeaponType type,
                             uint attribs, const CTransform4f& xf, const CDamageInfo& damageInfo,
                             float renderRadius, ushort placementSfx, ushort bounceSfx,
                             ushort explosionSfx, float gravityScale, float bounceRestitution)
: CWeapon(uid, areaId, true, ownerId, type, rstl::string_l("Bomb"), xf,
          CMaterialFilter::MakeIncludeExclude(
              CMaterialList(kMT_Unknown59, kMT_Trigger, kMT_NonSolidDamageable),
              CMaterialList(kMT_Projectile, kMT_Bomb)),
          CMaterialList(kMT_Projectile, kMT_Bomb), damageInfo, static_cast< int >(attribs),
          CModelData::CModelDataNull())
, mParticle(rs_new CElementGen(particle))
, mExplosionParticle(rs_new CElementGen(explosion))
, mVelocity(CVector3f::Zero())
, mAcceleration(CVector3f::Zero())
, mPrevLocation(xf.GetTranslation())
, mPlacementSfx(placementSfx)
, mBounceSfx(bounceSfx)
, mExplosionSfx(explosionSfx)
, mFuseTime(fuseTime)
, mTouchRadius(touchRadius)
, mRenderRadius(renderRadius)
, mGravityScale(gravityScale)
, mBounceRestitution(bounceRestitution)
, mExplosionElapsed(0.f)
, mBounceCount(0)
, mIsNotDetonated(true)
, mDisableFuse(false) {
  mParticle->SetGlobalTranslation(xf.GetTranslation());
  mExplosionParticle->SetGlobalTranslation(xf.GetTranslation());
  if (mFuseTime < 0.f) {
    mDisableFuse = true;
  }
}

CBouncingBomb::~CBouncingBomb() {}

void CBouncingBomb::HandleStaticCollision(CStateManager& mgr, const CRayCastResult& result) {
  if (result.GetMaterial().HasMaterial(kMT_Wall) || result.GetMaterial().HasMaterial(kMT_Ceiling)) {
    Explode(mgr);
    mBounceCount = 4;
    mVelocity = CVector3f::Zero();
    return;
  }

  if (mBounceCount < 4 && !(mVelocity.GetZ() > 0.f)) {
    ++mBounceCount;
    if (mBounceSfx != CSfxManager::kInternalInvalidSfxId) {
      CSfxManager::AddEmitter(mBounceSfx, GetTranslation(), GetCurrentAreaId().Value(), true, false,
                              CSfxManager::kMedPriority);
    }
    if (mBounceCount >= 4) {
      Explode(mgr);
      mVelocity = CVector3f::Zero();
      return;
    }

    ApplyGravity();
    const float speed = mVelocity.Magnitude();
    mVelocity.SetZ(CMath::AbsF(mVelocity.GetZ()));
    if (mVelocity.CanBeNormalized()) {
      mVelocity.Normalize();
      CVector3f horizontal(mVelocity.GetX(), mVelocity.GetY(), 0.f);
      if (horizontal.Magnitude() > mVelocity.GetZ()) {
        horizontal.Normalize();
        horizontal *= 0.5f;
        mVelocity.SetX(horizontal.GetX());
        mVelocity.SetY(horizontal.GetY());
        mVelocity.Normalize();
      }
      mVelocity *= speed * mBounceRestitution;
    }
  }
}

void CBouncingBomb::ApplyGravity() {
  mAcceleration = mGravityScale * (24.525f * CVector3f::Down());
}

void CBouncingBomb::Explode(CStateManager& mgr) {
  mIsNotDetonated = false;
  if (mExplosionSfx != CSfxManager::kInternalInvalidSfxId) {
    CSfxManager::AddEmitter(mExplosionSfx, GetTranslation(), GetCurrentAreaId().Value(), true,
                            false, CSfxManager::kMedPriority);
  }
  mExplosionElapsed = 0.f;
}

void CBouncingBomb::UpdateExplosion(float dt, CStateManager& mgr) {
  if (mExplosionElapsed >= 0.6f) {
    if (mExplosionParticle->IsSystemDeletable()) {
      mgr.DeleteObjectRequest(GetUniqueId());
    }
  } else {
    mExplosionElapsed += dt;
    if (mExplosionElapsed >= 0.6f) {
      mgr.ApplyDamageToWorld(GetOwnerId(), *this, GetTranslation(), mCurDamageInfo, GetFilter());
    }
  }
}

rstl::optional_object< CAABox > CBouncingBomb::GetTouchBounds() const {
  const float radius = mIsNotDetonated ? mTouchRadius : mCurDamageInfo.GetRadius();
  const CVector3f& position = GetTranslation();
  return rstl::optional_object< CAABox >(CAABox(
      (position.GetX() < mPrevLocation.GetX() ? position.GetX() : mPrevLocation.GetX()) - radius,
      (position.GetY() < mPrevLocation.GetY() ? position.GetY() : mPrevLocation.GetY()) - radius,
      (position.GetZ() < mPrevLocation.GetZ() ? position.GetZ() : mPrevLocation.GetZ()) - radius,
      (mPrevLocation.GetX() < position.GetX() ? position.GetX() : mPrevLocation.GetX()) + radius,
      (mPrevLocation.GetY() < position.GetY() ? position.GetY() : mPrevLocation.GetY()) + radius,
      (mPrevLocation.GetZ() < position.GetZ() ? position.GetZ() : mPrevLocation.GetZ()) + radius));
}

void CBouncingBomb::UpdateParticles(float dt) { mExplosionParticle->Update(dt); }

void CBouncingBomb::AddToRenderer(const CStateManager& mgr) const {
  const CVector3f origin = GetTranslation();
  const CVector3f extent(mRenderRadius, mRenderRadius, mRenderRadius);
  const CAABox bounds(origin - extent, origin + extent);
  const CVector3f closestPoint =
      bounds.ClosestPointAlongVector(CGraphics::GetViewMatrix().GetForward());

  if (mIsNotDetonated) {
    gpRender->AddParticleGen(*mParticle, closestPoint, bounds);
    if (mFuseTime < 0.015f) {
      gpRender->AddParticleGen(*mExplosionParticle, closestPoint, bounds);
    }
  } else {
    gpRender->AddParticleGen(*mExplosionParticle, closestPoint, bounds);
  }
}

void CBouncingBomb::Render(const CStateManager& mgr) const {}

void CBouncingBomb::Think(float dt, CStateManager& mgr) {
  CWeapon::Think(dt, mgr);

  if (mIsNotDetonated) {
    mParticle->Update(dt);
    if (!mDisableFuse) {
      if (mFuseTime <= 0.f) {
        Explode(mgr);
      }
      if (mFuseTime < 0.015f) {
        UpdateParticles(dt);
      }
      mFuseTime -= dt;
    }
    if (mPlacementSfx != CSfxManager::kInternalInvalidSfxId) {
      CSfxManager::AddEmitter(mPlacementSfx, GetTranslation(), GetCurrentAreaId().Value(), true,
                              false, CSfxManager::kMedPriority);
      mPlacementSfx = CSfxManager::kInternalInvalidSfxId;
    }
  } else {
    UpdateParticles(dt);
    UpdateExplosion(dt, mgr);
  }

  if (mIsNotDetonated) {
    if (mAcceleration.MagSquared() > 0.f) {
      mVelocity += dt * mAcceleration;
    }
    if (mVelocity.MagSquared() > 0.f) {
      mPrevLocation = GetTransform().GetTranslation();
      SetTranslation(GetTranslation() + dt * mVelocity);
      const CVector3f delta = GetTransform().GetTranslation() - mPrevLocation;
      const float distance = delta.Magnitude();
      if (close_enough(distance, 0.f)) {
        Explode(mgr);
      } else {
        static const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
            CMaterialList(kMT_Unknown59),
            CMaterialList(kMT_Character, kMT_Player, kMT_NoPlatformCollision));
        const CRayCastResult result =
            mgr.RayStaticIntersection(mPrevLocation, (1.f / distance) * delta, distance, filter);
        if (result.IsValid()) {
          HandleStaticCollision(mgr, result);
        } else {
          CVector3f toPlayer = mgr.GetPlayer(0)->GetTranslation() - GetTranslation();
          toPlayer.SetZ(0.f);
          if (toPlayer.MagSquared() < 17.64f) {
            Explode(mgr);
          }
        }
      }
    }
  }

  mParticle->SetGlobalTranslation(GetTranslation());
  mExplosionParticle->SetGlobalTranslation(GetTranslation());
}

void CBouncingBomb::Touch(CActor& actor, CStateManager& mgr) {
  if (mIsNotDetonated && actor.GetUniqueId() != GetOwnerId() &&
      actor.GetMaterialList().HasMaterial(kMT_NoStepLogic)) {
    if (CollisionUtil::AABoxSphereIntersection(*actor.GetTouchBounds(),
                                               CSphere(GetTranslation(), mTouchRadius))) {
      mFuseTime = -1.f;
      mDisableFuse = false;
    }
  }
}
