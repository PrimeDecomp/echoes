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

CEntity* LoadCameraBlurKeyframe(CStateManager& mgr, CInputStream& input, const CEntityInfo& info) {
  SLdrCameraBlurKeyframe sldrThis;
  sldrThis.blurType = 0;
  sldrThis.blurRadius = 0.f;
  sldrThis.whichFilterGroup = 0;
  sldrThis.interpolateInTime = 0.f;
  sldrThis.interpolateOutTime = 0.f;

  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    const int propertyId = input.Get< int >();
    const u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x255a4580:
      LoadTypedefSLdrEditorProperties(sldrThis.editorProperties, input);
      break;
    case 0xe9359148:
      sldrThis.blurType = input.ReadInt32();
      break;
    case 0x6f6eb1f4:
      sldrThis.blurRadius = input.ReadFloat();
      break;
    case 0x3fdc4b2e:
      sldrThis.whichFilterGroup = input.ReadInt32();
      break;
    case 0xabd41a36:
      sldrThis.interpolateInTime = input.ReadFloat();
      break;
    case 0x3eaf78fe:
      sldrThis.interpolateOutTime = input.ReadFloat();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }

  return rs_new CScriptCameraBlurKeyframe(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties),
      CCameraBlurPass::EBlurType(sldrThis.blurType), sldrThis.blurRadius,
      sldrThis.whichFilterGroup, sldrThis.interpolateInTime, sldrThis.interpolateOutTime);
}
