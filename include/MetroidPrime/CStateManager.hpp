#ifndef _CSTATEMANAGER
#define _CSTATEMANAGER

extern const int gkPVSEnabled;

#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CFilteredObjectList.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CScriptObjectLoaderHelper.hpp"
#include "MetroidPrime/Cameras/CCameraBlurPass.hpp"
#include "MetroidPrime/Cameras/CCameraFilterPass.hpp"
#include "MetroidPrime/Enemies/EListenNoiseType.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "TGameTypes.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Input/CFinalInput.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/TOneStatic.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/bit_vector.hpp"
#include "rstl/list.hpp"
#include "rstl/multimap.hpp"
#include "rstl/pair.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class CWorld;
class CPortalTransition;
class CArchitectureQueue;
class CEnvFxManager;
class CSafeZoneManager;
class CEntity;
class CActor;
class CMaterialFilter;
class CRayCastResult;
class CScriptMailbox;
class CMapWorldInfo;
class CPlayerState;
class CWorldTransManager;
class CPlayer;
class CPatterned;
class CCameraManager;
class CRumbleManager;
class CSaveGameScreen;
class CActorModelParticles;
class CWorldLayerState;
class CStateManagerContainer;
class CInputStream;
class CStateManager;
class CInGameGuiManagerSet;
class CDependencyGroup;

namespace SL {
class CSortedListManager;
}
class CWeaponMgr;
class CFluidPlaneManager;
class CDamageInfo;
class CAABox;
class CPlane;
class CTexture;
class CProjectedShadow;

typedef rstl::bit_vector<> MapWorldInfoAreas;

enum EStateManagerTransition {
  kSMT_InGame,
  kSMT_MapScreen,
  kSMT_PauseGame,
  kSMT_Unk, // Unknown; Echoes uses 4 for the log book and 5 for the save screen.
  kSMT_LogBook,
  kSMT_SaveGame,
  kSMT_MessageScreen
};

class CStateManager : public TOneStatic< CStateManager > {

  struct ScriptMsgArray {
    enum { kCapacity = 192 };

    CScriptMsg mMessages[kCapacity];
    uint mWriteIndex;
    uint mReadIndex;

    ScriptMsgArray() : mWriteIndex(0), mReadIndex(0) {}

    // Reconstructed operation names; the original spellings are unknown.
    void Append(const CScriptMsg& msg);
    int GetCount() const;
    CScriptMsg Dequeue();

    bool empty() const { return mWriteIndex == mReadIndex; }
  };

public:
  typedef rstl::multimap< TEditorId, TUniqueId > TIdList;
  typedef rstl::pair< TIdList::const_iterator, TIdList::const_iterator > TIdListResult;

  // Guessed phase names, derived from world initialization.
  enum EInitPhase { kIP_LoadAudioGroups, kIP_LoadWorld, kIP_LoadFirstArea, kIP_Done };
  // Guessed names, based on the update dispatch and Prime's corresponding state.
  enum EGameState { kGS_Running, kGS_SoftPaused };
  // Guessed names: Combat and Scan share the normal rendering mode.
  enum ERenderVisorMode { kRVM_Normal, kRVM_Echo, kRVM_Dark };

  // Reconstructed indices, established by construction and qualification predicates.
  enum EFilteredObjectListType {
    kFOL_Door,
    kFOL_Dock,
    kFOL_Type124, // The qualifying entity type's class remains unidentified.
    kFOL_ForgottenObject,
    kFOL_GameCamera,
    kFOL_GrapplePoint
  };

  CStateManager(const rstl::ncrc_ptr< CScriptMailbox >&, const rstl::ncrc_ptr< CMapWorldInfo >&,
                const rstl::reserved_vector< rstl::ncrc_ptr< CPlayerState >, 4 >&,
                const rstl::ncrc_ptr< CWorldTransManager >&,
                const rstl::ncrc_ptr< CWorldLayerState >&);
  ~CStateManager();

  // Prime-correlated names; callback signatures and behavior are target-derived.
  static void RendererDrawCallback(const void* drawable, const void* context, int type);
  void RecursiveDrawTree(TUniqueId uid) const;
  static const bool MemoryAllocatorAllocationFailedCallback(const void* context, uint size);
  bool SwapOutAllPossibleMemory();

  void FrameBegin(uint frame);
  void FrameEnd();
  void Update(float dt, CArchitectureQueue& queue);
  void ProcessInput(const CFinalInput& input);
  void InitializeState(CAssetId world, TAreaId area, CAssetId saveWorld);
  void DeleteSaveGameScreen();
  int SpecialSkipCinematic(); // Prime-correlated name; Echoes returns a three-way result.
  bool PrepareAreaTransition(TAreaId area);                      // Guessed name.
  rstl::single_ptr< CPortalTransition >& TakePortalTransition(); // Guessed name.
  bool HasPendingLayerLoads() const; // Guessed name, from the area query.
  void UpdateDynamicLayers();        // Guessed name, from the area update.
  void SetRandomAvailable(bool available) { mSkippingCinematic = available; }

  TUniqueId AllocateUniqueId();
  CScriptObjectLoaderHelper& ScriptObjectLoaderHelper();
  uint MaskUIdNumPlayers(TUniqueId id) const;
  void SetIsDarkWorld(bool);
  bool GetIsDarkWorld() const { return mIsDarkWorld; }
  CAssetId GetMapTeleportWorldId() const { return mMapTeleportWorldId; } // Guessed name
  void SetMapTeleportWorldId(CAssetId id) { mMapTeleportWorldId = id; }  // Guessed name
  void DisplayAlertAboutOutOfAmmo(const CPlayer&, CPlayerState::EItemType) const;
  rstl::pair< int, int > CalculateScanCompletionRate() const;

  //
  void ShowPausedHUDMemo(CAssetId strg, float time);
  void QueueMessage(int frameCount, CAssetId msg, float f1);
  int GetHUDMessageFrameCount() const { return mHudMessageFrameCount; }
  CAssetId GetPauseHUDMessage() const { return mPauseHudMessage; }
  // float GetHUDMessageTime() const { return mHudMessageTime; }
  void IncrementHUDMessageFrameCounter() { ++mHudMessageFrameCount; }

  void SendScriptMsg(const CScriptMsg& msg);
  void DeliverScriptMsg(const CScriptMsg& msg); // Guessed name
  void SendScriptMsg(CEntity*, TUniqueId, EScriptObjectMessage, TUniqueId);
  void SendScriptMsg(TUniqueId target, TUniqueId sender, EScriptObjectMessage message,
                     TUniqueId actor); // Guessed overload name.

  void AddObject(CEntity*);
  void AddObject(CEntity&);
  bool RenderLast(TUniqueId uid);               // Guessed name.
  bool RenderLastOverlay(const TUniqueId& uid); // Guessed name.
  bool RenderLastHUD(const TUniqueId& uid);     // Guessed name.
  void DeleteObjectRequest(TUniqueId);
  void UpdateObjectInLists(CEntity&);
  void AddWeaponId(TUniqueId owner, EWeaponType type);
  int GetWeaponIdCount(TUniqueId owner, EWeaponType type);
  bool CanCreateProjectile(TUniqueId owner, EWeaponType type, int maxAllowed) const;
  void RemoveWeaponId(TUniqueId owner, EWeaponType type);
  void ApplyDamageToWorld(TUniqueId owner, CActor& projectile, const CVector3f& position,
                          const CDamageInfo& damage, const CMaterialFilter& filter);
  void ApplyDamage(TUniqueId damager, TUniqueId damagee, TUniqueId weapon,
                   const CDamageInfo& damage, const CMaterialFilter& filter,
                   const CVector3f& direction);
  void ApplyRadiusDamage(const CActor& radiusSource, const CVector3f& position, CActor& damagee,
                         TUniqueId weapon, const CDamageInfo& damage);
  void KillPlayer(float previousHealth, TUniqueId victim, TUniqueId killer); // Guessed name.
  void DrawSpaceWarp(const CVector3f& position, float strength) const;
  void PreRender(uint playerIndex);                // Prime-correlated name.
  void DrawWorld(const CInGameGuiManagerSet& gui); // Prime-correlated name.
  void SetupPlayerViewport(uint playerIndex);      // Guessed name.
  void DrawUnusedViewport(int viewportIndex);      // Guessed name.
  void EndPlayerRender();                          // Guessed name.
  void Touch();                                    // Prime-correlated name.

  bool AddDrawableActor(const CActor& actor, const CVector3f& pos, const CAABox& bounds) const;
  void AddDrawableActorPlane(const CActor& actor, const CPlane& plane, const CAABox& bounds) const;
  bool IsActorVisible(const CActor& actor) const; // Reconstructed name/qualification.
  void SetupParticleHook(const CActor& actor) const;
  void BuildDynamicLightListForWorld(); // Guessed name, correlated with Prime.
  const CActorModelParticles* GetActorModelParticles() const { return mActorModelParticles; }

  CActorModelParticles* ActorModelParticles() { return mActorModelParticles; }

  CEntity* ObjectById(TUniqueId uid);
  const CEntity* GetObjectById(TUniqueId uid) const;
  CEntity* GetObjectByIdFromListAll(TUniqueId uid);
  bool RayCollideWorld(const CVector3f& start, const CVector3f& end, const CMaterialFilter& filter,
                       const CActor* damagee);
  bool RayCollideWorld(const CVector3f& start, const CVector3f& end,
                       const rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                       const CMaterialFilter& filter, const CActor* ignoreActor) const;
  CRayCastResult
  RayWorldIntersection(TUniqueId& idOut, const CVector3f& position, const CVector3f& direction,
                       float length, const CMaterialFilter& filter,
                       const rstl::reserved_vector< TUniqueId, 1024 >& nearList) const;
  CRayCastResult RayStaticIntersection(const CVector3f& position, const CVector3f& direction,
                                       float length, const CMaterialFilter& filter) const;
  void BuildNearList(rstl::reserved_vector< TUniqueId, 1024 >& nearList, const CVector3f& position,
                     const CVector3f& direction, float length, const CMaterialFilter& filter,
                     const CActor* ignoreActor) const;
  void BuildNearList(rstl::reserved_vector< TUniqueId, 1024 >& nearList, const CAABox& bounds,
                     const CMaterialFilter& filter, const CActor* ignoreActor) const;
  // Guessed name, correlated with Prime's actor-filtered near-list wrapper.
  void BuildColliderList(rstl::reserved_vector< TUniqueId, 1024 >& nearList, const CActor& actor,
                         const CAABox& bounds) const;

  TEditorId GetEditorIdForUniqueId(TUniqueId) const;
  TUniqueId GetIdForScript(TEditorId eid) const;
  TIdListResult GetIdListForScript(TEditorId) const;

  CWorld* World() { return mWorld.get(); }
  bool HasWorld() const; // Prime-correlated name.
  CWorldTransManager* WorldTransManager() const { return mWorldTransManager.GetPtr(); }
  CScriptMailbox* Mailbox() const { return mMailbox.GetPtr(); }
  void QuitGame() { mQuitGame = true; }
  const CWorld* GetWorld() const { return mWorld.get(); }
  bool IsFullyInitialized() const { return mInitPhase == kIP_Done; }
  CEnvFxManager* EnvFxManager() { return mEnvFxManager; }
  const CEnvFxManager* GetEnvFxManager() const { return mEnvFxManager; }
  const CSafeZoneManager* GetSafeZoneManager() const { return mSafeZoneManager; }
  CRandom16* Random() { return &mRandom; }
  int GetUpdateFrameIdx() const { return mUpdateFrameIdx; }
  int GetRenderFrameIndex() const { return mRenderFrameIndex; } // Guessed name

  TAreaId GetNextAreaId() const { return mNextAreaId; }
  TAreaId GetPreviousAreaId() const { return mPreviousAreaId; }
  void SetCurrentAreaId(TAreaId);
  void AreaLoaded(TAreaId area); // Guessed name, corresponding to Prime's area-load notification.
  void PrepareAreaUnload(TAreaId area); // Guessed name from Prime.
  void AreaUnloaded(TAreaId area);      // Guessed name from Prime.
  void SetActorAreaId(CActor& actor, TAreaId);
  // Guessed names.
  void SetPortalTransition(rstl::single_ptr< CPortalTransition >& transition);
  void AddProjectedShadow(CProjectedShadow* shadow);
  void SetPendingDockTransition(TAreaId area, int dock, bool showSoftTransition) {
    mPendingDockArea = area;
    mPendingDock = dock;
    mShowSoftTransition = showSoftTransition;
  }

  const CFrustumPlanes& GetFrustumPlanes() const { return mPlanes; }
  const CTexture* GetShadowTex() const { return mShadowTex.GetObject(); }
  CFluidPlaneManager* GetFluidPlaneManager() const { return mFluidPlaneManager; }
  ERenderVisorMode GetRenderVisorMode() const { return mRenderVisorMode; }

  int GetNumPlayers() const { return mNumPlayers; }
  CWeaponMgr* GetWeaponMgr() const { return mWeaponMgr; }
  TUniqueId GetForceTriggerId(int playerIndex) const {
    return mForceTriggerIds[playerIndex];
  }
  void SetForceTriggerId(int playerIndex, TUniqueId id) {
    mForceTriggerIds[playerIndex] = id;
  }
  int GetViewportLayoutIndex() const; // Guessed name
  typedef rstl::reserved_vector< rstl::reserved_vector< CCameraFilterPass, 11 >, 4 >
      TCameraFilterPasses;
  typedef rstl::reserved_vector< rstl::reserved_vector< CCameraBlurPass, 11 >, 4 >
      TCameraBlurPasses;
  CCameraFilterPass& CameraFilterPass(uint player, int stage) {
    return mCameraFilterPasses[player][stage];
  }
  CCameraBlurPass& CameraBlurPass(uint player, int stage) {
    return mCameraBlurPasses[player][stage];
  }
  ushort ReturnFirstIfSingleElseSecond(uint single, uint multi) const; // Guessed name.
  CPlayer* GetPlayer(int index) { return mPlayers[index]; }
  const CPlayer* GetPlayer(int index) const { return mPlayers[index]; }
  CPlayer* Player(int index) { return mPlayers[index]; }

  CObjectList& ObjectListById(EGameObjectList id) { return *mObjectLists[id]; }
  bool IsSkippingCinematic() const { return mSkippingCinematic; }
  const CObjectList& GetObjectListById(EGameObjectList id) const { return *mObjectLists[id]; }
  const rstl::vector< rstl::pair< TUniqueId, CLight > >& GetDynamicActorLights() const {
    return mDynamicActorLights;
  }
  // Guessed names. The first filtered list qualifies only CScriptDoor objects.
  const rstl::list< CEntity* >& GetDoorList() const {
    return mFilteredObjectLists[0]->GetObjects();
  }
  const rstl::list< CEntity* >& GetDockList() const {
    return mFilteredObjectLists[3]->GetObjects();
  }
  const rstl::list< CEntity* >& GetGrapplePointList() const {
    return mFilteredObjectLists[5]->GetObjects();
  }
  CMapWorldInfo* MapWorldInfo() { return mMapWorldInfo.GetPtr(); }

  void UpdateActorInSortedLists(CActor*);

  bool ApplyLocalDamage(const CVector3f& pos, const CVector3f& dir, CActor& damagee, float damage,
                        const TUniqueId& uid1, const TUniqueId& uid2, const CDamageInfo& info, int);

  void fn_8003dd88(CActor&, TUniqueId, const CDamageInfo& info, bool, int);
  // Guessed names, recovered from script deletion and object-list consumers.
  void AddToGraveyard(CEntity* entity);
  void ClearGraveyard(); // Prime-correlated name; deletes the queued entity batches.
  void RemoveObject(TUniqueId id);
  bool IsMultiplayer() const; // Guessed name
  void DispatchScriptMessages(); // Guessed name.
  void ThinkNewObjects(float dt); // Guessed name.
  void InformListeners(const CVector3f& position, EListenNoiseType type);
  void Think(float dt);
  void MoveActors(float dt);
  // Guessed helper names, recovered from their update-loop consumers.
  bool ShouldUpdatePatterned(const CPatterned& actor);
  void ThinkEntity(float dt, CEntity& entity);

  // State transitions
  EGameState GetGameState() const { return mGameState; }
  void DeferStateTransition(EStateManagerTransition t);
  void ResetEscapeSequenceTimer(float time);                             // Prime-correlated name
  float GetEscapeSequenceTimer() const; // Prime-correlated name; saved GameState timer in Echoes.
  void SetBossParams(TUniqueId bossId, float maxEnergy, uint stringIdx); // Prime name
  TUniqueId GetBossId() const { return mBossId; }
  float GetTotalBossEnergy() const { return mBossHealth; }
  uint GetBossStringIdx() const { return mBossLanguageTableIndex; }
  float IntegrateVisorFog(float f) const;
  void SetAreaClipPlane(TAreaId area, const CPlane& plane); // Guessed name
  void EnterMapScreen() { DeferStateTransition(kSMT_MapScreen); }
  void EnterPauseScreen() { DeferStateTransition(kSMT_PauseGame); }
  void EnterLogBookScreen() { DeferStateTransition(kSMT_LogBook); }
  void EnterSaveGameScreen() { DeferStateTransition(kSMT_SaveGame); }
  void EnterMessageScreen(uint, float);
  bool GetWantsToEnterMapScreen() const { return mDeferredTransition == kSMT_MapScreen; }
  bool GetInMapScreen() const { return mInMapScreen; }
  bool GetShowSoftTransition() const { return mShowSoftTransition; }
  void SetInMapScreen(bool inMapScreen) { mInMapScreen = inMapScreen; }
  bool GetWantsToEnterPauseScreen() const { return mDeferredTransition == kSMT_PauseGame; }
  void SetCinematicPause(bool paused) { mCinematicPause = paused; } // Guessed name
  void SetGameState(EGameState state);
  bool GetWantsToEnterLogBookScreen() const { return mDeferredTransition == kSMT_LogBook; }
  bool GetWantsToEnterSaveGameScreen() const { return mDeferredTransition == kSMT_SaveGame; }
  bool HasSaveGameScreen() const { return !mSaveGameScreen.null(); }
  TAreaId GetPendingDockArea() const { return mPendingDockArea; }
  bool GetWantsToEnterMessageScreen() const { return mDeferredTransition == kSMT_MessageScreen; }

  const CCameraManager* GetCameraManager(int playerIndex) const {
    return mCameraManagers[playerIndex];
  }
  CCameraManager* CameraManager(int playerIndex) { return mCameraManagers[playerIndex]; }
  const CPlayerState* GetPlayerState() const { return mPlayerState; }
  const CPlayer* GetCurrentRenderPlayer() const { return mCurrentRenderPlayer; } // Guessed name
  int GetCurrentRenderPlayerIndex() const { return mCurrentRenderPlayerIndex; }  // Guessed name
  const CCameraManager* GetCurrentRenderCameraManager() const {
    return mCameraManager;
  } // Guessed name
  const CPlayerState* GetPlayerState(int playerIndex) const { return mPlayerStates[playerIndex]; }
  CPlayerState* PlayerState(int playerIndex) { return mPlayerStates[playerIndex]; }
  CRumbleManager* RumbleManager(int playerIndex) { return mRumbleManagers[playerIndex]; }
  CArchitectureQueue& ArchQueue() { return *mArchQueue; }
  TUniqueId GetSkipCinematicSpecialFunction() const { return mSpecialFunctionId; }
  void SetSkipCinematicSpecialFunction(TUniqueId id) { mSpecialFunctionId = id; }
  void SetUnkFlagA3(bool value) { mUnkFlagA3 = value; }
  bool GetInSaveUI() const { return mInSaveUI; }
  void SetIsFullThreat(bool value) { mIsFullThreat = value; }

  bool fn_800366e4(const CActor*) const;

public:
  ushort mNextFreeIndex;
  rstl::reserved_vector< ushort, 1024 > mObjectIndexArray;                // x0x4
  rstl::reserved_vector< rstl::auto_ptr< CObjectList >, 8 > mObjectLists; // 0x808
  rstl::reserved_vector< CObjectList*, 8 > mDynamicObjectLists;
  rstl::reserved_vector< rstl::auto_ptr< CFilteredObjectList >, 6 > mFilteredObjectLists;
  rstl::reserved_vector< CFilteredObjectList*, 6 > mDynamicFilteredObjectLists;
  MapWorldInfoAreas mAllocatedObjectIndices;
  rstl::list< TUniqueId > mNewObjectIds; // Guessed name: IDs awaiting their first update.
  ScriptMsgArray mScriptMsgs;
  CArchitectureQueue* mArchQueue;
  int mNumPlayers;
  CPlayer* mPlayers[4];
  CPlayerState* mPlayerStates[4];
  CCameraManager* mCameraManagers[4];
  CRumbleManager* mRumbleManagers[4];
  CFinalInput mFinalInputs[4];
  rstl::reserved_vector< TUniqueId, 4 > mForceTriggerIds;
  CPlayer* mCurrentRenderPlayer; // 0x15f8, guessed name
  CPlayerState* mPlayerState;
  CCameraManager* mCameraManager;
  rstl::single_ptr< CWorld > mWorld; // Native teardown owns and deletes the world.
  rstl::list< rstl::reserved_vector< CEntity*, 32 > > mGraveyard; // 0x1608
  rstl::single_ptr< CStateManagerContainer > mStateManagerContainer;
  SL::CSortedListManager* mSortedListManager;
  CWeaponMgr* mWeaponMgr;
  CFluidPlaneManager* mFluidPlaneManager;
  CEnvFxManager* mEnvFxManager;               // 0x1630
  CActorModelParticles* mActorModelParticles; // 0x1634
  CSafeZoneManager* mSafeZoneManager; // 0x1638, target-derived pointee and role.
  TIdList mScriptIdMap;
  TToken< CDependencyGroup > mAudioGroupDependencies; // Guessed name, from audio initialization.
  rstl::reserved_vector< rstl::ncrc_ptr< CPlayerState >, 4 > mPlayerStateOwners; // Guessed name.
  rstl::ncrc_ptr< CScriptMailbox > mMailbox;
  rstl::ncrc_ptr< CMapWorldInfo > mMapWorldInfo;
  rstl::ncrc_ptr< CWorldTransManager > mWorldTransManager;
  rstl::ncrc_ptr< CWorldLayerState > mCurrentWorldLayerState;
  rstl::single_ptr< CSaveGameScreen > mSaveGameScreen; // x169C
  TAreaId mNextAreaId;                                 // x16a0
  TAreaId mPreviousAreaId;
  int mRenderFrameIndex; // Guessed name: visibility age used by projectile impacts.
  int mUpdateFrameIdx;   // 16AC
  uint mObjectDrawToken; // Prime-correlated name; deduplicates actor rendering.
  // Constructor writes zero; no runtime consumer identified in the selected DOL.
  uint mUnknown0x16b4;
  // Guessed names: actor-specific exclusions and the renderer's world-light list.
  rstl::vector< rstl::pair< TUniqueId, CLight > > mDynamicActorLights;
  rstl::vector< CLight > mDynamicLights;
  TCachedToken< CTexture > mShadowTex;
  CRandom16 mRandom;
  bool mSkippingCinematic : 1; // 0x16e8; set while a cinematic is being skipped.
  char x16e9_[3];
  EGameState mGameState;
  EInitPhase mInitPhase;
  TCameraFilterPasses mCameraFilterPasses; // 0x16f4
  TCameraBlurPasses mCameraBlurPasses;     // 0x1e98
  int mHintIdx; // Prime-correlated names, from hint selection and HUD memo dispatch.
  uint mHintPeriods;

  CAssetId mPauseHudMessage; // 0x2434
  float mEscapeTotalTime;
  float mCurTimeMod900; // Prime-correlated name; drives CTimeProvider during rendering.
  TUniqueId mBossId; // 0x2440
  float mBossHealth;
  uint mBossLanguageTableIndex;
  ERenderVisorMode mRenderVisorMode;
  TUniqueId mSpecialFunctionId;
  TUniqueId mPlayerActorHead; // Prime-correlated name; identifies the model-touch actor.
  float mHudMessageTime;               // 0x2454
  CProjectedShadow* mProjectedShadows; // 0x2458; head of this frame's shadow list.
  int mHudMessageFrameCount;           // 0x245c
  int mPausedHudMemoFrameCount;        // 0x2460
  CAssetId mPausedHudMemoAssetId;
  float mQueuedHudMemoDismissalDelay; // Guessed name, from the queued memo's parameters.
  CAssetId mMapTeleportWorldId; // Guessed name
  EStateManagerTransition mDeferredTransition;

  // Guessed names: one cached line-of-sight result per multiplayer player pair.
  uchar mPlayerLineOfSightPairs;
  uchar mNextPlayerLineOfSightPair;
  CFrustumPlanes mPlanes;        // 0x2478
  int mCurrentRenderPlayerIndex; // Guessed name
  TAreaId mVisAreaId; // Prime-correlated role: area containing the render viewpoint.
  rstl::reserved_vector< rstl::pair< int, CFrustumPlanes >, 10 > mAreaFrusta; // Guessed name.
  TAreaId mPendingDockArea;                                // Guessed name.
  int mPendingDock;                                        // Guessed name.
  rstl::single_ptr< CPortalTransition > mPortalTransition; // Guessed name.
  rstl::single_ptr< TCachedToken< CTexture > > mUnusedViewportTexture; // Guessed name.
  // Identity-initialized and reset during area transitions; no other consumer identified.
  CTransform4f mUnknown0x2908;

  // Guessed names from the renderer's DrawDarkWorldCloud arguments.
  CVector3f mDarkWorldCloudScale;
  float mDarkWorldCloudTime;
  CColor mDarkWorldCloudColor;
  bool mReadyToRender : 1; // Guessed name: set after the first update, gates PreRender.
  bool mQuitGame : 1;
  bool mUnkFlagA3 : 1; // Increment/Decrement script toggle; runtime purpose unresolved.
  bool mInMapScreen : 1;
  bool mInSaveUI : 1; // Prime-correlated name
  bool mCinematicPause : 1;
  bool mIsFullThreat : 1;       // Prime-correlated name
  bool mIsDarkWorld : 1;        // 0x294c
  bool mShowSoftTransition : 1; // Guessed name.
  bool mTearingDown : 1; // Guessed name: set on entry to the destructor.
  bool mDispatchingScriptMessages : 1;
  bool mLayerRestartPending : 1; // Guessed name, from layer activation and game-flow consumers.
  // Guessed names: per-player depletion within the current update, not warning history.
  uint mLightAmmoDepletedPlayers : 4;
  uint mDarkAmmoDepletedPlayers : 4;
};
CHECK_SIZEOF(CStateManager, 0x2950)
CHECK_OFFSETOF(CStateManager, mNewObjectIds, 0x8d4)
CHECK_OFFSETOF(CStateManager, mWorld, 0x1604)
CHECK_OFFSETOF(CStateManager, mEnvFxManager, 0x1630)
CHECK_OFFSETOF(CStateManager, mScriptIdMap, 0x163c)
CHECK_OFFSETOF(CStateManager, mAudioGroupDependencies, 0x1650)
CHECK_OFFSETOF(CStateManager, mPlayerStateOwners, 0x1658)
CHECK_OFFSETOF(CStateManager, mCurrentWorldLayerState, 0x1694)
CHECK_OFFSETOF(CStateManager, mForceTriggerIds, 0x15ec)
CHECK_OFFSETOF(CStateManager, mObjectDrawToken, 0x16b0)
CHECK_OFFSETOF(CStateManager, mShadowTex, 0x16d8)
CHECK_OFFSETOF(CStateManager, mHintIdx, 0x242c)
CHECK_OFFSETOF(CStateManager, mPlayerActorHead, 0x2452)
CHECK_OFFSETOF(CStateManager, mPlayerLineOfSightPairs, 0x2474)
CHECK_OFFSETOF(CStateManager, mAreaFrusta, 0x24e4)
CHECK_OFFSETOF(CStateManager, mUnusedViewportTexture, 0x2904)
CHECK_OFFSETOF(CStateManager, mUnknown0x2908, 0x2908)
CHECK_OFFSETOF(CStateManager, mDarkWorldCloudScale, 0x2938)

#endif // _CSTATEMANAGER
