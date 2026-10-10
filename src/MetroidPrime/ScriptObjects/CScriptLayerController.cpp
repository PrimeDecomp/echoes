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
                                               const bool isDynamic)
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
    if (mAreaSaveId == 0xffffffff || static_cast< uint >(mLayerId.Value()) == 0xffffffff) {
      break;
    }

    bool active = msg.GetMessage() == kSM_Load || msg.GetMessage() == kSM_Increment;
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
      const TAreaId areaId = GetAreaIdAndWorldLayerState(mgr, &layers);
      CGameArea* area = GetAreaForAreaId(mgr, areaId);
      if (area != nullptr) {
        const TLayerId layer = mLayerId;
        switch (area->GetLayerPhase(layer)) {
        case CGameArea::kLP_CancelPending:
          return;
        case CGameArea::kLP_Ready:
          area->ActivateLayerDynamic(mgr, layer);
          break;
        case CGameArea::kLP_RestartPending:
        case CGameArea::kLP_Loading:
          mgr.mLayerRestartPending = true;
          mActivateWhenLoaded = true;
          break;
        }
      }
    }
    break;
  case kSM_AreaLoaded:
    break;
  default:
    break;
  }
}

void CScriptLayerController::Think(float dt, CStateManager& mgr) {
  if (GetActive() && (mWaitingForLoad || mActivateWhenLoaded)) {
    CWorldLayerState* layers = nullptr;
    const TLayerId layer = mLayerId;
    const TAreaId areaId = GetAreaIdAndWorldLayerState(mgr, &layers);
    CGameArea* area = GetAreaForAreaId(mgr, areaId);
    if (area != nullptr && area->GetLayerPhase(layer) == CGameArea::kLP_Ready) {
      if (mWaitingForLoad) {
        mWaitingForLoad = false;
        SendScriptMsgs(kSS_Arrived, mgr);
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
  if (worldAndArea.first != kInvalidAssetId) {
    CWorldState& worldState = gpGameState->StateForWorld(worldAndArea.first);
    if (layers != nullptr) {
      *layers = worldState.GetLayerState().GetPtr();
    }
    return worldAndArea.second;
  }
  return kInvalidAreaId;
}

CGameArea* CScriptLayerController::GetAreaForAreaId(CStateManager& mgr, TAreaId area) {
  if (area.value != kInvalidAreaId.value && mgr.World()->DoesAreaExist(area) &&
      mgr.World()->IsAreaValid(area)) {
    return mgr.World()->Area(area);
  }
  return nullptr;
}

void LoadTypedefMasterLayer(SLdrMasterLayer& sldrThis, CInputStream& input) {
  sldrThis.areaID = input.ReadInt32();
  sldrThis.layer = input.ReadInt32();
}

CEntity* LoadScriptLayerController(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrScriptLayerController sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrScriptLayerController.inc"
  return rs_new CScriptLayerController(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                                       LdrToEntityInfo(info, sldrThis.editorProperties),
                                       sldrThis.masterLayer.areaID, sldrThis.masterLayer.layer,
                                       sldrThis.isDynamic);
}

CScriptLayerController::~CScriptLayerController() {}
