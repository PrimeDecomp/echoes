#ifndef _CSCRIPTSPECIALFUNCTION
#define _CSCRIPTSPECIALFUNCTION

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"

#include "Kyoto/TAverage.hpp"

class CScriptSpecialFunction : public CActor {
public:
  // Values up to 0x45 are named after the script-object templates, the native-only
  // 0x1000x values after Prime's equivalents, unless marked.
  enum ESpecialFunction {
    kSF_None = 0,
    kSF_PlayerFollowLocator = 1,
    kSF_SpinnerController = 2,
    kSF_ObjectFollowLocator = 3,
    kSF_ChaffTarget = 4,
    kSF_InventoryActivator = 5,
    kSF_MapStation = 6,
    kSF_SaveStation = 7,
    kSF_IntroBossRingController = 8,
    kSF_ViewFrustumTester = 9,
    kSF_ShotSpinnerController = 10,
    kSF_EscapeSequence = 11,
    kSF_BossEnergyBar = 12,
    kSF_EndGame = 13,
    kSF_DisableHud = 14,
    kSF_CinematicSkip = 15,
    kSF_ScriptLayerController = 16,
    kSF_RainSimulator = 17,
    kSF_AreaDamage = 18,
    kSF_ObjectFollowObject = 19,
    kSF_HintController = 20,
    kSF_DropBomb = 21,
    kSF_ScaleActor = 22,
    kSF_MissileStation = 23,
    kSF_Billboard = 24,
    kSF_PlayerInAreaRelay = 25,
    kSF_HUDTarget = 26,
    kSF_UnderwaterFog = 27,
    kSF_EnterLogbookScreen = 28,
    kSF_PowerBombStation = 29,
    kSF_EndingActivator = 30,
    kSF_FusionRelay = 31,
    kSF_WeaponSwitch = 32,
    kSF_LaunchPlayer = 33,
    kSF_RechargeStation = 34,
    kSF_WorldSwapper = 35,
    kSF_PlayerOffscreen = 36,
    kSF_Function37 = 37,
    kSF_Function38 = 38,
    kSF_Function39 = 39,
    kSF_SetInventoryCapacity = 40,
    kSF_SetInventoryAmount = 41,
    kSF_ModifyInventoryAmount = 42,
    kSF_ModifyInventoryCapacity = 43,
    kSF_ModifyInventoryAmountAndCapacity = 44,
    kSF_SetInventoryAmountAndCapacity = 45,
    kSF_Function46 = 46,
    kSF_SunPlacement = 47,
    kSF_Function48 = 48,
    kSF_TransparencyWipe = 49,
    kSF_Function50 = 50,
    kSF_ItemDepletion = 51, // Guessed name
    kSF_DemoTimeoutResetController = 52,
    kSF_SunGeneratorTeleporter = 53,
    kSF_SkyLighting = 54,
    kSF_OcclusionRelay = 55,
    kSF_MultiplayerCountdown = 56,
    kSF_ScaleSZ = 57,
    kSF_ObjectFollowJoint = 58,
    kSF_Function59 = 59,
    kSF_ExtraRenderClipPlane = 60,
    kSF_VisorBlowout = 61,
    kSF_AreaAutoLoadController = 62,
    kSF_SystemStateEnvVarController = 63,
    kSF_GameStateEnvVarController = 64,
    kSF_MultiplayerMusic = 65, // Guessed name; pause menu selects a streamed-music connection.
    kSF_UnmappableObject = 66,
    kSF_CinematicSkipSignal = 67,
    kSF_RemoveRezbitVirus = 68,
    kSF_CompletionScreen = 69,
    kSF_FogVolume = 0x10000,
    kSF_RadialDamage = 0x10001,
    kSF_EnvFxDensityController = 0x10002,
    kSF_RumbleEffect = 0x10003,
    kSF_Silhouette = 0x10004,  // Guessed name
    kSF_DamageActor = 0x10005 // Guessed name
  };

  enum ESpinnerControllerMode { kSCM_Spinner, kSCM_ShotSpinner };

  ESpecialFunction GetFunction() const { return mFunction; }

  CScriptSpecialFunction(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                         const CTransform4f& xf, ESpecialFunction function,
                         const rstl::string& stringParm, float value1, float value2, float value3,
                         float value4, int intParm1, int intParm2, const CVector3f& vectorParm,
                         const CColor& colorParm, const CDamageInfo& damageInfo,
                         CPlayerState::EItemType item, ushort sfx1, ushort sfx2, ushort sfx3);

  // CEntity
  ~CScriptSpecialFunction() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  void PreRenderAllViewports(CStateManager& mgr) override;
  rstl::optional_object< CAABox > GetTouchBounds() const override { return mTouchBounds; }

  void ThinkPlayerFollowLocator(float dt, CStateManager& mgr);
  void ThinkObjectFollowLocator(float dt, CStateManager& mgr);
  void ThinkObjectFollowObject(float dt, CStateManager& mgr);
  void ThinkChaffTarget(float dt, CStateManager& mgr);
  void ThinkSpinnerController(float dt, CStateManager& mgr, ESpinnerControllerMode mode);
  void ThinkRainSimulator(float dt, CStateManager& mgr);
  void ThinkAreaDamage(float dt, CStateManager& mgr);
  void ThinkActorScale(float dt, CStateManager& mgr);
  void ThinkPlayerInArea(float dt, CStateManager& mgr);
  void ThinkSaveStation(float dt, CStateManager& mgr);
  void DeleteEmitter(CSfxHandle& handle);

  // Guessed names
  void ThinkViewFrustumTester(float dt, CStateManager& mgr);
  void SendFrustumMessages(CStateManager& mgr);
  void SetInFrustum(bool inFrustum);
  void OnItemDepleted(CStateManager& mgr, int playerIndex, CPlayerState::EItemType item);
  int ResolvePlayerIndex(int playerIndex, const TUniqueId& originator, CStateManager& mgr);

  // Guessed names for the independently dispatched Echoes handlers.
  void AcceptCredits(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptStopRezbitState(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptMapObjectVisibility(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptMultiplayerResult(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptEnvironmentVariable(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptAreaDocks(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptBillboard(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptFogPlane(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptRezbitState(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptDamageActor(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptViewFrustumTester(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptMultiplayerEndConditions(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptAreaOcclusion(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptSkyboxLighting(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptPauseGame(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptSilhouette(CStateManager& mgr, const CScriptMsg& msg);
  // Original name unknown.
  void fn_80107458(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptLastDamager(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptGiveTimedItem(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptModifyItem(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptModifyItemCapacity(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptModifyItemAmount(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptSetTimedItemAmount(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptSetItemCapacity(CStateManager& mgr, const CScriptMsg& msg);
  // Original name unknown.
  void fn_80107994(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptPlayerSpawnPoint(CStateManager& mgr, const CScriptMsg& msg);
  // Original name unknown.
  void fn_80107a58(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptDarkWorld(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptPlayerVelocity(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptEnding(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptLogbook(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptFogFader(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptHUDTarget(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptPlayerInArea(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptHint(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptDropBomb(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptAreaDamage(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptInventoryActivator(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptRumble(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptEnvFxDensity(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptCinematicSkip(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptEndGame(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptBossEnergyBar(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptRadialDamage(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptEnergyTank(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptSaveStation(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptPowerBombStation(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptMissileStation(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptMapStation(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptShotSpinner(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptSpinner(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptEscapeSequence(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptHUDFadeIn(CStateManager& mgr, const CScriptMsg& msg);
  void AcceptChaffTarget(CStateManager& mgr, const CScriptMsg& msg);
  void ThinkRezbitState(float dt, CStateManager& mgr);
  void ThinkObjectFollowJoint(float dt, CStateManager& mgr);
  void ThinkTriggerScale(float dt, CStateManager& mgr);
  void ThinkMultiplayerEndConditions(float dt, CStateManager& mgr);
  void ThinkAreaOcclusion(float dt, CStateManager& mgr);
  void ThinkSkyboxLighting(float dt, CStateManager& mgr);
  void ThinkMapTeleport(float dt, CStateManager& mgr);
  void ThinkSilhouette(float dt, CStateManager& mgr);
  void ThinkConnectedEffectPlane(float dt, CStateManager& mgr);
  void ThinkPlayerOffset(float dt, CStateManager& mgr);
  void ThinkPlayerItemRelay(float dt, CStateManager& mgr);
  void ThinkPlayerFrustumTester(float dt, CStateManager& mgr);
  void ThinkBillboard(float dt, CStateManager& mgr);
  void RenderBillboard() const;
  void RenderSilhouette(const CStateManager& mgr) const;
  void RenderFogVolume(const CStateManager& mgr) const;
  void PreRenderBillboard(CStateManager& mgr);
  void PreRenderSilhouette(CStateManager& mgr);
  void PreRenderPlayerFrustumTester(CStateManager& mgr);
  void PreRenderViewFrustumTester(CStateManager& mgr);
  void PreRenderFogVolume(CStateManager& mgr);
  void AddSilhouetteToRenderer(const CStateManager& mgr) const;
  void AddFogVolumeToRenderer(const CStateManager& mgr) const;
  void SkipCinematic(CStateManager& mgr);
  void AddOrUpdateEmitter(float pitch, float maxDist, float falloff, CSfxHandle& handle, ushort id,
                          CVector3f position, uchar volume);

private:
  ESpecialFunction mFunction;
  rstl::string mStringParm;
  float mValue1;
  float mValue2;
  float mValue3;
  float mValue4;
  int mIntParm1;
  int mIntParm2;
  CVector3f mVectorParm;
  CColor mColorParm;
  TUniqueId x194_; // Guessed type; only invalid-ID initialization is understood.
  TUniqueId mLastOriginatorPlayer;
  CDamageInfo mDamageInfo;
  float mSpinnerPosition; // Guessed name
  CTransform4f mSpinnerInitialXf;
  float mShotSpinnerImpulse; // Guessed name
  ushort mSfx1;
  ushort mSfx2;
  ushort mSfx3;
  CSfxHandle mSfxHandle;
  float mPreviousSpinnerSpeed;      // Guessed name
  TAverage< float > mVolumeAverage; // Guessed name
  float mChaffTimer;                // Guessed name
  float mSilhouetteStrength;        // Guessed name
  float mTargetSilhouetteStrength;  // Guessed name
  CPlayerState::EItemType mItem;
  rstl::optional_object< CAABox > mTouchBounds;
  bool mSpinnerInitializedXf : 1;
  bool mSpinnerCanMove : 1;
  bool mSfx2Played : 1;
  bool mSfx3Played : 1;
  bool mInAreaDamage : 1;
  bool mDoSave : 1;
  bool mPlayerInArea : 1;
  bool mFrustumEntered : 1;
  bool mFrustumExited : 1;
  bool mInFrustum : 1; // Guessed name
};
CHECK_SIZEOF(CScriptSpecialFunction, 0x240)

#endif // _CSCRIPTSPECIALFUNCTION
