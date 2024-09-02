#ifndef _CSTATEMANAGER
#define _CSTATEMANAGER

#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "TGameTypes.hpp"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Input/CFinalInput.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/map.hpp"
#include "rstl/pair.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

class CWorld;
class CArchitectureQueue;
class CEnvFxManager;
class CEntity;
class CActor;
class CScriptMailbox;
class CMapWorldInfo;
class CPlayerState;
class CWorldTransManager;
class CPlayer;
class CCameraManager;
class CRumbleManager;
class CSaveGameScreen;
class CActorModelParticles;
class CRelayTracker;
class CWorldLayerState;
class CStateManagerContainer;
class CSortedListManager;
class CWeaponMgr;
class CFluidPlaneManager;

struct MapWorldInfoAreas {};

enum EStateManagerTransition {
  kSMT_InGame,
  kSMT_MapScreen,
  kSMT_PauseGame,
  kSMT_LogBook,
  kSMT_SaveGame,
  kSMT_Unk,
  kSMT_MessageScreen
};

class CStateManager {
  struct ScriptMsgArray {
    CScriptMsg msgs[192];
    int lastIndex;
    int otherIndex;

    void Append(const CScriptMsg& msg);
    int fn_8019E69C();
    const CScriptMsg& fn_8019E6BC() const;

    bool empty() const { return lastIndex == otherIndex; }
  };

public:
  typedef rstl::map< TEditorId, TUniqueId > TIdList;
  typedef rstl::pair< TIdList::const_iterator, TIdList::const_iterator > TIdListResult;

  CStateManager(const rstl::ncrc_ptr< CScriptMailbox >&, const rstl::ncrc_ptr< CMapWorldInfo >&,
                const rstl::ncrc_ptr< CPlayerState >&, const rstl::ncrc_ptr< CWorldTransManager >&);
  ~CStateManager();

  TUniqueId AllocateUniqueId();
  uint MaskUIdNumPlayers(TUniqueId id) const;
  void ShowPausedHUDMemo(CAssetId strg, float time);
  void SetIsDarkWorld(bool);

  void SendScriptMsg_fn_80037100(const CScriptMsg&);

  void AddObject(CEntity*);
  void DeleteObjectRequest(TUniqueId);

  CEntity* ObjectById(TUniqueId uid);
  const CEntity* GetObjectById(TUniqueId uid) const;
  CEntity* GetObjectByIdFromListAll(TUniqueId uid);

  TEditorId GetEditorIdForUniqueId(TUniqueId) const;
  TUniqueId GetIdForScript(TEditorId eid) const;
  TIdListResult GetIdListForScript(TEditorId) const;

  CWorld* World() { return m_world; }
  const CWorld* GetWorld() const { return m_world; }
  CEnvFxManager* EnvFxManager() { return m_envFxManager; }
  const CEnvFxManager* GetEnvFxManager() const { return m_envFxManager; }
  int GetUpdateFrameIdx() const { m_updateFrameIdx; }

  int GetNumPlayers() const { return m_numPlayers; }
  CPlayer* GetPlayer(int index) { return m_players[index]; }

  CObjectList& ObjectListById(EGameObjectList id) { return *m_objectLists[id]; }
  const CObjectList& GetObjectListById(EGameObjectList id) const { return *m_objectLists[id]; }

  void UpdateActorInSortedLists(CActor*);

  void fn_8003BF84(CEntity*);
  void fn_800412EC(TUniqueId);
  bool fn_80036F10() const; // Maybe_CheckIsMultiplayer
  void fn_8003BE54();

  // State transitions
  void DeferStateTransition(EStateManagerTransition t);
  void EnterMapScreen() { DeferStateTransition(kSMT_MapScreen); }
  void EnterPauseScreen() { DeferStateTransition(kSMT_PauseGame); }
  void EnterLogBookScreen() { DeferStateTransition(kSMT_LogBook); }
  void EnterSaveGameScreen() { DeferStateTransition(kSMT_SaveGame); }
  void EnterMessageScreen(uint, float);
  bool GetWantsToEnterMapScreen() const { return m_deferredTransition == kSMT_MapScreen; }
  bool GetWantsToEnterPauseScreen() const { return m_deferredTransition == kSMT_PauseGame; }
  bool GetWantsToEnterLogBookScreen() const { return m_deferredTransition == kSMT_LogBook; }
  bool GetWantsToEnterSaveGameScreen() const { return m_deferredTransition == kSMT_SaveGame; }
  bool GetWantsToEnterMessageScreen() const { return m_deferredTransition == kSMT_MessageScreen; }

  const CCameraManager* GetCameraManager(int playerIndex) const { return m_cameraManagers[playerIndex]; }

public:
  ushort m_nextFreeIndex;
  rstl::reserved_vector< ushort, 1024 > m_objectIndexArray;                // x0x4
  rstl::reserved_vector< rstl::auto_ptr< CObjectList >, 9 > m_objectLists; // 0x808
  MapWorldInfoAreas mapWorldInfoAreas;
  char pad1[0x94]; // 0x408
  ScriptMsgArray m_scriptMsgs;
  CArchitectureQueue* m_archQueue;
  int m_numPlayers;
  CPlayer* m_players[4];
  CPlayerState* m_playerStates[4];
  CCameraManager* m_cameraManagers[4];
  CRumbleManager* m_rumbleManagers[4];
  CFinalInput m_finalInputs[4];
  CPlayerState* m_playerState;
  CCameraManager* m_cameraManager;
  CWorld* m_world;                                                 // 0x1604
  rstl::list< rstl::reserved_vector< CEntity*, 32 > > m_graveyard; // 0x1608
  rstl::single_ptr< CStateManagerContainer > m_stateManagerContainer;
  CSortedListManager* m_sortedListManager;
  CWeaponMgr* m_weaponMgr;
  CFluidPlaneManager* m_fluidPlaneManager;
  CEnvFxManager* m_envFxManager;               // 0x1630
  CActorModelParticles* m_actorModelParticles; // 0x1634
  void* x1638;
  char pad2_2[0x48];
  rstl::rc_ptr< CRelayTracker > m_relayTracker;
  int x1684;
  int x1688;
  rstl::rc_ptr< CWorldTransManager > m_worldTransManager;
  CWorldLayerState* m_currentWorldLayerState;
  int* x1698;
  rstl::single_ptr< CSaveGameScreen > m_saveGameScreen; // x169C

  char pad3[0xC]; // 16A0
  int m_updateFrameIdx; // 16AC
  char pad4[0xD84]; // 16B0

  CAssetId m_pauseHudMessage; // 0x2434
  float x2438_escapeTotalTime;
  float x243c;
  TUniqueId m_bossId; // 0x2440
  float m_bossHealth;
  uint m_bossLanguageTableIndex;
  int x244c; // unk type
  TUniqueId m_uid_setBySpecialFunc;
  float m_hudMessageTime;     // 0x2454
  int x2458;                  // unk type
  int m_hudMessageFrameCount; // 0x245c
  int m_forPausedHudMemo;     // 0x2460
  CAssetId m_pausedHudMemoAssetId;
  float x2468;
  int x246c;
  EStateManagerTransition m_deferredTransition;
  char pad5[0x4C4]; // 0x246c

  CVector3f x2938;
  float x2944;
  CColor x2948;
  bool m_unkFlagA1 : 1;
  bool m_unkFlagA2 : 1;
  bool m_unkFlagA3 : 1;
  bool m_unkFlagA4 : 1;
  bool m_unkFlagA5 : 1;
  bool m_unkFlagA6 : 1;
  bool m_unkFlagA7 : 1;
  bool m_isDarkWorld : 1; // 0x294c
  bool m_unkFlagB1 : 1;
  bool m_unkFlagB2 : 1;
  bool m_unkFlagB3 : 1;
  bool m_unkFlagB4 : 1;
  bool m_unkFlagB5 : 1;
  bool m_unkFlagB6 : 1;
  bool m_unkFlagB7 : 1;
  bool m_unkFlagB8 : 1;
};
// CHECK_OFFSETOF(CStateManager, m_world, 0x1604)
// CHECK_OFFSETOF(CStateManager, m_envFxManager, 0x1630)

#endif // _CSTATEMANAGER
