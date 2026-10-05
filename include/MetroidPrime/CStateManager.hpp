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
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/TToken.hpp"
#include "Kyoto/TOneStatic.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/bit_vector.hpp"
#include "rstl/list.hpp"
#include "rstl/map.hpp"
#include "rstl/pair.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

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

namespace SL {
class CSortedListManager;
}
class CWeaponMgr;
class CFluidPlaneManager;
class CDamageInfo;
class CAABox;
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

    // Reconstructed operation names; the original spellings are unknown.
    void Append(const CScriptMsg& msg);
    int GetCount() const;
    CScriptMsg Dequeue();

    bool empty() const { return mWriteIndex == mReadIndex; }
  };

public:
  typedef rstl::map< TEditorId, TUniqueId > TIdList;
  typedef rstl::pair< TIdList::const_iterator, TIdList::const_iterator > TIdListResult;

  // Guessed phase names, derived from world initialization.
  enum EInitPhase { kIP_LoadAudioGroups, kIP_LoadWorld, kIP_LoadFirstArea, kIP_Done };
  // Guessed names, based on the update dispatch and Prime's corresponding state.
  enum EGameState { kGS_Running, kGS_SoftPaused };
  // Guessed names: Combat and Scan share the normal rendering mode.
  enum ERenderVisorMode { kRVM_Normal, kRVM_Echo, kRVM_Dark };

  CStateManager(const rstl::ncrc_ptr< CScriptMailbox >&, const rstl::ncrc_ptr< CMapWorldInfo >&,
                const rstl::ncrc_ptr< CPlayerState >&, const rstl::ncrc_ptr< CWorldTransManager >&);
  ~CStateManager();

  void FrameBegin(uint frame);
  void FrameEnd();
  void Update(float dt, CArchitectureQueue& queue);
  void ProcessInput(const CFinalInput& input);
  void InitializeState(CAssetId world, TAreaId area, CAssetId saveWorld);
  void DeleteSaveGameScreen();
  int SpecialSkipCinematic(); // Prime-correlated name; Echoes returns a three-way result.
  bool PrepareAreaTransition(TAreaId area); // Guessed name.
  rstl::single_ptr< CPortalTransition >& TakePortalTransition(); // Guessed name.
  bool HasPendingLayerLoads() const; // Guessed name, from the area query.
  void UpdateDynamicLayers(); // Guessed name, from the area update.
  void SetRandomAvailable(bool available) { mSkippingCinematic = available; }

  TUniqueId AllocateUniqueId();
  CScriptObjectLoaderHelper& ScriptObjectLoaderHelper();
  uint MaskUIdNumPlayers(TUniqueId id) const;
  void SetIsDarkWorld(bool);
  bool GetIsDarkWorld() const { return mIsDarkWorld; }
  CAssetId GetMapTeleportWorldId() const { return mMapTeleportWorldId; } // Guessed name
  void SetMapTeleportWorldId(CAssetId id) { mMapTeleportWorldId = id; } // Guessed name
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
  void RemoveWeaponId(TUniqueId owner, EWeaponType type);
  void ApplyDamageToWorld(TUniqueId owner, CActor& projectile, const CVector3f& position,
                          const CDamageInfo& damage, const CMaterialFilter& filter);
  void ApplyDamage(TUniqueId damager, TUniqueId damagee, TUniqueId weapon,
                   const CDamageInfo& damage, const CMaterialFilter& filter,
                   const CVector3f& direction);
  void ApplyRadiusDamage(const CActor& radiusSource, const CVector3f& position, CActor& damagee,
                         TUniqueId weapon, const CDamageInfo& damage);
  void KillPlayer(float remainingHealth, TUniqueId victim, TUniqueId killer); // Guessed name.
  void DrawSpaceWarp(const CVector3f& position, float strength) const;
  void PreRender(uint playerIndex); // Prime-correlated name.
  void DrawWorld(const CInGameGuiManagerSet& gui); // Prime-correlated name.
  void SetupPlayerViewport(uint playerIndex); // Guessed name.
  void DrawUnusedViewport(int viewportIndex); // Guessed name.
  void EndPlayerRender(); // Guessed name.
  void Touch(); // Prime-correlated name.

  bool AddDrawableActor(const CActor& actor, const CVector3f& pos, const CAABox& bounds) const;
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

  CWorld* World() { return mWorld; }
  CWorldTransManager* WorldTransManager() const { return mWorldTransManager.GetPtr(); }
  CScriptMailbox* Mailbox() const { return mMailbox.GetPtr(); }
  void QuitGame() { mQuitGame = true; }
  const CWorld* GetWorld() const { return mWorld; }
  bool IsFullyInitialized() const { return mInitPhase == kIP_Done; }
  CEnvFxManager* EnvFxManager() { return mEnvFxManager; }
  const CEnvFxManager* GetEnvFxManager() const { return mEnvFxManager; }
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
  const CTexture* GetShadowTex() const { return mShadowTex; }
  CFluidPlaneManager* GetFluidPlaneManager() const { return mFluidPlaneManager; }
  ERenderVisorMode GetRenderVisorMode() const { return mRenderVisorMode; }

  int GetNumPlayers() const { return mNumPlayers; }
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
  CMapWorldInfo* MapWorldInfo() { return mMapWorldInfo.GetPtr(); }

  void UpdateActorInSortedLists(CActor*);

  bool ApplyLocalDamage(const CVector3f& pos, const CVector3f& dir, CActor& damagee, float damage,
                        const TUniqueId& uid1, const TUniqueId& uid2, const CDamageInfo& info, int);

  void fn_8003dd88(CActor&, TUniqueId, const CDamageInfo& info, bool, int);
  void fn_8003BF84(CEntity*);
  void fn_800412EC(TUniqueId);
  bool IsMultiplayer() const; // Guessed name
  void fn_8003BE54();
  void InformListeners(const CVector3f& position, EListenNoiseType type);
  void Think(float dt);
  void MoveActors(float dt);
  // Guessed helper names, recovered from their update-loop consumers.
  bool ShouldUpdatePatterned(const CPatterned& actor);
  void ThinkEntity(float dt, CEntity& entity);

  // State transitions
  void DeferStateTransition(EStateManagerTransition t);
  void ResetEscapeSequenceTimer(float time); // Prime-correlated name
  void SetBossParams(TUniqueId bossId, float maxEnergy, uint stringIdx); // Prime name
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
  bool GetWantsToEnterLogBookScreen() const { return mDeferredTransition == kSMT_LogBook; }
  bool GetWantsToEnterSaveGameScreen() const { return mDeferredTransition == kSMT_SaveGame; }
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
  char x8d4_[0x18];
  ScriptMsgArray mScriptMsgs;
  CArchitectureQueue* mArchQueue;
  int mNumPlayers;
  CPlayer* mPlayers[4];
  CPlayerState* mPlayerStates[4];
  CCameraManager* mCameraManagers[4];
  CRumbleManager* mRumbleManagers[4];
  CFinalInput mFinalInputs[4];
  char x15ec_[0xc];
  CPlayer* mCurrentRenderPlayer; // 0x15f8, guessed name
  CPlayerState* mPlayerState;
  CCameraManager* mCameraManager;
  CWorld* mWorld;                                                 // 0x1604
  rstl::list< rstl::reserved_vector< CEntity*, 32 > > mGraveyard; // 0x1608
  rstl::single_ptr< CStateManagerContainer > mStateManagerContainer;
  SL::CSortedListManager* mSortedListManager;
  CWeaponMgr* mWeaponMgr;
  CFluidPlaneManager* mFluidPlaneManager;
  CEnvFxManager* mEnvFxManager;               // 0x1630
  CActorModelParticles* mActorModelParticles; // 0x1634
  void* x1638;
  char mUnknownData1[0x40];
  rstl::rc_ptr< CScriptMailbox > mMailbox;
  rstl::rc_ptr< CMapWorldInfo > mMapWorldInfo;
  rstl::rc_ptr< CWorldTransManager > mWorldTransManager;
  CWorldLayerState* mCurrentWorldLayerState;
  int* x1698;
  rstl::single_ptr< CSaveGameScreen > mSaveGameScreen; // x169C
  TAreaId mNextAreaId;                                 // x16a0
  TAreaId mPreviousAreaId;
  int mRenderFrameIndex; // Guessed name: visibility age used by projectile impacts.
  int mUpdateFrameIdx;   // 16AC
  char x16b0_[8];
  // Guessed names: actor-specific exclusions and the renderer's world-light list.
  rstl::vector< rstl::pair< TUniqueId, CLight > > mDynamicActorLights;
  rstl::vector< CLight > mDynamicLights;
  char x16d8_[8]; // Token storage; full resource ownership remains unresolved here.
  CTexture* mShadowTex; // 0x16e0
  CRandom16 mRandom;
  bool mSkippingCinematic : 1; // 0x16e8; set while a cinematic is being skipped.
  char x16e9_[3];
  EGameState mGameState;
  EInitPhase mInitPhase;
  TCameraFilterPasses mCameraFilterPasses; // 0x16f4
  TCameraBlurPasses mCameraBlurPasses;     // 0x1e98
  char x242c_[8];

  CAssetId mPauseHudMessage; // 0x2434
  float mEscapeTotalTime;
  float x243c;
  TUniqueId mBossId; // 0x2440
  float mBossHealth;
  uint mBossLanguageTableIndex;
  ERenderVisorMode mRenderVisorMode;
  TUniqueId mSpecialFunctionId;
  float mHudMessageTime;        // 0x2454
  CProjectedShadow* mProjectedShadows; // 0x2458; head of this frame's shadow list.
  int mHudMessageFrameCount;    // 0x245c
  int mPausedHudMemoFrameCount; // 0x2460
  CAssetId mPausedHudMemoAssetId;
  float x2468;
  CAssetId mMapTeleportWorldId; // Guessed name
  EStateManagerTransition mDeferredTransition;

  char mUnknownData3[4];
  CFrustumPlanes mPlanes;        // 0x2478
  int mCurrentRenderPlayerIndex; // Guessed name
  char mUnknownData4[0x28f8 - 0x24e0];
  TAreaId mPendingDockArea;                                // Guessed name.
  int mPendingDock;                                        // Guessed name.
  rstl::single_ptr< CPortalTransition > mPortalTransition; // Guessed name.
  char x2904_[0x2938 - 0x2904];

  CVector3f x2938;
  float x2944;
  CColor x2948;
  bool mUnkFlagA1 : 1;
  bool mQuitGame : 1;
  bool mUnkFlagA3 : 1;
  bool mInMapScreen : 1;
  bool mInSaveUI : 1;       // Prime-correlated name
  bool mCinematicPause : 1;
  bool mIsFullThreat : 1;   // Prime-correlated name
  bool mIsDarkWorld : 1;        // 0x294c
  bool mShowSoftTransition : 1; // Guessed name.
  bool mUnkFlagB2 : 1;
  bool mDispatchingScriptMessages : 1;
  bool mUnkFlagB4 : 1;
  bool mUnkFlagB5 : 1;
  bool mUnkFlagB6 : 1;
  bool mUnkFlagB7 : 1;
  bool mUnkFlagB8 : 1;
};
// CHECK_OFFSETOF(CStateManager, mWorld, 0x1604)
// CHECK_OFFSETOF(CStateManager, mEnvFxManager, 0x1630)

#endif // _CSTATEMANAGER
