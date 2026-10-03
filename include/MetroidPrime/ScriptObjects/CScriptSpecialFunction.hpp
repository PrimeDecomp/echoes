#ifndef _CSCRIPTSPECIALFUNCTION
#define _CSCRIPTSPECIALFUNCTION

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"

#include "Kyoto/TAverage.hpp"

class CScriptSpecialFunction : public CActor {
public:
  enum ESpecialFunction {
    kSF_PlayerFollowLocator = 1,
    kSF_SpinnerController = 2,
    kSF_ObjectFollowLocator = 3,
    kSF_ChaffTarget = 4,
    kSF_SaveStation = 7,
    kSF_ViewFrustumTester = 9,
    kSF_ShotSpinnerController = 10,
    kSF_RainSimulator = 17,
    kSF_AreaDamage = 18,
    kSF_ObjectFollowObject = 19,
    kSF_ScaleActor = 22,
    kSF_PlayerInAreaRelay = 25,
    kSF_HUDTarget = 26,
    kSF_ItemDepletion = 51,    // Guessed name
    kSF_RadialDamage = 0x10001 // Guessed name; native message-handler dispatch.
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
  int ResolvePlayerIndex(int playerIndex, TUniqueId originator, CStateManager& mgr);

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
