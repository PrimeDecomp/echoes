#include "MetroidPrime/Weapons/CBomb.hpp"

#include "Collision/COBBox.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Collision/CollisionUtil.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CSphere.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/SFX/SamusPowerups_MP.h"
#include "MetroidPrime/SFX/Weapons.h"
#include "MetroidPrime/SFX/Weapons_MP.h"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/WeaponSound.hpp"
#include "rstl/math.hpp"

static const ushort skPlacementSfx[] = {SFXsam_a_bombset_00_oneshot, SFXsa2_a_bombset_00_oneshot};
static const ushort skExplosionSfx[] = {SFXsam_a_bombexp_00_oneshot, SFXsa2_a_bombexp_00_oneshot};

CBomb::CBomb(TToken< CGenDescription > particle1, TToken< CGenDescription > particle2,
             TUniqueId uid, TAreaId areaId, TUniqueId ownerId,
             const CMaterialList& triggerMaterials, EWeaponType type, int attribs, float fuseTime,
             float triggerRadius, const CTransform4f& xf, const CDamageInfo& damageInfo)
: CWeapon(uid, areaId, true, ownerId, type, rstl::string_l("Bomb"), xf,
          CMaterialFilter::MakeIncludeExclude(
              CMaterialList(kMT_Solid, kMT_Trigger, kMT_NonSolidDamageable),
              CMaterialList(kMT_Projectile, kMT_Bomb)),
          CMaterialList(kMT_Projectile, kMT_Bomb), damageInfo, attribs | kPA_Bombs,
          CModelData::CModelDataNull())
, mTriggerMaterials(triggerMaterials)
, mVelocity(CVector3f::Zero())
, mAcceleration(CVector3f::Zero())
, mPrevLocation(xf.GetTranslation())
, mFuseTime(fuseTime)
, mTriggerRadius(triggerRadius)
, mParticle1(rs_new CElementGen(particle1))
, mParticle2(rs_new CElementGen(particle2))
, mLightId(kInvalidUniqueId)
, mParticle2Id(CToken(particle2).GetTag().GetId())
, mIsNotDetonated(true)
, mBeingDragged(false)
, mDisableFuse(false) {
  mParticle1->SetGlobalTranslation(xf.GetTranslation());
  mParticle2->SetGlobalTranslation(xf.GetTranslation());
  if (mFuseTime < 0.f) {
    mDisableFuse = true;
  }
}

CBomb::~CBomb() {}

void CBomb::Explode(CStateManager& mgr, const rstl::optional_object< CVector3f >& position) {
  if (position.valid()) {
    SetTranslation(*position);
  }

  const CVector3f explosionPosition = GetTranslation();
  mgr.ApplyDamageToWorld(GetOwnerId(), *this, GetTranslation(), mCurDamageInfo, GetFilter());
  AddEmitter(*this, mgr.ReturnFirstIfSingleElseSecond(skExplosionSfx[0], skExplosionSfx[1]), true,
             false, CSfxManager::kMedPriority, 127, 20, 150.f, 1.f);
  mgr.InformListeners(explosionPosition, kLNT_BombExplode);

  if (const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(GetOwnerId()))) {
    if (player->GetPlayerState()->GetItemAmount(CPlayerState::kIT_DoubleDamage, true)) {
      PlaySfxForPlayer(nullptr, SFXsa2_a_massdam_00_oneshot, player->GetSoundPan(CPlayer::kMSP_4),
                       mgr.GetNextAreaId().Value(), GetFluidCount() != 0, false);
    }
  }

  mgr.RemoveWeaponId(GetOwnerId(), GetType());
  if (CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(mLightId))) {
    light->SetActive(true);
  }
  mIsNotDetonated = false;
}

void CBomb::Touch(CActor& actor, CStateManager& mgr) {
  if (mIsNotDetonated) {
    switch (mBeingDragged) {
    case false:
      if (actor.GetUniqueId() != GetOwnerId() &&
          mTriggerMaterials.SharesMaterials(actor.GetMaterialList())) {
        if (CollisionUtil::AABoxSphereIntersection(*actor.GetTouchBounds(),
                                                   CSphere(GetTranslation(), mTriggerRadius))) {
          mFuseTime = -1.f;
          mDisableFuse = false;
        }
      }
    }
  }
}

void CBomb::AddToRenderer(const CStateManager& mgr) const {
  const CVector3f origin = GetTranslation();
  float radius = 1.f;
  if (const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(GetOwnerId()))) {
    radius = 0.9f * player->GetMorphBall()->GetBallRadius();
  }

  const CVector3f extent(radius, radius, radius);
  const CAABox bounds(origin - extent, origin + extent);
  const CVector3f forward = CGraphics::GetViewMatrix().GetForward();
  const CVector3f closestPoint = bounds.ClosestPointAlongVector(forward);

  if (mIsNotDetonated) {
    gpRender->AddParticleGen(*mParticle1, closestPoint, bounds);
    if (mFuseTime < 0.015f) {
      gpRender->AddParticleGen(*mParticle2, closestPoint, bounds);
    }
  } else {
    gpRender->AddParticleGen(*mParticle2, closestPoint, bounds);
  }
}

void CBomb::Render(const CStateManager& mgr) const {}

void CBomb::Think(float dt, CStateManager& mgr) {
  CWeapon::Think(dt, mgr);
  if (mIsNotDetonated) {
    mParticle1->Update(dt);
    if (!mDisableFuse) {
      if (mFuseTime <= 0.f) {
        Explode(mgr, rstl::optional_object_null());
      }
      if (mFuseTime < 0.015f) {
        UpdateLight(dt, mgr);
      }
      mFuseTime -= dt;
    }
  } else {
    UpdateLight(dt, mgr);
    if (mParticle2->IsSystemDeletable()) {
      mgr.DeleteObjectRequest(GetUniqueId());
    }
  }

  if (mIsNotDetonated) {
    if (mAcceleration.MagSquared() > 0.f) {
      mVelocity += dt * mAcceleration;
    }
    if (mVelocity.MagSquared() > 0.f) {
      mPrevLocation = GetTransform().GetTranslation();
      const CVector3f vel = dt * mVelocity;
      SetTranslation(GetTranslation() + vel);
      const CVector3f delta = GetTransform().GetTranslation() - mPrevLocation;
      const float distance = delta.Magnitude();
      if (close_enough(distance, 0.f)) {
        Explode(mgr, rstl::optional_object_null());
      } else {
        static const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
            CMaterialList(kMT_Solid, kMT_NonSolidDamageable),
            CMaterialList(kMT_Character, kMT_Player, kMT_ProjectilePassthrough));
        const CRayCastResult result =
            mgr.RayStaticIntersection(mPrevLocation, (1.f / distance) * delta, distance, filter);
        if (result.IsValid()) {
          Explode(mgr, rstl::optional_object_null());
        }
      }
    }
  }

  mParticle1->SetGlobalTranslation(GetTranslation());
  mParticle2->SetGlobalTranslation(GetTranslation());
}

void CBomb::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Create:
    if (mParticle2->SystemHasLight()) {
      mLightId = mgr.AllocateUniqueId();
      const CAssetId sourceId = mParticle2Id;
      mgr.AddObject(rs_new CGameLight(mLightId, GetCurrentAreaId(), false, rstl::string_l(""),
                                      GetTransform(), GetUniqueId(), mParticle2->GetLight(),
                                      sourceId, 1, 0.f));
    }
    mgr.AddWeaponId(GetOwnerId(), GetType());
    AddEmitter(*this, mgr.ReturnFirstIfSingleElseSecond(skPlacementSfx[0], skPlacementSfx[1]), true,
               false, CSfxManager::kMedPriority, 127, 20, 150.f, 1.f);
    mgr.InformListeners(GetTranslation(), kLNT_BombExplode);
    break;
  case kSM_Delete:
    if (mLightId != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(mLightId);
    }
    if (mIsNotDetonated) {
      mgr.RemoveWeaponId(GetOwnerId(), GetType());
    }
    break;
  default:
    break;
  }
  CActor::AcceptScriptMsg(mgr, msg);
}

rstl::optional_object< CAABox > CBomb::GetTouchBounds() const {
  const float radius = mIsNotDetonated ? mTriggerRadius : mCurDamageInfo.GetRadius();
  return CAABox(rstl::min_val(mPrevLocation.GetX(), GetTranslation().GetX()) - radius,
                rstl::min_val(mPrevLocation.GetY(), GetTranslation().GetY()) - radius,
                rstl::min_val(mPrevLocation.GetZ(), GetTranslation().GetZ()) - radius,
                rstl::max_val(mPrevLocation.GetX(), GetTranslation().GetX()) + radius,
                rstl::max_val(mPrevLocation.GetY(), GetTranslation().GetY()) + radius,
                rstl::max_val(mPrevLocation.GetZ(), GetTranslation().GetZ()) + radius);
}

void CBomb::UpdateLight(float dt, CStateManager& mgr) {
  mParticle2->Update(dt);
  if (mLightId == kInvalidUniqueId) {
    return;
  }

  CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(mLightId));
  if (light && GetActive()) {
    light->SetLight(mParticle2->GetLight());
    light->SetTransform(GetTransform());
  }
}

void DebugDrawOBB(const COBBox& box, float r, float g, float b, float a) {}

extern "C" void fn_80083FDC() {
}
