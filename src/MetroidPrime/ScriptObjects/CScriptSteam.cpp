#include "MetroidPrime/ScriptObjects/CScriptSteam.hpp"

#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CEnvFxManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSteam.hpp"

#include "Kyoto/Math/CloseEnough.hpp"
#include "rstl/math.hpp"

CScriptSteam::CScriptSteam(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                           const CVector3f& position, const CAABox& bounds,
                           const CDamageInfo& damage, const CVector3f& forceField, uint flags,
                           CAssetId texture, float strength, float fadeInRate, float fadeOutRate,
                           float radius, bool splash)
: CScriptTrigger(uid, name, info, position, bounds, damage, forceField, flags, false, false)
, mEnableSplash(splash)
, mTexture(texture)
, mStrength(strength)
, mAlphaInDuration(fadeInRate / strength)
, mAlphaOutDuration(fadeOutRate / strength)
, mMaxDistance(0.f)
, mInverseMaxDistance(0.f) {
  const float extent =
      rstl::min_val(bounds.GetMaxPoint().GetX(),
                    rstl::min_val(bounds.GetMaxPoint().GetY(), bounds.GetMaxPoint().GetZ()));
  mMaxDistance = close_enough(radius, 0.f) ? extent : rstl::min_val(radius, extent);
  mInverseMaxDistance = 1.f / mMaxDistance;
}

CScriptSteam::~CScriptSteam() {}

void CScriptSteam::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Deactivate:
    for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
      mgr.Player(i)->SetVisorSteam(0.f, mAlphaInDuration, mAlphaOutDuration, kInvalidAssetId);
    }
    break;
  default:
    break;
  }

  CScriptTrigger::AcceptScriptMsg(mgr, msg);
}

void CScriptSteam::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  CScriptTrigger::Think(dt, mgr);
  for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
    CPlayer* player = mgr.Player(i);
    if (GetPlayerInside(i) &&
        mgr.GetCameraManager(i)->GetCurrentCamera(mgr, true)->GetFluidCount() == 0) {
      const CVector3f eyePosition = player->GetEyePosition();
      const float distance = (GetTranslation() - eyePosition).Magnitude();
      const float alpha =
          distance >= mMaxDistance
              ? 0.f
              : CMath::FastCosR(distance * 1.5707964f * mInverseMaxDistance) * GetStrength();
      player->SetVisorSteam(alpha, mAlphaInDuration, mAlphaOutDuration, mTexture);
      if (mEnableSplash) {
        mgr.EnvFxManager()->SetSplashRate(2.f * alpha);
      }
    } else {
      player->SetVisorSteam(0.f, mAlphaInDuration, mAlphaOutDuration, kInvalidAssetId);
    }
  }
}

SLdrSteam::SLdrSteam() : editorProperties(), trigger(), steam(kInvalidAssetId) {
  editorProperties.transform.scale = CVector3f(1.f, 1.f, 1.f);
  strength = 0.34999999f;
  fadeInRate = 1.0f;
  fadeOutRate = 2.0f;
  radius = 0.0f;
  unknown_0xa366c949 = false;
}

CEntity* LoadSteam(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrSteam sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrSteam.inc"

  const CVector3f& halfExtent = 0.5f * sldrThis.editorProperties.transform.scale;
  const CVector3f negHalfExtent = -(sldrThis.editorProperties.transform.scale * 0.5f);
  const CAABox bounds = CAABox(negHalfExtent, halfExtent);
  return rs_new CScriptSteam(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                             LdrToEntityInfo(info, sldrThis.editorProperties),
                             sldrThis.editorProperties.transform.position, bounds,
                             LdrToDamageInfo(sldrThis.trigger.damage), sldrThis.trigger.forceField,
                             sldrThis.trigger.flagsTrigger, sldrThis.steam, sldrThis.strength,
                             sldrThis.fadeInRate, sldrThis.fadeOutRate, sldrThis.radius,
                             sldrThis.unknown_0xa366c949);
}
