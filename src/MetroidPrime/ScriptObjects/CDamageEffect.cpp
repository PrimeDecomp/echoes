#include "MetroidPrime/ScriptObjects/CDamageEffect.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Collision/CMaterialList.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"

static EMaterialTypes SolidMaterial = kMT_Solid;
static EMaterialTypes ProjectileMaterial = kMT_Projectile;

CDamageEffect::CDamageEffect(const TToken< CGenDescription >& effect, TUniqueId uid, TAreaId areaId,
                             bool active, TUniqueId owner, const CTransform4f& xf,
                             const CDamageInfo& damage, const CAABox& bounds, float timeScale,
                             const CVector3f& scale, bool affectsVisor, float visorAlpha,
                             CAssetId visorTexture, float visorFadeIn, float visorFadeOut,
                             bool showInCombat, bool showInDark, bool showInEcho)
: CActor(uid, rstl::string_l("Damage Effect"), CEntityInfo(areaId, NullConnectionList, active), 0,
         xf, CModelData::CModelDataNull(), CMaterialList(ProjectileMaterial),
         CActorParameters::None(), kInvalidUniqueId)
, mParticle(rs_new CElementGen(effect))
, mOwner(owner)
, mDamage(damage)
, mScaledDamage(damage)
, mBounds(bounds)
, mTimeScale(timeScale)
, mShowInCombat(showInCombat)
, mShowInDark(showInDark)
, mShowInEcho(showInEcho)
, mShowAlways(showInEcho && showInDark && showInCombat)
, x1b8_28_(false)
, mAffectsVisor(affectsVisor)
, mVisorAlpha(visorAlpha)
, mVisorTexture(visorTexture)
, mVisorFadeIn(visorFadeIn)
, mVisorFadeOut(visorFadeOut)
, mTimer(0.f) {
  mParticle->SetGlobalScale(scale);
  mParticle->SetTranslation(xf.GetTranslation());
}

CDamageEffect::~CDamageEffect() {}

void CDamageEffect::Touch(CActor& actor, CStateManager& mgr) {
  if (actor.GetUniqueId() == mOwner) {
    return;
  }

  mgr.ApplyDamage(
      GetUniqueId(), actor.GetUniqueId(), GetUniqueId(), mScaledDamage,
      CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList()),
      CVector3f::Zero());
}

rstl::optional_object< CAABox > CDamageEffect::GetTouchBounds() const {
  if (GetActive()) {
    return mBounds;
  }
  return rstl::optional_object_null();
}

void CDamageEffect::AddToRenderer(const CStateManager& mgr) const {
  bool drawParticles = true;
  if (!mShowAlways) {
    switch (mgr.GetPlayerState()->GetActiveVisor(mgr)) {
    case CPlayerState::kPV_Combat:
    case CPlayerState::kPV_Scan:
      drawParticles = mShowInCombat;
      break;
    case CPlayerState::kPV_Echo:
      drawParticles = mShowInEcho;
      break;
    case CPlayerState::kPV_Dark:
      drawParticles = mShowInDark;
      break;
    }
  }

  if (drawParticles) {
    gpRender->AddParticleGen(*mParticle);
  }
  CActor::AddToRenderer(mgr);
}

void CDamageEffect::Think(float dt, CStateManager& mgr) {
  const float particleCount = static_cast< float >(mParticle->GetParticleCount()) /
                              static_cast< float >(mParticle->GetMaxParticles());
  if (GetActive()) {
    mParticle->Update(dt * mTimeScale);
    mScaledDamage = CDamageInfo(mDamage, dt * (particleCount > 0.5f ? particleCount : 0.f));
  }

  bool doFree = false;
  if (mParticle->IsSystemDeletable()) {
    doFree = true;
  }

  if (mAffectsVisor) {
    for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
      CPlayer* player = mgr.Player(i);
      if (player->GetTouchBounds()->DoBoundsOverlap(*GetTouchBounds()) && !doFree &&
          particleCount > 0.5f) {
        player->SetVisorSteam(particleCount * mVisorAlpha, mVisorFadeIn, mVisorFadeOut,
                              mVisorTexture);
      } else {
        player->SetVisorSteam(0.f, 1.f, 1.f, kInvalidAssetId);
      }
    }
  }

  mTimer += dt;
  if (mTimer > 45.f) {
    doFree = true;
  }

  if (doFree) {
    mgr.DeleteObjectRequest(GetUniqueId());
  }
}

void CDamageEffect::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CActor::AcceptScriptMsg(mgr, msg);

  switch (message) {
  case kSM_XCRT:
    mParticle->SetParticleEmission(true);
    SetActive(true);
    break;
  default:
    break;
  }
}
