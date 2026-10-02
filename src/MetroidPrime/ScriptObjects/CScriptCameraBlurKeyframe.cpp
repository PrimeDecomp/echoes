#include "MetroidPrime/ScriptObjects/CScriptCameraBlurKeyframe.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrCameraBlurKeyframe.hpp"

CScriptCameraBlurKeyframe::CScriptCameraBlurKeyframe(TUniqueId uid, const rstl::string& name,
                                                     const CEntityInfo& info,
                                                     CCameraBlurPass::EBlurType type, float amount,
                                                     int filterGroup, float timeIn, float timeOut)
: CEntity(uid, info, name, 0)
, mType(type)
, mAmount(amount)
, mFilterGroup(filterGroup)
, mTimeIn(timeIn)
, mTimeOut(timeOut) {}

CScriptCameraBlurKeyframe::~CScriptCameraBlurKeyframe() {}

void CScriptCameraBlurKeyframe::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CEntity::AcceptScriptMsg(mgr, msg);

  switch (message) {
  case kSM_Increment:
    if (GetActive()) {
      const CCameraBlurPass::EBlurType type = mType;
      for (int i = 0; i < 4; ++i) {
        mgr.CameraBlurPass(i, 0).SetBlur(type, mAmount, mTimeIn, false);
      }
    }
    break;
  case kSM_Decrement:
    if (GetActive()) {
      for (int i = 0; i < 4; ++i) {
        mgr.CameraBlurPass(i, 0).DisableBlur(mTimeOut);
      }
    }
    break;
  case kSM_Deactivate:
  case kSM_XDelete:
    if (GetActive()) {
      for (int i = 0; i < 4; ++i) {
        mgr.CameraBlurPass(i, 0).DisableBlur(0.f);
      }
    }
    break;
  default:
    break;
  }
}

CEntity* LoadCameraBlurKeyframe(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrCameraBlurKeyframe sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrCameraBlurKeyframe.inc"

  return rs_new CScriptCameraBlurKeyframe(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties),
      CCameraBlurPass::EBlurType(sldrThis.blurType), sldrThis.blurRadius,
      sldrThis.whichFilterGroup, sldrThis.interpolateInTime, sldrThis.interpolateOutTime);
}
