#include "MetroidPrime/Enemies/CSporbNeedle.hpp"

#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Enemies/CSporbPowerBomb.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSporbNeedle.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "REL/REL_Setup.h"

static EMaterialTypes skDamageSolid = kMT_Solid;            // Guessed name
static EMaterialTypes skSolid = kMT_Solid;                  // Guessed name
static EMaterialTypes skIncludeSolid = kMT_Solid;           // Guessed name
static EMaterialTypes skIncludePlayer = kMT_Player;         // Guessed name
static EMaterialTypes skExcludeProjectile = kMT_Projectile; // Guessed name
static EMaterialTypes skExcludeCharacter = kMT_Character;   // Guessed name
static EMaterialTypes skCollideSolid = kMT_Solid;           // Guessed name
static EMaterialTypes skCollideCeiling = kMT_Ceiling;       // Guessed name
static EMaterialTypes skCollideWall = kMT_Wall;             // Guessed name
static EMaterialTypes skCollideFloor = kMT_Floor;           // Guessed name
static EMaterialTypes skNearCharacter = kMT_Character;      // Guessed name
static EMaterialTypes skNearPlayer = kMT_Player;            // Guessed name

CSporbPowerBomb::CSporbPowerBomb(bool active, const TToken< CWeaponDescription >& description,
                                 EWeaponType type, const CTransform4f& xf,
                                 EMaterialTypes excludeMaterial, const CDamageInfo& damage,
                                 TUniqueId uid, TAreaId areaId, TUniqueId owner,
                                 TUniqueId homingTarget, uint attribs, bool underwater,
                                 const CVector3f& scale, const CImpactVisorEffect& visorEffect,
                                 bool unused, bool playImpactSound, float fuseTime,
                                 float startDamageTime, float endDamageTime, float damageWaitTime)
: CEnergyProjectile(active, description, type, xf, excludeMaterial, damage, uid, areaId, owner,
                    homingTarget, attribs, underwater, scale, visorEffect, unused, playImpactSound,
                    false, 1.f, 4.f, 4.f)
, mFuseTime(fuseTime)
, mFuseTimer(0.f)
, mElapsedTime(0.f)
, mCurrentRadius(0.f)
, mRadiusGrowthRate(damage.GetRadius() / (endDamageTime - startDamageTime))
, mStartDamageTime(startDamageTime)
, mEndDamageTime(endDamageTime)
, mExplosionNormal(CVector3f::Up())
, mState(kS_Invalid)
, mDamageWaitTime(damageWaitTime)
, mDamageWaitTimer(0.f) {}

void CSporbPowerBomb::Think(float dt, CStateManager& mgr) {
  mDamageWaitTimer = rstl::max_val(0.f, mDamageWaitTimer - dt);
  if ((mState == kS_HitActor || mState == kS_HitWorld) && mFuseTimer >= mFuseTime) {
    if (mElapsedTime >= mStartDamageTime && mElapsedTime <= mEndDamageTime) {
      SetExplodePending(true);
      mOrigDamageInfo.SetRadius(mCurrentRadius);
      const float negativeRadius = -mCurrentRadius;
      CMaterialFilter filter = mFilter;
      filter.ExcludeList().Remove(kMT_Character);
      const CAABox bounds(
          GetTranslation() + CVector3f(negativeRadius, negativeRadius, negativeRadius),
          GetTranslation() + CVector3f(mCurrentRadius, mCurrentRadius, mCurrentRadius));
      rstl::reserved_vector< TUniqueId, 1024 > nearList;
      mgr.BuildNearList(nearList, bounds, filter, this);
      for (rstl::reserved_vector< TUniqueId, 1024 >::iterator it = nearList.begin();
           it != nearList.end(); ++it) {
        const TUniqueId id = *it;
        const CVector3f center = GetTranslation();
        if (CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(id))) {
          if ((player->GetTranslation() - center).MagSquared() < mCurrentRadius * mCurrentRadius) {
            if (mDamageWaitTimer <= 0.f) {
              if (mgr.TestRayDamage(center, *player, nearList)) {
                mgr.ApplyRadiusDamage(*this, center, *player, GetUniqueId(), mOrigDamageInfo);
                mDamageWaitTimer = mDamageWaitTime;
              }
            } else if (mgr.TestRayDamage(center, *player, nearList)) {
              CDamageInfo info = mOrigDamageInfo;
              info.SetDamage(0.f);
              info.SetRadiusDamage(0.f);
              mgr.ApplyRadiusDamage(*this, center, *player, GetUniqueId(), info);
            }
          }
        } else if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(id))) {
          if ((actor->GetTranslation() - center).MagSquared() < mCurrentRadius * mCurrentRadius) {
            if (mgr.TestRayDamage(center, *actor, nearList)) {
              mgr.ApplyRadiusDamage(*this, center, *actor, GetUniqueId(), mOrigDamageInfo);
            }
          }
        }
      }
      mCurrentRadius += mRadiusGrowthRate * dt;
    }
    mElapsedTime += dt;
  } else if (mState == kS_HitWorld) {
    mFuseTimer += dt;
  }
  CEnergyProjectile::Think(dt, mgr);
}

void CSporbPowerBomb::ResolveCollisionWithWorld(const CRayCastResult& result, CStateManager& mgr) {
  mState = kS_HitWorld;
  mExplosionNormal = result.GetPlane().GetNormal();
  mProjectile.SetVelocity(CVector3f::Zero());
  mProjectile.SetGravity(CVector3f::Zero());
}

void CSporbPowerBomb::ResolveCollisionWithActor(const CRayCastResult& result, CActor& actor,
                                                CStateManager& mgr) {
  mState = kS_HitActor;
  mFuseTimer = mFuseTime;
  mExplosionNormal = result.GetPlane().GetNormal();
  mProjectile.SetVelocity(CVector3f::Zero());
  mProjectile.SetGravity(CVector3f::Zero());
}

CRayCastResult
CSporbPowerBomb::RayCollisionCheckWithWorld(TUniqueId& idOut, const CVector3f& start,
                                            const CVector3f& end, float magnitude,
                                            rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                                            CStateManager& mgr, EStaticGeometryTest staticTest) {
  return CGameProjectile::RayCollisionCheckWithWorld(idOut, start, end, magnitude, nearList, mgr,
                                                     kSGT_CollisionGeometry);
}

void CSporbPowerBomb::Render(const CStateManager& mgr) const { CEnergyProjectile::Render(mgr); }

CSporbPowerBomb::~CSporbPowerBomb() {}

static CElementGen* CreateElementGen(CAssetId id) {
  if (id != kInvalidAssetId) {
    TLockedToken< CGenDescription > desc = gpSimplePool->GetObj(SObjectTag('PART', id));
    return rs_new CElementGen(desc, CElementGen::kMOT_Normal, CElementGen::kOSF_One);
  }
  return nullptr;
}

CSporbNeedle::CSporbNeedle(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                           const CTransform4f& xf, const CModelData& modelData,
                           const CActorParameters& actorParams, TUniqueId ownerId,
                           CAssetId explosionEffect, CAssetId trailEffect, ushort launchSound,
                           ushort flightSound, ushort hitPlayerSound, ushort collisionSound,
                           ushort explosionSound, const CDamageInfo& attackDamage,
                           float initialSpeed, float mass, float fuseTime)
: CPhysicsActor(uid, name, info, 0, xf, modelData, CMaterialList(skSolid),
                !modelData.IsNull()
                    ? modelData.GetBounds()
                    : CAABox(CVector3f(-0.5f, -0.5f, -0.5f), CVector3f(0.5f, 0.5f, 0.5f)),
                SMoverData(mass), actorParams, skDefaultStepData)
, mOwnerId(ownerId)
, mFuseTime(fuseTime)
, mFuseTimer(0.f)
, mExplosionGen(CreateElementGen(explosionEffect))
, mTrailGen(CreateElementGen(trailEffect))
, mLaunchSound(launchSound)
, mFlightSound(flightSound)
, mHitPlayerSound(hitPlayerSound)
, mCollisionSound(collisionSound)
, mExplosionSound(explosionSound)
, mAttackDamage(attackDamage)
, mThinkCount(0)
, mSphere(CSphere(CVector3f::Zero(), 0.2f), GetMaterialList()) {
  mExploded = false;
  mHitWorld = false;
  mLaunchSoundPlayed = false;
  SetVelocityWR(initialSpeed * xf.GetForward());
  SetMomentumWR(CVector3f::Zero());
  mExplosionGen->SetParticleEmission(false);
  if (!mTrailGen.null()) {
    mTrailGen->SetParticleEmission(true);
  }
  const CMaterialList include(skIncludeSolid, skIncludePlayer);
  const CMaterialList exclude(skExcludeProjectile, skExcludeCharacter);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include, exclude));
}

CSporbNeedle::~CSporbNeedle() {}

void CSporbNeedle::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Create:
    break;
  case kSM_AreaLoaded:
    AddMaterial(kMT_Projectile, mgr);
    break;
  default:
    break;
  }
  CActor::AcceptScriptMsg(mgr, msg);
}

void CSporbNeedle::PreThink(float dt, CStateManager& mgr) { CEntity::PreThink(dt, mgr); }

void CSporbNeedle::Think(float dt, CStateManager& mgr) {
  CActor::Think(dt, mgr);
  if (!mLaunchSoundPlayed) {
    PlaySfx(mLaunchSound, mgr);
    mLaunchSoundPlayed = true;
  }
  if (mThinkCount != 0) {
    PlayLoopedSfx(mFlightSound, mgr);
  } else {
    ++mThinkCount;
  }
  UpdateEffects(dt, mgr);
  if (mExplosionGen->IsSystemDeletable()) {
    mgr.DeleteObjectRequest(GetUniqueId());
  }
  if (mHitWorld) {
    Stop();
  }
}

void CSporbNeedle::Touch(CActor& actor, CStateManager& mgr) { CActor::Touch(actor, mgr); }

void CSporbNeedle::CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                                CStateManager& mgr) {
  static const CMaterialList skSolidTypes(skCollideSolid, skCollideCeiling, skCollideWall,
                                          skCollideFloor);
  bool hitOther = false;
  if (id != mOwnerId) {
    const CEntity* entity = mgr.GetObjectById(id);
    const CSporbNeedle* needle = TCastToConstPtr< CSporbNeedle >(entity);
    if (entity != nullptr && needle == nullptr) {
      hitOther = true;
    }
  }
  if (hitOther) {
    Explode(mgr, id);
    StopLoopedSounds();
    PlaySfx(mHitPlayerSound, mgr);
  } else {
    for (int i = 0; i < list.GetCount(); ++i) {
      if (list[i].GetMaterialLeft().SharesMaterials(skSolidTypes)) {
        mHitWorld = true;
        Stop();
        StopLoopedSounds();
        PlaySfx(mCollisionSound, mgr);
        break;
      }
    }
  }
  CPhysicsActor::CollidedWith(id, list, mgr);
}

void CSporbNeedle::Render(const CStateManager& mgr) const {
  if (!mExploded && HasModelData()) {
    GetModelData()->Render(mgr, GetTransform(), nullptr, CModelFlags(CModelFlags::kT_Opaque, 1.f));
  }
}

void CSporbNeedle::AddToRenderer(const CStateManager& mgr) const {
  CActor::AddToRenderer(mgr);
  if (mExploded) {
    if (!mExplosionGen.null()) {
      gpRender->AddParticleGen(*mExplosionGen);
    }
  } else if (!mTrailGen.null()) {
    gpRender->AddParticleGen(*mTrailGen);
  }
}

void CSporbNeedle::UpdateEffects(float dt, CStateManager& mgr) {
  if (GetActive()) {
    const CTransform4f orientation = GetTransform().GetRotation();
    const CVector3f translation = GetTranslation();
    const CVector3f scale = HasModelData() ? GetModelData()->GetScale() : CVector3f(1.f, 1.f, 1.f);
    if (mExploded) {
      Stop();
      mExplosionGen->SetOrientation(orientation);
      mExplosionGen->SetGlobalTranslation(translation);
      mExplosionGen->SetGlobalScale(scale);
      mExplosionGen->Update(dt);
    } else if (!mTrailGen.null()) {
      mTrailGen->SetOrientation(orientation);
      mTrailGen->SetTranslation(translation);
      mTrailGen->SetGlobalScale(scale);
      mTrailGen->Update(dt);
    }
    UpdateFuse(dt, mgr);
  }
}

void CSporbNeedle::Explode(CStateManager& mgr, TUniqueId hitId) {
  if (mExploded) {
    return;
  }
  RemoveMaterial(kMT_Solid, mgr);
  mExploded = true;
  CSfxManager::AddEmitter(mExplosionSound, GetTranslation(), GetCurrentAreaId().Value(), true,
                          false, CSfxManager::kMedPriority);
  mExplosionGen->SetParticleEmission(true);
  if (!mTrailGen.null()) {
    mTrailGen->SetParticleEmission(false);
  }

  bool isOwner = hitId == mOwnerId;
  if (const CCollisionActor* actor = TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(hitId))) {
    isOwner = actor->GetOwnerId() == mOwnerId;
  }
  if (hitId != kInvalidUniqueId && !isOwner) {
    mgr.ApplyDamage(
        GetUniqueId(), hitId, GetUniqueId(), mAttackDamage,
        CMaterialFilter::MakeIncludeExclude(CMaterialList(skDamageSolid), CMaterialList()),
        CVector3f::Zero());
  }

  if (mAttackDamage.GetRadius() > 0.f && hitId == kInvalidUniqueId) {
    const CVector3f pos = GetTranslation();
    const CAABox bounds(pos - CVector3f(mAttackDamage.GetRadius(), mAttackDamage.GetRadius(),
                                        mAttackDamage.GetRadius()),
                        pos + CVector3f(mAttackDamage.GetRadius(), mAttackDamage.GetRadius(),
                                        mAttackDamage.GetRadius()));
    const CMaterialFilter filter =
        CMaterialFilter::MakeInclude(CMaterialList(skNearCharacter, skNearPlayer));
    rstl::reserved_vector< TUniqueId, 1024 > nearList;
    mgr.BuildNearList(nearList, bounds, filter, nullptr);
    for (rstl::reserved_vector< TUniqueId, 1024 >::const_iterator it = nearList.begin();
         it != nearList.end(); ++it) {
      bool isNearOwner = *it == mOwnerId;
      if (const CCollisionActor* actor =
              TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(*it))) {
        isNearOwner = actor->GetOwnerId() == mOwnerId;
      }
      if (isNearOwner) {
        continue;
      }
      if (const CActor* actor = static_cast< CActor* >(mgr.ObjectById(*it))) {
        const CVector3f delta = actor->GetTranslation() - GetTranslation();
        const float distance = delta.Magnitude();
        if (distance < mAttackDamage.GetRadius()) {
          const float scale = (mAttackDamage.GetRadius() - distance) / mAttackDamage.GetRadius();
          const CDamageInfo info(mAttackDamage.GetWeaponMode(), scale * mAttackDamage.GetDamage(),
                                 mAttackDamage.GetRadius(),
                                 scale * mAttackDamage.GetKnockBackPower(), false, false);
          mgr.ApplyDamage(
              GetUniqueId(), *it, GetUniqueId(), info,
              CMaterialFilter::MakeIncludeExclude(CMaterialList(skDamageSolid), CMaterialList()),
              CVector3f::Zero());
        }
      }
    }
  }
}

void CSporbNeedle::UpdateFuse(float dt, CStateManager& mgr) {
  if (!mExploded && mHitWorld) {
    mFuseTimer += dt;
    if (mFuseTimer >= mFuseTime) {
      Explode(mgr, kInvalidUniqueId);
      PlaySfx(mExplosionSound, mgr);
    }
  }
}

float CSporbNeedle::GetClosestCameraDistanceSq(CStateManager& mgr) const {
  float distanceSquared = 3.4028235e38f;
  const CVector3f position = GetTranslation();
  for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
    const CGameCamera* camera = mgr.GetCameraManager(i)->GetCurrentCamera(mgr, true);
    const CVector3f delta = camera->GetTranslation() - position;
    const float cameraDistanceSquared = delta.MagSquared();
    if (cameraDistanceSquared < distanceSquared) {
      distanceSquared = cameraDistanceSquared;
    }
  }
  return distanceSquared;
}

rstl::optional_object< CAABox > CSporbNeedle::GetTouchBounds() const {
  return GetCollisionPrimitive()->CalculateAABox(GetPrimitiveTransform());
}

CEntity* LoadSporbNeedle(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrSporbNeedle sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrSporbNeedle.inc"

  if (sldrThis.model == kInvalidAssetId) {
    return nullptr;
  }

  return rs_new CSporbNeedle(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      CModelData(CStaticRes(sldrThis.model, sldrThis.editorProperties.transform.scale)),
      LdrToActorParameters(sldrThis.actorInformation), kInvalidUniqueId, sldrThis.explosionEffect,
      sldrThis.trailEffect, sldrThis.launchSound, sldrThis.flightSound, sldrThis.hitPlayerSound,
      sldrThis.collisionSound, sldrThis.explosionSound, LdrToDamageInfo(sldrThis.attackDamage),
      sldrThis.initialSpeed, sldrThis.mass, sldrThis.fuseTime);
}
