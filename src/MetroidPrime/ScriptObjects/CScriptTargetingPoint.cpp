#include "MetroidPrime/ScriptObjects/CScriptTargetingPoint.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTargetingPoint.hpp"

CScriptTargetingPoint::CScriptTargetingPoint(TUniqueId uid, const rstl::string& name,
                                             const CEntityInfo& info, const CTransform4f& xf)
: CActor(uid, name, info, 0, xf, CModelData::CModelDataNull(), CMaterialList(kMT_NoStepLogic),
         CActorParameters::None(), kInvalidUniqueId)
, mUnknownFlag(false)
, mUnknownId(kInvalidUniqueId)
, mTime(0.f) {}

void CScriptTargetingPoint::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CActor::AcceptScriptMsg(mgr, msg);

  switch (message) {
  case kSM_Deactivate:
  case kSM_Activate:
    SendScriptMsgs(kSS_Attack, mgr, kSM_None);
    break;
  default:
    break;
  }
}

bool CScriptTargetingPoint::GetLocked() const { return GetConnectionList().size() > 0; }

void CScriptTargetingPoint::Think(float dt, CStateManager&) {
  if (mTime > 0.f) {
    mTime -= dt;
  }
}

void CScriptTargetingPoint::AddToRenderer(const CStateManager&) const {}

void CScriptTargetingPoint::Render(const CStateManager&) const {}

CEntity* LoadTargetingPoint(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrTargetingPoint sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrTargetingPoint.inc"
  return rs_new CScriptTargetingPoint(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                                      LdrToEntityInfo(info, sldrThis.editorProperties),
                                      LdrToTransform4f(sldrThis.editorProperties));
}
