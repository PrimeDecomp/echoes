#include "MetroidPrime/ScriptObjects/CScriptDistanceFog.hpp"

#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDistanceFog.hpp"
#include "MetroidPrime/ScriptLoader/SLdrWorldLightFader.hpp"

#include "Kyoto/Math/CloseEnough.hpp"

CScriptDistanceFog::CScriptDistanceFog(TUniqueId uid, const rstl::string& name,
                                       const CEntityInfo& info, ERglFogMode mode,
                                       const CColor& color, const CVector2f& range,
                                       float colorDelta, CVector2f rangeDelta, float lightTarget,
                                       float lightSpeed, bool explicitFog)
: CEntity(uid, info, name, 0)
, mMode(mode)
, mColor(color)
, mRange(range)
, mColorDelta(colorDelta)
, mRangeDelta(rangeDelta)
, mLightTarget(lightTarget)
, mLightSpeed(lightSpeed)
, mExplicit(explicitFog)
, mNonZero(!close_enough(rangeDelta, CVector2f(0.f, 0.f)) || !close_enough(colorDelta, 0.f)) {}

CScriptDistanceFog::~CScriptDistanceFog() {}

void CScriptDistanceFog::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CEntity::AcceptScriptMsg(mgr, msg);

  if (GetCurrentAreaId() != kInvalidAreaId && GetActive()) {
    switch (message) {
    case kSM_XALD:
      if (mExplicit) {
        const TAreaId aid = GetCurrentAreaId();
        CGameArea::CAreaFog* fog = mgr.World()->Area(aid)->GetPostConstructed()->mAreaFog.get();
        if (mMode == kRFM_None) {
          fog->DisableFog();
        } else {
          fog->SetFogExplicit(mMode, mColor, mRange);
        }
      }
      break;
    case kSM_Action:
      if (mNonZero) {
        const TAreaId aid = GetCurrentAreaId();
        CGameArea::CAreaFog* fog = mgr.World()->Area(aid)->GetPostConstructed()->mAreaFog.get();
        if (mMode != kRFM_None) {
          fog->FadeFog(mMode, mColor, mRange, mColorDelta, mRangeDelta);
        } else {
          fog->RollFogOut(mRangeDelta.GetX(), mColorDelta, mColor);
        }
      }
      if (!close_enough(mLightSpeed, 0.f)) {
        const TAreaId aid = GetCurrentAreaId();
        mgr.World()->Area(aid)->SetXRaySpeedAndTarget(mLightSpeed, mLightTarget);
      }
      break;
    default:
      break;
    }
  }
}

CEntity* LoadDistanceFog(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrDistanceFog sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrDistanceFog.inc"

  return rs_new CScriptDistanceFog(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), FogSelectionToFogMode(sldrThis.mode),
      sldrThis.color, LdrToVector2f(sldrThis.nearFarPlane), sldrThis.colorRate,
      LdrToVector2f(sldrThis.distanceRate), 0.f, 0.f, sldrThis.forceSettings);
}

CEntity* LoadWorldLightFader(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrWorldLightFader sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrWorldLightFader.inc"

  return rs_new CScriptDistanceFog(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                                   LdrToEntityInfo(info, sldrThis.editorProperties), kRFM_None,
                                   CColor::Black(), CVector2f(0.f, 0.f), 0.f, CVector2f(0.f, 0.f),
                                   sldrThis.targetLight, sldrThis.targetLightRate, false);
}
