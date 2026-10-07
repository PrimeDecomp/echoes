#include "MetroidPrime/ScriptObjects/CScriptPointOfInterest.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPointOfInterest.hpp"

#include "Kyoto/Math/CAABox.hpp"

CScriptPointOfInterest::CScriptPointOfInterest(TUniqueId uid, const rstl::string& name,
                                               const CEntityInfo& info, const CTransform4f& xf,
                                               const CScannableParameters& scanParms, bool lookAt,
                                               float scanOffset)
: CActor(uid, name, info, 0, xf, CModelData::CModelDataNull(),
         CMaterialList(kMT_Orbit, kMT_Scannable), CActorParameters::None().Scannable(scanParms),
         kInvalidUniqueId)
, mScanOffset(scanOffset)
, mLookAt(lookAt) {}

CScriptPointOfInterest::~CScriptPointOfInterest() {}

void CScriptPointOfInterest::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CActor::AcceptScriptMsg(mgr, msg);
}

rstl::optional_object< CAABox > CScriptPointOfInterest::GetTouchBounds() const {
  return CAABox(GetTranslation(), GetTranslation());
}

void CScriptPointOfInterest::Render(const CStateManager&) const {}

void CScriptPointOfInterest::AddToRenderer(const CStateManager&) const {}

void CScriptPointOfInterest::Think(float dt, CStateManager& mgr) {
  for (int i = 0; i < uint(mgr.GetNumPlayers()); ++i) {
    SetValidTarget(i, mgr.GetPlayerState(i)->GetCurrentVisor() == CPlayerState::kPV_Scan);
  }
  CActor::Think(dt, mgr);
}

void CScriptPointOfInterest::PreRenderAllViewports(CStateManager& mgr) {
  const CVector3f origin = GetTranslation();
  const CVector3f extent(1.f, 1.f, 1.f);
  const CAABox bounds(origin - extent, origin + extent);
  SetOtherBounds(bounds);
  SetRenderBounds(bounds);
  UpdatePortalSystemState(mgr);
}

CEntity* LoadPointOfInterest(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrPointOfInterest sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrPointOfInterest.inc"

  return rs_new CScriptPointOfInterest(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      LdrToScannableParameters(sldrThis.scanInfo), sldrThis.lookAtPOI, sldrThis.scanOffset);
}
