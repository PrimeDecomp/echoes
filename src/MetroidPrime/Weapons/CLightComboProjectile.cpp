#include "MetroidPrime/Weapons/CLightComboProjectile.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Enemies/CSwarmBasics.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerGun.hpp"
#include "MetroidPrime/Weapons/CPlasmaProjectile.hpp"
#include "MetroidPrime/Weapons/CWeaponAssetInfo.hpp"
#include "MetroidPrime/Weapons/WeaponCommon.hpp"

#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"

// Guessed name
static const CColor sRayColor(1.f, 0.5f, 0.5f, 0.5f);

CLightComboProjectile::CLightComboProjectile(const TToken< CWeaponDescription >& description,
                                             EWeaponType type, const CTransform4f& xf,
                                             EMaterialTypes material, const CDamageInfo& damage,
                                             TUniqueId uid, TAreaId areaId, TUniqueId owner,
                                             TUniqueId homingTarget, bool underwater, uint attribs,
                                             float radius)
: CEnergyProjectile(true, description, type, xf, material, damage, uid, areaId, owner, homingTarget,
                    attribs, underwater, CVector3f::One(), CImpactVisorEffect(), false, true, false,
                    1.f, 4.f, 4.f)
, mRadius(radius)
, mRaySpawnTimer(0.2f)
, mRayProjectile(NWeaponTypes::get_asset_id_from_name("LightPlasmaWeapon"),
                 gpTweakPlayerGun->GetSunBurstRaysDamage())
, mRayBeamInfo(0xb1, NWeaponTypes::get_asset_id_from_name("LightPlasmaBurn"),
               NWeaponTypes::get_asset_id_from_name("LightPlasmaPulse"),
               NWeaponTypes::get_asset_id_from_name("LightPlasmaGlow"),
               NWeaponTypes::get_asset_id_from_name("LightPlasmaGlow"), mRadius, 0.26f, 50.f, 10.f,
               20.f, 0.f, 1.f, 2.f, sRayColor, sRayColor, 150.f, kInvalidAssetId)
, mRayListsDirty(false) {}

CLightComboProjectile::~CLightComboProjectile() {}

rstl::optional_object< CAABox > CLightComboProjectile::GetTouchBounds() const {
  if (!GetActive()) {
    return rstl::optional_object_null();
  }
  const CVector3f& position = GetTranslation();
  return CAABox(position.GetX() - mRadius, position.GetY() - mRadius, position.GetZ() - mRadius,
                position.GetX() + mRadius, position.GetY() + mRadius, position.GetZ() + mRadius);
}

void CLightComboProjectile::Touch(CActor& actor, CStateManager& mgr) {
  CGameProjectile::Touch(actor, mgr);
}

void CLightComboProjectile::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  CEnergyProjectile::Think(dt, mgr);
  const CVector3f position = GetTranslation();
  mRaySpawnTimer += dt;

  if (HasExploded()) {
    RequestRayResets(mgr, true, true);
  } else {
    const rstl::optional_object< CAABox > touchBounds = GetTouchBounds();
    if (touchBounds) {
      const CMaterialFilter targetFilter =
          CMaterialFilter::MakeInclude(CMaterialList(kMT_Target, kMT_Player));
      const CMaterialFilter occluderFilter = CMaterialFilter::MakeIncludeExclude(
          CMaterialList(kMT_Unknown59), CMaterialList(kMT_NoPlatformCollision, kMT_CollisionActor));

      rstl::reserved_vector< TUniqueId, 1024 > nearList;
      rstl::reserved_vector< TUniqueId, 1024 > occluderList;
      mgr.BuildNearList(nearList, *touchBounds, targetFilter, nullptr);
      if (nearList.size() == 0) {
        RequestRayResets(mgr, false, false);
      } else {
        mgr.BuildNearList(occluderList, *touchBounds, occluderFilter, nullptr);
      }

      for (const TUniqueId* it = nearList.begin(); it != nearList.end(); ++it) {
        if (*it == GetOwnerId()) {
          continue;
        }
        const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(*it));
        if (TCastToConstPtr< CSwarmBasics >(actor) != nullptr || actor == nullptr) {
          continue;
        }

        int state = 4;
        const CHealthInfo* healthInfo = actor->GetHealthInfo();
        if (healthInfo == nullptr || healthInfo->GetHP() > 0.f) {
          const CVector3f aimPosition = actor->GetAimPosition(mgr, 0.f);
          const CVector3f delta = aimPosition - position;
          if (!delta.CanBeNormalized()) {
            state = 1;
          } else if (delta.MagSquared() >= mRadius * mRadius) {
            state = 2;
          } else if (!mgr.RayCollideWorld(position, position + delta, occluderList, occluderFilter,
                                          actor)) {
            state = 3;
          } else {
            state = 0;
            if (mRaySpawnTimer > 0.2f) {
              if (rstl::find_by_key(mTargetRays, *it) == mTargetRays.end()) {
                CreateRay(0.3f, mgr, *it);
              }
            }
          }
        }

        if (state != 0) {
          TTargetRays::iterator targetRay = rstl::find_by_key_nc(mTargetRays, *it);
          if (targetRay != mTargetRays.end()) {
            TRays::iterator rayIt = rstl::find_by_key_nc(mRays, targetRay->second);
            if (rayIt != mRays.end()) {
              const CPlasmaProjectile* ray =
                  static_cast< CPlasmaProjectile* >(mgr.ObjectById(rayIt->first));
              if (ray != nullptr && ray->IsFiring()) {
                rayIt->second.mResetRequested = true;
              }
            }
          }
        }
      }
    }
  }

  if (mRayListsDirty) {
    rstl::sort_by_key(mRays);
    rstl::sort_by_key(mTargetRays);
  }

  for (TRays::iterator it = mRays.begin(); it != mRays.end();) {
    SRayInfo& info = it->second;
    info.mResetDelay -= dt;
    CPlasmaProjectile* ray = static_cast< CPlasmaProjectile* >(mgr.ObjectById(it->first));
    if (ray != nullptr && ray->GetActive()) {
      if (info.mTargetId != kInvalidUniqueId) {
        const CActor* target = TCastToConstPtr< CActor >(mgr.GetObjectById(info.mTargetId));
        if (target != nullptr) {
          info.mTargetPosition = target->GetAimPosition(mgr, 0.f);
          ray->SetMaxLength(
              rstl::max_val(0.01f, (position - info.mTargetPosition).Magnitude() - 0.01f));
        } else if (!info.mResetRequested && ray->IsFiring()) {
          info.mResetRequested = true;
        }
      }

      ray->UpdateFx(CTransform4f::LookAt(position, info.mTargetPosition, CVector3f::Up()), dt, mgr);

      CColor color = ray->GetOuterColor();
      if (color.GetRed() == 1.f) {
        color.SetGreen(rstl::min_val(1.f, color.GetGreen() + 0.1f));
        color.SetBlue(rstl::max_val(0.f, color.GetBlue() - 0.1f));
      }
      if (color.GetGreen() == 1.f) {
        color.SetRed(rstl::max_val(0.f, color.GetRed() - 0.1f));
        color.SetBlue(rstl::min_val(1.f, color.GetBlue() + 0.1f));
      }
      if (color.GetBlue() == 1.f) {
        color.SetRed(rstl::min_val(1.f, color.GetRed() + 0.1f));
        color.SetGreen(rstl::max_val(0.f, color.GetGreen() - 0.1f));
      }
      ray->SetOuterColor(color);
    } else {
      if (info.mTargetId != kInvalidUniqueId) {
        TTargetRays::iterator targetRay = rstl::find_by_key_nc(mTargetRays, info.mTargetId);
        if (targetRay != mTargetRays.end()) {
          mTargetRays.erase(targetRay);
        }
      }
      mgr.DeleteObjectRequest(it->first);
      it = mRays.erase(it);
      continue;
    }

    if (info.mResetRequested && ray->IsFiring() && (info.mResetDelay < 0.f || info.mFullReset)) {
      ray->ResetBeam(mgr, info.mFullReset);
    }
    ++it;
  }
}

void CLightComboProjectile::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Delete:
    FreeRays(mgr);
    break;
  default:
    break;
  }
  CEnergyProjectile::AcceptScriptMsg(mgr, msg);
}

TUniqueId CLightComboProjectile::CreateRay(float resetDelay, CStateManager& mgr, TUniqueId target) {
  return CreateRay(resetDelay, mgr, target, CVector3f::Zero());
}

TUniqueId CLightComboProjectile::CreateRay(float resetDelay, CStateManager& mgr,
                                           const CVector3f& position) {
  return CreateRay(resetDelay, mgr, kInvalidUniqueId, position);
}

TUniqueId CLightComboProjectile::CreateRay(float resetDelay, CStateManager& mgr, TUniqueId target,
                                           const CVector3f& position) {
  if (!CanCreateRay()) {
    return kInvalidUniqueId;
  }

  CVector3f targetPosition = position;
  if (const CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(target))) {
    targetPosition = actor->GetTranslation();
  } else {
    target = kInvalidUniqueId;
  }

  CPlasmaProjectile* ray = rs_new CPlasmaProjectile(
      mRayProjectile.Token(), rstl::string_l("LightComboRay"), kWT_Light, mRayBeamInfo,
      CTransform4f::Identity(), kMT_NoPlatformCollision, mRayProjectile.GetDamage(),
      mgr.AllocateUniqueId(), GetCurrentAreaId(), GetOwnerId(), CWeaponAssetInfo(), false, 0x20000);
  if (!ray) {
    return kInvalidUniqueId;
  }

  CRandom16& random = *mgr.Random();
  const int fullColorChannel = random.Range(0, 2);
  float channels[3];
  for (int i = 0; i < 3; ++i) {
    channels[i] = i == fullColorChannel ? 1.f : 0.99f * random.Float();
  }
  ray->SetOuterColor(CColor(channels[0], channels[1], channels[2], 0.5f));
  ray->Fire(CTransform4f::LookAt(GetTranslation(), targetPosition, CVector3f::Up()), mgr, false);

  mRays.push_back(TRay(ray->GetUniqueId(), SRayInfo(target, targetPosition, resetDelay)));
  if (target != kInvalidUniqueId) {
    mTargetRays.push_back(TTargetRay(target, ray->GetUniqueId()));
  }
  mRayListsDirty = true;
  mgr.AddObject(*ray);
  mRaySpawnTimer = 0.f;
  return ray->GetUniqueId();
}

bool CLightComboProjectile::UpdateRayTarget(CStateManager& mgr, TUniqueId rayId,
                                            const CVector3f& position) {
  TRays::iterator it = rstl::find_by_key_nc(mRays, rayId);
  if (it != mRays.end()) {
    CPlasmaProjectile* ray = static_cast< CPlasmaProjectile* >(mgr.ObjectById(it->first));
    if (ray && ray->GetActive()) {
      it->second.mTargetPosition = position;
      ray->SetMaxLength(rstl::max_val(
          0.01f, (GetTranslation() - it->second.mTargetPosition).Magnitude() - 0.01f));
    }
  }
  return false;
}

void CLightComboProjectile::RequestRayReset(CStateManager& mgr, TUniqueId rayId, bool fullReset) {
  TRays::iterator it = rstl::find_by_key_nc(mRays, rayId);
  if (it == mRays.end()) {
    return;
  }
  const CPlasmaProjectile* ray = static_cast< CPlasmaProjectile* >(mgr.ObjectById(it->first));
  if (ray && (ray->IsFiring() || (ray->GetActive() && fullReset))) {
    it->second.mResetRequested = true;
    it->second.mFullReset = fullReset;
  }
}

void CLightComboProjectile::FreeRays(CStateManager& mgr) {
  for (TRays::const_iterator it = mRays.begin(); it != mRays.end(); ++it) {
    mgr.DeleteObjectRequest(it->first);
  }
  mRays.clear();
}

void CLightComboProjectile::RequestRayResets(CStateManager& mgr, bool includeUntargeted,
                                             bool fullReset) {
  for (TRays::iterator it = mRays.begin(); it != mRays.end(); ++it) {
    const CPlasmaProjectile* ray = static_cast< CPlasmaProjectile* >(mgr.ObjectById(it->first));
    if (!ray || (!ray->IsFiring() && (!ray->GetActive() || !fullReset))) {
      continue;
    }
    SRayInfo& info = it->second;
    if (info.mTargetId == kInvalidUniqueId && !includeUntargeted) {
      continue;
    }
    if (info.mResetRequested && (!fullReset || info.mFullReset)) {
      continue;
    }
    info.mResetRequested = true;
    info.mFullReset = fullReset;
  }
}

bool CLightComboProjectile::CanCreateRay() const {
  return mRaySpawnTimer > 0.2f && mRays.size() < 8;
}
