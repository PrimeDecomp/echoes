#include "MetroidPrime/ScriptObjects/CScriptEMPulse.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEMPulse.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "MetaRender/CCubeRenderer.hpp"

CScriptEMPulse::CScriptEMPulse(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                               const CTransform4f& xf, float initialRadius, float finalRadius,
                               float duration, float minHudDisableTime, float maxHudDisableTime,
                               float minHudDisableAmount, float maxHudDisableAmount,
                               CAssetId particleId)
: CActor(uid, name, info, 0, xf, CModelData::CModelDataNull(), CMaterialList(kMT_Projectile),
         CActorParameters::None(), kInvalidUniqueId)
, mDuration(duration)
, mFinalRadius(finalRadius)
, mCurrentRadius(initialRadius)
, mInitialRadius(initialRadius)
, mMinHudDisableTime(minHudDisableTime)
, mMaxHudDisableTime(maxHudDisableTime)
, mMinHudDisableAmount(minHudDisableAmount)
, mMaxHudDisableAmount(maxHudDisableAmount)
, mParticleDesc(gpSimplePool->GetObj(SObjectTag('PART', particleId))) {}

CEntity* LoadEMPulse(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrEMPulse sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrEMPulse.inc"

  return rs_new CScriptEMPulse(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                               LdrToEntityInfo(info, sldrThis.editorProperties),
                               LdrToTransform4f(sldrThis.editorProperties), sldrThis.initialSize,
                               sldrThis.finalSize, sldrThis.duration, sldrThis.minHudDisableTime,
                               sldrThis.maxHudDisableTime, sldrThis.minHudDisableAmount,
                               sldrThis.maxHudDisableAmount, sldrThis.explosion);
}

void CScriptEMPulse::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CActor::AcceptScriptMsg(mgr, msg);

  switch (message) {
  case kSM_Activate:
    mParticleGen =
        rs_new CElementGen(mParticleDesc, CElementGen::kMOT_Normal, CElementGen::kOSF_One);
    mParticleGen->SetOrientation(GetTransform().GetRotation());
    mParticleGen->SetGlobalTranslation(GetTransform().GetTranslation());
    mParticleGen->SetParticleEmission(true);

    for (long i = 0; i < static_cast< uint >(mgr.GetNumPlayers()); ++i) {
      mgr.PlayerState(i)->StaticInterference().AddSource(GetUniqueId(), mMinHudDisableAmount,
                                                         mMinHudDisableTime);
    }
    break;
  default:
    break;
  }
}

void CScriptEMPulse::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    const float step = (mFinalRadius - mInitialRadius) / mDuration;
    mCurrentRadius += step * dt;
    if (mCurrentRadius >= mFinalRadius) {
      mgr.DeleteObjectRequest(GetUniqueId());
    }
    mParticleGen->Update(dt);
  }
}

CAABox CScriptEMPulse::CalculateBoundingBox() const {
  const float radius = mCurrentRadius;
  const CVector3f position = GetTranslation();
  return CAABox(position - CVector3f(radius, radius, radius),
                position + CVector3f(radius, radius, radius));
}

rstl::optional_object< CAABox > CScriptEMPulse::GetTouchBounds() const {
  return CalculateBoundingBox();
}

void CScriptEMPulse::PreRenderAllViewports(CStateManager& mgr) {
  const CAABox& bounds = CalculateBoundingBox();
  SetOtherBounds(bounds);
  SetRenderBounds(bounds);
  UpdatePortalSystemState(mgr);
}

void CScriptEMPulse::Touch(CActor& actor, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  if (CPlayer* player = TCastToPtr< CPlayer >(actor)) {
    const float distance = (GetTranslation() - player->GetTranslation()).Magnitude();
    if (distance < mFinalRadius) {
      const float proximity = 1.f - distance / mFinalRadius;
      const float duration =
          proximity * (mMaxHudDisableTime - mMinHudDisableTime) + mMinHudDisableTime;
      const float magnitude =
          proximity * (mMaxHudDisableAmount - mMinHudDisableAmount) + mMinHudDisableAmount;

      if (duration > player->GetStaticTimer()) {
        player->SetHudDisable(duration);
        player->SetOrbitRequestForTarget(player->GetOrbitTargetId(),
                                         CPlayer::kOR_ActivateOrbitSource, mgr);
      }
      player->GetPlayerState()->StaticInterference().AddSource(GetUniqueId(), magnitude, duration);
    }
  }
}

void CScriptEMPulse::AddToRenderer(const CStateManager& mgr) const {
  CActor::AddToRenderer(mgr);
  if (GetActive()) {
    gpRender->AddParticleGen(*mParticleGen);
  }
}

CScriptEMPulse::~CScriptEMPulse() {}
