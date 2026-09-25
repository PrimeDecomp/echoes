#ifndef _CSAMUSHUD
#define _CSAMUSHUD

#include "types.h"

#include "GuiSys/CGuiTextSupport.hpp"
#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/CTargetReticles.hpp"
#include "MetroidPrime/Cameras/CCameraFilterPass.hpp"
#include "MetroidPrime/HUD/CHUDMemoParms.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/string.hpp"

class CActorLights;
class CFinalInput;
class CLight;
class CStringTable;
class CGuiFrame;
class CGuiFrameLoader; // Guessed name
class CGuiWidget;
class CGuiTextPane;
class CGuiCamera;
class CAuiEnergyBarT01;
class CAuiMeter;
class CHudVisorBeamMenu;
class CHudRadarInterface;
class CHudBossEnergyInterface;
class CHudDecoInterfaceScan;

class CSamusHud {
public:
  enum EHudState { kHS_Combat, kHS_Echo, kHS_Scan, kHS_Dark, kHS_Ball, kHS_None };
  enum ELoadPhase { kLP_Targeting, kLP_Frames, kLP_Complete };
  enum ETransitionState { kTS_Idle, kTS_FadeOut, kTS_Loading, kTS_FadeIn };

  struct SCachedHudLight {
    SCachedHudLight()
    : mPosition(CVector3f::Zero())
    , mColor(CColor::White())
    , mAttenuationConstant(0.f)
    , mAttenuationLinear(0.f)
    , mAttenuationQuadratic(0.f)
    , mFade(0.f) {}
    CVector3f mPosition;
    CColor mColor;
    float mAttenuationConstant;
    float mAttenuationLinear;
    float mAttenuationQuadratic;
    float mFade;
  };

  // Guessed name
  struct SProfileInfo {
    SProfileInfo() : mUpdateTime(0), mDrawTime(0) {}
    u64 mUpdateTime;
    u64 mDrawTime;
  };

  CSamusHud(const CStateManager& mgr, CGuiFrameLoader& hud, CGuiFrameLoader& memo,
            CGuiFrameLoader* helmet, int playerIndex);
  virtual ~CSamusHud();
  bool CheckLoadComplete(const CStateManager& mgr);
  void Update(float dt, const CStateManager& mgr, uint helmetVisibility, bool hudVisible,
              bool targetingVisible);
  void Draw(const CStateManager& mgr, float alpha, uint helmetVisibility, bool hudVisible,
            bool targetingVisible) const;
  void EnterFirstPerson(const CStateManager& mgr);
  void LeaveFirstPerson(const CStateManager& mgr);
  void StopSounds(const CStateManager& mgr);
  void ProcessControllerInput(const CFinalInput& input);
  void PrepareScanDisplay(const CStateManager& mgr, int playerIndex);
  static CHudDecoInterfaceScan* GetScanInterface(int playerIndex);
  void ShowDamage(CVector3f position, float damage, float previousDamage, const CStateManager& mgr);
  const CTargetingManager& GetTargetingManager() const;
  static void DisplayHudMemo(const rstl::wstring& text, const CHUDMemoParms& info);
  static void DeferHintMemo(CAssetId stringTable, uint index, const CHUDMemoParms& info);
  static bool IsHudMemoVisible(int playerIndex);
  static void RefreshBeamMenu(const CStateManager& mgr, int playerIndex);
  static CTransform4f BuildFinalCameraTransform(const CQuaternion& rotation, const CVector3f& pivot,
                                                const CVector3f& cameraPosition);
  static rstl::pair< CVector3f, CVector3f > BallEnergyCoordFunc(float t);
  static rstl::pair< CVector3f, CVector3f > CombatEnergyCoordFunc(float t);

private:
  void DrawBossLockOnWarning() const;
  void UpdateBossLockOnWarning(float dt, const CStateManager& mgr);
  void SetMessage(const rstl::wstring& text, const CHUDMemoParms& info);
  void ApplyClassicLag(const CUnitVector3f& lookDirection, CQuaternion& rotation,
                       const CStateManager& mgr, float dt, bool invert);
  void UpdateHudLag(float dt, const CStateManager& mgr);
  CRelAngle GetRelativeDirection(const CVector3f& position, const CStateManager& mgr) const;
  EHudState GetDesiredHudState(const CStateManager& mgr) const;
  EHudState GetNextState() const;
  CColor ModulateColor(const CColor& color) const;
  void DrawHudMemo() const;
  void DrawHelmet(const CStateManager& mgr, float cameraYOffset) const;
  void DrawPlayerFilter(const CStateManager& mgr) const;
  void DrawAttachedEnemyEffect(const CStateManager& mgr) const;
  void DrawLockOnIndicators(const CStateManager& mgr) const;
  void DrawLockOnIndicators(const CStateManager& mgr,
                            const rstl::reserved_vector< TUniqueId, 12 >& targets) const;
  rstl::reserved_vector< bool, 4 > BuildPlayerHasBeams(const CStateManager& mgr) const;
  rstl::reserved_vector< bool, 4 > BuildPlayerHasVisors(const CStateManager& mgr) const;
  void UpdateHudMemo(float dt, const CStateManager& mgr);
  void UpdateBootSequence(float dt, const CStateManager& mgr);
  void UpdateHudFrame(float dt, const CStateManager& mgr);
  void UpdateStateTransition(float dt, const CStateManager& mgr);
  void UpdateHudDamage(float dt, const CStateManager& mgr);
  CColor GetVisorHudLightColor(const CColor& color, const CStateManager& mgr) const;
  void UpdateHudDynamicLights(float dt, const CStateManager& mgr);
  int FindEmptyHudLightSlot(const CLight& light) const;
  bool IsAreaLightInCachedLights(const CLight& light) const;
  bool IsCachedLightInAreaLights(const SCachedHudLight& light, const CActorLights& lights) const;
  void fn_8006653c(const CStateManager& mgr, bool init);
  void UpdateThreatAssessment(float dt, const CStateManager& mgr);
  bool ResolveLockOnTexture();
  void UpdateBallMode(const CStateManager& mgr);
  void UpdateBeamAmmo(const CStateManager& mgr, bool init);
  void UpdateMissile(float dt, const CStateManager& mgr, bool init);
  void UpdateEnergy(float dt, const CStateManager& mgr, bool init);
  void UpdateHudColor();
  void UpdateStaticSfx(const CStateManager& mgr, CSfxHandle& sound, float& cycle, ushort soundId,
                       float dt, float previousInterference, float threshold);
  void UpdateStaticInterference(float dt, const CStateManager& mgr);
  void UpdateFreeLook(float dt, const CStateManager& mgr);
  void UpdateVisorAndBeamMenus(float dt, const CStateManager& mgr);
  void UpdateEnergyLow(float dt, const CStateManager& mgr);
  void RefreshHudStringTable();
  void InternalDeferHintMemo(CAssetId stringTable, uint index, const CHUDMemoParms& info);
  void InternalDisplayHudMemo(const rstl::wstring& text, const CHUDMemoParms& info);
  void UpdateHudWidgetColors();
  void UpdateHelmetWidgets();
  void UninitializeFrameGlueMutable();
  void InitializeFrameGlueMutable(const CStateManager& mgr);
  void InitializeFrameGluePermanent();

  int mPlayerIndex;
  ELoadPhase mLoadPhase;
  CTargetingManager mTargetingManager;
  rstl::auto_ptr< CGuiFrame > mHudFrame;
  CGuiFrame* mLoadedHudFrame;
  rstl::auto_ptr< CGuiFrame > mHelmetFrame;
  CGuiFrame* mLoadedHelmetFrame;
  rstl::auto_ptr< CGuiFrame > mMemoFrame;
  CGuiFrame* mLoadedMemoFrame;
  EHudState mPreviousState;
  EHudState mNextState;
  EHudState mDesiredState;
  ETransitionState mTransitionState;
  float mTransitionFactor;
  float mPlayerHealth;
  int mEnergyTankCapacity;
  int mMissileAmount;
  int mMissileCapacity;
  float mMissileModeTransition;
  CPlayerState::EBeamId mAmmoBeam;
  int mDarkAmmo;
  int mLightAmmo;
  bool mInFreeLook : 1;
  bool mLookControlHeld : 1;
  bool mFirstPerson : 1;
  bool mEnergyLow : 1;
  bool mEnglishOnly : 1;
  CPlayerState::EBeamId mMenuBeam;
  CPlayerState::EPlayerVisor mMenuVisor;
  int mMissileEnabled;
  uint x35c;
  CVector3f mPreviousCameraDirection;
  CQuaternion mHudLag;
  CQuaternion mInverseHudLag;
  CGuiWidget* mHelmetLightingWidget;
  rstl::single_ptr< CActorLights > mLights;
  rstl::reserved_vector< SCachedHudLight, 3 > mHudLights;
  TLockedToken< CStringTable > mHudStringTable;
  CSfxHandle mDamageSound;
  CCameraFilterPass mDamageFilter;
  float mDamageTime;
  float mDamagePulse;
  float mDamageFilterDuration;
  float mDamageFilterRemaining;
  float mDamageFilterGain;
  float mDamageHighlightDuration;
  float mDamageHighlightRemaining;
  rstl::reserved_vector< float, 12 > mDamageSectorDurations;
  rstl::reserved_vector< float, 12 > mDamageSectorRemaining;
  rstl::reserved_vector< float, 12 > mDamageSectorIntensity;
  TCachedToken< CTexture > mDamageRingTexture;
  CVector3f mDamagerToPlayer;
  float mShakeTranslationAmount;
  float mShakeTranslationVelocity;
  CVector3f mShakeTranslation;
  CMatrix3f mShakeRotation;
  CQuaternion mHudLagShake;
  float mShakeDuration;
  float mShakeRemaining;
  float mShakeGain;
  float mViewportScaleX;
  float mViewportScaleY;
  CSfxHandle mStaticSoundLow;
  CSfxHandle mStaticSoundHigh;
  float mStaticInterference;
  float mStaticCycleLow;
  float mStaticCycleHigh;
  CCameraFilterPass mStaticFilter;
  CHUDMemoParms mHudMemoParms;
  rstl::wstring mMessageText;
  rstl::single_ptr< TToken< CStringTable > > mHudMemoString;
  int mHudMemoIndex;
  CGuiWidget* mMessageRoot;
  CGuiTextPane* mMessagePane;
  CGuiWidget* mMessageAButton;
  float mMessageTime;
  float mLastMessageSoundChars;
  uint x5d8;
  CSfxHandle mFreeLookSound;
  CVector3f mPreviousFreeLookDirection;
  float mFreeLookDirectionDot;
  float mFreeLookSoundCycle;
  CGuiWidget* mFreeLookLeft;
  CGuiWidget* mFreeLookRight;
  CTransform4f mFreeLookLeftTransform;
  CTransform4f mFreeLookRightTransform;
  float mFreeLookFade;
  float mEnergyLowTimer;
  float mEnergyLowPulse;
  float mEnergyLowFade;
  float mAButtonPulse;
  float x670;
  float mThreatAmount;
  rstl::optional_object< TCachedToken< CTexture > > mLockedOnIndicator;
  CPlayerState::EBeamId mCurrentBeam;
  CPlayerState::EBeamId mPreviousBeam;
  float mBeamMenuTransition;
  rstl::single_ptr< CHudVisorBeamMenu > mVisorMenu;
  CGuiWidget* mVisorBracket;
  rstl::single_ptr< CHudVisorBeamMenu > mBeamMenu;
  rstl::single_ptr< CHudRadarInterface > mRadar;
  float mMissilePickupPulse;
  float mLightAmmoPickupPulse;
  float mDarkAmmoPickupPulse;
  rstl::single_ptr< CHudBossEnergyInterface > mBossEnergy;
  CGuiTextPane* mEnergyDigits;
  CGuiTextPane* mMissileDigits;
  CGuiWidget* mMissileFraction;
  CGuiTextPane* mEnergyWarning;
  CGuiTextPane* mLightAmmoDigits;
  CGuiTextPane* mDarkAmmoDigits;
  CGuiTextPane* mCounter;
  CGuiWidget* mDarkVisor;
  CGuiWidget* mDarkVisorBacking;
  rstl::reserved_vector< CGuiWidget*, 14 > mFilledEnergyTanks;
  rstl::reserved_vector< CGuiWidget*, 14 > mEmptyEnergyTanks;
  CTransform4f mEnergyBracketTransform;
  CGuiWidget* mEnergyBracket;
  CAuiEnergyBarT01* mEnergyBar;
  CAuiMeter* mMissileGauge;
  rstl::reserved_vector< CGuiWidget*, 5 > mDarkAmmoSegments;
  rstl::reserved_vector< CAuiMeter*, 5 > mDarkAmmoMeters;
  rstl::reserved_vector< CGuiWidget*, 5 > mLightAmmoSegments;
  rstl::reserved_vector< CAuiMeter*, 5 > mLightAmmoMeters;
  float x7ec;
  CGuiWidget* mDecorationRoot;
  CGuiWidget* mThreatIcon;
  CGuiWidget* mThreatBar;
  CGuiWidget* mThreatRoot;
  CAuiMeter* mThreatGauge;
  CGuiWidget* mMissileIcon;
  CGuiWidget* mLightAmmoIcon;
  CGuiWidget* mDarkAmmoIcon;
  float mThreatAlpha;
  rstl::single_ptr< CHudDecoInterfaceScan > mScanInterface;
  CGuiTextPane* mPowerBombDigits;
  CGuiWidget* mPowerBombIcon;
  CGuiWidget* mPowerBombDecoration;
  rstl::reserved_vector< CGuiWidget*, 3 > mBombIndicators;
  float mBallBeamTransition;
  CPlayerState::EBeamId mPreviousBallBeam;
  CGuiCamera* mHudCamera;
  CGuiWidget* mAutomapperRoot;
  CGuiWidget* mAutomapperModel;
  rstl::auto_ptr< CGuiFrameLoader > mPendingHudFrame;
  int mGuiState;
  CColor mHudColor;
  float mBootTimer;
  float mBootTextFade;
  float mHudBootAlpha;
  float mCorruptTextTimer;
  CGuiTextSupport mBootText;
  bool mBooting;
  rstl::auto_ptr< CGuiFrameLoader > mBossLockOnFrameLoader;
  rstl::auto_ptr< CGuiFrame > mBossLockOnFrame;
  rstl::reserved_vector< SProfileInfo, 17 > mProfileInfo;
};

CHECK_SIZEOF(CSamusHud, 0x1698)

#endif // _CSAMUSHUD
