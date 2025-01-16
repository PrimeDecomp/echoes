#include "MetroidPrime/ScriptObjects/CScriptSpawnPoint.hpp"

#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "rstl/math.hpp"

CScriptSpawnPoint::CScriptSpawnPoint(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
    const rstl::reserved_vector< int, int(CPlayerState::kIT_Max) >& amountForItem,
    const rstl::reserved_vector< int, int(CPlayerState::kIT_Max) >& capacityForItem,
    bool firstSpawn, bool isMorphed)
: CEntity(uid, info, name, 0)
, m_xf(xf)
, m_amountForItem(amountForItem)
, m_capacityForItem(capacityForItem)
, m_firstSpawn(firstSpawn)
, m_morphed(isMorphed) {}

CScriptSpawnPoint::~CScriptSpawnPoint() {}

const CTransform4f& CScriptSpawnPoint::GetTransform() const { return m_xf; }

int CScriptSpawnPoint::GetItemAmount(const CPlayerState::EItemType& type) const {
  if (CPlayerState::kIT_Max <= type || type < 0) {
    return m_amountForItem.front();
  }
  return m_amountForItem[type];
}

int CScriptSpawnPoint::GetItemCapacity(const CPlayerState::EItemType& type) const {
  if (CPlayerState::kIT_Max <= type || type < 0) {
    return m_amountForItem.front();
  }
  int amount = m_amountForItem[type];
  int capacity = m_capacityForItem[type];
  if (amount < capacity) {
    return capacity;
  }
  return amount;
}

void CScriptSpawnPoint::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CEntity::AcceptScriptMsg(mgr, msg);

  switch (msg.GetMessage()) {
  case kSM_Reset:
    for (int playerIndex = 0; playerIndex < mgr.GetNumPlayers(); ++playerIndex) {
      for (int i = 0; i < CPlayerState::kIT_Max; ++i) {
        const CPlayerState::EItemType e = CPlayerState::EItemType(i);
        mgr.PlayerState(playerIndex)->ReInitializePowerUp(e, GetItemCapacity(e));
        mgr.PlayerState(playerIndex)->ResetAndIncrPickUp(e, GetItemAmount(e));
      }
    }
  case kSM_SetToZero:
    if (GetActive()) {
      for (int playerIndex = 0; playerIndex < mgr.GetNumPlayers(); ++playerIndex) {
        CPlayer* player = mgr.Player(playerIndex);
        TAreaId thisAreaId = GetCurrentAreaId();
        TAreaId nextAreaId = mgr.GetNextAreaId();

        if (thisAreaId != nextAreaId) {
          bool propagateAgain = false;

          CGameArea* area = mgr.World()->Area(thisAreaId);
          CGameArea::EOcclusionState occlusionState =
              area->IsLoaded() ? area->GetOcclusionState() : CGameArea::kOS_Occluded;

          if (occlusionState == CGameArea::kOS_Occluded) {
            while (!area->TryTakingOutOfARAM()) {
            }
            CWorld::PropogateAreaChain(CGameArea::kOS_Visible, area, mgr.World());
            propagateAgain = true;
          }

          mgr.SetCurrentAreaId(thisAreaId);
          mgr.SetActorAreaId(*player, thisAreaId);
          player->Teleport(m_xf, mgr, false);
          player->SetSpawnedMorphBallState(
              m_morphed ? CPlayer::kMS_Morphed : CPlayer::kMS_Unmorphed, mgr);

          if (propagateAgain) {
            CWorld::PropogateAreaChain(CGameArea::kOS_Occluded, mgr.World()->Area(nextAreaId),
                                       mgr.World());
          }

        } else {
          player->Teleport(m_xf, mgr, false);
          player->SetSpawnedMorphBallState(
              m_morphed ? CPlayer::kMS_Morphed : CPlayer::kMS_Unmorphed, mgr);
        }

        if (player->GetCameraManager()->IsInCinematicCamera()) {
          player->fn_80019E40(mgr, 0);
        } else {
          player->fn_80019E40(mgr, 1);
        }
      }
      CEntity::SendScriptMsgs(kSS_Zero, mgr, kInvalidUniqueId, kSM_None);
    }
  }
}
