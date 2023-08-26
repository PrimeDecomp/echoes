#include "MetroidPrime/CStateManager.hpp"

#include "MetroidPrime/CSaveGameScreen.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CGameState.hpp"


#include "Kyoto/Basics/RAssertDolphin.hpp"

struct queryOutput {
  int* unk0;
  int unk4;
};
void fn_80041518(queryOutput&, MapWorldInfoAreas& mapWorldInfoAreas, ushort ourIndex);

CStateManager::CStateManager(const rstl::ncrc_ptr< CScriptMailbox >&,
                             const rstl::ncrc_ptr< CMapWorldInfo >&,
                             const rstl::ncrc_ptr< CPlayerState >&,
                             const rstl::ncrc_ptr< CWorldTransManager >&)
: m_nextFreeIndex(0), m_bossId(kInvalidUniqueId), m_uid_setBySpecialFunc(kInvalidUniqueId) {}

TUniqueId CStateManager::AllocateUniqueId() {

  const ushort lastIndex = m_nextFreeIndex;
  ushort ourIndex;
  queryOutput query;
  do {
    ourIndex = m_nextFreeIndex;
    m_nextFreeIndex = (ourIndex + 1) % 1024;
    if (m_nextFreeIndex == lastIndex) {
      rs_debugger_printf("Object list full!");
    }
    fn_80041518(query, mapWorldInfoAreas, ourIndex);
  } while ((query.unk4 & *query.unk0) != 0);

  m_objectIndexArray[ourIndex] = (m_objectIndexArray[ourIndex] + 1) & 0x3f;
  if (TUniqueId(m_objectIndexArray[ourIndex], ourIndex) == kInvalidUniqueId) {
    m_objectIndexArray[ourIndex] = 0;
  }

  fn_80041518(query, mapWorldInfoAreas, ourIndex);
  *query.unk0 = *query.unk0 | query.unk4;

  return TUniqueId(m_objectIndexArray[ourIndex], ourIndex);
}

const CEntity* CStateManager::GetObjectById(TUniqueId uid) const {
  return GetObjectListById(kOL_All).fn_8000B538(uid);
}

void CStateManager::SetIsDarkWorld(bool b) {
  m_isDarkWorld = b;
  gpGameState->SetIsDarkWorld(m_isDarkWorld);
}

void CStateManager::DeferStateTransition(EStateManagerTransition t) {
  if (!fn_80036F10()) {
    if (t == kSMT_InGame) {
      if (m_deferredTransition != kSMT_InGame) {
        m_world->SetLoadPauseState(false);
        m_deferredTransition = kSMT_InGame;
      }
    } else if (m_deferredTransition == kSMT_InGame) {
      m_world->SetLoadPauseState(true);
      m_deferredTransition = t;
      if (m_deferredTransition == kSMT_Unk) {
        m_saveGameScreen =
            new CSaveGameScreen(1, gpGameState->GetCardSerialB(), gpGameState->GetCardSerialA());
      }
    }
  }
}

void CStateManager::ShowPausedHUDMemo(CAssetId strg, float time) {
  m_hudMessageTime = time;
  m_pauseHudMessage = strg;
  DeferStateTransition(kSMT_MessageScreen);
}

void CStateManager::SendScriptMsg_fn_80037100(const CScriptMsg& msg) {
  m_scriptMsgs.Append(msg);
  int v = m_scriptMsgs.fn_8019E69C();
  if (0x80 < v && !m_unkFlagB3) {
    m_unkFlagB3 = true;
    fn_8003BE54();
    m_unkFlagB3 = false;
  }
}

uint CStateManager::MaskUIdNumPlayers(TUniqueId id) const {
  // TODO
  return id.Value() & m_numPlayers;
}
