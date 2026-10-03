#include "MetroidPrime/Weapons/CBouncingBomb.hpp"

#include "Kyoto/Particles/CElementGen.hpp"

void CBouncingBomb::Touch(CActor& actor, CStateManager& mgr) {}

void CBouncingBomb::Think(float dt, CStateManager& mgr) {}

void CBouncingBomb::Render(const CStateManager& mgr) const {}

void CBouncingBomb::AddToRenderer(const CStateManager& mgr) const {}

void CBouncingBomb::UpdateParticles(float dt) {}

rstl::optional_object< CAABox > CBouncingBomb::GetTouchBounds() const {}

void CBouncingBomb::UpdateExplosion(float dt, CStateManager& mgr) {}

void CBouncingBomb::Explode(CStateManager& mgr) {}

void CBouncingBomb::ApplyGravity() {}

void CBouncingBomb::HandleStaticCollision(CStateManager& mgr, const CRayCastResult& result) {}

CBouncingBomb::~CBouncingBomb() {}

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
, mDisableFuse(false) {}

CEntity* CBouncingBomb::TypesMatch(int typeId) const {}
