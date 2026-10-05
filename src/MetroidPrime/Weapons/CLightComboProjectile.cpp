#include "MetroidPrime/Weapons/CLightComboProjectile.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerGun.hpp"
#include "MetroidPrime/Weapons/CPlasmaProjectile.hpp"
#include "MetroidPrime/Weapons/CWeaponAssetInfo.hpp"
#include "MetroidPrime/Weapons/WeaponCommon.hpp"

#include "rstl/algorithm.hpp"

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
  const CVector3f extent(mRadius, mRadius, mRadius);
  return CAABox(GetTranslation() - extent, GetTranslation() + extent);
}

void CLightComboProjectile::Touch(CActor& actor, CStateManager& mgr) {
  CGameProjectile::Touch(actor, mgr);
}

void CLightComboProjectile::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  CEnergyProjectile::Think(dt, mgr);
  mRaySpawnTimer += dt;

  // TODO: Recover target acquisition, ray lifetime updates, and colour cycling.
  // This is a nonfunctional scaffold for the remaining Light-combo behavior.
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
    CPlasmaProjectile* ray =
        static_cast< CPlasmaProjectile* >(mgr.GetObjectByIdFromListAll(it->first));
    if (ray && ray->GetActive()) {
      it->second.mTargetPosition = position;
      ray->SetMaxLength(rstl::max_val(0.01f, (GetTranslation() - position).Magnitude() - 0.01f));
    }
  }
  return false;
}

void CLightComboProjectile::RequestRayReset(CStateManager& mgr, TUniqueId rayId, bool fullReset) {
  TRays::iterator it = rstl::find_by_key_nc(mRays, rayId);
  if (it == mRays.end()) {
    return;
  }
  const CPlasmaProjectile* ray =
      static_cast< CPlasmaProjectile* >(mgr.GetObjectByIdFromListAll(it->first));
  if (!ray || (!ray->IsFiring() && (ray->GetActive() || !fullReset))) {
    return;
  }
  it->second.mResetRequested = true;
  it->second.mFullReset = fullReset;
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
    const CPlasmaProjectile* ray =
        static_cast< CPlasmaProjectile* >(mgr.GetObjectByIdFromListAll(it->first));
    if (!ray || (!ray->IsFiring() && (ray->GetActive() || !fullReset))) {
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
