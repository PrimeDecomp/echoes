#include "MetroidPrime/ScriptObjects/CScriptCameraFilterKeyframe.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrCameraFilterKeyframe.hpp"

CScriptCameraFilterKeyframe::CScriptCameraFilterKeyframe(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
    CCameraFilterPass::EFilterType type, CCameraFilterPass::EFilterShape shape, int filterStage,
    int filterGroup, float colorR, float colorG, float colorB, float colorA, float timeIn,
    float timeOut, CAssetId txtr)
: CEntity(uid, info, name, 0)
, mType(type)
, mShape(shape)
, mFilterStage(filterStage)
, mFilterGroup(filterGroup)
, mColor(colorR, colorG, colorB, colorA)
, mTimeIn(timeIn)
, mTimeOut(timeOut)
, mTxtr(txtr) {}

CScriptCameraFilterKeyframe::~CScriptCameraFilterKeyframe() {}

void CScriptCameraFilterKeyframe::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Increment:
    if (GetActive()) {
      const int stage = mFilterStage;
      const CCameraFilterPass::EFilterType type = mType;
      const CCameraFilterPass::EFilterShape shape = mShape;
      for (uint i = 0; i < mgr.GetNumPlayers(); ++i) {
        mgr.CameraFilterPass(i, stage).SetFilter(type, shape, mTimeIn, mColor, mTxtr);
      }
    }
    break;
  case kSM_Decrement:
    if (GetActive()) {
      const int stage = mFilterStage;
      for (uint i = 0; i < mgr.GetNumPlayers(); ++i) {
        mgr.CameraFilterPass(i, stage).DisableFilter(mTimeOut);
      }
    }
    break;
  case kSM_SetToMax:
    if (GetActive()) {
      const int stage = mFilterStage;
      const CCameraFilterPass::EFilterType type = mType;
      const CCameraFilterPass::EFilterShape shape = mShape;
      for (uint i = 0; i < mgr.GetNumPlayers(); ++i) {
        mgr.CameraFilterPass(i, stage).SetFilter(type, shape, 0.f, mColor, mTxtr);
      }
    }
    break;
  case kSM_Deactivate:
  case kSM_SetToZero:
    if (GetActive()) {
      const int stage = mFilterStage;
      for (uint i = 0; i < mgr.GetNumPlayers(); ++i) {
        mgr.CameraFilterPass(i, stage).DisableFilter(0.f);
      }
    }
    break;
  default:
    break;
  }

  CEntity::AcceptScriptMsg(mgr, msg);
}

CEntity* LoadCameraFilterKeyframe(CStateManager& mgr, CInputStream& input,
                                  const CEntityInfo& info) {
  SLdrCameraFilterKeyframe sldrThis;
  sldrThis.filterType = 0;
  sldrThis.filterShape = 0;
  sldrThis.filterStage = 0;
  sldrThis.whichFilterGroup = 0;
  sldrThis.color = CColor(1.f, 1.f, 1.f, 1.f);
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
    case 0x7975db5b:
      sldrThis.filterType = input.ReadInt32();
      break;
    case 0x6a3e9a3d:
      sldrThis.filterShape = input.ReadInt32();
      break;
    case 0x58bdbd7b:
      sldrThis.filterStage = input.ReadInt32();
      break;
    case 0x3fdc4b2e:
      sldrThis.whichFilterGroup = input.ReadInt32();
      break;
    case 0x37c7d09d:
      sldrThis.color = CColor(input);
      break;
    case 0xabd41a36:
      sldrThis.interpolateInTime = input.ReadFloat();
      break;
    case 0x3eaf78fe:
      sldrThis.interpolateOutTime = input.ReadFloat();
      break;
    case 0xd1f65872:
      sldrThis.texture = input.ReadInt32();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }

  return rs_new CScriptCameraFilterKeyframe(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties),
      CCameraFilterPass::EFilterType(sldrThis.filterType),
      CCameraFilterPass::EFilterShape(sldrThis.filterShape), sldrThis.filterStage,
      sldrThis.whichFilterGroup, sldrThis.color.GetRed(), sldrThis.color.GetGreen(),
      sldrThis.color.GetBlue(), sldrThis.color.GetAlpha(), sldrThis.interpolateInTime,
      sldrThis.interpolateOutTime, sldrThis.texture);
}
