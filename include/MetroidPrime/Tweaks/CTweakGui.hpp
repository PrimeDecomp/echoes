#ifndef _CTWEAKGUI
#define _CTWEAKGUI

#include "Kyoto/Graphics/CGraphics.hpp"

#include "rstl/single_ptr.hpp"
#include "rstl/string.hpp"

struct SLdrTweakGui;
class CColor;
class CVector3f;
class CMayaSpline;

class CTweakGui {
public:
  // Accessor spellings are reconstructed. Named SLdr fields without a non-matching
  // annotation have verified property identities, including the renamed accessors.
  explicit CTweakGui(const SLdrTweakGui& data) : mData(&data) {}

  // Guessed names; reconstructed from native consumers and Prime's interface.
  float GetScanSpeed(int speed) const;
  void GetDarkVisorFrame(int* left, int* top, int* width, int* height) const;
  static float FaceReflectionDistanceDebugValueToActualValue(float value);
  static float FaceReflectionHeightDebugValueToActualValue(float value);
  static float FaceReflectionAspectDebugValueToActualValue(float value);
  static float FaceReflectionOrthoWidthDebugValueToActualValue(float value);
  static float FaceReflectionOrthoHeightDebugValueToActualValue(float value);

  float GetMapAlphaInterpolant() const;

  // Guessed names; native lock-on, damage-ring, face-light and logbook consumers.
  const CColor& GetLogBookScanlineColor() const;
  const CColor& GetLockOnIndicatorColor() const;
  float GetLockOnIndicatorScale() const;
  float GetLockOnIndicatorVerticalOffset() const;
  float GetFaceReflectionLightFalloffMultQuadratic() const;
  float GetFaceReflectionLightFalloffMultLinear() const;
  float GetFaceReflectionLightFalloffMultConstant() const;
  float GetHUDDamageIndicatorRadius() const;
  const CColor& GetPlayerLockOnIndicatorColor(int playerSelection) const;
  float GetEchoPulseRadiusScale() const;
  float GetBallViewportYReduction() const;

  // Guessed names; native scan-decoration initialization corroborates Prime.
  float GetScanSidesPositionStart() const;

  // Guessed names; GUI-manager settings retain their unresolved integer domains.
  uint GetHelmetVisMode() const;
  uint GetEnablePlayerVisor() const;
  uint GetEnableTargetingManager() const;
  uint GetEnableAutoMapper() const;
  uint GetHudVisMode() const;

  // Guessed names; native radar, pause-screen and menu consumers establish their roles.
  float GetPauseBlurFactor() const;
  float GetRadarWorldHalfHeight() const;
  float GetRadarZCloseRadius() const;
  float GetBeamVisorMenuAnimTime() const;
  float GetVisorBeamMenuItemActiveScale() const;
  float GetVisorBeamMenuItemInactiveScale() const;
  float GetVisorBeamMenuItemTranslate() const;
  float GetRadarScopeCoordRadius() const;
  float GetRadarPlayerPaintRadius() const;

  // Guessed names; raw integer controls are converted by the FaceReflection helpers.
  int GetFaceReflectionDistanceDebugValue() const;
  int GetFaceReflectionHeightDebugValue() const;
  int GetFaceReflectionAspectDebugValue() const;
  int GetFaceReflectionOrthoWidthDebugValue() const;
  int GetFaceReflectionOrthoHeightDebugValue() const;

  // Guessed names, recovered from pause-screen model, camera and slider consumers.
  float GetLogBookModelRotationClampLowerLimit() const;
  float GetLogBookModelRotationClampUpperLimit() const;
  float GetLogBookSliderTextWidthScale() const;
  float GetLogBookSliderTextHeightScale() const;
  float GetLogBookModelXOffset() const;
  float GetLogBookModelZOffset() const;
  float GetLogBookCameraXOffset() const;
  float GetLogBookCameraZOffset() const;
  float GetLogBookCameraDistance() const;

  // Guessed names; native energy-bar and free-look sound consumers corroborate Prime.
  float GetEnergyBarFilledDrainSpeed() const;
  float GetEnergyBarShadowDrainSpeed() const;
  float GetEnergyBarShadowDrainDelay() const;
  bool GetEnergyBarAlwaysResetDelay() const;
  float GetFreeLookSfxPitchScale() const;
  bool GetNoAbsoluteFreeLookSfxPitch() const;

  // Guessed names; reconstructed from native HUD light, damage and message consumers.
  float GetExplosionLightFalloffMultQuadratic() const;
  float GetExplosionLightFalloffMultLinear() const;
  float GetExplosionLightFalloffMultConstant() const;
  float GetWorldTransManagerCharsPerSfx() const;
  float GetHudLagOffsetScale() const;
  float GetHudDecoShakeTranslateGain() const;
  float GetHudDamageColorGain() const;
  float GetHudDamagePulseDuration() const;
  bool GetEnergyDrainFilterAdditive() const;
  bool GetEnergyDrainSinusoidalPulse() const;
  float GetEnergyDrainModPeriod() const;
  float GetHudDamagePeakFactor() const;

  // Guessed names; native scan-window and scan-model animation consumers.
  float GetScanWindowScanningAspect() const;
  float GetScanWindowMagnification() const;
  float GetScanWindowActiveHeight() const;
  float GetScanWindowActiveWidth() const;
  float GetScanWindowIdleHeight() const;
  float GetScanWindowIdleWidth() const;
  CMayaSpline& GetScanObjectScaleTransitionSpline() const;
  CMayaSpline& GetScanObjectRotationTransitionSpline() const;
  CMayaSpline& GetScanObjectTranslateTransitionSpline() const;
  float GetScanSidesEndTime() const;
  float GetScanSidesStartTime() const;
  float GetScanSidesDuration() const;

  // Guessed names
  const CColor& GetMapBackgroundColor() const;
  float GetMapBackgroundPulseWidth() const;
  float GetMapBackgroundCycleTime() const;

  // Guessed names, recovered from the credits settings and their consumers.
  rstl::string GetCreditsTable() const;
  rstl::string GetCreditsFont() const;
  CColor GetCreditsFontColor() const;
  CColor GetCreditsOutlineColor() const;
  float GetCreditsTotalTime() const;
  float GetCreditsTextFadeTime() const;
  float GetCreditsMovieFadeTime() const;
  int GetCreditsVolume() const;

  // Guessed names, recovered from the completion screen and movie consumers.
  rstl::string GetCompletionScreenTable() const;
  rstl::string GetCompletionScreenTitleFont() const;
  rstl::string GetCompletionScreenBodyFont() const;
  CColor GetCompletionScreenTitleFontColor() const;
  CColor GetCompletionScreenTitleOutlineColor() const;
  CColor GetCompletionScreenStatsFontColor() const;
  CColor GetCompletionScreenStatsOutlineColor() const;
  CColor GetCompletionScreenUnlockFontColor() const;
  CColor GetCompletionScreenUnlockOutlineColor() const;
  float GetCompletionScreenTextDelay() const;
  float GetCompletionScreenPulseTime() const;
  int GetCompletionScreenVolume() const;
  int GetEndingPart2Volume() const;
  int GetEndingPart2BVolume() const;
  int GetEndingPart3Volume() const;
  int GetResultsMovieVolume() const;
  int GetSpecialEndingVolume() const;
  int GetDeathMovieVolume() const;

  // Guessed accessor names derived from verified SLdr fields and native access widths.
  float GetRadarWorldRadius() const;
  float GetHUDFlashMagnitudeConstant() const;
  float GetHUDFlashMagnitudeLinear() const;
  float GetHUDFlashTimeConstant() const;
  float GetHUDFlashTimeScaleLinear() const;
  float GetHUDDamageJostleMagnitudeConstant() const;
  float GetHUDDamageJostleMagnitudeLinear() const;
  float GetHUDDamageJostleMaxOffset() const;
  float GetHUDDamageJostleReturnAcceleration() const;
  float GetHUDDamageDistortionMagnitudeConstant() const;
  float GetHUDDamageDistortionMagnitudeLinear() const;
  float GetHUDDamageDistortionTimeConstant() const;
  float GetHUDDamageDistortionTimeLinear() const;
  float GetHUDDamageDistortionMaxMagnitude() const;
  float GetThreatWorldRadius() const;
  float GetMissileWarningThreshold() const;
  float GetFlashPassMagnitudeConstant() const;
  float GetFlashPassMagnitudeLinear() const;
  float GetFlashPassTimerConstant() const;
  float GetFlashPassTimerLinear() const;
  float GetScanObjectModelScale() const;
  CColor GetHelmetBaseAmbientColorCombatLightWorld() const;
  CColor GetHelmetBaseAmbientColorEchoLightWorld() const;
  CColor GetHelmetBaseAmbientColorScanLightWorld() const;
  CColor GetHelmetBaseAmbientColorDarkLightWorld() const;
  CColor GetHelmetBaseAmbientColorCombatDarkWorld() const;
  CColor GetHelmetBaseAmbientColorEchoDarkWorld() const;
  CColor GetHelmetBaseAmbientColorScanDarkWorld() const;
  CColor GetHelmetBaseAmbientColorDarkDarkWorld() const;
  CColor GetHelmetLightAmbientModCombatLightWorld() const;
  CColor GetHelmetLightAmbientModEchoLightWorld() const;
  CColor GetHelmetLightAmbientModScanLightWorld() const;
  CColor GetHelmetLightAmbientModDarkLightWorld() const;
  CColor GetHelmetLightAmbientModCombatDarkWorld() const;
  CColor GetHelmetLightAmbientModEchoDarkWorld() const;
  CColor GetHelmetLightAmbientModScanDarkWorld() const;
  CColor GetHelmetLightAmbientModDarkDarkWorld() const;
  CColor GetDarkWorldBaseColor() const;
  const CColor& GetDarkVisorStaticColor() const;
  const CColor& GetDarkVisorPaletteModulate() const;
  float GetDarkVisorBlurSpeed() const;
  CColor GetEchoBaseColor() const;
  CColor GetEchoOutlineColor() const;
  CColor GetEchoDamageColor() const;
  CColor GetEchoYellowDamageColor() const;
  ERglFogMode GetEchoFogMode() const;
  float GetEchoFogNearZ() const;
  float GetEchoFogFarZ() const;
  float GetEchoBigRingScale() const;
  float GetEchoBigRingScanTime() const;
  float GetEchoBigRingFadeStart() const;
  float GetEchoAuraSmallSize() const;
  float GetEchoAuraBigSize() const;
  CColor GetEchoRingColor() const;
  const CColor& GetScanVisorInactiveColor() const;
  const CColor& GetScanVisorInactiveExternalColor() const;
  const CColor& GetScanVisorNonCriticalColor() const;
  const CColor& GetScanVisorCriticalColor() const;
  const CColor& GetScanVisorPreviouslyScannedColor() const;
  const CColor& GetScanVisorCriticalPreviouslyScannedColor() const;
  const CColor& GetScanVisorBurnInColor() const;
  const CColor& GetScanVisorHighlightColor() const;
  const CColor& GetScanVisorCriticalHighlightColor() const;
  const CColor& GetScanVisorPreviouslyScannedHighlightColor() const;
  const CColor& GetScanVisorCriticalPreviouslyScannedHighlightColor() const;
  const CColor& GetScanVisorSweepBarColor() const;
  const CColor& GetScanVisorHackedColor() const;
  const CColor& GetScanVisorHackedHighlightedColor() const;
  float GetScanVisorBurnInTime() const;
  float GetScanVisorFadeOutTime() const;
  const CColor& GetLogBookMainWindowBorderColor() const;
  const CColor& GetLogBookMainWindowTextColor() const;
  const CColor& GetLogBookMainWindowSelectedTextColor() const;
  const CColor& GetLogBookNodeColor() const;
  // Guessed name; the style-zero logbook background.
  const CColor& GetLogBookNodeBackgroundColor() const;
  // Guessed accessor spellings; main-window roles are verified SLdr field names.
  const CColor& GetLogBookMainWindowUnviewedSelectedColor() const;
  const CColor& GetLogBookMainWindowUnviewedColor() const;
  const CColor& GetLogBookSelectedNodeColor() const;
  const CColor& GetLogBookLegendBackgroundColor() const;
  float GetLogBookBranchLength() const;
  float GetLogBookTextScale() const;
  float GetLogBookSelectedTextScale() const;
  float GetLogBookTransitionTime() const;
  CMayaSpline& GetLogBookNodeCollapseMotion() const;
  const CMayaSpline& GetLogBookSelectedNodeCollapseMotion() const;
  CMayaSpline& GetLogBookNodeExpandMotion() const;
  float GetLogBookRotationSpeed() const;
  float GetLogBookNodeScale() const;
  float GetLogBookSelectedNodeScale() const;
  const CColor& GetLogBookScanTextWindowBackgroundColor() const;
  const CColor& GetLogBookScanTextWindowBorderColor() const;
  const CColor& GetLogBookScanTextWindowFontColor() const;
  const CColor& GetLogBookLegendWindowBackgroundColor() const;
  const CColor& GetLogBookLegendWindowBorderColor() const;
  const CColor& GetLogBookLegendWindowFontColor() const;
  float GetLogBookLegendHideTime() const;
  float GetLogBookScanModelScale() const;
  const CMayaSpline& GetLogBookScanObjectFadeInSpline() const;
  const CColor& GetLogBookScanObjectFadeInFlashColor() const;
  float GetLogBookScanObjectFadeInTime() const;
  float GetLogBookFogNear() const;
  float GetLogBookFogFar() const;
  const CColor& GetLogBookFogColor() const;
  const CColor& GetLogBookHistoryUnselectedTitle() const;
  const CColor& GetLogBookHistoryUnselectedFrame() const;
  const CColor& GetLogBookHistorySelectedTitle() const;
  const CColor& GetLogBookHistorySelectedFrame() const;
  const CColor& GetLogBookHistoryCursorColor() const;
  const CColor& GetLogBookHistoryPercentBarUnselected() const;
  const CColor& GetLogBookHistoryPercentBarSelected() const;
  const CColor& GetLogBookHistoryPercentBarBackgroundSelected() const;
  const CColor& GetLogBookHistoryPercentBarBackgroundUnselected() const;
  const CColor& GetLogBookFrameColor() const;
  const CColor& GetLogBookSliderBackgroundColor() const;
  const CColor& GetLogBookSliderSelectionColor() const;
  float GetLogBookSliderScale() const;
  float GetLogBookSliderSpeed() const;
  const CColor& GetLogBookMenuOptionColor() const;
  const CColor& GetLogBookMenuOptionEnabledArrowColor() const;
  const CColor& GetLogBookMenuOptionDisabledArrowColor() const;
  float GetLogBookMenuOptionScale() const;
  float GetLogBookMenuOptionArrowScale() const;
  const CVector3f& GetLogBookModelLight1Position() const;
  const CColor& GetLogBookModelLight1Color() const;
  const CVector3f& GetLogBookModelLight2Position() const;
  const CColor& GetLogBookModelLight2Color() const;
  const CColor& GetLogBookModelAmbientLightColor() const;

private:
  const SLdrTweakGui* mData;
};
CHECK_SIZEOF(CTweakGui, 0x4)

extern rstl::single_ptr< CTweakGui > gpTweakGui;

#endif // _CTWEAKGUI
