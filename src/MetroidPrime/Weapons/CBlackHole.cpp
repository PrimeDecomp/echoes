#include "MetroidPrime/Weapons/CBlackHole.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "MetaRender/IRenderer.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
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
      for (uint i = 0; i < mgr.GetNumPlayers(); ++i) {
        CPlayer* player = mgr.GetPlayer(i);
        const bool isOwner = GetOwnerId() == player->GetUniqueId();
        const CVector3f center = player->GetTouchBounds()->GetCenterPoint();
        const CVector3f toHole = GetTranslation() - center;
        const float distance = toHole.Magnitude();
        const bool atCenter = distance < 5.f;
        if (!atCenter && !isOwner) {
          const CVector3f direction = toHole.AsNormalized();
          if (static_cast< float >(cos(0.017453292f * mPullConeAngleDegrees)) <
                  CVector3f::Dot(-direction, mPullDirection) &&
              distance < mAttractionRange) {
            const float speed = rstl::min_val(
                (1.f / dt) * distance, mPullStrength * (mAttractionRange / (distance * distance)));
            const CVector3f impulse = player->GetMass() * direction;
            player->ApplyForceWR(speed * impulse, CAxisAngle::Identity());
          }
        }
        if (atCenter && !isOwner) {
          const CVector3f position = (GetTranslation() - center) + player->GetTranslation();
          player->Stop();
          player->MoveToWR(position, dt);
        }

        const bool insideRadius = distance < mRadius;
        player->SetHoldScreenFilterAlpha(insideRadius);
        if (insideRadius) {
          static const CColor skFilterColor(uchar(120), uchar(135), uchar(70), uchar(0));
          CColor filterColor = player->GetScreenFilterColor();
          if (!(skFilterColor.WithAlphaOf(0.f) == filterColor.WithAlphaOf(0.f)) &&
              filterColor.GetAlpha() == 0.f) {
            filterColor = skFilterColor;
          }
          filterColor.SetAlpha(rstl::min_val(dt + filterColor.GetAlpha(), 1.f));
          player->SetScreenFilterColor(filterColor);
        }
      }
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
      static const ushort skCreationSfx[2] = {0x1fda, 0x25aa};
      CSfxManager::AddEmitter(mgr.ReturnFirstIfSingleElseSecond(skCreationSfx[0], skCreationSfx[1]),
                              GetTranslation(), GetCurrentAreaId().Value(), true, false,
                              CSfxManager::kMedPriority);
      mgr.InformListeners(GetTranslation(), kLNT_BombExplode);
    }
    if (!mParticleGen.null() && mParticleGen->SystemHasLight()) {
      mLightId = mgr.AllocateUniqueId();
      const CAssetId sourceId = mSourceId;
      mgr.AddObject(rs_new CGameLight(mLightId, GetCurrentAreaId(), GetActive(), rstl::string_l(""),
                                      GetTransform(), GetUniqueId(), mParticleGen->GetLight(),
                                      sourceId, 1, 0.f));
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
    float radius = 0.f;
    const CElementGen::CAdvancedValues* data = mParticleGen->ParticleAdditionalData(0);
    if (data) {
      radius = rstl::max_val(0.f, data->mValues[0]);
    }
    mRadius = radius;
  }
}
