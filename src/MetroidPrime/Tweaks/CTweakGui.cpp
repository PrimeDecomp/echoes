#include "MetroidPrime/Tweaks/CTweakGui.hpp"

#include "MetroidPrime/ScriptLoader/SLdrTweakGui.hpp"

// Guessed name; converts the serialized fog selector to the shared graphics enum.
static ERglFogMode ConvertFogMode(int mode) {
  switch (mode) {
  case 0:
  default:
    return kRFM_None;
  case 1:
    return kRFM_PerspLin;
  case 2:
    return kRFM_PerspExp;
  case 3:
    return kRFM_PerspExp2;
  }
}

float CTweakGui::FaceReflectionDistanceDebugValueToActualValue(float value) {
  return 0.2f + 0.015f * value;
}

float CTweakGui::FaceReflectionHeightDebugValueToActualValue(float value) {
  return 0.005f * value - 0.05f;
}

float CTweakGui::FaceReflectionAspectDebugValueToActualValue(float value) {
  return 1.f + 0.05f * value;
}

float CTweakGui::FaceReflectionOrthoWidthDebugValueToActualValue(float value) {
  return 0.02f + 0.007f * value;
}

float CTweakGui::FaceReflectionOrthoHeightDebugValueToActualValue(float value) {
  return 0.02f + 0.007f * value;
}

float CTweakGui::GetMapAlphaInterpolant() const { return mData->misc.minHUDAlpha; }

float CTweakGui::GetPauseBlurFactor() const { return mData->misc.unknown_0x744165c4; }

float CTweakGui::GetRadarWorldRadius() const { return mData->misc.radarWorldRadius; }

float CTweakGui::GetRadarWorldHalfHeight() const { return mData->misc.radarWorldHalfHeight; }

float CTweakGui::GetRadarZCloseRadius() const { return mData->misc.unknown_0x889ef9ea; }

float CTweakGui::GetEnergyBarFilledDrainSpeed() const {
  return mData->misc.energyBarFilledDrainSpeed;
}

float CTweakGui::GetEnergyBarShadowDrainSpeed() const {
  return mData->misc.energyBarShadowDrainSpeed;
}

float CTweakGui::GetEnergyBarShadowDrainDelay() const {
  return mData->misc.energyBarShadowDrainDelay;
}

bool CTweakGui::GetEnergyBarAlwaysResetDelay() const { return mData->misc.unknown_0x2821bbca; }

float CTweakGui::GetHUDDamageIndicatorRadius() const {
  return mData->misc.hudDamageIndicatorRadius;
}

float CTweakGui::GetHUDFlashMagnitudeConstant() const {
  return mData->misc.hudFlashMagnitudeConstant;
}

float CTweakGui::GetHUDFlashMagnitudeLinear() const { return mData->misc.hudFlashMagnitudeLinear; }

float CTweakGui::GetHUDFlashTimeConstant() const { return mData->misc.hudFlashTimeConstant; }

float CTweakGui::GetHUDFlashTimeScaleLinear() const { return mData->misc.hudFlashTimeScaleLinear; }

float CTweakGui::GetHUDDamageJostleMagnitudeConstant() const {
  return mData->misc.hudDamageJostleMagnitudeConstant;
}

float CTweakGui::GetHUDDamageJostleMagnitudeLinear() const {
  return mData->misc.hudDamageJostleMagnitudeLinear;
}

float CTweakGui::GetHUDDamageJostleMaxOffset() const {
  return mData->misc.hudDamageJostleMaxOffset;
}

float CTweakGui::GetHUDDamageJostleReturnAcceleration() const {
  return mData->misc.hudDamageJostleReturnAcceleration;
}

float CTweakGui::GetHUDDamageDistortionMagnitudeConstant() const {
  return mData->misc.hudDamageDistortionMagnitudeConstant;
}

float CTweakGui::GetHUDDamageDistortionMagnitudeLinear() const {
  return mData->misc.hudDamageDistortionMagnitudeLinear;
}

float CTweakGui::GetHUDDamageDistortionTimeConstant() const {
  return mData->misc.hudDamageDistortionTimeConstant;
}

float CTweakGui::GetHUDDamageDistortionTimeLinear() const {
  return mData->misc.hudDamageDistortionTimeLinear;
}

float CTweakGui::GetHUDDamageDistortionMaxMagnitude() const {
  return mData->misc.hudDamageDistortionMaxMagnitude;
}

float CTweakGui::GetBeamVisorMenuAnimTime() const { return mData->misc.unknown_0xba53888e; }

float CTweakGui::GetVisorBeamMenuItemActiveScale() const { return mData->misc.unknown_0xcc7ae923; }

float CTweakGui::GetVisorBeamMenuItemInactiveScale() const {
  return mData->misc.unknown_0x8c723b8f;
}

float CTweakGui::GetVisorBeamMenuItemTranslate() const { return mData->misc.unknown_0x1f228e64; }

float CTweakGui::GetThreatWorldRadius() const { return mData->misc.threatWorldRadius; }

float CTweakGui::GetRadarScopeCoordRadius() const { return mData->misc.unknown_0x78174d4b; }

float CTweakGui::GetRadarPlayerPaintRadius() const { return mData->misc.unknown_0xfc30bb21; }

uint CTweakGui::GetHudVisMode() const { return mData->misc.unknown_0xf3565ff4; }

uint CTweakGui::GetEnableAutoMapper() const { return mData->misc.unknown_0x72d4d899; }

uint CTweakGui::GetEnableTargetingManager() const { return mData->misc.unknown_0xa1417b38; }

uint CTweakGui::GetEnablePlayerVisor() const { return mData->misc.unknown_0x71b207b4; }

float CTweakGui::GetMissileWarningThreshold() const { return mData->misc.missileWarningThreshold; }

float CTweakGui::GetFreeLookSfxPitchScale() const { return mData->misc.unknown_0xc0004f50; }

bool CTweakGui::GetNoAbsoluteFreeLookSfxPitch() const { return mData->misc.unknown_0x4ee9c251; }

int CTweakGui::GetFaceReflectionOrthoWidthDebugValue() const {
  return mData->misc.unknown_0x4ff930a5;
}

int CTweakGui::GetFaceReflectionOrthoHeightDebugValue() const {
  return mData->misc.unknown_0x1c106df3;
}

int CTweakGui::GetFaceReflectionDistanceDebugValue() const {
  return mData->misc.unknown_0xdaadf917;
}

int CTweakGui::GetFaceReflectionHeightDebugValue() const { return mData->misc.unknown_0xe3d55457; }

int CTweakGui::GetFaceReflectionAspectDebugValue() const { return mData->misc.unknown_0xdd39f60a; }

float CTweakGui::GetFaceReflectionLightFalloffMultConstant() const {
  return mData->misc.unknown_0xcfb88ceb;
}

float CTweakGui::GetFaceReflectionLightFalloffMultLinear() const {
  return mData->misc.unknown_0x5e388dd0;
}

float CTweakGui::GetFaceReflectionLightFalloffMultQuadratic() const {
  return mData->misc.unknown_0x86bc055e;
}

float CTweakGui::GetHudDamagePeakFactor() const { return mData->misc.unknown_0x3aaf2a8c; }

float CTweakGui::GetFlashPassMagnitudeConstant() const {
  return mData->misc.flashPassMagnitudeConstant;
}

float CTweakGui::GetFlashPassMagnitudeLinear() const {
  return mData->misc.flashPassMagnitudeLinear;
}

float CTweakGui::GetFlashPassTimerConstant() const { return mData->misc.flashPassTimerConstant; }

float CTweakGui::GetFlashPassTimerLinear() const { return mData->misc.flashPassTimerLinear; }

float CTweakGui::GetEnergyDrainModPeriod() const { return mData->misc.unknown_0x02a2198a; }

bool CTweakGui::GetEnergyDrainSinusoidalPulse() const { return mData->misc.unknown_0x8b64dc44; }

bool CTweakGui::GetEnergyDrainFilterAdditive() const { return mData->misc.unknown_0x7161446b; }

float CTweakGui::GetHudDamagePulseDuration() const { return mData->misc.unknown_0xaaff9224; }

float CTweakGui::GetHudDamageColorGain() const { return mData->misc.unknown_0xfa4a836c; }

float CTweakGui::GetHudDecoShakeTranslateGain() const { return mData->misc.unknown_0x23661b4f; }

float CTweakGui::GetHudLagOffsetScale() const { return mData->misc.unknown_0x992b647a; }

float CTweakGui::GetScanSidesDuration() const { return mData->misc.unknown_0xeb6a7f2a; }

float CTweakGui::GetScanSidesStartTime() const { return mData->misc.unknown_0xd05eb27a; }

float CTweakGui::GetScanSidesEndTime() const {
  return mData->misc.unknown_0xeb6a7f2a + mData->misc.unknown_0xd05eb27a;
}

float CTweakGui::GetScanObjectModelScale() const { return mData->misc.scanObjectModelScale; }

CMayaSpline& CTweakGui::GetScanObjectTranslateTransitionSpline() const {
  return const_cast< CMayaSpline& >(mData->misc.scanObjectTranslateTransitionSpline);
}

CMayaSpline& CTweakGui::GetScanObjectRotationTransitionSpline() const {
  return const_cast< CMayaSpline& >(mData->misc.scanObjectRotationTransitionSpline);
}

CMayaSpline& CTweakGui::GetScanObjectScaleTransitionSpline() const {
  return const_cast< CMayaSpline& >(mData->misc.scanObjectScaleTransitionSpline);
}

float CTweakGui::GetBallViewportYReduction() const { return mData->misc.unknown_0xeeb7839b; }

float CTweakGui::GetScanWindowIdleWidth() const { return mData->misc.unknown_0x24cf1719; }

float CTweakGui::GetScanWindowIdleHeight() const { return mData->misc.unknown_0xa4adf6ea; }

float CTweakGui::GetScanWindowActiveWidth() const { return mData->misc.unknown_0xe3755dda; }

float CTweakGui::GetScanWindowActiveHeight() const { return mData->misc.unknown_0xa607dfaa; }

float CTweakGui::GetScanWindowMagnification() const { return mData->misc.unknown_0xf5f7a748; }

float CTweakGui::GetScanWindowScanningAspect() const { return mData->misc.unknown_0x61215643; }

float CTweakGui::GetScanSidesPositionStart() const { return mData->misc.unknown_0xa3f4095e; }

float CTweakGui::GetWorldTransManagerCharsPerSfx() const { return mData->misc.unknown_0x22d4c6a3; }

CColor CTweakGui::GetHelmetBaseAmbientColorCombatLightWorld() const {
  return mData->misc.helmetBaseAmbientColorCombatLightWorld;
}

CColor CTweakGui::GetHelmetBaseAmbientColorEchoLightWorld() const {
  return mData->misc.helmetBaseAmbientColorEchoLightWorld;
}

CColor CTweakGui::GetHelmetBaseAmbientColorScanLightWorld() const {
  return mData->misc.helmetBaseAmbientColorScanLightWorld;
}

CColor CTweakGui::GetHelmetBaseAmbientColorDarkLightWorld() const {
  return mData->misc.helmetBaseAmbientColorDarkLightWorld;
}

CColor CTweakGui::GetHelmetBaseAmbientColorCombatDarkWorld() const {
  return mData->misc.helmetBaseAmbientColorCombatDarkWorld;
}

CColor CTweakGui::GetHelmetBaseAmbientColorEchoDarkWorld() const {
  return mData->misc.helmetBaseAmbientColorEchoDarkWorld;
}

CColor CTweakGui::GetHelmetBaseAmbientColorScanDarkWorld() const {
  return mData->misc.helmetBaseAmbientColorScanDarkWorld;
}

CColor CTweakGui::GetHelmetBaseAmbientColorDarkDarkWorld() const {
  return mData->misc.helmetBaseAmbientColorDarkDarkWorld;
}

CColor CTweakGui::GetHelmetLightAmbientModCombatLightWorld() const {
  return mData->misc.helmetLightAmbientModCombatLightWorld;
}

CColor CTweakGui::GetHelmetLightAmbientModEchoLightWorld() const {
  return mData->misc.helmetLightAmbientModEchoLightWorld;
}

CColor CTweakGui::GetHelmetLightAmbientModScanLightWorld() const {
  return mData->misc.helmetLightAmbientModScanLightWorld;
}

CColor CTweakGui::GetHelmetLightAmbientModDarkLightWorld() const {
  return mData->misc.helmetLightAmbientModDarkLightWorld;
}

CColor CTweakGui::GetHelmetLightAmbientModCombatDarkWorld() const {
  return mData->misc.helmetLightAmbientModCombatDarkWorld;
}

CColor CTweakGui::GetHelmetLightAmbientModEchoDarkWorld() const {
  return mData->misc.helmetLightAmbientModEchoDarkWorld;
}

CColor CTweakGui::GetHelmetLightAmbientModScanDarkWorld() const {
  return mData->misc.helmetLightAmbientModScanDarkWorld;
}

CColor CTweakGui::GetHelmetLightAmbientModDarkDarkWorld() const {
  return mData->misc.helmetLightAmbientModDarkDarkWorld;
}

float CTweakGui::GetExplosionLightFalloffMultConstant() const {
  return mData->misc.unknown_0x5b888032;
}

float CTweakGui::GetExplosionLightFalloffMultLinear() const {
  return mData->misc.unknown_0xb7322d26;
}

float CTweakGui::GetExplosionLightFalloffMultQuadratic() const {
  return mData->misc.unknown_0x79275f22;
}

float CTweakGui::GetLockOnIndicatorVerticalOffset() const { return mData->misc.unknown_0xf405af55; }

float CTweakGui::GetLockOnIndicatorScale() const { return mData->misc.unknown_0x3f85eb28; }

const CColor& CTweakGui::GetPlayerLockOnIndicatorColor(int playerSelection) const {
  switch (playerSelection) {
  case 0:
    return mData->misc.unknown_0x19c5f88b;
  case 1:
    return mData->misc.unknown_0xd84b274b;
  case 2:
    return mData->misc.unknown_0x41a9414a;
  case 3:
    return mData->misc.unknown_0x80279e8a;
  default:
    return CColor::Green();
  }
}

const CColor& CTweakGui::GetLockOnIndicatorColor() const { return mData->misc.unknown_0x98d8e1ba; }

rstl::string CTweakGui::GetCreditsTable() const { return mData->credits.unknown_0x81fc78c2; }

rstl::string CTweakGui::GetCreditsFont() const { return mData->credits.englishFont; }

CColor CTweakGui::GetCreditsFontColor() const { return mData->credits.fontColor; }

CColor CTweakGui::GetCreditsOutlineColor() const { return mData->credits.fontOutlineColor; }

float CTweakGui::GetCreditsTotalTime() const { return mData->credits.totalTime; }

float CTweakGui::GetCreditsTextFadeTime() const { return mData->credits.textFadeTime; }

float CTweakGui::GetCreditsMovieFadeTime() const { return mData->credits.movieFadeTime; }

rstl::string CTweakGui::GetCompletionScreenTable() const {
  return mData->completion.unknown_0x81fc78c2;
}

rstl::string CTweakGui::GetCompletionScreenTitleFont() const { return mData->completion.mainFont; }

rstl::string CTweakGui::GetCompletionScreenBodyFont() const {
  return mData->completion.secondaryFont;
}

CColor CTweakGui::GetCompletionScreenTitleFontColor() const {
  return mData->completion.mainFontColor;
}

CColor CTweakGui::GetCompletionScreenTitleOutlineColor() const {
  return mData->completion.mainFontOutlineColor;
}

CColor CTweakGui::GetCompletionScreenStatsFontColor() const {
  return mData->completion.statsFontColor;
}

CColor CTweakGui::GetCompletionScreenStatsOutlineColor() const {
  return mData->completion.statsFontOutlineColor;
}

CColor CTweakGui::GetCompletionScreenUnlockFontColor() const {
  return mData->completion.unlockFontColor;
}

CColor CTweakGui::GetCompletionScreenUnlockOutlineColor() const {
  return mData->completion.unlockFontOutlineColor;
}

float CTweakGui::GetCompletionScreenPulseTime() const {
  return mData->completion.unknown_0xb6fe7398;
}

float CTweakGui::GetCompletionScreenTextDelay() const { return mData->completion.textStartDelay; }

CColor CTweakGui::GetDarkWorldBaseColor() const {
  return mData->darkVisorLightWorld.darkWorldBaseColor;
}

const CColor& CTweakGui::GetDarkVisorStaticColor() const {
  return mData->darkVisorLightWorld.darkVisorStaticColor;
}

const CColor& CTweakGui::GetDarkVisorPaletteModulate() const {
  return mData->darkVisorLightWorld.darkVisorPaletteModulate;
}

float CTweakGui::GetDarkVisorBlurSpeed() const {
  return mData->darkVisorLightWorld.darkVisorBlurSpeed;
}

void CTweakGui::GetDarkVisorFrame(int* left, int* top, int* width, int* height) const {
  if (left != nullptr) {
    *left = mData->darkVisorLightWorld.darkVisorFrameLeft;
  }
  if (top != nullptr) {
    *top = mData->darkVisorLightWorld.darkVisorFrameTop;
  }
  if (width != nullptr) {
    *width = mData->darkVisorLightWorld.darkVisorFrameWidth;
  }
  if (height != nullptr) {
    *height = mData->darkVisorLightWorld.darkVisorFrameHeight;
  }
}

CColor CTweakGui::GetEchoBaseColor() const { return mData->echoVisor.echoBaseColor; }

CColor CTweakGui::GetEchoOutlineColor() const { return mData->echoVisor.echoOutlineColor; }

CColor CTweakGui::GetEchoDamageColor() const { return mData->echoVisor.echoDamageColor; }

CColor CTweakGui::GetEchoYellowDamageColor() const {
  return mData->echoVisor.echoYellowDamageColor;
}

ERglFogMode CTweakGui::GetEchoFogMode() const {
  return ConvertFogMode(mData->echoVisor.echoFogMode);
}

float CTweakGui::GetEchoFogNearZ() const { return mData->echoVisor.echoFogNearZ; }

float CTweakGui::GetEchoFogFarZ() const { return mData->echoVisor.echoFogFarZ; }

float CTweakGui::GetEchoBigRingScale() const { return mData->echoVisor.echoBigRingScale; }

float CTweakGui::GetEchoPulseRadiusScale() const { return mData->echoVisor.unknown_0x5708b903; }

float CTweakGui::GetEchoBigRingScanTime() const { return mData->echoVisor.echoBigRingScanTime; }

float CTweakGui::GetEchoBigRingFadeStart() const { return mData->echoVisor.echoBigRingFadeStart; }

float CTweakGui::GetEchoAuraSmallSize() const { return mData->echoVisor.echoAuraSmallSize; }

float CTweakGui::GetEchoAuraBigSize() const { return mData->echoVisor.echoAuraBigSize; }

CColor CTweakGui::GetEchoRingColor() const { return mData->echoVisor.echoRingColor; }

const CColor& CTweakGui::GetScanVisorInactiveColor() const {
  return mData->scanVisor.inactiveColor;
}

const CColor& CTweakGui::GetScanVisorInactiveExternalColor() const {
  return mData->scanVisor.inactiveExternalColor;
}

const CColor& CTweakGui::GetScanVisorNonCriticalColor() const {
  return mData->scanVisor.nonCriticalColor;
}

const CColor& CTweakGui::GetScanVisorCriticalColor() const {
  return mData->scanVisor.criticalColor;
}

const CColor& CTweakGui::GetScanVisorPreviouslyScannedColor() const {
  return mData->scanVisor.previouslyScannedColor;
}

const CColor& CTweakGui::GetScanVisorCriticalPreviouslyScannedColor() const {
  return mData->scanVisor.criticalPreviouslyScannedColor;
}

const CColor& CTweakGui::GetScanVisorBurnInColor() const { return mData->scanVisor.burnInColor; }

const CColor& CTweakGui::GetScanVisorHighlightColor() const {
  return mData->scanVisor.highlightColor;
}

const CColor& CTweakGui::GetScanVisorCriticalHighlightColor() const {
  return mData->scanVisor.criticalHighlightColor;
}

const CColor& CTweakGui::GetScanVisorPreviouslyScannedHighlightColor() const {
  return mData->scanVisor.previouslyScannedHighlightColor;
}

const CColor& CTweakGui::GetScanVisorCriticalPreviouslyScannedHighlightColor() const {
  return mData->scanVisor.criticalPreviouslyScannedHighlightColor;
}

const CColor& CTweakGui::GetScanVisorSweepBarColor() const {
  return mData->scanVisor.sweepBarColor;
}

const CColor& CTweakGui::GetScanVisorHackedColor() const { return mData->scanVisor.hackedColor; }

const CColor& CTweakGui::GetScanVisorHackedHighlightedColor() const {
  return mData->scanVisor.hackedHighlightedColor;
}

float CTweakGui::GetScanVisorBurnInTime() const { return mData->scanVisor.burnInTime; }

float CTweakGui::GetScanVisorFadeOutTime() const { return mData->scanVisor.fadeOutTime; }

const CColor& CTweakGui::GetLogBookMainWindowBorderColor() const {
  return mData->logBook.mainWindowBorderColor;
}

const CColor& CTweakGui::GetLogBookMainWindowTextColor() const {
  return mData->logBook.mainWindowTextColor;
}

const CColor& CTweakGui::GetLogBookMainWindowSelectedTextColor() const {
  return mData->logBook.mainWindowSelectedTextColor;
}

const CColor& CTweakGui::GetLogBookMainWindowUnviewedColor() const {
  return mData->logBook.mainWindowUnviewedColor;
}

const CColor& CTweakGui::GetLogBookMainWindowUnviewedSelectedColor() const {
  return mData->logBook.mainWindowUnviewedSelectedColor;
}

const CColor& CTweakGui::GetLogBookNodeColor() const { return mData->logBook.nodeColor; }

const CColor& CTweakGui::GetLogBookSelectedNodeColor() const {
  return mData->logBook.selectedNodeColor;
}

const CColor& CTweakGui::GetLogBookNodeBackgroundColor() const {
  return mData->logBook.unknown_0x56843943;
}

const CColor& CTweakGui::GetLogBookLegendBackgroundColor() const {
  return mData->logBook.legendBackgroundColor;
}

float CTweakGui::GetLogBookBranchLength() const { return mData->logBook.branchLength; }

float CTweakGui::GetLogBookTextScale() const { return mData->logBook.textScale; }

float CTweakGui::GetLogBookSelectedTextScale() const { return mData->logBook.selectedTextScale; }

float CTweakGui::GetLogBookTransitionTime() const { return mData->logBook.transitionTime; }

CMayaSpline& CTweakGui::GetLogBookNodeCollapseMotion() const {
  return const_cast< CMayaSpline& >(mData->logBook.nodeCollapseMotion);
}

CMayaSpline& CTweakGui::GetLogBookSelectedNodeCollapseMotion() const {
  return const_cast< CMayaSpline& >(mData->logBook.selectedNodeCollapseMotion);
}

CMayaSpline& CTweakGui::GetLogBookNodeExpandMotion() const {
  return const_cast< CMayaSpline& >(mData->logBook.nodeExpandMotion);
}

float CTweakGui::GetLogBookRotationSpeed() const { return mData->logBook.rotationSpeed; }

float CTweakGui::GetLogBookNodeScale() const { return mData->logBook.nodeScale; }

float CTweakGui::GetLogBookSelectedNodeScale() const { return mData->logBook.selectedNodeScale; }

const CColor& CTweakGui::GetLogBookScanTextWindowBackgroundColor() const {
  return mData->logBook.scanTextWindowBackgroundColor;
}

const CColor& CTweakGui::GetLogBookScanTextWindowBorderColor() const {
  return mData->logBook.scanTextWindowBorderColor;
}

const CColor& CTweakGui::GetLogBookScanTextWindowFontColor() const {
  return mData->logBook.scanTextWindowFontColor;
}

const CColor& CTweakGui::GetLogBookLegendWindowBackgroundColor() const {
  return mData->logBook.legendWindowBackgroundColor;
}

const CColor& CTweakGui::GetLogBookLegendWindowBorderColor() const {
  return mData->logBook.legendWindowBorderColor;
}

const CColor& CTweakGui::GetLogBookLegendWindowFontColor() const {
  return mData->logBook.legendWindowFontColor;
}

float CTweakGui::GetLogBookLegendHideTime() const { return mData->logBook.legendHideTime; }

float CTweakGui::GetLogBookScanModelScale() const { return mData->logBook.scanModelScale; }

const CMayaSpline& CTweakGui::GetLogBookScanObjectFadeInSpline() const {
  return mData->logBook.scanObjectFadeInSpline;
}

const CColor& CTweakGui::GetLogBookScanObjectFadeInFlashColor() const {
  return mData->logBook.scanObjectFadeInFlashColor;
}

float CTweakGui::GetLogBookScanObjectFadeInTime() const {
  return mData->logBook.scanObjectFadeInTime;
}

float CTweakGui::GetLogBookFogNear() const { return mData->logBook.fogNear; }

float CTweakGui::GetLogBookFogFar() const { return mData->logBook.fogFar; }

const CColor& CTweakGui::GetLogBookFogColor() const { return mData->logBook.fogColor; }

float CTweakGui::GetLogBookCameraDistance() const { return mData->logBook.unknown_0xeef15783; }

float CTweakGui::GetLogBookCameraZOffset() const { return mData->logBook.unknown_0x78055ab2; }

float CTweakGui::GetLogBookCameraXOffset() const { return mData->logBook.unknown_0x09e3197f; }

float CTweakGui::GetLogBookModelZOffset() const { return mData->logBook.unknown_0xfaffce1f; }

float CTweakGui::GetLogBookModelXOffset() const { return mData->logBook.unknown_0xe5280e7c; }

const CColor& CTweakGui::GetMapBackgroundColor() const {
  return mData->logBook.backgroundSweepColor;
}

float CTweakGui::GetMapBackgroundPulseWidth() const { return mData->logBook.backgroundSweepRadius; }

float CTweakGui::GetMapBackgroundCycleTime() const { return mData->logBook.backgroundSweepTime; }

const CColor& CTweakGui::GetLogBookHistoryUnselectedTitle() const {
  return mData->logBook.historyUnselectedTitle;
}

const CColor& CTweakGui::GetLogBookHistoryUnselectedFrame() const {
  return mData->logBook.historyUnselectedFrame;
}

const CColor& CTweakGui::GetLogBookHistorySelectedTitle() const {
  return mData->logBook.historySelectedTitle;
}

const CColor& CTweakGui::GetLogBookHistorySelectedFrame() const {
  return mData->logBook.historySelectedFrame;
}

const CColor& CTweakGui::GetLogBookHistoryCursorColor() const {
  return mData->logBook.historyCursorColor;
}

const CColor& CTweakGui::GetLogBookHistoryPercentBarUnselected() const {
  return mData->logBook.historyPercentBarUnselected;
}

const CColor& CTweakGui::GetLogBookHistoryPercentBarSelected() const {
  return mData->logBook.historyPercentBarSelected;
}

const CColor& CTweakGui::GetLogBookHistoryPercentBarBackgroundSelected() const {
  return mData->logBook.historyPercentBarBackgroundSelected;
}

const CColor& CTweakGui::GetLogBookHistoryPercentBarBackgroundUnselected() const {
  return mData->logBook.historyPercentBarBackgroundUnselected;
}

const CColor& CTweakGui::GetLogBookFrameColor() const { return mData->logBook.frameColor; }

const CColor& CTweakGui::GetLogBookScanlineColor() const { return mData->logBook.scanlineColor; }

const CColor& CTweakGui::GetLogBookSliderBackgroundColor() const {
  return mData->logBook.sliderBackgroundColor;
}

const CColor& CTweakGui::GetLogBookSliderSelectionColor() const {
  return mData->logBook.sliderSelectionColor;
}

float CTweakGui::GetLogBookSliderScale() const { return mData->logBook.sliderScale; }

float CTweakGui::GetLogBookSliderSpeed() const { return mData->logBook.sliderSpeed; }

float CTweakGui::GetLogBookSliderTextHeightScale() const {
  return mData->logBook.unknown_0x58795865;
}

float CTweakGui::GetLogBookSliderTextWidthScale() const {
  return mData->logBook.unknown_0x35e0bd31;
}

const CColor& CTweakGui::GetLogBookMenuOptionColor() const {
  return mData->logBook.menuOptionColor;
}

const CColor& CTweakGui::GetLogBookMenuOptionEnabledArrowColor() const {
  return mData->logBook.menuOptionEnabledArrowColor;
}

const CColor& CTweakGui::GetLogBookMenuOptionDisabledArrowColor() const {
  return mData->logBook.menuOptionDisabledArrowColor;
}

float CTweakGui::GetLogBookMenuOptionScale() const { return mData->logBook.menuOptionScale; }

float CTweakGui::GetLogBookMenuOptionArrowScale() const {
  return mData->logBook.menuOptionArrowScale;
}

float CTweakGui::GetLogBookModelRotationClampLowerLimit() const {
  return mData->logBook.modelRotationClampLowerLimit;
}

float CTweakGui::GetLogBookModelRotationClampUpperLimit() const {
  return mData->logBook.modelRotationClampUpperLimit;
}

const CVector3f& CTweakGui::GetLogBookModelLight1Position() const {
  return mData->logBook.modelLight1Position;
}

const CColor& CTweakGui::GetLogBookModelLight1Color() const {
  return mData->logBook.modelLight1Color;
}

const CVector3f& CTweakGui::GetLogBookModelLight2Position() const {
  return mData->logBook.modelLight2Position;
}

const CColor& CTweakGui::GetLogBookModelLight2Color() const {
  return mData->logBook.modelLight2Color;
}

const CColor& CTweakGui::GetLogBookModelAmbientLightColor() const {
  return mData->logBook.modelAmbientLightColor;
}

int CTweakGui::GetDeathMovieVolume() const { return mData->movieVolumes.unknown_0xae149646; }

int CTweakGui::GetEndingPart2Volume() const { return mData->movieVolumes.unknown_0xc1a2e858; }

int CTweakGui::GetEndingPart2BVolume() const { return mData->movieVolumes.unknown_0x138c3bb8; }

int CTweakGui::GetEndingPart3Volume() const { return mData->movieVolumes.unknown_0xe5587648; }

int CTweakGui::GetResultsMovieVolume() const { return mData->movieVolumes.unknown_0x9ed00248; }

int CTweakGui::GetSpecialEndingVolume() const { return mData->movieVolumes.unknown_0x6f135424; }

int CTweakGui::GetCreditsVolume() const { return mData->movieVolumes.unknown_0xdb2260b7; }

int CTweakGui::GetCompletionScreenVolume() const { return mData->movieVolumes.unknown_0xf38093f5; }

float CTweakGui::GetScanSpeed(int speed) const {
  if (speed != 0) {
    return mData->scannableObjectDownloadTimes.slow;
  }
  return mData->scannableObjectDownloadTimes.fast;
}

uint CTweakGui::GetHelmetVisMode() const { return mData->misc.unknown_0x7d3c03eb; }
