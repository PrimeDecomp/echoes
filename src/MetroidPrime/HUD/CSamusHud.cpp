#include "MetroidPrime/HUD/CSamusHud.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiFrameLoader.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CResFactory.hpp"
#include "MetroidPrime/HUD/CHudBossEnergyInterface.hpp"
#include "MetroidPrime/HUD/CHudDecoInterfaceScan.hpp"
#include "MetroidPrime/HUD/CHudRadarInterface.hpp"
#include "MetroidPrime/HUD/CHudVisorBeamMenu.hpp"

#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"

// Structure-first scaffold. Unrecovered behavior is marked at each entry point.

CSamusHud* gpSamusHud[4] = {nullptr, nullptr, nullptr, nullptr};

static void StopSound(CSfxHandle& sound) {
  if (sound) {
    CSfxManager::SfxStop(sound);
    sound.Clear();
  }
}

void CSamusHud::StopSounds(const CStateManager&) {
  StopSound(mDamageSound);
  StopSound(mStaticSoundLow);
  StopSound(mStaticSoundHigh);
  StopSound(mFreeLookSound);
}

void CSamusHud::ProcessControllerInput(const CFinalInput& input) {
  if (!mScanInterface.null()) {
    mScanInterface->ProcessControllerInput(input);
  }
}

void CSamusHud::PrepareScanDisplay(const CStateManager& mgr, int playerIndex) {
  if (!mScanInterface.null()) {
    mScanInterface->PrepareScanDisplay(mgr, playerIndex);
  }
}

CHudDecoInterfaceScan* CSamusHud::GetScanInterface(int playerIndex) {
  CSamusHud* hud = gpSamusHud[playerIndex];
  return hud != nullptr ? hud->mScanInterface.get() : nullptr;
}

CSamusHud::CSamusHud(const CStateManager& mgr, CGuiFrameLoader& hud, CGuiFrameLoader& memo,
                     CGuiFrameLoader* helmet, int playerIndex)
: mPlayerIndex(playerIndex)
, mLoadPhase(kLP_Targeting)
, mTargetingManager(mgr, playerIndex)
, mHudFrame(hud.CreateFrame())
, mLoadedHudFrame(mHudFrame.get())
, mHelmetFrame(helmet != nullptr ? helmet->CreateFrame() : nullptr)
, mLoadedHelmetFrame(mHelmetFrame.get())
, mMemoFrame(memo.CreateFrame())
, mLoadedMemoFrame(mMemoFrame.get())
, mPreviousState(kHS_None)
, mNextState(kHS_None)
, mDesiredState(kHS_Combat)
, mTransitionState(kTS_Idle)
, mTransitionFactor(1.f)
, mPlayerHealth(0.f)
, mEnergyTankCapacity(0)
, mMissileAmount(0)
, mMissileCapacity(0)
, mMissileModeTransition(0.f)
, mAmmoBeam(CPlayerState::kBI_Power)
, mDarkAmmo(0)
, mLightAmmo(0)
, mInFreeLook(false)
, mLookControlHeld(false)
, mFirstPerson(true)
, mEnergyLow(false)
, mEnglishOnly(false)
, mMenuBeam(CPlayerState::kBI_Power)
, mMenuVisor(CPlayerState::kPV_Combat)
, mMissileEnabled(0)
, mPreviousCameraDirection(CVector3f::Forward())
, mHudLag(CQuaternion::NoRotation())
, mInverseHudLag(CQuaternion::NoRotation())
, mHelmetLightingWidget(mLoadedHelmetFrame != nullptr
                            ? mLoadedHelmetFrame->FindWidget("BaseWidget_Helmet_TX")
                            : nullptr)
, mLights(rs_new CActorLights(8, CVector3f::Zero(), 4, 1, 0.1f, true, false, false, false))
, mHudLights(3, SCachedHudLight())
, mHudStringTable(gpSimplePool->GetObj("STRG_Hud"))
, mDamageTime(0.f)
, mDamagePulse(0.f)
, mDamageFilterDuration(1.f)
, mDamageFilterRemaining(0.f)
, mDamageFilterGain(0.f)
, mDamageHighlightDuration(1.f)
, mDamageHighlightRemaining(0.f)
, mDamageSectorDurations(12, 0.f)
, mDamageSectorRemaining(12, 0.f)
, mDamageSectorIntensity(12, 0.f)
, mDamageRingTexture(gpSimplePool->GetObj("TXTR_QuarterCurve"))
, mDamagerToPlayer(CVector3f::Zero())
, mShakeTranslationAmount(0.f)
, mShakeTranslationVelocity(0.f)
, mShakeTranslation(CVector3f::Zero())
, mShakeRotation(CMatrix3f::Identity())
, mHudLagShake(CQuaternion::NoRotation())
, mShakeDuration(0.f)
, mShakeRemaining(0.f)
, mShakeGain(0.f)
, mViewportScaleX(1.f)
, mViewportScaleY(1.f)
, mStaticInterference(0.f)
, mStaticCycleLow(0.f)
, mStaticCycleHigh(0.f)
, mHudMemoParms(0.f, false, false, false, 0xf, true)
, mHudMemoIndex(0)
, mMessageRoot(nullptr)
, mMessagePane(nullptr)
, mMessageAButton(nullptr)
, mMessageTime(0.f)
, mLastMessageSoundChars(0.f)
, mPreviousFreeLookDirection(CVector3f::Forward())
, mFreeLookDirectionDot(1.f)
, mFreeLookSoundCycle(0.f)
, mFreeLookLeft(nullptr)
, mFreeLookRight(nullptr)
, mFreeLookLeftTransform(CTransform4f::Identity())
, mFreeLookRightTransform(CTransform4f::Identity())
, mFreeLookFade(0.f)
, mEnergyLowTimer(0.f)
, mEnergyLowPulse(0.f)
, mEnergyLowFade(0.f)
, mAButtonPulse(0.f)
, x670(9999.f)
, mThreatAmount(0.f)
, mCurrentBeam(mgr.GetPlayerState(playerIndex)->GetCurrentBeam())
, mPreviousBeam(mCurrentBeam)
, mBeamMenuTransition(1.f)
, mVisorBracket(nullptr)
, mMissilePickupPulse(0.f)
, mLightAmmoPickupPulse(0.f)
, mDarkAmmoPickupPulse(0.f)
, mEnergyDigits(nullptr)
, mMissileDigits(nullptr)
, mMissileFraction(nullptr)
, mEnergyWarning(nullptr)
, mLightAmmoDigits(nullptr)
, mDarkAmmoDigits(nullptr)
, mCounter(nullptr)
, mDarkVisor(nullptr)
, mDarkVisorBacking(nullptr)
, mEnergyBracketTransform(CTransform4f::Identity())
, mEnergyBracket(nullptr)
, mEnergyBar(nullptr)
, mMissileGauge(nullptr)
, x7ec(0.f)
, mDecorationRoot(nullptr)
, mThreatIcon(nullptr)
, mThreatBar(nullptr)
, mThreatRoot(nullptr)
, mThreatGauge(nullptr)
, mMissileIcon(nullptr)
, mLightAmmoIcon(nullptr)
, mDarkAmmoIcon(nullptr)
, mThreatAlpha(1.f)
, mPowerBombDigits(nullptr)
, mPowerBombIcon(nullptr)
, mPowerBombDecoration(nullptr)
, mBallBeamTransition(0.f)
, mPreviousBallBeam(CPlayerState::kBI_Power)
, mHudCamera(nullptr)
, mAutomapperRoot(nullptr)
, mAutomapperModel(nullptr)
, mGuiState(3)
, mHudColor(0u)
, mBootTimer(0.f)
, mBootTextFade(0.f)
, mHudBootAlpha(1.f)
, mCorruptTextTimer(0.f)
, mBootText(gpResourceFactory->GetResourceIdByName("FONT_Deface13B")->id, 420, 400,
            CGuiTextProperties(true, kJustification_Left, kVerticalJustification_Bottom),
            CColor::White(), CColor::White().WithAlphaOf(0.f), CColor::White(), gpSimplePool)
, mBooting(false)
, mProfileInfo(17, SProfileInfo()) {
  // TODO: Initialize camera direction, energy-low state, language and boot-text tint from shared
  // APIs.
  // TODO: Select the multiplayer damage-ring texture.
  gpSamusHud[mPlayerIndex] = this;
  mDamageRingTexture.Lock();
  if (mgr.fn_80036F10()) {
    mLockedOnIndicator = TCachedToken< CTexture >(gpSimplePool->GetObj("TXTR_LockedOnIndicator"));
    mLockedOnIndicator->Lock();
  }
  mNextState = kHS_Combat;
  if (mgr.GetPlayer(mPlayerIndex)->GetMorphballTransitionState() != CPlayer::kMS_Unmorphed) {
    mTransitionFactor = 0.f;
  }
  UpdateHudColor();
  InitializeFrameGluePermanent();
}

CSamusHud::~CSamusHud() {
  if (mDamageSound) {
    CSfxManager::SfxStop(mDamageSound);
  }
  gpSamusHud[mPlayerIndex] = nullptr;
}

CSamusHud::EHudState CSamusHud::GetDesiredHudState(const CStateManager& mgr) const {
  const CPlayer::EPlayerMorphBallState morphState =
      mgr.GetPlayer(mPlayerIndex)->GetMorphballTransitionState();
  if (morphState == CPlayer::kMS_Morphed || morphState == CPlayer::kMS_Morphing ||
      morphState == CPlayer::kMS_Unmorphing) {
    return kHS_Ball;
  }
  switch (mgr.GetPlayerState(mPlayerIndex)->GetTransitioningVisor()) {
  case CPlayerState::kPV_Combat:
    return kHS_Combat;
  case CPlayerState::kPV_Echo:
    return kHS_Echo;
  case CPlayerState::kPV_Scan:
    return kHS_Scan;
  case CPlayerState::kPV_Dark:
    return kHS_Dark;
  default:
    return kHS_None;
  }
}

CSamusHud::EHudState CSamusHud::GetNextState() const {
  return mNextState == kHS_None ? kHS_Combat : mNextState;
}

const CTargetingManager& CSamusHud::GetTargetingManager() const { return mTargetingManager; }

CColor CSamusHud::ModulateColor(const CColor& color) const {
  return CColor::Modulate(mHudColor, color);
}

rstl::reserved_vector< bool, 4 > CSamusHud::BuildPlayerHasBeams(const CStateManager& mgr) const {
  const CPlayerState& state = *mgr.GetPlayerState(mPlayerIndex);
  rstl::reserved_vector< bool, 4 > result;
  result.push_back(state.HasPowerUp(CPlayerState::kIT_PowerBeam));
  result.push_back(state.HasPowerUp(CPlayerState::kIT_DarkBeam));
  result.push_back(state.HasPowerUp(CPlayerState::kIT_LightBeam));
  result.push_back(state.HasPowerUp(CPlayerState::kIT_AnnihilatorBeam));
  return result;
}

rstl::reserved_vector< bool, 4 > CSamusHud::BuildPlayerHasVisors(const CStateManager& mgr) const {
  const CPlayerState& state = *mgr.GetPlayerState(mPlayerIndex);
  rstl::reserved_vector< bool, 4 > result;
  result.push_back(state.HasPowerUp(CPlayerState::kIT_CombatVisor));
  result.push_back(state.HasPowerUp(CPlayerState::kIT_EchoVisor));
  result.push_back(state.HasPowerUp(CPlayerState::kIT_ScanVisor));
  result.push_back(state.HasPowerUp(CPlayerState::kIT_DarkVisor));
  return result;
}

int CSamusHud::FindEmptyHudLightSlot(const CLight&) const {
  for (int i = 0; i < 3; ++i) {
    if (mHudLights[i].mFade == 0.f) {
      return i;
    }
  }
  return -1;
}

bool CSamusHud::IsAreaLightInCachedLights(const CLight& light) const {
  for (int i = 0; i < 3; ++i) {
    const SCachedHudLight& cached = mHudLights[i];
    if (cached.mFade != 0.f && cached.mColor == light.GetColor() &&
        cached.mPosition == light.GetPosition()) {
      return true;
    }
  }
  return false;
}

void CSamusHud::fn_8006653c(const CStateManager&, bool) {}

bool CSamusHud::ResolveLockOnTexture() {
  return mLockedOnIndicator && mLockedOnIndicator->IsLoaded();
}

CTransform4f CSamusHud::BuildFinalCameraTransform(const CQuaternion& rotation,
                                                  const CVector3f& pivot,
                                                  const CVector3f& cameraPosition) {
  const CQuaternion inverse = rotation.BuildInverted();
  return CTransform4f(inverse.BuildTransform(), inverse.Transform(cameraPosition - pivot) + pivot);
}

rstl::pair< CVector3f, CVector3f > CSamusHud::BallEnergyCoordFunc(float t) {
  return rstl::pair< CVector3f, CVector3f >(CVector3f(8.5f * t, 0.f, 0.f),
                                            CVector3f(8.5f * t, 0.f, 0.4f));
}

rstl::pair< CVector3f, CVector3f > CSamusHud::CombatEnergyCoordFunc(float t) {
  const float angle = 0.5294118f * t - 0.20262942f;
  const float x = 17.f * sin(angle);
  const float y = 0.2f + (17.f * cos(angle) - 17.f);
  return rstl::pair< CVector3f, CVector3f >(CVector3f(x, y, 0.4f), CVector3f(x, y, 0.f));
}

void CSamusHud::InternalDisplayHudMemo(const rstl::wstring& text, const CHUDMemoParms& info) {
  mHudMemoParms = info;
  mHudMemoString = nullptr;
  SetMessage(text, info);
}

void CSamusHud::InternalDeferHintMemo(CAssetId stringTable, uint index, const CHUDMemoParms& info) {
  mHudMemoParms = info;
  mHudMemoString =
      rs_new TToken< CStringTable >(gpSimplePool->GetObj(SObjectTag('STRG', stringTable)));
  mHudMemoString->Lock();
  mHudMemoIndex = index;
}

void CSamusHud::DisplayHudMemo(const rstl::wstring& text, const CHUDMemoParms& info) {
  for (int i = 0; i < 4; ++i) {
    if (gpSamusHud[i] != nullptr && info.EnabledForPlayer(i)) {
      gpSamusHud[i]->InternalDisplayHudMemo(text, info);
    }
  }
}

void CSamusHud::DeferHintMemo(CAssetId stringTable, uint index, const CHUDMemoParms& info) {
  for (int i = 0; i < 4; ++i) {
    if (gpSamusHud[i] != nullptr && info.EnabledForPlayer(i)) {
      gpSamusHud[i]->InternalDeferHintMemo(stringTable, index, info);
    }
  }
}

void CSamusHud::DrawBossLockOnWarning() const {
  // TODO: draw the loaded boss-lock warning frame.
}

void CSamusHud::UpdateBossLockOnWarning(float dt, const CStateManager& mgr) {
  // TODO: load and animate the boss-lock warning frame.
}

void CSamusHud::SetMessage(const rstl::wstring& text, const CHUDMemoParms& info) {
  // TODO: apply memo visibility, typewriter settings and fade transitions.
}

void CSamusHud::ApplyClassicLag(const CUnitVector3f& lookDirection, CQuaternion& rotation,
                                const CStateManager& mgr, float dt, bool invert) {
  // TODO: recover camera lag through shared quaternion helpers.
}

void CSamusHud::UpdateHudLag(float dt, const CStateManager& mgr) {
  // TODO: combine camera lag with damage shake.
}

CRelAngle CSamusHud::GetRelativeDirection(const CVector3f& position,
                                          const CStateManager& mgr) const {
  // TODO: measure the direction in the player's camera plane.
  return CRelAngle::FromRadians(0.f);
}

void CSamusHud::DrawHudMemo() const {
  // TODO: draw the loaded memo frame.
}

void CSamusHud::DrawHelmet(const CStateManager& mgr, float cameraYOffset) const {
  // TODO: draw helmet geometry with its camera offset.
}

void CSamusHud::Draw(const CStateManager& mgr, float alpha, uint helmetVisibility, bool hudVisible,
                     bool targetingVisible) const {
  // TODO: restore the HUD, targeting, overlays and profiling draw sequence.
}

void CSamusHud::LeaveFirstPerson(const CStateManager& mgr) {
  // TODO: mute both static-interference sounds through CSfxManager.
}

void CSamusHud::EnterFirstPerson(const CStateManager& mgr) {
  // TODO: restore both static-interference sound volumes.
}

void CSamusHud::DrawPlayerFilter(const CStateManager& mgr) const {
  // TODO: draw the player-specific color filter.
}

void CSamusHud::DrawAttachedEnemyEffect(const CStateManager& mgr) const {
  // TODO: draw the attached-enemy overlay.
}

void CSamusHud::DrawLockOnIndicators(const CStateManager& mgr) const {
  // TODO: collect multiplayer opponents or the single-player locking actor.
}

void CSamusHud::DrawLockOnIndicators(const CStateManager& mgr,
                                     const rstl::reserved_vector< TUniqueId, 12 >& targets) const {
  // TODO: draw directional warnings for the supplied targets.
}

void CSamusHud::Update(float dt, const CStateManager& mgr, uint helmetVisibility, bool hudVisible,
                       bool targetingVisible) {
  // TODO: restore update ordering, visibility changes and profiling.
}

void CSamusHud::UpdateHudMemo(float dt, const CStateManager& mgr) {
  // TODO: resolve deferred text and update message and A-button fades.
}

void CSamusHud::UpdateBootSequence(float dt, const CStateManager& mgr) {
  // TODO: animate BootText, GearLostText and corrupted HUD text.
}

void CSamusHud::UpdateHudFrame(float dt, const CStateManager& mgr) {
  UpdateStateTransition(dt, mgr);
  if (mLoadedHudFrame != nullptr) {
    mLoadedHudFrame->Update(dt);
  }
}

void CSamusHud::UpdateStateTransition(float dt, const CStateManager& mgr) {
  // TODO: fade out, load the desired frame, rebind widgets and fade in.
}

void CSamusHud::UpdateHudDamage(float dt, const CStateManager& mgr) {
  // TODO: update the directional damage sectors, filters and shake.
}

CColor CSamusHud::GetVisorHudLightColor(const CColor& color, const CStateManager& mgr) const {
  // TODO: apply the visor-dependent light tint.
  return color;
}

void CSamusHud::UpdateHudDynamicLights(float dt, const CStateManager& mgr) {
  // TODO: reconcile the three cached lights with area lighting.
}

bool CSamusHud::IsCachedLightInAreaLights(const SCachedHudLight& light,
                                          const CActorLights& lights) const {
  // TODO: compare position and color using CActorLights' recovered area-light interface.
  return false;
}

void CSamusHud::UpdateThreatAssessment(float dt, const CStateManager& mgr) {
  // TODO: evaluate nearby threats and update the warning gauge.
}

void CSamusHud::UpdateBallMode(const CStateManager& mgr) {
  // TODO: update bomb availability, power-bomb digits and ball beam transition.
}

void CSamusHud::UpdateBeamAmmo(const CStateManager& mgr, bool init) {
  // TODO: update dark/light ammo digits, segmented meters and pickup flashes.
}

void CSamusHud::UpdateMissile(float dt, const CStateManager& mgr, bool init) {
  // TODO: update missile amount, capacity, enabled state and pickup pulse.
}

void CSamusHud::UpdateEnergy(float dt, const CStateManager& mgr, bool init) {
  // TODO: update health digits, energy tanks and the energy bar.
}

void CSamusHud::UpdateHudColor() {
  // TODO: select the HUD tint from the current visor and tweaks.
}

void CSamusHud::UpdateStaticSfx(const CStateManager& mgr, CSfxHandle& sound, float& cycle,
                                ushort soundId, float dt, float previousInterference,
                                float threshold) {
  // TODO: start, cycle or stop one interference sound.
}

void CSamusHud::UpdateStaticInterference(float dt, const CStateManager& mgr) {
  // TODO: combine player interference with HUD alpha, filters and sounds.
}

void CSamusHud::UpdateFreeLook(float dt, const CStateManager& mgr) {
  // TODO: update freelook indicators, transforms and movement sound.
}

void CSamusHud::UpdateVisorAndBeamMenus(float dt, const CStateManager& mgr) {
  // TODO: update menu selections and their transition factors.
}

bool CSamusHud::CheckLoadComplete(const CStateManager& mgr) {
  switch (mLoadPhase) {
  case kLP_Targeting:
    if (!mTargetingManager.CheckLoadComplete()) {
      return false;
    }
    mLoadPhase = kLP_Frames;
    InitializeFrameGlueMutable(mgr);
    UpdateEnergy(0.f, mgr, true);
    UpdateMissile(0.f, mgr, true);
    UpdateBeamAmmo(mgr, true);
    UpdateBallMode(mgr);
    fn_8006653c(mgr, true);
    ResolveLockOnTexture();
    // Fall through.
  case kLP_Frames:
    if (!mLoadedHudFrame->GetIsFinishedLoading() ||
        (mLoadedHelmetFrame != nullptr && !mLoadedHelmetFrame->GetIsFinishedLoading())) {
      return false;
    }
    mLoadPhase = kLP_Complete;
    // Fall through.
  case kLP_Complete:
    return true;
  default:
    return false;
  }
}

void CSamusHud::UpdateEnergyLow(float dt, const CStateManager& mgr) {
  // TODO: animate the low-energy warning and pulse.
}

void CSamusHud::RefreshHudStringTable() {
  // TODO: select STRG_Hud or STRG_HudEngOnly from the language option.
}

bool CSamusHud::IsHudMemoVisible(int playerIndex) {
  // TODO: query the player's memo widgets for visibility.
  return false;
}

void CSamusHud::RefreshBeamMenu(const CStateManager& mgr, int playerIndex) {
  CSamusHud* hud = gpSamusHud[playerIndex];
  if (hud != nullptr) {
    const rstl::reserved_vector< bool, 4 > enables = hud->BuildPlayerHasBeams(mgr);
    if (!hud->mBeamMenu.null()) {
      hud->mBeamMenu->SetPlayerHas(enables, mgr.GetPlayerState(playerIndex)->GetCurrentBeam());
    }
  }
}

void CSamusHud::UpdateHudWidgetColors() {
  // TODO: apply visor-specific widget colors and rebuild boss-energy glue.
}

void CSamusHud::UpdateHelmetWidgets() {
  // TODO: show the current visor's helmet-light pair.
}

void CSamusHud::UninitializeFrameGlueMutable() {
  mFilledEnergyTanks.clear();
  mEmptyEnergyTanks.clear();
  mDarkAmmoSegments.clear();
  mDarkAmmoMeters.clear();
  mLightAmmoSegments.clear();
  mLightAmmoMeters.clear();
  mBombIndicators.clear();
  mVisorMenu = nullptr;
  mBeamMenu = nullptr;
  mRadar = nullptr;
  mDecorationRoot = nullptr;
  mEnergyBar = nullptr;
  mEnergyDigits = nullptr;
  mMissileDigits = nullptr;
  mMissileFraction = nullptr;
  mEnergyWarning = nullptr;
  mVisorBracket = nullptr;
  mThreatIcon = nullptr;
  mThreatRoot = nullptr;
  mThreatBar = nullptr;
  mMissileIcon = nullptr;
  mDarkVisor = nullptr;
  mDarkVisorBacking = nullptr;
  mLightAmmoDigits = nullptr;
  mDarkAmmoDigits = nullptr;
  mLightAmmoIcon = nullptr;
  mDarkAmmoIcon = nullptr;
  mFreeLookLeft = nullptr;
  mFreeLookRight = nullptr;
  mMissileGauge = nullptr;
  mThreatGauge = nullptr;
  mEnergyBracket = nullptr;
  mBossEnergy = nullptr;
  mHudCamera = nullptr;
  mAutomapperRoot = nullptr;
  mAutomapperModel = nullptr;
  mPowerBombDigits = nullptr;
  mPowerBombIcon = nullptr;
  mPowerBombDecoration = nullptr;
  mCounter = nullptr;
  mLoadedHudFrame = nullptr;
  mHudFrame = rstl::auto_ptr< CGuiFrame >();
}

void CSamusHud::InitializeFrameGlueMutable(const CStateManager& mgr) {
  // TODO: bind visor/ball widgets and construct their typed HUD interfaces.
}

void CSamusHud::InitializeFrameGluePermanent() {
  // TODO: bind the memo text, A-button and message root.
}

void CSamusHud::ShowDamage(CVector3f position, float damage, float previousDamage,
                           const CStateManager& mgr) {
  // TODO: set the directional damage sector, sound, filter and shake parameters.
}
