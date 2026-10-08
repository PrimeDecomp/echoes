#include "MetroidPrime/Weapons/CPowerBomb.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/TCastTo.hpp"

CColor CPowerBomb::kFadeColor(0xffffff7f);
const float CPowerBomb::kEndingTime = 4.25f;

CPowerBomb::CPowerBomb(TToken< CGenDescription > particle, TUniqueId uid, TAreaId areaId,
                       TUniqueId ownerId, EWeaponType type, uint flags, const CTransform4f& xf,
                       const CDamageInfo& damageInfo)
: CWeapon(uid, areaId, true, ownerId, type, rstl::string_l("PowerBomb"), xf,
          CMaterialFilter::MakeIncludeExclude(
              CMaterialList(kMT_Trigger, kMT_Immovable, kMT_Solid, kMT_NonSolidDamageable),
              CMaterialList(kMT_Projectile, kMT_PowerBomb)),
          CMaterialList(kMT_Projectile, kMT_PowerBomb), damageInfo, kPA_PowerBombs,
          CModelData::CModelDataNull())
, mCanStartFilter(true)
, mFilterEnabled(false)
, mBombTime(0.f)
, mCurRadius(0.f)
, mRadiusIncrement(damageInfo.GetRadius() / 2.5f)
, mDamageStartTime(1.f)
, mParticle(rs_new CElementGen(particle))
, mLightId(kInvalidUniqueId)
, mParticleId(CToken(particle).GetTag().GetId())
, mFlags(flags)
, mCallbackDelay(0.f)
, mExplosionSound()
, mCallback(rstl::optional_object_null()) {
  mParticle->SetGlobalTranslation(xf.GetTranslation());
}

CPowerBomb::~CPowerBomb() {}

void CPowerBomb::ApplyDynamicDamage(const CVector3f& position, CStateManager& mgr) {
  mgr.ApplyDamageToWorld(GetOwnerId(), *this, position, mCurDamageInfo, CMaterialFilter(mFilter));
}

void CPowerBomb::Touch(CActor&, CStateManager&) {
  if (mCanStartFilter) {
    return;
  }
}

rstl::optional_object< CAABox > CPowerBomb::GetTouchBounds() const {
  return rstl::optional_object_null();
}

void CPowerBomb::AddToRenderer(const CStateManager&) const { gpRender->AddParticleGen(*mParticle); }

void CPowerBomb::Render(const CStateManager&) const {}

void CPowerBomb::Think(float dt, CStateManager& mgr) {
  CWeapon::Think(dt, mgr);

  if (mCanStartFilter) {
    if (mBombTime > 1.f && mFilterEnabled != true) {
      mFilterEnabled = true;
    }
    if (mBombTime > 2.5f) {
      mCanStartFilter = false;
    }
  } else {
    if (mBombTime > 3.75f && mFilterEnabled) {
      mFilterEnabled = false;
    }
    if (mBombTime > 30.f) {
      mgr.DeleteObjectRequest(GetUniqueId());
      return;
    }
  }

  if (mBombTime > 0.25f && mParticle->IsSystemDeletable() &&
      !CSfxManager::IsPlaying(mExplosionSound)) {
    mgr.DeleteObjectRequest(GetUniqueId());
  }

  if (mFlags & kF_InstantDamage) {
    if (mBombTime == 0.f) {
      mgr.ApplyDamageToWorld(GetOwnerId(), *this, GetTranslation(), mCurDamageInfo,
                             CMaterialFilter(mFilter));
    }
  } else if (mBombTime > mDamageStartTime && mBombTime < 4.f) {
    mOrigDamageInfo.SetRadius(mCurRadius);
    ApplyDynamicDamage(GetTranslation(), mgr);
    mCurRadius += mRadiusIncrement * dt;
  }

  mParticle->Update(dt);
  if (mLightId != kInvalidUniqueId) {
    if (CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(mLightId))) {
      if (GetActive()) {
        light->SetLight(mParticle->GetLight());
        light->SetTransform(GetTransform());
      }
    }
  }

  if (mCallback.valid() && mBombTime > mCallbackDelay) {
    (*mCallback)(mgr, *this);
    mCallback = rstl::optional_object_null();
  }
  mBombTime += dt;
}

void CPowerBomb::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Create: {
    mgr.AddWeaponId(GetOwnerId(), GetType());
    if (mFlags & kF_NoDamageDelay) {
      mDamageStartTime = 0.f;
      mOrigDamageInfo.SetRadius(0.f);
    }

    bool ownerDead = false;
    if (const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.ObjectById(GetOwnerId()))) {
      if (!player->GetPlayerState()->IsPlayerAlive()) {
        ownerDead = true;
      }
    }
    if (ownerDead) {
      if (CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(GetOwnerId()))) {
        player->ApplySubmergedPitchBend(
            CSfxManager::SfxStart(mgr.ReturnFirstIfSingleElseSecond(0x245c, 0x25b8), 127,
                                  player->GetSoundPan(CPlayer::kMSP_4), CSfxManager::kAllAreas,
                                  false, false, CSfxManager::kMedPriority));
      }
    } else {
      mExplosionSound =
          CSfxManager::AddEmitter(mgr.ReturnFirstIfSingleElseSecond(0xec, 0x25c1), GetTranslation(),
                                  GetCurrentAreaId().Value(), true, false);
      mgr.InformListeners(GetTranslation(), kLNT_BombExplode);
    }

    if (mParticle->SystemHasLight()) {
      mLightId = mgr.AllocateUniqueId();
      const int sourceId = mParticleId;
      mgr.AddObject(rs_new CGameLight(mLightId, GetCurrentAreaId(), true,
                                      rstl::string_l("PowerBombLight"), GetTransform(),
                                      GetUniqueId(), mParticle->GetLight(), sourceId, 1, 0.f));
    }
    break;
  }
  case kSM_Delete:
    mgr.RemoveWeaponId(GetOwnerId(), GetType());
    if (mLightId != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(mLightId);
    }
    if (mExplosionSound) {
      CSfxManager::RemoveEmitter(mExplosionSound);
    }
    break;
  default:
    break;
  }

  CActor::AcceptScriptMsg(mgr, msg);
}
