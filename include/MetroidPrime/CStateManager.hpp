#ifndef _CSTATEMANAGER
#define _CSTATEMANAGER

extern const int gkPVSEnabled;

#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "TGameTypes.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Input/CFinalInput.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/list.hpp"
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
class CMaterialFilter;
class CRayCastResult;
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
class CInputStream;
class CStateManager;

// Partial interface; class name corroborated by Echoes Wii exports.
class CScriptObjectLoaderHelper {
public:
  void LoadScriptObjects(TAreaId aid, CInputStream& in, rstl::vector< TEditorId >& ids,
                         CStateManager& mgr);                                 // Guessed name
  void InitScriptObjects(rstl::vector< TEditorId >& ids, CStateManager& mgr); // Guessed name
  void RemoveLayerObjects(TAreaId area, TLayerId layer, CStateManager& mgr);  // Guessed name
};
namespace SL {
class CSortedListManager;
}
class CWeaponMgr;
class CFluidPlaneManager;
class CDamageInfo;
class CAABox;

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
  CScriptObjectLoaderHelper& ScriptObjectLoaderHelper();
  uint MaskUIdNumPlayers(TUniqueId id) const;
  void SetIsDarkWorld(bool);
  bool GetIsDarkWorld() const { return m_isDarkWorld; }
  void SetMapTeleportWorldId(CAssetId id) { mMapTeleportWorldId = id; } // Guessed name
  void DisplayAlertAboutOutOfAmmo(const CPlayer&, CPlayerState::EItemType) const;
  rstl::pair< int, int > CalculateScanCompletionRate() const;

  //
  void ShowPausedHUDMemo(CAssetId strg, float time);
  void QueueMessage(int frameCount, CAssetId msg, float f1);
  int GetHUDMessageFrameCount() const { return mHudMessageFrameCount; }
  // float GetHUDMessageTime() const { return mHudMessageTime; }
  void IncrementHUDMessageFrameCounter() { ++mHudMessageFrameCount; }

  void SendScriptMsg_fn_80037100(const CScriptMsg&);
  void DeliverScriptMsg(const CScriptMsg& msg); // Guessed name
  void SendScriptMsg(CEntity*, TUniqueId, EScriptObjectMessage, TUniqueId);

  void AddObject(CEntity*);
  void DeleteObjectRequest(TUniqueId);
  void UpdateObjectInLists(CEntity&);
  void AddWeaponId(TUniqueId owner, EWeaponType type);
  void RemoveWeaponId(TUniqueId owner, EWeaponType type);
  void ApplyDamageToWorld(TUniqueId owner, CActor& projectile, const CVector3f& position,
                          const CDamageInfo& damage, const CMaterialFilter& filter);
  void DrawSpaceWarp(const CVector3f& position, float strength) const;

  bool AddDrawableActor(const CActor& actor, const CVector3f& pos, const CAABox& bounds) const;
  void SetupParticleHook(const CActor& actor) const;
  const CActorModelParticles* GetActorModelParticles() const { return m_actorModelParticles; }

  CEntity* ObjectById(TUniqueId uid);
  const CEntity* GetObjectById(TUniqueId uid) const;
  CEntity* GetObjectByIdFromListAll(TUniqueId uid);
  bool RayCollideWorld(const CVector3f& start, const CVector3f& end,
                       const CMaterialFilter& filter, const CActor* damagee);
  CRayCastResult RayStaticIntersection(const CVector3f& position, const CVector3f& direction,
                                       float length, const CMaterialFilter& filter) const;
  void BuildNearList(rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                     const CVector3f& position, const CVector3f& direction, float length,
                     const CMaterialFilter& filter, const CActor* ignoreActor) const;

  TEditorId GetEditorIdForUniqueId(TUniqueId) const;
  TUniqueId GetIdForScript(TEditorId eid) const;
  TIdListResult GetIdListForScript(TEditorId) const;

  CWorld* World() { return m_world; }
  const CWorld* GetWorld() const { return m_world; }
  CEnvFxManager* EnvFxManager() { return m_envFxManager; }
  const CEnvFxManager* GetEnvFxManager() const { return m_envFxManager; }
  CRandom16* Random() { return &mRandom; }
  int GetUpdateFrameIdx() const { return m_updateFrameIdx; }
  int GetRenderFrameIndex() const { return mRenderFrameIndex; } // Guessed name

  TAreaId GetNextAreaId() const { return m_nextAreaId; }
  void SetCurrentAreaId(TAreaId);
  void SetActorAreaId(CActor& actor, TAreaId);

  const CFrustumPlanes& GetFrustumPlanes() const { return m_planes; }
  int Get0x244c() const { return x244c; }

  int GetNumPlayers() const { return m_numPlayers; }
  CPlayer* GetPlayer(int index) { return m_players[index]; }
  const CPlayer* GetPlayer(int index) const { return m_players[index]; }
  CPlayer* Player(int index) { return m_players[index]; }

  CObjectList& ObjectListById(EGameObjectList id) { return *m_objectLists[id]; }
  const CObjectList& GetObjectListById(EGameObjectList id) const { return *m_objectLists[id]; }

  void UpdateActorInSortedLists(CActor*);

  bool ApplyLocalDamage(const CVector3f& pos, const CVector3f& dir, CActor& damagee, float damage, const TUniqueId& uid1, const TUniqueId& uid2, const CDamageInfo& info, int);

  void fn_8003dd88(CActor&, TUniqueId, const CDamageInfo& info, bool, int);
  void fn_8003BF84(CEntity*);
  void fn_800412EC(TUniqueId);
  bool fn_80036F10() const; // Maybe_CheckIsMultiplayer
  void fn_8003BE54();
  void fn_8003C4B8(const CVector3f&, int);

  // State transitions
  void DeferStateTransition(EStateManagerTransition t);
  void EnterMapScreen() { DeferStateTransition(kSMT_MapScreen); }
  void EnterPauseScreen() { DeferStateTransition(kSMT_PauseGame); }
  void EnterLogBookScreen() { DeferStateTransition(kSMT_LogBook); }
  void EnterSaveGameScreen() { DeferStateTransition(kSMT_SaveGame); }
  void EnterMessageScreen(uint, float);
  bool GetWantsToEnterMapScreen() const { return m_deferredTransition == kSMT_MapScreen; }
  bool GetWantsToEnterPauseScreen() const { return m_deferredTransition == kSMT_PauseGame; }
  void SetCinematicPause(bool paused) { mCinematicPause = paused; } // Guessed name
  bool GetWantsToEnterLogBookScreen() const { return m_deferredTransition == kSMT_LogBook; }
  bool GetWantsToEnterSaveGameScreen() const { return m_deferredTransition == kSMT_SaveGame; }
  bool GetWantsToEnterMessageScreen() const { return m_deferredTransition == kSMT_MessageScreen; }

  const CCameraManager* GetCameraManager(int playerIndex) const { return m_cameraManagers[playerIndex]; }
  CCameraManager* CameraManager(int playerIndex) { return m_cameraManagers[playerIndex]; }
  const CPlayerState* GetPlayerState() const { return m_playerState; }
  const CPlayer* GetCurrentRenderPlayer() const { return mCurrentRenderPlayer; } // Guessed name
  int GetCurrentRenderPlayerIndex() const { return mCurrentRenderPlayerIndex; } // Guessed name
  const CCameraManager* GetCurrentRenderCameraManager() const { return m_cameraManager; } // Guessed name
  const CPlayerState* GetPlayerState(int playerIndex) const { return m_playerStates[playerIndex]; }
  CPlayerState* PlayerState(int playerIndex) { return m_playerStates[playerIndex]; }
  CRumbleManager* RumbleManager(int playerIndex) { return m_rumbleManagers[playerIndex]; }

  int fn_800366e4(CActor*);

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
  char x15ec_[0xc];
  CPlayer* mCurrentRenderPlayer; // 0x15f8, guessed name
  CPlayerState* m_playerState;
  CCameraManager* m_cameraManager;
  CWorld* m_world;                                                 // 0x1604
  rstl::list< rstl::reserved_vector< CEntity*, 32 > > m_graveyard; // 0x1608
  rstl::single_ptr< CStateManagerContainer > m_stateManagerContainer;
  SL::CSortedListManager* m_sortedListManager;
  CWeaponMgr* m_weaponMgr;
  CFluidPlaneManager* m_fluidPlaneManager;
  CEnvFxManager* m_envFxManager;               // 0x1630
  CActorModelParticles* m_actorModelParticles; // 0x1634
  void* x1638;
  char pad2_2[0x48];
  rstl::rc_ptr< CRelayTracker > m_relayTracker;
  rstl::rc_ptr< CWorldTransManager > m_worldTransManager;
  CWorldLayerState* m_currentWorldLayerState;
  int* x1698;
  rstl::single_ptr< CSaveGameScreen > m_saveGameScreen; // x169C
  TAreaId m_nextAreaId; // x16a0
  char x16a4_[4];
  int mRenderFrameIndex; // Guessed name: visibility age used by projectile impacts.
  int m_updateFrameIdx; // 16AC
  char pad4[0x34]; // 16B0
  CRandom16 mRandom;
  char x16e8_[0xD4C];

  CAssetId m_pauseHudMessage; // 0x2434
  float mEscapeTotalTime;
  float x243c;
  TUniqueId m_bossId; // 0x2440
  float m_bossHealth;
  uint m_bossLanguageTableIndex;
  int x244c; // unk type
  TUniqueId m_uid_setBySpecialFunc;
  float m_hudMessageTime;     // 0x2454
  int x2458;                  // unk type
  int mHudMessageFrameCount; // 0x245c
  int m_forPausedHudMemo;     // 0x2460
  CAssetId m_pausedHudMemoAssetId;
  float x2468;
  CAssetId mMapTeleportWorldId; // Guessed name
  EStateManagerTransition m_deferredTransition;

  char pad5[4]; // 0x246c
  CFrustumPlanes m_planes; // 0x2478
  int mCurrentRenderPlayerIndex; // Guessed name
  char pad6[0x2938 - 0x24e0];

  CVector3f x2938;
  float x2944;
  CColor x2948;
  bool m_unkFlagA1 : 1;
  bool m_unkFlagA2 : 1;
  bool m_unkFlagA3 : 1;
  bool m_unkFlagA4 : 1;
  bool m_unkFlagA5 : 1;
  bool mCinematicPause : 1;
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
