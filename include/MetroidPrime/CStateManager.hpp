#ifndef _CSTATEMANAGER
#define _CSTATEMANAGER

extern const int gkPVSEnabled;

#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CFilteredObjectList.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CScriptObjectLoaderHelper.hpp"
#include "MetroidPrime/Enemies/EListenNoiseType.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "TGameTypes.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Input/CFinalInput.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/bit_vector.hpp"
#include "rstl/list.hpp"
#include "rstl/map.hpp"
#include "rstl/pair.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

class CWorld;
class CPortalTransition;
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

namespace SL {
class CSortedListManager;
}
class CWeaponMgr;
class CFluidPlaneManager;
class CDamageInfo;
class CAABox;

typedef rstl::bit_vector<> MapWorldInfoAreas;

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

  // Guessed phase names, derived from world initialization.
  enum EInitPhase { kIP_LoadAudioGroups, kIP_LoadWorld, kIP_LoadFirstArea, kIP_Done };

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

  void SendScriptMsg(const CScriptMsg& msg);
  void DeliverScriptMsg(const CScriptMsg& msg); // Guessed name
  void SendScriptMsg(CEntity*, TUniqueId, EScriptObjectMessage, TUniqueId);
  void SendScriptMsg(TUniqueId target, TUniqueId sender, EScriptObjectMessage message,
                     TUniqueId actor); // Guessed overload name.

  void AddObject(CEntity*);
  void DeleteObjectRequest(TUniqueId);
  void UpdateObjectInLists(CEntity&);
  void AddWeaponId(TUniqueId owner, EWeaponType type);
  int GetWeaponIdCount(TUniqueId owner, EWeaponType type);
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
  void BuildNearList(rstl::reserved_vector< TUniqueId, 1024 >& nearList, const CAABox& bounds,
                     const CMaterialFilter& filter, const CActor* ignoreActor) const;

  TEditorId GetEditorIdForUniqueId(TUniqueId) const;
  TUniqueId GetIdForScript(TEditorId eid) const;
  TIdListResult GetIdListForScript(TEditorId) const;

  CWorld* World() { return m_world; }
  const CWorld* GetWorld() const { return m_world; }
  bool IsFullyInitialized() const { return mInitPhase == kIP_Done; }
  CEnvFxManager* EnvFxManager() { return m_envFxManager; }
  const CEnvFxManager* GetEnvFxManager() const { return m_envFxManager; }
  CRandom16* Random() { return &mRandom; }
  int GetUpdateFrameIdx() const { return m_updateFrameIdx; }
  int GetRenderFrameIndex() const { return mRenderFrameIndex; } // Guessed name

  TAreaId GetNextAreaId() const { return m_nextAreaId; }
  TAreaId GetPreviousAreaId() const { return mPreviousAreaId; }
  void SetCurrentAreaId(TAreaId);
  void AreaLoaded(TAreaId area); // Guessed name, corresponding to Prime's area-load notification.
  void PrepareAreaUnload(TAreaId area); // Guessed name from Prime.
  void AreaUnloaded(TAreaId area);      // Guessed name from Prime.
  void SetActorAreaId(CActor& actor, TAreaId);
  // Guessed names.
  void SetPortalTransition(rstl::single_ptr< CPortalTransition >& transition);
  void SetPendingDockTransition(TAreaId area, int dock, bool showSoftTransition) {
    mPendingDockArea = area;
    mPendingDock = dock;
    mShowSoftTransition = showSoftTransition;
  }

  const CFrustumPlanes& GetFrustumPlanes() const { return m_planes; }
  int Get0x244c() const { return x244c; }

  int GetNumPlayers() const { return m_numPlayers; }
  uint ReturnFirstIfSingleElseSecond(uint single, uint multi) const; // Guessed name.
  CPlayer* GetPlayer(int index) { return m_players[index]; }
  const CPlayer* GetPlayer(int index) const { return m_players[index]; }
  CPlayer* Player(int index) { return m_players[index]; }

  CObjectList& ObjectListById(EGameObjectList id) { return *m_objectLists[id]; }
  const CObjectList& GetObjectListById(EGameObjectList id) const { return *m_objectLists[id]; }
  // Guessed names. The first filtered list qualifies only CScriptDoor objects.
  const rstl::list< CEntity* >& GetDoorList() const { return mFilteredObjectLists[0]->GetObjects(); }
  CMapWorldInfo* MapWorldInfo() { return mMapWorldInfo.GetPtr(); }

  void UpdateActorInSortedLists(CActor*);

  bool ApplyLocalDamage(const CVector3f& pos, const CVector3f& dir, CActor& damagee, float damage, const TUniqueId& uid1, const TUniqueId& uid2, const CDamageInfo& info, int);

  void fn_8003dd88(CActor&, TUniqueId, const CDamageInfo& info, bool, int);
  void fn_8003BF84(CEntity*);
  void fn_800412EC(TUniqueId);
  bool fn_80036F10() const; // Maybe_CheckIsMultiplayer
  void fn_8003BE54();
  void InformListeners(const CVector3f& position, EListenNoiseType type);

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
  rstl::reserved_vector< rstl::auto_ptr< CObjectList >, 8 > m_objectLists; // 0x808
  rstl::reserved_vector< CObjectList*, 8 > mDynamicObjectLists;
  rstl::reserved_vector< rstl::auto_ptr< CFilteredObjectList >, 6 > mFilteredObjectLists;
  rstl::reserved_vector< CFilteredObjectList*, 6 > mDynamicFilteredObjectLists;
  MapWorldInfoAreas mapWorldInfoAreas;
  char x8d4_[0x18];
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
  char pad2_2[0x40];
  rstl::rc_ptr< CRelayTracker > m_relayTracker;
  rstl::rc_ptr< CMapWorldInfo > mMapWorldInfo;
  rstl::rc_ptr< CWorldTransManager > m_worldTransManager;
  CWorldLayerState* m_currentWorldLayerState;
  int* x1698;
  rstl::single_ptr< CSaveGameScreen > m_saveGameScreen; // x169C
  TAreaId m_nextAreaId; // x16a0
  TAreaId mPreviousAreaId;
  int mRenderFrameIndex; // Guessed name: visibility age used by projectile impacts.
  int m_updateFrameIdx; // 16AC
  char pad4[0x34]; // 16B0
  CRandom16 mRandom;
  char x16e8_[8];
  EInitPhase mInitPhase;
  char x16f4_[0xD40];

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
  char pad6[0x28f8 - 0x24e0];
  TAreaId mPendingDockArea; // Guessed name.
  int mPendingDock; // Guessed name.
  rstl::single_ptr< CPortalTransition > mPortalTransition; // Guessed name.
  char x2904_[0x2938 - 0x2904];

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
  bool mShowSoftTransition : 1; // Guessed name.
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
