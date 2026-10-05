#include "MetroidPrime/ScriptObjects/CScriptCameraShaker.hpp"

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CCameraShakerManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrCameraShaker.hpp"
#include "MetroidPrime/TCastTo.hpp"

CScriptCameraShaker::CScriptCameraShaker(TUniqueId uid, const rstl::string& name,
                                         const CEntityInfo& info,
                                         const CCameraShakerData& shakeData)
: CEntity(uid, info, name, 0), mShakeData(shakeData) {
  for (int i = 0; i < 4; ++i) {
    mPlayerShakeIds[i] = -1;
  }
}

void CScriptCameraShaker::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  if (msg.GetMessage() == kSM_Action) {
    if (mShakeData.GetFlags() & 0x80) {
      const CActor* actor = nullptr;
      if (msg.GetOriginator() != kInvalidUniqueId) {
        actor = TCastToConstPtr< CActor >(mgr.GetObjectById(msg.GetOriginator()));
      }
      if (!actor) {
        actor = TCastToConstPtr< CActor >(mgr.GetObjectById(msg.GetUnk()));
      }
      if (actor) {
        mShakeData.SetPosition(actor->GetTranslation());
      }
    }
    if (GetActive() && GetCurrentAreaId() != kInvalidAreaId &&
        mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()).GetOcclusionState() !=
            CGameArea::kOS_Occluded) {
      if (mShakeData.GetFlags() & 2) {
        for (uint i = 0; i < uint(mgr.GetNumPlayers()); ++i) {
          mPlayerShakeIds[i] = mgr.CameraManager(i)->CameraShakerManager()->AddCameraShaker(
              mShakeData, mgr, true, true);
        }
      } else if (CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(msg.GetOriginator()))) {
        const int managerIndex = mgr.MaskUIdNumPlayers(player->GetUniqueId());
        mPlayerShakeIds[player->GetPlayerIndex()] =
            mgr.CameraManager(managerIndex)->CameraShakerManager()->AddCameraShaker(
                mShakeData, mgr, true, true);
      }
    }
  } else if (msg.GetMessage() == kSM_Stop) {
    if (mShakeData.GetFlags() & 2) {
      for (uint i = 0; i < uint(mgr.GetNumPlayers()); ++i) {
        mgr.CameraManager(i)->CameraShakerManager()->RemoveCameraShaker(mPlayerShakeIds[i]);
      }
    } else if (CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(msg.GetOriginator()))) {
      const int managerIndex = mgr.MaskUIdNumPlayers(player->GetUniqueId());
      mgr.CameraManager(managerIndex)->CameraShakerManager()->RemoveCameraShaker(
          mPlayerShakeIds[player->GetPlayerIndex()]);
    }
  }
  CEntity::AcceptScriptMsg(mgr, msg);
}

CEntity* LoadCameraShaker(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrCameraShaker sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrCameraShaker.inc"
  return rs_new CScriptCameraShaker(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties),
      LdrToCameraShakerData(sldrThis.shakerData, sldrThis.editorProperties.transform.position));
}

CScriptCameraShaker::~CScriptCameraShaker() {}
