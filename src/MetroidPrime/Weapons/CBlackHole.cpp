#include "MetroidPrime/Weapons/CBlackHole.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "MetaRender/IRenderer.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "rstl/math.hpp"

#include <float.h>

extern IRenderer* gpRender;

CBlackHole::CBlackHole(const rstl::optional_object< TToken< CGenDescription > >& particle,
                       TUniqueId uid, TAreaId areaId, TUniqueId owner, const CTransform4f& xf,
                       const CDamageInfo& damage, const rstl::string& name, float radius,
                       float duration, uint flags)
: CWeapon(uid, areaId, true, owner, kWT_Dark, name, xf,
          CMaterialFilter::MakeIncludeExclude(
              CMaterialList(kMT_Trigger, kMT_Immovable, kMT_Unknown59, kMT_NonSolidDamageable),
              CMaterialList(kMT_Projectile, kMT_PowerBomb)),
          CMaterialList(kMT_Projectile, kMT_PowerBomb), damage, kPA_Light,
          CModelData::CModelDataNull())
, mElapsedTime(0.f)
, mPullStrength(1.f)
, mPullConeAngleDegrees(360.f)
, mAttractionRange(30.f)
, mPullDirection(CVector3f::Zero())
, mParticleGen(particle
                   ? rs_new CElementGen(*particle, CElementGen::kMOT_Normal, CElementGen::kOSF_One)
                   : nullptr)
, mSourceId(particle ? particle->GetTag().GetId() : kInvalidAssetId)
, mLightId(kInvalidUniqueId)
, mRadius(radius)
, mDuration(duration)
, mFlags(flags) {}

CBlackHole::~CBlackHole() {}

void CBlackHole::ApplyDamageToWorld(const CVector3f& position, CStateManager& mgr) {
  mgr.ApplyDamageToWorld(GetOwnerId(), *this, position, mCurDamageInfo, GetFilter());
}

void CBlackHole::Touch(CActor&, CStateManager&) {}

rstl::optional_object< CAABox > CBlackHole::GetTouchBounds() const {
  return rstl::optional_object< CAABox >();
}

void CBlackHole::AddToRenderer(const CStateManager&) const {
  if (!mParticleGen.null()) {
    gpRender->AddParticleGen(*mParticleGen);
  }
}

void CBlackHole::Render(const CStateManager&) const {}

void CBlackHole::Think(float dt, CStateManager& mgr) {
  CWeapon::Think(dt, mgr);
  if (mElapsedTime > mDuration || (!mParticleGen.null() && mParticleGen->IsSystemDeletable())) {
    mgr.DeleteObjectRequest(GetUniqueId());
  }

  if (mElapsedTime > 30.f) {
    mgr.DeleteObjectRequest(GetUniqueId());
  } else {
    if (mElapsedTime > 0.f && mElapsedTime < FLT_MAX) {
      mOrigDamageInfo.SetRadius(mRadius);
      ApplyDamageToWorld(GetTranslation(), mgr);
    }

    if (!mParticleGen.null()) {
      mParticleGen->Update(dt);
    }
    UpdateRadius();
    if (mLightId != kInvalidUniqueId) {
      CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(mLightId));
      if (light && GetActive()) {
        light->SetLight(mParticleGen->GetLight());
      }
    }
    mElapsedTime += dt;

    if (mFlags & kF_PullPlayers) {
      // TODO: recover the player pull/color operation and its unresolved player flag accessor.
    }
  }
}

void CBlackHole::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Create:
    if (!mParticleGen.null()) {
      mParticleGen->SetGlobalTranslation(GetTransform().GetTranslation());
    }
    mgr.AddWeaponId(GetOwnerId(), GetType());
    mOrigDamageInfo.SetRadius(mRadius);

    if (mFlags & kF_CreationSound) {
      CSfxManager::AddEmitter(mgr.ReturnFirstIfSingleElseSecond(0x1fda, 0x25aa), GetTranslation(),
                              GetCurrentAreaId().Value(), true, false, CSfxManager::kMedPriority);
      mgr.InformListeners(GetTranslation(), kLNT_BombExplode);
    }
    if (!mParticleGen.null() && mParticleGen->SystemHasLight()) {
      mLightId = mgr.AllocateUniqueId();
      mgr.AddObject(rs_new CGameLight(mLightId, GetCurrentAreaId(), GetActive(), rstl::string_l(""),
                                      GetTransform(), GetUniqueId(), mParticleGen->GetLight(),
                                      mSourceId, 1, 0.f));
    }
    break;
  case kSM_Delete:
    mgr.RemoveWeaponId(GetOwnerId(), GetType());
    if (mLightId != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(mLightId);
      mLightId = kInvalidUniqueId;
    }
    // TODO: clear the unresolved player-effect flag through a supported player accessor.
    break;
  default:
    break;
  }

  CActor::AcceptScriptMsg(mgr, msg);
}

void CBlackHole::UpdateRadius() {
  if (!mParticleGen.null()) {
    const CElementGen::CAdvancedValues* data = mParticleGen->ParticleAdditionalData(0);
    mRadius = data ? rstl::max_val(0.f, data->mValues[0]) : 0.f;
  }
}
