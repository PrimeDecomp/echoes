#include "MetroidPrime/ScriptObjects/CScriptLayerController.hpp"

#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CMemoryCard.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/CWorldLayerState.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CWorldState.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrScriptLayerController.hpp"

CScriptLayerController::CScriptLayerController(TUniqueId uid, const rstl::string& name,
                                               const CEntityInfo& info, uint areaSaveId, int layer,
                                               bool isDynamic)
: CEntity(uid, info, name, 0)
, mAreaSaveId(areaSaveId)
, mLayerId(layer)
, mIsDynamic(isDynamic)
, mWaitingForLoad(false)
, mActivateWhenLoaded(false) {}

void CScriptLayerController::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CEntity::AcceptScriptMsg(mgr, msg);
  if (!GetActive()) {
    return;
  }

  switch (msg.GetMessage()) {
  case kSM_Load:
  case kSM_Increment:
  case kSM_Unload:
  case kSM_Decrement: {
    if (mAreaSaveId == 0xffffffff || mLayerId.Value() == -1) {
      break;
    }

    const bool active = msg.GetMessage() == kSM_Load || msg.GetMessage() == kSM_Increment;
    CWorldLayerState* layers = nullptr;
    const TAreaId areaId = GetAreaIdAndWorldLayerState(mgr, &layers);
    if (areaId == kInvalidAreaId) {
      break;
    }

    const TLayerId layer = mLayerId;
    layers->SetLayerActive(areaId, layer, active);
    if (mIsDynamic) {
      CGameArea* area = GetAreaForAreaId(mgr, areaId);
      if (area != nullptr) {
        const CGameArea::ELayerPhase phase = area->GetLayerPhase(layer);
        if (active) {
          area->LoadLayerDynamic(mgr, layer);
          if (phase != CGameArea::kLP_Ready && phase != CGameArea::kLP_Active) {
            mWaitingForLoad = true;
          }
        } else {
          area->UnloadLayerDynamic(mgr, layer);
          mWaitingForLoad = false;
        }
      }
    }
    break;
  }
  case kSM_Play:
    if (mIsDynamic) {
      CWorldLayerState* layers = nullptr;
      CGameArea* area = GetAreaForAreaId(mgr, GetAreaIdAndWorldLayerState(mgr, &layers));
      if (area != nullptr) {
        const TLayerId layer = mLayerId;
        const CGameArea::ELayerPhase phase = area->GetLayerPhase(layer);
        if (phase == CGameArea::kLP_Ready) {
          area->ActivateLayerDynamic(mgr, layer);
        } else if (phase == CGameArea::kLP_RestartPending || phase == CGameArea::kLP_Loading) {
          mgr.mUnkFlagB4 = true;
          mActivateWhenLoaded = true;
        }
      }
    }
    break;
  default:
    break;
  }
}

void CScriptLayerController::Think(float dt, CStateManager& mgr) {
  if (GetActive() && (mWaitingForLoad || mActivateWhenLoaded)) {
    CWorldLayerState* layers = nullptr;
    const TLayerId layer = mLayerId;
    CGameArea* area = GetAreaForAreaId(mgr, GetAreaIdAndWorldLayerState(mgr, &layers));
    if (area != nullptr && area->GetLayerPhase(layer) == CGameArea::kLP_Ready) {
      if (mWaitingForLoad) {
        mWaitingForLoad = false;
        SendScriptMsgs(kSS_Arrived, mgr, kSM_None);
      }
      if (mActivateWhenLoaded) {
        mActivateWhenLoaded = false;
        area->ActivateLayerDynamic(mgr, layer);
      }
    }
  }

  CEntity::Think(dt, mgr);
}

TAreaId CScriptLayerController::GetAreaIdAndWorldLayerState(CStateManager& mgr,
                                                            CWorldLayerState** layers) {
  const TAreaId area = mgr.World()->GetAreaIdForSaveId(mAreaSaveId);
  if (area != kInvalidAreaId) {
    if (layers != nullptr) {
      *layers = mgr.mCurrentWorldLayerState.GetPtr();
    }
    return area;
  }

  const rstl::pair< CAssetId, TAreaId > worldAndArea =
      gpMemoryCard->GetAreaAndWorldIdForSaveId(mAreaSaveId);
  if (worldAndArea.first == kInvalidAssetId) {
    return kInvalidAreaId;
  }

  CWorldState& worldState = gpGameState->StateForWorld(worldAndArea.first);
  if (layers != nullptr) {
    *layers = worldState.GetLayerState().GetPtr();
  }
  return worldAndArea.second;
}

CGameArea* CScriptLayerController::GetAreaForAreaId(CStateManager& mgr, TAreaId area) {
  if (area != kInvalidAreaId && mgr.World()->DoesAreaExist(area) &&
      mgr.World()->IsAreaValid(area)) {
    return mgr.World()->Area(area);
  }
  return nullptr;
}

CEntity* LoadScriptLayerController(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  // TODO: the generated master-area default is zero; the native loader uses -1.
  SLdrScriptLayerController sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrScriptLayerController.inc"
  return rs_new CScriptLayerController(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                                       LdrToEntityInfo(info, sldrThis.editorProperties),
                                       sldrThis.masterLayer.areaID, sldrThis.masterLayer.layer,
                                       sldrThis.isDynamic);
}

CScriptLayerController::~CScriptLayerController() {}
