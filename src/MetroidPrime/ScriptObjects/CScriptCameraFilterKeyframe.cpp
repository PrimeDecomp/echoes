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
      for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
        mgr.CameraFilterPass(i, stage).SetFilter(type, shape, mTimeIn, mColor, mTxtr);
      }
    }
    break;
  case kSM_Decrement:
    if (GetActive()) {
      const int stage = mFilterStage;
      for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
        mgr.CameraFilterPass(i, stage).DisableFilter(mTimeOut);
      }
    }
    break;
  case kSM_SetToMax:
    if (GetActive()) {
      const int stage = mFilterStage;
      const CCameraFilterPass::EFilterType type = mType;
      const CCameraFilterPass::EFilterShape shape = mShape;
      for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
        mgr.CameraFilterPass(i, stage).SetFilter(type, shape, 0.f, mColor, mTxtr);
      }
    }
    break;
  case kSM_Deactivate:
  case kSM_SetToZero:
    if (GetActive()) {
      const int stage = mFilterStage;
      for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
        mgr.CameraFilterPass(i, stage).DisableFilter(0.f);
      }
    }
    break;
  default:
    break;
  }

  CEntity::AcceptScriptMsg(mgr, msg);
}

CEntity* LoadCameraFilterKeyframe(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrCameraFilterKeyframe sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrCameraFilterKeyframe.inc"

  return rs_new CScriptCameraFilterKeyframe(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties),
      CCameraFilterPass::EFilterType(sldrThis.filterType),
      CCameraFilterPass::EFilterShape(sldrThis.filterShape), sldrThis.filterStage,
      sldrThis.whichFilterGroup, sldrThis.color.GetRed(), sldrThis.color.GetGreen(),
      sldrThis.color.GetBlue(), sldrThis.color.GetAlpha(), sldrThis.interpolateInTime,
      sldrThis.interpolateOutTime, sldrThis.texture);
}
