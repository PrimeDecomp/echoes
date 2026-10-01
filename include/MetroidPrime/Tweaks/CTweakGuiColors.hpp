#ifndef _CTWEAKGUICOLORS
#define _CTWEAKGUICOLORS

#include "Kyoto/Graphics/CColor.hpp"
#include "MetroidPrime/HUD/CSamusHud.hpp"
#include "rstl/single_ptr.hpp"

struct SLdrTweakGuiColors;
struct SLdrTweakGui_VisorColorSchemeTypedef;

class CTweakGuiColors {
public:
  // Accessor spellings are reconstructed; named SLdr field roles are verified
  // unless their property comment explicitly says non-matching.
  // Guessed name; a borrowed view of one visor's color scheme.
  class SVisorColorScheme {
  public:
    explicit SVisorColorScheme(const SLdrTweakGui_VisorColorSchemeTypedef& data) : mData(&data) {}
    const CColor& GetHUDHue() const;
    const CColor& GetGlassTint() const;

  private:
    const SLdrTweakGui_VisorColorSchemeTypedef* mData;
  };

  explicit CTweakGuiColors(const SLdrTweakGuiColors& data) : mData(&data) {}

  const CColor& GetHUDDecorativeColor() const;
  const CColor& GetThreatGroupActiveColor() const;
  const CColor& GetThreatGroupInactiveColor() const;
  const CColor& GetFreeLookBarColor() const;
  const CColor& GetMissileGroupActiveColor() const;
  const CColor& GetMissileGroupInactiveColor() const;
  // Guessed name.
  const CColor& GetMissileGroupChangeFlash() const;
  const CColor& GetEnergyBarFilledColor() const;
  const CColor& GetEnergyBarShadowColor() const;
  const CColor& GetEnergyBarEmptyColor() const;
  const CColor& GetEnergyTanksFilledColor() const;
  const CColor& GetEnergyTanksEmptyColor() const;
  const CColor& GetRadarWidgetColor() const;
  const CColor& GetActiveTextForegroundColor() const;
  const CColor& GetInactiveTextForegroundColor() const;
  const CColor& GetTextShadowOutlineColor() const;
  const CColor& GetPauseScreenBGModulateColor() const;
  // Guessed name.
  const CColor& GetRadarPlayerPaintColor() const;
  // Guessed name.
  const CColor& GetRadarEnemyPaintColor() const;
  // Guessed name.
  const CColor& GetRadarEnemyTeamPaintColor() const;
  // Guessed name.
  const CColor& GetRadarFriendTeamPaintColor() const;
  // Guessed name.
  const CColor& GetRadarEchoPulseColor() const;
  const CColor& GetHUDMemoTextForegroundColor() const;
  const CColor& GetHUDMemoTextOutlineColor() const;
  const CColor& GetSelectedVisorBeamColor() const;
  const CColor& GetUnselectedVisorBeamColor() const;
  const CColor& GetEnergyBarLowEmptyColor() const;
  const CColor& GetHUDDamageModulateColor() const;
  const CColor& GetDamageIndicatorColor() const;
  const CColor& GetVisorMenuTitleForegroundColor() const;
  const CColor& GetVisorMenuTitleOutlineColor() const;
  const CColor& GetBeamMenuTitleForegroundColor() const;
  const CColor& GetBeamMenuTitleOutlineColor() const;
  const CColor& GetVisorBeamMenuIconSelectedColor() const;
  const CColor& GetVisorBeamMenuIconUnselectedColor() const;
  const CColor& GetVisorMenuIconColor0() const;
  const CColor& GetVisorMenuIconColor1() const;
  const CColor& GetVisorMenuIconColor2() const;
  const CColor& GetVisorMenuIconColor3() const;
  const CColor& GetBeamMenuIconColor0() const;
  const CColor& GetBeamMenuIconColor1() const;
  const CColor& GetBeamMenuIconColor2() const;
  const CColor& GetBeamMenuIconColor3() const;
  const CColor& GetEnergyWarningColor() const;
  const CColor& GetThreatWarningColor() const;
  const CColor& GetMissileWarningColor() const;
  const CColor& GetMissileDepletionColor() const;
  const CColor& GetThreatBarFilledColor() const;
  // Guessed name.
  const CColor& GetVisorBeamMenuLozColor() const;
  const CColor& GetEnergyWarningOutlineColor() const;
  const CColor& GetFlashPassColor() const;
  const CColor& GetScanWindowFrameBaseColor() const;
  const CColor& GetScanWindowFrameActiveColor() const;
  // Guessed name.
  const CColor& GetScanWindowFrameFlashAddColor() const;
  // Guessed name.
  const CColor& GetScanVisorHUDLightMultiply() const;
  // Guessed name.
  const CColor& GetScanWindowTintColor() const;
  // Guessed name.
  const CColor& GetScanVisorScreenDimColor() const;
  const CColor& GetScanHudHierarchyFrameColor() const;
  const CColor& GetScanHudHierarchyInactiveFrameColor() const;
  const CColor& GetScanHudHierarchyTextFrameColor() const;
  const CColor& GetScanHudHierarchyFinalTextFrameColor() const;
  const CColor& GetScanHudHierarchyFlashIconColor() const;
  const CColor& GetScanHudHierarchyCompleteFlashIconColor() const;
  const CColor& GetScanHudHierarchyFlashFlashIconColor() const;
  const CColor& GetScanHudHierarchyTextColor() const;
  const CColor& GetScanHudHierarchyFinalTextColor() const;
  const CColor& GetScanHudHierarchyPercentTextColor() const;
  const CColor& GetScanHudHierarchyBarMeterColor() const;
  // Guessed name.
  const CColor& GetDarkVisorHelmetLightModulateColor() const;
  const CColor& GetMetroidSuckPulseColor() const;
  // Guessed name.
  const CColor& GetDamageAmbientPulseColor() const;
  const CColor& GetEnergyBarDamageColor() const;
  const CColor& GetMorphBallPowerBombDigitsForegroundColor() const;
  const CColor& GetMorphBallPowerBombDigitsOutlineColor() const;
  const CColor& GetMorphBallBombCounterFilledColor() const;
  const CColor& GetMorphBallBombCounterEmptyColor() const;
  const CColor& GetMorphBallPowerBombIconColor() const;
  const CColor& GetMorphBallEnergyDecoColor() const;
  // Guessed name.
  const CColor& GetMorphBallEmptyPowerBombDigitsForegroundColor() const;
  const CColor& GetMorphBallEmptyPowerBombDigitsOutlineColor() const;
  const CColor& GetMorphBallEmptyPowerBombIconColor() const;
  const CColor& GetThreatGroupDamageColor() const;
  const CColor& GetCountdownForegroundColor() const;
  const CColor& GetCountdownOutlineColor() const;
  const CColor& GetCombatMissileDigitsForegroundColor() const;
  const CColor& GetCombatMissileDigitsOutlineColor() const;
  const CColor& GetLightAmmoTankFullSelectedColor() const;
  const CColor& GetLightAmmoTankFullUnselectedColor() const;
  const CColor& GetLightAmmoTankEmptySelectedColor() const;
  const CColor& GetLightAmmoTankEmptyUnselectedColor() const;
  const CColor& GetLightAmmoMeterSelectedFillColor() const;
  const CColor& GetLightAmmoMeterSelectedShadowColor() const;
  const CColor& GetLightAmmoMeterUnselectedFillColor() const;
  const CColor& GetLightAmmoMeterUnselectedShadowColor() const;
  const CColor& GetLightAmmoIconSelectedColor() const;
  const CColor& GetLightAmmoIconUnselectedColor() const;
  const CColor& GetLightAmmoDigitsOutlineColor() const;
  const CColor& GetLightAmmoDigitsSelectedColor() const;
  const CColor& GetLightAmmoDigitsUnselectedColor() const;
  const CColor& GetLightAmmoChangeFlash() const;
  const CColor& GetLightAmmoTankWarningColor() const;
  const CColor& GetLightAmmoMeterWarningColor() const;
  const CColor& GetLightAmmoDigitWarningColor() const;
  const CColor& GetLightAmmoDepletionColor() const;
  const CColor& GetLightAmmoEmptyTankWarningColor() const;
  const CColor& GetDarkAmmoTankFullSelectedColor() const;
  const CColor& GetDarkAmmoTankFullUnselectedColor() const;
  const CColor& GetDarkAmmoTankEmptySelectedColor() const;
  const CColor& GetDarkAmmoTankEmptyUnselectedColor() const;
  const CColor& GetDarkAmmoMeterSelectedFillColor() const;
  const CColor& GetDarkAmmoMeterSelectedShadowColor() const;
  const CColor& GetDarkAmmoMeterUnselectedFillColor() const;
  const CColor& GetDarkAmmoMeterUnselectedShadowColor() const;
  const CColor& GetDarkAmmoIconSelectedColor() const;
  const CColor& GetDarkAmmoIconUnselectedColor() const;
  const CColor& GetDarkAmmoDigitsOutlineColor() const;
  const CColor& GetDarkAmmoDigitsSelectedColor() const;
  const CColor& GetDarkAmmoDigitsUnselectedColor() const;
  const CColor& GetDarkAmmoChangeFlash() const;
  const CColor& GetDarkAmmoTankWarningColor() const;
  const CColor& GetDarkAmmoMeterWarningColor() const;
  const CColor& GetDarkAmmoDigitWarningColor() const;
  const CColor& GetDarkAmmoDepletionColor() const;
  const CColor& GetDarkAmmoEmptyTankWarningColor() const;
  const CColor& GetMultiplayerScoreTextColor() const;
  const CColor& GetMultiplayerWinningScoreTextColor() const;
  const CColor& GetMultiplayerTimerTextColor() const;
  const CColor& GetMultiplayerTimerTextBlinkColor() const;
  const CColor& GetMultiplayerScoreboardDecoColor() const;
  const CColor& GetMultiplayerScoreboardBackgroundColor() const;
  // Guessed name.
  const CColor& GetMultiplayerScoreGainColor() const;
  // Guessed name.
  const CColor& GetMultiplayerScoreLossColor() const;
  const CColor& GetTurretHUDFrameColor() const;
  const CColor& GetTurretHUDFontColor() const;
  const CColor& GetTurretHUDFontOutlineColor() const;
  const CColor& GetTurretHUDEnergyBarFillColor() const;
  const CColor& GetTurretHUDEnergyBarShadowColor() const;
  const CColor& GetTurretHUDEnergyBarEmptyColor() const;
  SVisorColorScheme GetVisorColorScheme(CSamusHud::EHudState hudState) const;

private:
  const SLdrTweakGuiColors* mData;
};
CHECK_SIZEOF(CTweakGuiColors, 0x4)
extern int SVisorColorSchemeCheck[check_sizeof< CTweakGuiColors::SVisorColorScheme, 0x4 >::value];

extern rstl::single_ptr< CTweakGuiColors > gpTweakGuiColors;

#endif // _CTWEAKGUICOLORS
