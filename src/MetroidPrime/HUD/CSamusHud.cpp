#include "MetroidPrime/HUD/CSamusHud.hpp"
#include "GuiSys/CAuiBitmapMeter.hpp"
#include "GuiSys/CAuiEnergyBarT01.hpp"
#include "GuiSys/CGuiCamera.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiFrameLoader.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "GuiSys/CGuiWidgetDrawParms.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/CResFactory.hpp"
#include "MetroidPrime/HUD/CHudBossEnergyInterface.hpp"
#include "MetroidPrime/HUD/CHudDecoInterfaceScan.hpp"
#include "MetroidPrime/HUD/CHudRadarInterface.hpp"
#include "MetroidPrime/HUD/CHudVisorBeamMenu.hpp"

#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCinematicCamera.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerGun.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakGuiColors.hpp"
#include "rstl/StringExtras.hpp"

#include <stdio.h>

// Structure-first scaffold. Unrecovered behavior is marked at each entry point.

// Guessed name; no surviving direct consumer of this native table is identified.
static const int sHudViewportWidths[3] = {320, 320, 160};

const char* const CSamusHud::skHudElementNames[17] = {
    "              Radar", "                Lag", "             Lights", "          Targeting",
    "             Damage", "             Threat", "               Ammo", "          FrameGlue",
    "          BaseFrame", "        EnergyGroup", "        ThreatGroup", "       MissileGroup",
    "      FreeLookGroup", "        HelmetGroup", "          DecoGroup", "           CamDebug",
    "              Total",
};

static const char* const sDecorativeWidgets[] = {
    "model_energy",
    "model_frame",
    "model_visorbracket",
    "model_bossframe1",
};
static const char* const sMemoDecorationWidgets[] = {"model_messageframe", "model_zbuttonring"};
static const char* const sDarkVisorWidgets[] = {
    "model_darkspinner",
    "model_darkholder",
    "model_darkring",
    "basewidget_bossdark",
};
static const char* const sNonScanWidgets[] = {
    "textpane_missiledigits",
    "model_missileicon",
    "basewidget_ammo",
    "textpane_lightammodigits",
    "model_frame",
    "basewidget_automapper",
    "basewidget_radar",
    "model_threatguage",
    "barmeter_threatguagequad",
    "model_threaticon",
    "model_threatbar",
    "model_radar",
    "model_glass",
    "textpane_lightammodigits",
    "textpane_darkammodigits",
};
static const char* const sCombatHudNames[] = {
    "FRME_SamusHud1Combat",
    "FRME_SamusHud2Combat",
    "FRME_SamusHud4Combat",
};
static const char* const sBallHudNames[] = {
    "FRME_SamusHud1Ball",
    "FRME_SamusHud2Ball",
    "FRME_SamusHud4Ball",
};
static const char* const sHelmetLightWidgets[5][2] = {
    {"model_lighthole", "model_lightholebottom"},
    {"model_lighthole", "model_lightholebottom"},
    {"model_lighthole", "model_lightholebottom"},
    {"model_lighthole_dark", "model_lightholebottom_dark"},
    {"", ""},
};

CSamusHud* gpSamusHud[4] = {nullptr, nullptr, nullptr, nullptr};

static void StopSound(CSfxHandle& sound) {
  if (sound) {
    CSfxManager::SfxStop(sound);
    sound.Clear();
  }
}

const char* CSamusHud::GetHudFrameName(int viewportLayout) {
  return sCombatHudNames[viewportLayout];
}

rstl::pair< CVector3f, CVector3f > CSamusHud::CombatEnergyCoordFunc(float t) {
  const float angle = 0.5294118f * t - 0.20262942f;
  const float x = 17.f * sin(angle);
  const float y = 0.2f + (17.f * cos(angle) - 17.f);
  return rstl::pair< CVector3f, CVector3f >(CVector3f(x, y, 0.4f), CVector3f(x, y, 0.f));
}

rstl::pair< CVector3f, CVector3f > CSamusHud::BallEnergyCoordFunc(float t) {
  return rstl::pair< CVector3f, CVector3f >(CVector3f(8.5f * t, 0.f, 0.f),
                                            CVector3f(8.5f * t, 0.f, 0.4f));
}

CTransform4f CSamusHud::BuildFinalCameraTransform(const CQuaternion& rotation,
                                                  const CVector3f& pivot,
                                                  const CVector3f& cameraPosition) {
  const CQuaternion inverse = rotation.BuildInverted();
  return CTransform4f(inverse.BuildTransform(), inverse.Transform(cameraPosition - pivot) + pivot);
}

void CSamusHud::InitializeFrameGluePermanent() {
  mMessagePane = static_cast< CGuiTextPane* >(mLoadedMemoFrame->FindWidget("textpane_message"));
  mMessageAButton = mLoadedMemoFrame->FindWidget("model_abutton");
  mMessageRoot = mLoadedMemoFrame->FindWidget("basewidget_message");
  if (mMessageRoot != nullptr) {
    for (CGuiWidget* child = static_cast< CGuiWidget* >(mMessageRoot->ChildObject());
         child != nullptr; child = static_cast< CGuiWidget* >(child->NextSibling())) {
      child->SetDepthTest(false);
    }
    mMessageRoot->SetVisibility(false, kTM_Children);
  }
  mMessageAButton = mLoadedMemoFrame->FindWidget("model_abutton");
  if (mMessagePane != nullptr) {
    mMessagePane->TextSupport().SetFontColor(gpTweakGuiColors->GetHUDMemoTextForegroundColor());
    mMessagePane->TextSupport().SetOutlineColor(gpTweakGuiColors->GetHUDMemoTextOutlineColor());
    mMessagePane->TextSupport().SetControlTXTRMap(&gpGameState->GameOptions().GetControlTXTRMap());
  }
}

void CSamusHud::InitializeFrameGlueMutable(const CStateManager& mgr) {
  if (mLoadedHudFrame->FindWidget("textpane_visormenu") != nullptr) {
    mVisorMenu =
        rs_new CHudVisorBeamMenu(*mLoadedHudFrame, mHudStringTable, CHudVisorBeamMenu::kVBM_Visor,
                                 BuildPlayerHasVisors(mgr), 0, mgr.IsMultiplayer());
    mMenuVisor = CPlayerState::kPV_Combat;
  }
  if (mLoadedHudFrame->FindWidget("textpane_beammenu") != nullptr) {
    const CPlayer& player = *mgr.GetPlayer(mPlayerIndex);
    const CPlayerState& playerState = *mgr.GetPlayerState(mPlayerIndex);
    const CPlayerGun& gun = *player.GetPlayerGun();
    const CPlayerState::EBeamId beam = player.GetMorphballTransitionState() == CPlayer::kMS_Morphed
                                           ? playerState.GetCurrentBeam()
                                           : gun.GetPrimaryWeaponId();
    mBeamMenu =
        rs_new CHudVisorBeamMenu(*mLoadedHudFrame, mHudStringTable, CHudVisorBeamMenu::kVBM_Beam,
                                 BuildPlayerHasBeams(mgr), beam, mgr.IsMultiplayer());
    mMenuBeam = beam;
  }
  if (mLoadedHudFrame->FindWidget("basewidget_radar") != nullptr) {
    mRadar = rs_new CHudRadarInterface(*mLoadedHudFrame, mgr, mPlayerIndex,
                                       ModulateColor(gpTweakGuiColors->GetRadarWidgetColor()));
  }

  mDecorationRoot = mLoadedHudFrame->FindWidget(rstl::string_l("basewidget_decogroup"));
  mEnergyBar = static_cast< CAuiEnergyBarT01* >(
      mLoadedHudFrame->FindWidget(rstl::string_l("energybart01_energybar")));
  mEnergyDigits =
      static_cast< CGuiTextPane* >(mLoadedHudFrame->FindWidget("textpane_energydigits"));
  mMissileDigits =
      static_cast< CGuiTextPane* >(mLoadedHudFrame->FindWidget("textpane_missiledigits"));
  mMissileFraction = mLoadedHudFrame->FindWidget("model_fraction");
  mEnergyWarning =
      static_cast< CGuiTextPane* >(mLoadedHudFrame->FindWidget("textpane_energywarning"));
  CGuiTextPane* memo = static_cast< CGuiTextPane* >(mLoadedHudFrame->FindWidget("textpane_boss"));
  if (memo != nullptr) {
    memo->TextSupport().SetFontColor(gpTweakGuiColors->GetHUDMemoTextForegroundColor());
    memo->TextSupport().SetOutlineColor(gpTweakGuiColors->GetHUDMemoTextOutlineColor());
  }
  mCounter = static_cast< CGuiTextPane* >(mLoadedHudFrame->FindWidget("textpane_counter"));
  if (mCounter != nullptr) {
    mCounter->TextSupport().SetFontColor(gpTweakGuiColors->GetCountdownForegroundColor());
    mCounter->TextSupport().SetOutlineColor(gpTweakGuiColors->GetCountdownOutlineColor());
  }

  mVisorBracket = mLoadedHudFrame->FindWidget("model_visorbracket");
  mThreatIcon = mLoadedHudFrame->FindWidget("model_threaticon");
  mThreatRoot = mLoadedHudFrame->FindWidget("basewidget_threat");
  mThreatBar = mLoadedHudFrame->FindWidget("model_threatbar");
  mMissileIcon = mLoadedHudFrame->FindWidget("model_missileicon");
  mDarkVisor = mLoadedHudFrame->FindWidget("model_darkvisor");
  mDarkVisorBacking = mLoadedHudFrame->FindWidget("model_darkvisor_black");
  mLightAmmoDigits =
      static_cast< CGuiTextPane* >(mLoadedHudFrame->FindWidget("textpane_lightammodigits"));
  mDarkAmmoDigits =
      static_cast< CGuiTextPane* >(mLoadedHudFrame->FindWidget("textpane_darkammodigits"));
  mLightAmmoIcon = mLoadedHudFrame->FindWidget("model_lighticon");
  mDarkAmmoIcon = mLoadedHudFrame->FindWidget("model_darkicon");
  mFreeLookLeft = mLoadedHudFrame->FindWidget("model_freelookleft");
  mFreeLookRight = mLoadedHudFrame->FindWidget("model_freelookright");
  mMissileGauge =
      static_cast< CAuiBitmapMeter* >(mLoadedHudFrame->FindWidget("barmeter_missileguagequad"));
  mThreatGauge =
      static_cast< CAuiBitmapMeter* >(mLoadedHudFrame->FindWidget("barmeter_threatguagequad"));
  if (mThreatGauge != nullptr) {
    mThreatGauge->SetIncreaseSpeed(50.f);
  }
  mEnergyBracket = mLoadedHudFrame->FindWidget("model_rightenergymeterbracket");
  mHudCamera = mLoadedHudFrame->GetFrameCamera();
  mAutomapperRoot = mLoadedHudFrame->FindWidget(rstl::string_l("basewidget_automapper"));
  mAutomapperModel = mLoadedHudFrame->FindWidget(rstl::string_l("model_automapper"));
  mPowerBombDigits =
      static_cast< CGuiTextPane* >(mLoadedHudFrame->FindWidget("textpane_bombdigits"));
  mPowerBombIcon = mLoadedHudFrame->FindWidget("model_bombicon");
  mPowerBombDecoration = mLoadedHudFrame->FindWidget("model_bombdeco");
  if (mPowerBombDecoration != nullptr) {
    mPowerBombDecoration->SetColor(gpTweakGuiColors->GetMorphBallEnergyDecoColor());
  }
  if (mDarkVisor != nullptr && mDarkVisorBacking != nullptr) {
    const CPlayerState::EPlayerVisor visor =
        mgr.GetPlayer(mPlayerIndex)->GetPlayerState()->GetCurrentVisor();
    mDarkVisor->SetIsVisible(visor == CPlayerState::kPV_Dark);
    mDarkVisorBacking->SetIsVisible(visor == CPlayerState::kPV_Dark);
  }

  for (int i = 0; i < 3; ++i) {
    CGuiWidget* indicator = mLoadedHudFrame->FindWidget(CBasics::Stringize("model_bombcount%d", i));
    if (indicator != nullptr) {
      mBombIndicators.push_back(indicator);
      indicator->SetVisibility(false, kTM_Children);
    }
  }
  if (mAutomapperModel != nullptr) {
    mAutomapperModel->SetDrawFlags(CGuiWidget::kGMDF_Additive);
    mAutomapperModel->SetDepthWrite(true);
    mAutomapperModel->SetColor(CColor(uchar(0), uchar(0), uchar(0), uchar(1)));
  }
  mThreatRoot = mLoadedHudFrame->FindWidget("basewidget_threat");
  if (mFreeLookLeft != nullptr) {
    mFreeLookLeftTransform = mFreeLookLeft->GetWorldTransform();
  }
  if (mFreeLookRight != nullptr) {
    mFreeLookRightTransform = mFreeLookRight->GetWorldTransform();
  }

  for (int i = 0; i < 14; ++i) {
    CGuiWidget* tank = mLoadedHudFrame->FindWidget(CBasics::Stringize("model_Etankfill0%db", i));
    if (tank != nullptr) {
      tank->SetIsVisible(false);
      mFilledEnergyTanks.push_back(tank);
    }
  }
  for (int i = 0; i < 14; ++i) {
    CGuiWidget* tank = mLoadedHudFrame->FindWidget(CBasics::Stringize("model_Etankempty0%d", i));
    if (tank != nullptr) {
      tank->SetIsVisible(false);
      mEmptyEnergyTanks.push_back(tank);
    }
  }
  if (mEnergyBracket != nullptr) {
    mEnergyBracketTransform = mEnergyBracket->GetWorldTransform();
  }
  if (mEnergyBar != nullptr) {
    const CColor empty = ModulateColor(gpTweakGuiColors->GetEnergyBarEmptyColor());
    const CColor filled = ModulateColor(gpTweakGuiColors->GetEnergyBarFilledColor());
    const CColor shadow = ModulateColor(gpTweakGuiColors->GetEnergyBarShadowColor());
    mEnergyBar->SetMaxEnergy(CPlayerState::GetBaseHealthCapacity());
    mEnergyBar->SetFilledColor(filled);
    mEnergyBar->SetShadowColor(shadow);
    mEnergyBar->SetEmptyColor(empty);
    mEnergyBar->SetFilledDrainSpeed(gpTweakGui->GetEnergyBarFilledDrainSpeed());
    mEnergyBar->SetShadowDrainSpeed(gpTweakGui->GetEnergyBarShadowDrainSpeed());
    mEnergyBar->SetShadowDrainDelay(gpTweakGui->GetEnergyBarShadowDrainDelay());
    mEnergyBar->SetIsAlwaysResetTimer(gpTweakGui->GetEnergyBarAlwaysResetDelay());
    mEnergyBar->SetTesselation(0.1f);
    if (mNextState == kHS_Ball) {
      mEnergyBar->SetCoordFunc(BallEnergyCoordFunc);
    } else {
      mEnergyBar->SetCoordFunc(CombatEnergyCoordFunc);
    }
  }

  for (int i = 0; i < 5; ++i) {
    CGuiWidget* segment = mLoadedHudFrame->FindWidget(CBasics::Stringize("model_darkbeam%d", i));
    if (segment != nullptr) {
      segment->SetIsVisible(false);
      segment->SetColor(gpTweakGuiColors->GetDarkAmmoTankEmptyUnselectedColor());
      mDarkAmmoSegments.push_back(segment);
    }
  }
  for (int i = 0; i < 5; ++i) {
    CAuiBitmapMeter* meter = static_cast< CAuiBitmapMeter* >(
        mLoadedHudFrame->FindWidget(CBasics::Stringize("barmeter_darkammo%d", i)));
    if (meter != nullptr) {
      meter->SetIsVisible(false);
      meter->SetColor(gpTweakGuiColors->GetDarkAmmoMeterUnselectedFillColor());
      meter->SetShadowColor(gpTweakGuiColors->GetDarkAmmoMeterUnselectedShadowColor());
      mDarkAmmoMeters.push_back(meter);
    }
  }
  for (int i = 0; i < 5; ++i) {
    CGuiWidget* segment = mLoadedHudFrame->FindWidget(CBasics::Stringize("model_lightbeam%d", i));
    if (segment != nullptr) {
      segment->SetIsVisible(false);
      segment->SetColor(gpTweakGuiColors->GetLightAmmoTankEmptyUnselectedColor());
      mLightAmmoSegments.push_back(segment);
    }
  }
  for (int i = 0; i < 5; ++i) {
    CAuiBitmapMeter* meter = static_cast< CAuiBitmapMeter* >(
        mLoadedHudFrame->FindWidget(CBasics::Stringize("barmeter_lightammo%d", i)));
    if (meter != nullptr) {
      meter->SetIsVisible(false);
      meter->SetColor(gpTweakGuiColors->GetLightAmmoMeterUnselectedFillColor());
      meter->SetShadowColor(gpTweakGuiColors->GetLightAmmoMeterUnselectedShadowColor());
      mLightAmmoMeters.push_back(meter);
    }
  }
  if (mLightAmmoIcon != nullptr) {
    mLightAmmoIcon->SetColor(gpTweakGuiColors->GetLightAmmoIconUnselectedColor());
    mLightAmmoIcon->SetIsVisible(false);
  }
  if (mDarkAmmoIcon != nullptr) {
    mDarkAmmoIcon->SetColor(gpTweakGuiColors->GetDarkAmmoIconUnselectedColor());
    mDarkAmmoIcon->SetIsVisible(false);
  }
  if (mLightAmmoDigits != nullptr) {
    mLightAmmoDigits->TextSupport().SetFontColor(
        gpTweakGuiColors->GetLightAmmoDigitsUnselectedColor());
    mLightAmmoDigits->TextSupport().SetOutlineColor(
        gpTweakGuiColors->GetLightAmmoDigitsOutlineColor());
  }
  if (mDarkAmmoDigits != nullptr) {
    mDarkAmmoDigits->TextSupport().SetFontColor(
        gpTweakGuiColors->GetDarkAmmoDigitsUnselectedColor());
    mDarkAmmoDigits->TextSupport().SetOutlineColor(
        gpTweakGuiColors->GetDarkAmmoDigitsOutlineColor());
  }
  if (mMissileDigits != nullptr) {
    mMissileDigits->TextSupport().SetFontColor(
        gpTweakGuiColors->GetCombatMissileDigitsForegroundColor());
    mMissileDigits->TextSupport().SetOutlineColor(
        gpTweakGuiColors->GetCombatMissileDigitsOutlineColor());
    if (mMissileFraction != nullptr) {
      mMissileFraction->SetColor(gpTweakGuiColors->GetCombatMissileDigitsForegroundColor());
    }
  }

  if (mLoadedHelmetFrame != nullptr) {
    CGuiCamera* camera = mLoadedHelmetFrame->GetFrameCamera();
    const CGuiCamera* hudCamera = mLoadedHudFrame->GetFrameCamera();
    camera->SetParms(hudCamera->GetParms());
    camera->SetO2WTransform(CTransform4f::Translate(hudCamera->GetLocalPosition()));
  }
  UpdateHelmetWidgets();
  UpdateHudWidgetColors();
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

void CSamusHud::UpdateHelmetWidgets() {
  const EHudState state = GetNextState();
  if (mLoadedHelmetFrame == nullptr) {
    return;
  }

  for (int visor = 0; visor < 4; ++visor) {
    for (uint i = 0; i < 2; ++i) {
      CGuiWidget* widget = mLoadedHelmetFrame->FindWidget(sHelmetLightWidgets[visor][i]);
      if (widget != nullptr) {
        widget->SetIsVisible(false);
      }
    }
  }
  for (uint i = 0; i < 2; ++i) {
    CGuiWidget* widget = mLoadedMemoFrame->FindWidget(sMemoDecorationWidgets[i]);
    if (widget != nullptr) {
      widget->SetColor(ModulateColor(gpTweakGuiColors->GetHUDDecorativeColor()));
    }
  }
  for (uint i = 0; i < 2; ++i) {
    CGuiWidget* widget = mLoadedHelmetFrame->FindWidget(sHelmetLightWidgets[state][i]);
    if (widget != nullptr) {
      widget->SetIsVisible(true);
    }
  }
}

void CSamusHud::UpdateHudWidgetColors() {
  const bool darkVisor = mNextState == kHS_Dark;
  const EHudState state = GetNextState();
  const CTweakGuiColors::SVisorColorScheme& scheme = gpTweakGuiColors->GetVisorColorScheme(state);
  for (uint i = 0; i < 4; ++i) {
    CGuiWidget* widget = mLoadedHudFrame->FindWidget(sDecorativeWidgets[i]);
    if (widget != nullptr) {
      widget->SetColor(ModulateColor(gpTweakGuiColors->GetHUDDecorativeColor()));
    }
  }

  const CColor decorative = ModulateColor(gpTweakGuiColors->GetHUDDecorativeColor());
  for (uint i = 0; i < 4; ++i) {
    CGuiWidget* widget = mLoadedHudFrame->FindWidget(sDarkVisorWidgets[i]);
    if (widget != nullptr) {
      widget->SetVisibility(darkVisor, kTM_Children);
      widget->SetColor(decorative);
    }
  }
  if (mLoadedHelmetFrame != nullptr) {
    CGuiWidget* glass = mLoadedHelmetFrame->FindWidget("model_glass");
    if (glass != nullptr) {
      glass->SetColor(scheme.GetGlassTint());
    }
  }
  const bool nonScan = mNextState != kHS_Scan;
  for (uint i = 0; i < 15; ++i) {
    CGuiWidget* widget = mLoadedHudFrame->FindWidget(sNonScanWidgets[i]);
    if (widget != nullptr) {
      widget->SetVisibility(nonScan, kTM_Children);
    }
  }

  if (mFreeLookLeft != nullptr) {
    mFreeLookLeft->SetColor(ModulateColor(gpTweakGuiColors->GetFreeLookBarColor()));
  }
  if (mFreeLookRight != nullptr) {
    mFreeLookRight->SetColor(ModulateColor(gpTweakGuiColors->GetFreeLookBarColor()));
  }
  if (mMissileIcon != nullptr) {
    mMissileIcon->SetColor(ModulateColor(gpTweakGuiColors->GetMissileGroupActiveColor()));
  }
  if (mMissileGauge != nullptr) {
    const CColor& filled = ModulateColor(gpTweakGuiColors->GetEnergyTanksFilledColor());
    const CColor& shadow = ModulateColor(gpTweakGuiColors->GetEnergyBarShadowColor());
    mMissileGauge->SetColor(filled);
    mMissileGauge->SetShadowColor(shadow);
  }
  if (mThreatGauge != nullptr) {
    const CColor& filled = ModulateColor(gpTweakGuiColors->GetThreatGroupInactiveColor());
    const CColor& shadow = CColor::Black();
    mThreatGauge->SetColor(filled);
    mThreatGauge->SetShadowColor(shadow);
  }
  if (mThreatIcon != nullptr) {
    mThreatIcon->SetColor(ModulateColor(gpTweakGuiColors->GetThreatGroupInactiveColor()));
  }
  if (mEnergyDigits != nullptr) {
    mEnergyDigits->TextSupport().SetFontColor(
        ModulateColor(gpTweakGuiColors->GetActiveTextForegroundColor()));
    mEnergyDigits->TextSupport().SetOutlineColor(
        ModulateColor(gpTweakGuiColors->GetTextShadowOutlineColor()));
  }

  const CColor& empty = ModulateColor(gpTweakGuiColors->GetEnergyTanksEmptyColor());
  const CColor& filled = ModulateColor(gpTweakGuiColors->GetEnergyTanksFilledColor());
  for (int i = 0; i < mFilledEnergyTanks.size(); ++i) {
    if (mFilledEnergyTanks[i] != nullptr) {
      mFilledEnergyTanks[i]->SetColor(filled);
    }
  }
  for (int i = 0; i < mEmptyEnergyTanks.size(); ++i) {
    if (mEmptyEnergyTanks[i] != nullptr) {
      mEmptyEnergyTanks[i]->SetColor(empty);
    }
  }
  if (mLoadedHudFrame->FindWidget("basewidget_bossenergy") != nullptr) {
    mBossEnergy = rs_new CHudBossEnergyInterface(*mLoadedHudFrame, GetNextState());
  }
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

bool CSamusHud::IsHudMemoVisible(int playerIndex) {
  const CSamusHud* hud = gpSamusHud[playerIndex];
  if (hud == nullptr || hud->mMessageRoot == nullptr || hud->mMessagePane == nullptr) {
    return false;
  }
  return hud->mMessageRoot->GetIsVisible() || hud->mMessagePane->GetIsVisible();
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
, mEnergyLow(mgr.GetPlayer(playerIndex)->IsEnergyLow())
, mMenuBeam(CPlayerState::kBI_Power)
, mMenuVisor(CPlayerState::kPV_Combat)
, mMissileEnabled(0)
, mPreviousCameraDirection(
      mgr.GetCameraManager(playerIndex)->GetFirstPersonCamera()->GetTransform().GetForward())
, mHudLag(CQuaternion::NoRotation())
, mInverseHudLag(CQuaternion::NoRotation())
, mHelmetLightingWidget(
      mLoadedHelmetFrame != nullptr ? mLoadedHelmetFrame->FindWidget("BaseWidget_Helmet") : nullptr)
, mLights(rs_new CActorLights(8, CVector3f::Zero(), 4, 1, CActorLights::kDefaultMinPosChange, true,
                              false, false, false))
, mHudLights(3, SCachedHudLight(CVector3f::Zero(), CColor::White(), 0.f, 0.f, 0.f, 0.f))
, mHudStringTable(static_cast< CStringTable* >(nullptr))
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
, mDamageRingTexture(mgr.IsMultiplayer() ? gpSimplePool->GetObj("TXTR_QuarterCurveMP")
                                         : gpSimplePool->GetObj("TXTR_QuarterCurve"))
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
, mMessageTime(0.f)
, mLastMessageSoundChars(0.f)
, mPreviousFreeLookDirection(
      mgr.GetCameraManager(playerIndex)->GetFirstPersonCamera()->GetTransform().GetForward())
, mFreeLookDirectionDot(1.f)
, mFreeLookSoundCycle(0.f)
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
, mMissilePickupPulse(0.f)
, mLightAmmoPickupPulse(0.f)
, mDarkAmmoPickupPulse(0.f)
, mEnergyBracketTransform(CTransform4f::Identity())
, x7ec(0.f)
, mThreatAlpha(1.f)
, mBallBeamTransition(0.f)
, mPreviousBallBeam(CPlayerState::kBI_Power)
, mGuiState(3)
, mHudColor(0u)
, mBootTimer(0.f)
, mBootTextFade(0.f)
, mHudBootAlpha(1.f)
, mCorruptTextTimer(0.f)
, mBootText(gpResourceFactory->GetResourceIdByName("FONT_Deface13B")->GetId(), 420, 400,
            CGuiTextProperties(true, kJustification_Left, kVerticalJustification_Bottom),
            gpTweakGuiColors->GetHUDMemoTextForegroundColor().WithAlphaOf(0.5f),
            CColor::White().WithAlphaOf(0.f), CColor::White(), gpSimplePool)
, mBooting(false)
, mProfileInfo(17, SProfileInfo()) {
  RefreshHudStringTable();
  gpSamusHud[mPlayerIndex] = this;
  mDamageRingTexture.Lock();
  if (mgr.IsMultiplayer()) {
    mLockedOnIndicator = TCachedToken< CTexture >(gpSimplePool->GetObj("TXTR_LockedOnIndicator"));
    mLockedOnIndicator->Lock();
  }
  mDesiredState = kHS_Combat;
  mNextState = kHS_Combat;
  if (mgr.GetPlayer(playerIndex)->GetMorphballTransitionState() != CPlayer::kMS_Unmorphed) {
    mTransitionFactor = 0.f;
  }
  mgr.GetViewportLayoutIndex();
  UpdateHudColor();
  InitializeFrameGluePermanent();
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

void CSamusHud::RefreshHudStringTable() {
  mHudStringTable = TLockedToken< CStringTable >(gpSimplePool->GetObj(
      gpGameState->GameOptions().GetIsHudEnglish() ? "STRG_HudEngOnly" : "STRG_Hud"));
}

CHudDecoInterfaceScan* CSamusHud::GetScanInterface(int playerIndex) {
  CSamusHud* hud = gpSamusHud[playerIndex];
  return hud != nullptr ? hud->mScanInterface.get() : nullptr;
}

void CSamusHud::UpdateEnergyLow(float dt, const CStateManager& mgr) {
  const bool cineCam =
      TCastToConstPtr< CCinematicCamera >(
          mgr.GetCameraManager(mPlayerIndex)->GetCurrentCamera(mgr, true)) != nullptr;
  const float oldTimer = mEnergyLowTimer;
  mEnergyLowTimer = fmod(mEnergyLowTimer + dt, 0.5);
  mEnergyLowPulse =
      mEnergyLowTimer < 0.25f ? mEnergyLowTimer / 0.25f : (0.5f - mEnergyLowTimer) / 0.25f;
  if (mEnergyLow) {
    mEnergyLowFade = rstl::min_val(1.f, mEnergyLowFade + 2.f * dt);
  } else {
    mEnergyLowFade = rstl::max_val(0.f, mEnergyLowFade - 2.f * dt);
  }
  if (mEnergyWarning != nullptr) {
    CColor fontColor = gpTweakGuiColors->GetEnergyWarningColor();
    fontColor.SetAlpha(mEnergyLowPulse * mEnergyLowFade);
    mEnergyWarning->TextSupport().SetFontColor(fontColor);
    CColor outlineColor = gpTweakGuiColors->GetEnergyWarningOutlineColor();
    outlineColor.SetAlpha(mEnergyLowPulse * mEnergyLowFade);
    mEnergyWarning->TextSupport().SetOutlineColor(outlineColor);
  }
  if (!cineCam && mEnergyLow && mEnergyLowTimer < oldTimer) {
    CSfxManager::SfxStart(0x37, 127, mgr.GetPlayer(mPlayerIndex)->GetSoundPan(CPlayer::kMSP_4),
                          CSfxManager::kAllAreas, false, false, CSfxManager::kMedPriority);
  }
}

CSamusHud::~CSamusHud() {
  if (mDamageSound) {
    CSfxManager::SfxStop(mDamageSound);
  }
  gpSamusHud[mPlayerIndex] = nullptr;
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

void CSamusHud::UpdateVisorAndBeamMenus(float dt, const CStateManager& mgr) {
  // TODO: update menu selections and their transition factors.
}

void CSamusHud::UpdateFreeLook(float dt, const CStateManager& mgr) {
  // TODO: update freelook indicators, transforms and movement sound.
}

void CSamusHud::UpdateStaticInterference(float dt, const CStateManager& mgr) {
  // TODO: combine player interference with HUD alpha, filters and sounds.
}

void CSamusHud::UpdateStaticSfx(const CStateManager& mgr, CSfxHandle& sound, float& cycle,
                                ushort soundId, float dt, float previousInterference,
                                float threshold) {
  const bool crossed = (previousInterference > threshold && mStaticInterference <= threshold) ||
                       (previousInterference <= threshold && mStaticInterference > threshold);
  if (crossed) {
    cycle = 0.f;
  } else if (cycle < 0.1f) {
    cycle = rstl::min_val(0.1f, cycle + dt);
    if (cycle == 0.1f) {
      if (mStaticInterference > threshold) {
        if (!sound) {
          sound = CSfxManager::SfxStart(
              soundId, 127, mgr.GetPlayer(mPlayerIndex)->GetSoundPan(CPlayer::kMSP_4),
              CSfxManager::kAllAreas, false, true, CSfxManager::kMedPriority);
        }
      } else {
        CSfxManager::SfxStop(sound);
        sound.Clear();
      }
    }
  }
}

void CSamusHud::UpdateHudColor() {
  const EHudState state = GetNextState();
  const CTweakGuiColors::SVisorColorScheme& scheme = gpTweakGuiColors->GetVisorColorScheme(state);
  mHudColor = scheme.GetHUDHue();
}

void CSamusHud::UpdateEnergy(float dt, const CStateManager& mgr, bool init) {
  const CPlayerState& state = *mgr.GetPlayerState(mPlayerIndex);
  const CPlayer& player = *mgr.GetPlayer(mPlayerIndex);
  const float energy = rstl::max_val(0.f, CMath::CeilingF(state.GetHealthInfo().GetHP()));
  const int numEnergyTanks = state.GetItemCapacity(CPlayerState::kIT_EnergyTanks);
  const bool energyLow = player.IsEnergyLow();
  if (init || energy != mPlayerHealth || numEnergyTanks != mEnergyTankCapacity ||
      energyLow != mEnergyLow) {
    float lastTankEnergy = energy;
    int filledTanks = 0;
    while (lastTankEnergy > CPlayerState::GetBaseHealthCapacity()) {
      ++filledTanks;
      lastTankEnergy -= CPlayerState::GetEnergyTankCapacity();
    }
    if (mEnergyBar != nullptr) {
      mEnergyBar->SetCurrEnergy(lastTankEnergy, CAuiEnergyBarT01::kSM_Normal);
    }
    if (energyLow != mEnergyLow || init) {
      const rstl::wstring warning = energyLow
                                        ? rstl::wstring_l(mHudStringTable->GetString("EnergyLow"))
                                        : rstl::wstring_l(L"");
      if (mEnergyWarning != nullptr) {
        mEnergyWarning->TextSupport().SetText(warning);
      }
      if (energyLow) {
        CSfxManager::SfxStart(0x37, 127, 64, CSfxManager::kAllAreas, false, false,
                              CSfxManager::kMedPriority);
      }
      mEnergyLow = energyLow;
    }
    for (int i = 0; i < mFilledEnergyTanks.size(); ++i) {
      CGuiWidget* filled = mFilledEnergyTanks[i];
      CGuiWidget* empty = mEmptyEnergyTanks[i];
      if (filled != nullptr && empty != nullptr) {
        if (i < numEnergyTanks) {
          const bool full = i < filledTanks;
          filled->SetVisibility(full, kTM_Children);
          empty->SetVisibility(!full, kTM_Children);
        } else {
          filled->SetVisibility(false, kTM_Children);
          empty->SetVisibility(false, kTM_Children);
        }
      }
    }
    char digits[16];
    sprintf(digits, "%02d", int(lastTankEnergy));
    mEnergyDigits->TextSupport().SetText(CStringExtras::ConvertToUNICODE(rstl::string_l(digits)));
    float currentTankEnergy = mPlayerHealth;
    while (currentTankEnergy > CPlayerState::GetBaseHealthCapacity()) {
      currentTankEnergy -= CPlayerState::GetEnergyTankCapacity();
    }
    mPlayerHealth = energy;
    mEnergyTankCapacity = numEnergyTanks;
  }
  if (mEnergyBar != nullptr) {
    const CColor emptyColor = ModulateColor(gpTweakGuiColors->GetEnergyBarEmptyColor());
    const CColor filledColor = ModulateColor(gpTweakGuiColors->GetEnergyBarFilledColor());
    const CColor shadowColor = ModulateColor(gpTweakGuiColors->GetEnergyBarShadowColor());
    const CColor lowEmptyColor = gpTweakGuiColors->GetEnergyBarLowEmptyColor();
    const CColor lowFilledColor = filledColor;
    const CColor lowShadowColor = shadowColor;
    const CColor finalEmpty = mEnergyLow ? lowEmptyColor : emptyColor;
    const CColor finalFilled = mEnergyLow ? lowFilledColor : filledColor;
    const CColor finalShadow = mEnergyLow ? lowShadowColor : shadowColor;
    CColor damageColor = CColor::Lerp(finalFilled, gpTweakGuiColors->GetEnergyBarDamageColor(),
                                      mDamageHighlightRemaining / mDamageHighlightDuration);
    if (mEnergyLow) {
      damageColor = CColor::Lerp(damageColor, CColor(1.f, 0.f, 0.f, 1.f), mEnergyLowTimer);
    }
    mEnergyBar->SetFilledColor(damageColor);
    mEnergyBar->SetShadowColor(finalShadow);
    mEnergyBar->SetEmptyColor(finalEmpty);
    if (mEnergyDigits != nullptr) {
      mEnergyDigits->SetColor(damageColor);
    }
  }
  if (mBossEnergy.get() != nullptr) {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mgr.GetBossId()))) {
      if (const CHealthInfo* health = actor->GetHealthInfo()) {
        const float bossEnergy = CMath::CeilingF(health->GetHP());
        const float maxEnergy = mgr.GetTotalBossEnergy();
        const rstl::wstring name =
            rstl::wstring_l(gpStringTable->GetString(mgr.GetBossStringIdx()));
        mBossEnergy->SetBossParams(true, name, bossEnergy, maxEnergy);
      } else {
        mBossEnergy->SetBossParams(false, rstl::wstring_l(L""), 0.f, 0.f);
      }
    } else {
      mBossEnergy->SetBossParams(false, rstl::wstring_l(L""), 0.f, 0.f);
    }
  }
}

void CSamusHud::UpdateMissile(float dt, const CStateManager& mgr, bool init) {
  if (mMissileDigits == nullptr) {
    return;
  }
  const CPlayerGun& gun = *mgr.GetPlayer(mPlayerIndex)->GetPlayerGun();
  const CPlayerState& state = *mgr.GetPlayerState(mPlayerIndex);
  const int enabled = !gun.GetMissileMode();
  const int missiles = state.GetItemAmount(CPlayerState::kIT_Missile, true);
  const int capacity = state.GetItemCapacity(CPlayerState::kIT_Missile);
  if (init || missiles != mMissileAmount || enabled != mMissileEnabled ||
      capacity != mMissileCapacity || CMath::AbsF(mMissilePickupPulse) >= 0.00001f ||
      CMath::AbsF(mMissileModeTransition) >= 0.00001f) {
    if (GetNextState() != kHS_Scan) {
      if (missiles > mMissileAmount) {
        mMissilePickupPulse = 0.5f;
      }
      mMissilePickupPulse = rstl::max_val(0.f, mMissilePickupPulse - dt);
      const float pickup = CMath::FastSinR(M_PIF * (mMissilePickupPulse / 0.5f));
      const CColor flash =
          CColor::Lerp(CColor::Black(), gpTweakGuiColors->GetMissileGroupChangeFlash(), pickup);
      mMissileModeTransition = rstl::max_val(0.f, mMissileModeTransition - 3.f * dt);
      if (mMissileEnabled != enabled) {
        mMissileModeTransition = 1.f;
      }
      const float transition =
          gun.GetMissileMode() ? mMissileModeTransition : 1.f - mMissileModeTransition;
      const CColor active =
          CColor::Add(ModulateColor(gpTweakGuiColors->GetMissileGroupActiveColor()), flash);
      const CColor inactive =
          CColor::Add(ModulateColor(gpTweakGuiColors->GetMissileGroupInactiveColor()), flash);
      const CColor& depletion = gpTweakGuiColors->GetMissileDepletionColor();
      const CColor iconColor =
          missiles == 0 ? depletion : CColor::Lerp(active, inactive, transition);
      const bool visible = mgr.IsMultiplayer() ? missiles != 0 : capacity != 0;
      if (mMissileIcon != nullptr) {
        mMissileIcon->SetColor(iconColor);
        mMissileIcon->SetVisibility(visible, kTM_Children);
      }
      const CColor activeText =
          CColor::Add(ModulateColor(gpTweakGuiColors->GetActiveTextForegroundColor()), flash);
      const CColor inactiveText =
          CColor::Add(ModulateColor(gpTweakGuiColors->GetInactiveTextForegroundColor()), flash);
      const CColor textColor =
          missiles == 0 ? depletion : CColor::Lerp(activeText, inactiveText, transition);
      char digits[16];
      if (mgr.IsMultiplayer()) {
        sprintf(digits, "%02d", missiles);
      } else {
        sprintf(digits, "%02d\n%02d", missiles, capacity);
      }
      mMissileDigits->TextSupport().SetText(
          CStringExtras::ConvertToUNICODE(rstl::string_l(digits)));
      mMissileDigits->TextSupport().SetFontColor(textColor);
      mMissileDigits->SetVisibility(visible, kTM_Children);
      if (mMissileFraction != nullptr) {
        mMissileFraction->SetColor(textColor);
        mMissileFraction->SetVisibility(visible, kTM_Children);
      }
      if (mMissileGauge != nullptr && capacity != 0) {
        mMissileGauge->SetTargetFraction(float(missiles) / float(capacity));
      }
      mMissileAmount = missiles;
      mMissileEnabled = enabled;
      mMissileCapacity = capacity;
    }
  }
  if (missiles != 0 && capacity != 0 &&
      float(missiles) <= float(capacity) * gpTweakGui->GetMissileWarningThreshold()) {
    const float transition =
        gun.GetMissileMode() ? mMissileModeTransition : 1.f - mMissileModeTransition;
    const float pulse =
        (1.f + CMath::FastCosR(CMath::WrapPi(M_2PIF * CGraphics::GetSecondsMod900() / 1.5f))) *
        0.5f;
    if (mMissileIcon != nullptr) {
      const CColor base =
          CColor::Lerp(ModulateColor(gpTweakGuiColors->GetMissileGroupActiveColor()),
                       ModulateColor(gpTweakGuiColors->GetMissileGroupInactiveColor()), transition);
      const CColor color = CColor::Lerp(base, gpTweakGuiColors->GetMissileWarningColor(), pulse);
      mMissileIcon->SetColor(color);
    }
    if (mMissileDigits != nullptr) {
      const CColor base = CColor::Lerp(
          ModulateColor(gpTweakGuiColors->GetActiveTextForegroundColor()),
          ModulateColor(gpTweakGuiColors->GetInactiveTextForegroundColor()), transition);
      const CColor color = CColor::Lerp(base, gpTweakGuiColors->GetMissileWarningColor(), pulse);
      mMissileDigits->TextSupport().SetFontColor(color);
      if (mMissileFraction != nullptr) {
        mMissileFraction->SetColor(color);
      }
    }
  }
}

void CSamusHud::UpdateBeamAmmo(const CStateManager& mgr, bool init) {
  if (mDesiredState == kHS_Scan) {
    return;
  }
  const CPlayerState& state = *mgr.GetPlayerState(mPlayerIndex);
  const CPlayer& player = *mgr.GetPlayer(mPlayerIndex);
  CPlayerState::EBeamId beam;
  if (player.GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
    beam = mBallBeamTransition > 0.3f ? mPreviousBallBeam : state.GetCurrentBeam();
  } else {
    beam = player.GetPlayerGun()->GetPrimaryWeaponId();
  }
  const float beamFactor = CMath::Clamp(0.f, mBeamMenuTransition, 1.f);
  const int darkAmmo = state.GetItemAmount(CPlayerState::kIT_DarkAmmo, true);
  const int lightAmmo = state.GetItemAmount(CPlayerState::kIT_LightAmmo, true);
  if (init || darkAmmo != mDarkAmmo || beam != mAmmoBeam ||
      float(darkAmmo) <= float(state.GetItemCapacity(CPlayerState::kIT_DarkAmmo)) *
                             gpTweakGui->GetMissileWarningThreshold() ||
      CMath::AbsF(mDarkAmmoPickupPulse) >= 0.00001f ||
      (CMath::AbsF(beamFactor) >= 0.00001f && CMath::AbsF(beamFactor - 1.f) >= 0.00001f)) {
    if (state.GetItemAmount(CPlayerState::kIT_DarkBeam, true) < 1 &&
        state.GetItemAmount(CPlayerState::kIT_AnnihilatorBeam, true) < 1) {
      if (mDarkAmmoIcon != nullptr) {
        mDarkAmmoIcon->SetIsVisible(false);
      }
      for (int i = 0; i < mDarkAmmoSegments.size(); ++i) {
        mDarkAmmoSegments[i]->SetColor(gpTweakGuiColors->GetDarkAmmoTankEmptyUnselectedColor());
        mDarkAmmoMeters[i]->SetVisibility(false, kTM_Children);
        mDarkAmmoSegments[i]->SetVisibility(false, kTM_Children);
      }

    } else {
      if (darkAmmo > mDarkAmmo) {
        mDarkAmmoPickupPulse = 0.5f;
      }
      mDarkAmmoPickupPulse = rstl::max_val(0.f, mDarkAmmoPickupPulse - 0.0166f);
      const float pickup = CMath::FastSinR(M_PIF * (mDarkAmmoPickupPulse / 0.5f));
      const CColor flash =
          CColor::Lerp(CColor::Black(), gpTweakGuiColors->GetDarkAmmoChangeFlash(), pickup);
      const bool selected = beam == CPlayerState::kBI_Dark || beam == CPlayerState::kBI_Annihilator;
      const float selection = selected ? beamFactor : 0.f;
      float warning = 0.f;
      if (darkAmmo != 0 &&
          float(darkAmmo) <= float(state.GetItemCapacity(CPlayerState::kIT_DarkAmmo)) *
                                 gpTweakGui->GetMissileWarningThreshold()) {
        warning =
            (1.f + CMath::FastCosR(CMath::WrapPi(M_2PIF * CGraphics::GetSecondsMod900() / 1.5f))) *
            0.5f;
      }
      const CColor selectedEmpty =
          CColor::Lerp(gpTweakGuiColors->GetDarkAmmoTankEmptySelectedColor(),
                       gpTweakGuiColors->GetDarkAmmoEmptyTankWarningColor(), warning);
      const CColor selectedFull =
          CColor::Lerp(gpTweakGuiColors->GetDarkAmmoTankFullSelectedColor(),
                       gpTweakGuiColors->GetDarkAmmoTankWarningColor(), warning);
      const CColor selectedFill =
          CColor::Lerp(gpTweakGuiColors->GetDarkAmmoMeterSelectedFillColor(),
                       gpTweakGuiColors->GetDarkAmmoMeterWarningColor(), warning);
      const CColor empty = CColor::Lerp(gpTweakGuiColors->GetDarkAmmoTankEmptyUnselectedColor(),
                                        selectedEmpty, selection);
      const CColor full = CColor::Lerp(gpTweakGuiColors->GetDarkAmmoTankFullUnselectedColor(),
                                       selectedFull, selection);
      const CColor fill =
          CColor::Add(CColor::Lerp(gpTweakGuiColors->GetDarkAmmoMeterUnselectedFillColor(),
                                   selectedFill, selection),
                      flash);
      const CColor shadow =
          CColor::Lerp(gpTweakGuiColors->GetDarkAmmoMeterUnselectedShadowColor(),
                       gpTweakGuiColors->GetDarkAmmoMeterSelectedShadowColor(), selection);
      const CColor icon = CColor::Lerp(gpTweakGuiColors->GetDarkAmmoIconUnselectedColor(),
                                       gpTweakGuiColors->GetDarkAmmoIconSelectedColor(), selection);
      const CColor baseDigits =
          CColor::Lerp(gpTweakGuiColors->GetDarkAmmoDigitsUnselectedColor(),
                       gpTweakGuiColors->GetDarkAmmoDigitsSelectedColor(), selection);
      const CColor digits =
          CColor::Lerp(baseDigits, gpTweakGuiColors->GetDarkAmmoDigitWarningColor(), warning);
      if (mDarkAmmoDigits != nullptr) {
        mDarkAmmoDigits->TextSupport().SetFontColor(
            darkAmmo == 0 ? gpTweakGuiColors->GetDarkAmmoDepletionColor() : digits);
      }
      if (mDarkAmmoSegments.size() != 0) {
        const int perTank =
            CPlayerState::GetPowerUpMaxValue(CPlayerState::kIT_DarkAmmo) / mDarkAmmoSegments.size();
        const int capacity = state.GetItemCapacity(CPlayerState::kIT_DarkAmmo);
        const int filledTanks = darkAmmo / perTank;
        const int activeTanks = filledTanks + 1;
        const int capacityTanks = (capacity + perTank - 1) / perTank;
        for (int i = 0; i < mDarkAmmoSegments.size(); ++i) {
          mDarkAmmoSegments[i]->SetVisibility(capacity != 0, kTM_Children);
          mDarkAmmoMeters[i]->SetVisibility(capacity != 0, kTM_Children);
          mDarkAmmoMeters[i]->SetColor(fill);
          mDarkAmmoMeters[i]->SetShadowColor(shadow);
          mDarkAmmoSegments[i]->SetColor(i < capacityTanks ? full : empty);
          mDarkAmmoMeters[i]->SetTargetFraction(i < activeTanks ? 1.f : 0.f);
        }
        if (activeTanks > 0 && activeTanks <= mDarkAmmoSegments.size()) {
          mDarkAmmoMeters[filledTanks]->SetTargetFraction(float(darkAmmo - filledTanks * perTank) /
                                                          float(perTank));
        }
      }
      if (mDarkAmmoDigits != nullptr) {
        char buffer[16];
        sprintf(buffer, "%02d", darkAmmo);
        mDarkAmmoDigits->TextSupport().SetText(
            CStringExtras::ConvertToUNICODE(rstl::string_l(buffer)));
      }
      mDarkAmmo = darkAmmo;
      if (mDarkAmmoIcon != nullptr) {
        mDarkAmmoIcon->SetIsVisible(true);
        mDarkAmmoIcon->SetColor(darkAmmo < 1 ? empty : full);
      }
    }
  }
  if (init || lightAmmo != mLightAmmo || beam != mAmmoBeam ||
      float(lightAmmo) <= float(state.GetItemCapacity(CPlayerState::kIT_LightAmmo)) *
                              gpTweakGui->GetMissileWarningThreshold() ||
      CMath::AbsF(mLightAmmoPickupPulse) >= 0.00001f ||
      (CMath::AbsF(beamFactor) >= 0.00001f && CMath::AbsF(beamFactor - 1.f) >= 0.00001f)) {
    if (state.GetItemAmount(CPlayerState::kIT_LightBeam, true) < 1 &&
        state.GetItemAmount(CPlayerState::kIT_AnnihilatorBeam, true) < 1) {
      if (mLightAmmoIcon != nullptr) {
        mLightAmmoIcon->SetIsVisible(false);
      }
      for (int i = 0; i < mLightAmmoSegments.size(); ++i) {
        mLightAmmoSegments[i]->SetColor(gpTweakGuiColors->GetLightAmmoTankEmptyUnselectedColor());
        mLightAmmoMeters[i]->SetVisibility(false, kTM_Children);
        mLightAmmoSegments[i]->SetVisibility(false, kTM_Children);
      }
      if (mLightAmmoDigits != nullptr) {
        mLightAmmoDigits->TextSupport().SetFontColor(
            gpTweakGuiColors->GetLightAmmoDepletionColor());
      }
    } else {
      if (lightAmmo > mLightAmmo) {
        mLightAmmoPickupPulse = 0.5f;
      }
      mLightAmmoPickupPulse = rstl::max_val(0.f, mLightAmmoPickupPulse - 0.0166f);
      const float pickup = CMath::FastSinR(M_PIF * (mLightAmmoPickupPulse / 0.5f));
      const CColor flash =
          CColor::Lerp(CColor::Black(), gpTweakGuiColors->GetLightAmmoChangeFlash(), pickup);
      const bool selected =
          beam == CPlayerState::kBI_Light || beam == CPlayerState::kBI_Annihilator;
      const float selection = selected ? beamFactor : 0.f;
      float warning = 0.f;
      if (lightAmmo != 0 &&
          float(lightAmmo) <= float(state.GetItemCapacity(CPlayerState::kIT_LightAmmo)) *
                                  gpTweakGui->GetMissileWarningThreshold()) {
        warning =
            (1.f + CMath::FastCosR(CMath::WrapPi(M_2PIF * CGraphics::GetSecondsMod900() / 1.5f))) *
            0.5f;
      }
      const CColor selectedEmpty =
          CColor::Lerp(gpTweakGuiColors->GetLightAmmoTankEmptySelectedColor(),
                       gpTweakGuiColors->GetLightAmmoEmptyTankWarningColor(), warning);
      const CColor selectedFull =
          CColor::Lerp(gpTweakGuiColors->GetLightAmmoTankFullSelectedColor(),
                       gpTweakGuiColors->GetLightAmmoTankWarningColor(), warning);
      const CColor selectedFill =
          CColor::Lerp(gpTweakGuiColors->GetLightAmmoMeterSelectedFillColor(),
                       gpTweakGuiColors->GetLightAmmoMeterWarningColor(), warning);
      const CColor empty = CColor::Lerp(gpTweakGuiColors->GetLightAmmoTankEmptyUnselectedColor(),
                                        selectedEmpty, selection);
      const CColor full = CColor::Lerp(gpTweakGuiColors->GetLightAmmoTankFullUnselectedColor(),
                                       selectedFull, selection);
      const CColor fill =
          CColor::Add(CColor::Lerp(gpTweakGuiColors->GetLightAmmoMeterUnselectedFillColor(),
                                   selectedFill, selection),
                      flash);
      const CColor shadow =
          CColor::Lerp(gpTweakGuiColors->GetLightAmmoMeterUnselectedShadowColor(),
                       gpTweakGuiColors->GetLightAmmoMeterSelectedShadowColor(), selection);
      const CColor icon =
          CColor::Lerp(gpTweakGuiColors->GetLightAmmoIconUnselectedColor(),
                       gpTweakGuiColors->GetLightAmmoIconSelectedColor(), selection);
      const CColor baseDigits =
          CColor::Lerp(gpTweakGuiColors->GetLightAmmoDigitsUnselectedColor(),
                       gpTweakGuiColors->GetLightAmmoDigitsSelectedColor(), selection);
      const CColor digits =
          CColor::Lerp(baseDigits, gpTweakGuiColors->GetLightAmmoDigitWarningColor(), warning);
      if (mLightAmmoDigits != nullptr) {
        mLightAmmoDigits->TextSupport().SetFontColor(
            lightAmmo == 0 ? gpTweakGuiColors->GetLightAmmoDepletionColor() : digits);
      }
      if (mLightAmmoMeters.size() != 0) {
        const int perTank = CPlayerState::GetPowerUpMaxValue(CPlayerState::kIT_LightAmmo) /
                            mLightAmmoSegments.size();
        const int capacity = state.GetItemCapacity(CPlayerState::kIT_LightAmmo);
        const int filledTanks = lightAmmo / perTank;
        const int activeTanks = filledTanks + 1;
        const int capacityTanks = (capacity + perTank - 1) / perTank;
        for (int i = 0; i < mLightAmmoSegments.size(); ++i) {
          mLightAmmoSegments[i]->SetVisibility(capacity != 0, kTM_Children);
          mLightAmmoMeters[i]->SetVisibility(capacity != 0, kTM_Children);
          mLightAmmoMeters[i]->SetColor(fill);
          mLightAmmoMeters[i]->SetShadowColor(shadow);
          mLightAmmoSegments[i]->SetColor(i < capacityTanks ? full : empty);
          mLightAmmoMeters[i]->SetTargetFraction(i < activeTanks ? 1.f : 0.f);
        }
        if (activeTanks > 0 && activeTanks <= mLightAmmoSegments.size()) {
          mLightAmmoMeters[filledTanks]->SetTargetFraction(
              float(lightAmmo - filledTanks * perTank) / float(perTank));
        }
      }
      char buffer[16];
      sprintf(buffer, "%02d", lightAmmo);
      mLightAmmoDigits->TextSupport().SetText(
          CStringExtras::ConvertToUNICODE(rstl::string_l(buffer)));
      mLightAmmo = lightAmmo;
      if (mLightAmmoIcon != nullptr) {
        mLightAmmoIcon->SetIsVisible(true);
        mLightAmmoIcon->SetColor(lightAmmo < 1 ? empty : full);
      }
    }
  }
  if (mNextState != kHS_Scan) {
    if (mLightAmmoDigits != nullptr) {
      const bool available = state.GetItemAmount(CPlayerState::kIT_LightBeam, true) > 0 ||
                             state.GetItemAmount(CPlayerState::kIT_AnnihilatorBeam, true) > 0;
      mLightAmmoDigits->SetIsVisible(available);
    }
    if (mDarkAmmoDigits != nullptr) {
      const bool available = state.GetItemAmount(CPlayerState::kIT_DarkBeam, true) > 0 ||
                             state.GetItemAmount(CPlayerState::kIT_AnnihilatorBeam, true) > 0;
      mDarkAmmoDigits->SetIsVisible(available);
    }
  }
  mAmmoBeam = beam;
}

void CSamusHud::UpdateBallMode(const CStateManager& mgr) {
  if (mPowerBombDigits == nullptr && mPowerBombIcon == nullptr && mBombIndicators.size() != 3) {
    return;
  }

  const CPlayerState& playerState = *mgr.GetPlayerState(mPlayerIndex);
  const CPlayerGun& gun = *mgr.GetPlayer(mPlayerIndex)->GetPlayerGun();
  const int powerBombs = playerState.GetItemAmount(CPlayerState::kIT_Powerbomb, false);
  const int powerBombCapacity = playerState.GetItemCapacity(CPlayerState::kIT_Powerbomb);
  const int bombsAvailable = gun.GetBombsAvailable(const_cast< CStateManager& >(mgr));
  const bool hasBombs = playerState.HasPowerUp(CPlayerState::kIT_MorphBallBombs);
  const bool powerBombReady =
      !gun.AreBombsDisabled() &&
      mgr.GetPlayer(mPlayerIndex)->GetMorphballTransitionState() == CPlayer::kMS_Morphed;

  if (mPowerBombDigits != nullptr) {
    mPowerBombDigits->SetVisibility(powerBombCapacity > 0, kTM_Children);
    char buffer[16];
    if (mgr.IsMultiplayer()) {
      sprintf(buffer, "%d", powerBombs);
    } else {
      sprintf(buffer, "%d/%d", powerBombs, powerBombCapacity);
    }
    mPowerBombDigits->TextSupport().SetText(rstl::string(buffer), false);
    mPowerBombDigits->TextSupport().SetFontColor(
        powerBombs == 0 ? gpTweakGuiColors->GetMorphBallEmptyPowerBombDigitsForegroundColor()
                        : gpTweakGuiColors->GetMorphBallPowerBombDigitsForegroundColor());
    mPowerBombDigits->TextSupport().SetOutlineColor(
        powerBombs == 0 ? gpTweakGuiColors->GetMorphBallEmptyPowerBombDigitsOutlineColor()
                        : gpTweakGuiColors->GetMorphBallPowerBombDigitsOutlineColor());
  }

  if (mPowerBombIcon != nullptr) {
    mPowerBombIcon->SetVisibility(powerBombCapacity > 0, kTM_Children);
    mPowerBombIcon->SetColor(powerBombReady && powerBombs > 0
                                 ? gpTweakGuiColors->GetMorphBallPowerBombIconColor()
                                 : gpTweakGuiColors->GetMorphBallEmptyPowerBombIconColor());
  }
  if (mPowerBombDecoration != nullptr) {
    mPowerBombDecoration->SetVisibility(powerBombCapacity > 0, kTM_Children);
  }

  for (int i = 0; i < mBombIndicators.size(); ++i) {
    if (mBombIndicators[i] != nullptr) {
      mBombIndicators[i]->SetVisibility(hasBombs, kTM_Children);
      mBombIndicators[i]->SetColor(i < bombsAvailable
                                       ? gpTweakGuiColors->GetMorphBallBombCounterFilledColor()
                                       : gpTweakGuiColors->GetMorphBallBombCounterEmptyColor());
    }
  }
}

void CSamusHud::ResolveLockOnTexture() {
  if (mLockedOnIndicator) {
    mLockedOnIndicator->IsLoaded();
  }
}

void CSamusHud::UpdateThreatAssessment(float dt, const CStateManager& mgr) {
  // TODO: evaluate nearby threats and update the warning gauge.
}

void CSamusHud::fn_8006653c(const CStateManager&, bool) {}

bool CSamusHud::IsCachedLightInAreaLights(const SCachedHudLight& light,
                                          const CActorLights& lights) const {
  const uint count = lights.GetActiveAreaLightCount();
  for (uint i = 0; i < count; ++i) {
    const CLight& areaLight = lights.GetLight(i);
    if (areaLight.GetColor() == light.mColor && areaLight.GetPosition() == light.mPosition) {
      return true;
    }
  }
  return false;
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

int CSamusHud::FindEmptyHudLightSlot(const CLight&) const {
  for (int i = 0; i < 3; ++i) {
    if (mHudLights[i].mFade == 0.f) {
      return i;
    }
  }
  return -1;
}

void CSamusHud::UpdateHudDynamicLights(float dt, const CStateManager& mgr) {
  // TODO: reconcile the three cached lights with area lighting.
}

CColor CSamusHud::GetVisorHudLightColor(const CColor& color, const CStateManager& mgr) const {
  const CPlayerState& state = *mgr.GetPlayerState(mPlayerIndex);
  const CPlayerState::EPlayerVisor visor = state.GetCurrentVisor();
  const float t = state.GetVisorTransitionFactor();
  CColor result = color;
  switch (visor) {
  case CPlayerState::kPV_Scan: {
    const CColor& white = CColor::White();
    const CColor multiplier =
        CColor::Lerp(white, gpTweakGuiColors->GetScanVisorHUDLightMultiply(), t);
    result = CColor::Modulate(result, multiplier);
    break;
  }
  case CPlayerState::kPV_Dark: {
    const CColor multiplier = gpTweakGuiColors->GetDarkVisorHelmetLightModulateColor();
    result = CColor::Modulate(result, multiplier);
    break;
  }
  case CPlayerState::kPV_Echo:
    result = CColor(uint(0));
    break;
  default:
    break;
  }
  return result;
}

void CSamusHud::UpdateHudDamage(float dt, const CStateManager& mgr) {
  // TODO: update the directional damage sectors, filters and shake.
}

void CSamusHud::UpdateStateTransition(float dt, const CStateManager& mgr) {
  // TODO: fade out, load the desired frame, rebind widgets and fade in.
}

void CSamusHud::UpdateHudFrame(float dt, const CStateManager& mgr) {
  UpdateStateTransition(dt, mgr);
  if (mLoadedHudFrame != nullptr) {
    mLoadedHudFrame->Update(dt);
  }
}

void CSamusHud::UpdateBootSequence(float dt, const CStateManager& mgr) {
  // TODO: animate BootText, GearLostText and corrupted HUD text.
}

void CSamusHud::UpdateHudMemo(float dt, const CStateManager& mgr) {
  // TODO: resolve deferred text and update message and A-button fades.
}

void CSamusHud::Update(float dt, const CStateManager& mgr, uint helmetVisibility, bool hudVisible,
                       bool targetingVisible) {
  // TODO: restore update ordering, visibility changes and profiling.
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

rstl::reserved_vector< bool, 4 > CSamusHud::BuildPlayerHasBeams(const CStateManager& mgr) const {
  const CPlayerState& state = *mgr.GetPlayerState(mPlayerIndex);
  rstl::reserved_vector< bool, 4 > result;
  result.push_back(state.HasPowerUp(CPlayerState::kIT_PowerBeam));
  result.push_back(state.HasPowerUp(CPlayerState::kIT_DarkBeam));
  result.push_back(state.HasPowerUp(CPlayerState::kIT_LightBeam));
  result.push_back(state.HasPowerUp(CPlayerState::kIT_AnnihilatorBeam));
  return result;
}

void CSamusHud::DrawLockOnIndicators(const CStateManager& mgr,
                                     const rstl::reserved_vector< TUniqueId, 12 >& targets) const {
  // TODO: draw directional warnings for the supplied targets.
}

void CSamusHud::DrawLockOnIndicators(const CStateManager& mgr) const {
  // TODO: collect multiplayer opponents or the single-player locking actor.
}

void CSamusHud::DrawAttachedEnemyEffect(const CStateManager& mgr) const {
  // TODO: draw the attached-enemy overlay.
}

void CSamusHud::DrawPlayerFilter(const CStateManager& mgr) const {
  // TODO: draw the player-specific color filter.
}

void CSamusHud::EnterFirstPerson(const CStateManager& mgr) {
  CSfxManager::SfxVolume(mStaticSoundLow, 127);
  CSfxManager::SfxVolume(mStaticSoundHigh, 127);
}

void CSamusHud::LeaveFirstPerson(const CStateManager& mgr) {
  CSfxManager::SfxVolume(mStaticSoundLow, 0);
  CSfxManager::SfxVolume(mStaticSoundHigh, 0);
}

void CSamusHud::Draw(const CStateManager& mgr, float alpha, uint helmetVisibility, bool hudVisible,
                     bool targetingVisible) const {
  // TODO: restore the HUD, targeting, overlays and profiling draw sequence.
}

void CSamusHud::DrawHelmet(const CStateManager& mgr, float cameraYOffset) const {
  if (mLoadedHelmetFrame == nullptr || mgr.GetPlayer(mPlayerIndex)->IsInTurret()) {
    return;
  }
  const bool unmorphed =
      mgr.GetPlayer(mPlayerIndex)->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed;
  if (mLoadedHelmetFrame != nullptr && unmorphed && mNextState != kHS_Ball) {
    const float alpha = mPreviousState == kHS_Ball ? mTransitionFactor : 1.f;
    const CGuiWidgetDrawParms parms(alpha * gpGameState->GameOptions().GetHelmetAlpha(),
                                    CVector3f(0.f, 15.f * cameraYOffset, 0.f));
    mLoadedHelmetFrame->Draw(parms);
  }
}

void CSamusHud::DrawHudMemo() const {
  if (mLoadedMemoFrame != nullptr) {
    mLoadedMemoFrame->Draw(CGuiWidgetDrawParms::Default());
  }
}

void CSamusHud::ProcessControllerInput(const CFinalInput& input) {
  if (!mScanInterface.null()) {
    mScanInterface->ProcessControllerInput(input);
  }
}

const CTargetingManager& CSamusHud::GetTargetingManager() const { return mTargetingManager; }

CColor CSamusHud::ModulateColor(const CColor& color) const {
  return CColor::Modulate(mHudColor, color);
}

CSamusHud::EHudState CSamusHud::GetNextState() const {
  return mNextState == kHS_None ? kHS_Combat : mNextState;
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

CRelAngle CSamusHud::GetRelativeDirection(const CVector3f& position,
                                          const CStateManager& mgr) const {
  // TODO: measure the direction in the player's camera plane.
  return CRelAngle::FromRadians(0.f);
}

void CSamusHud::ShowDamage(CVector3f position, float damage, float previousDamage,
                           const CStateManager& mgr) {
  // TODO: set the directional damage sector, sound, filter and shake parameters.
}

void CSamusHud::UpdateHudLag(float dt, const CStateManager& mgr) {
  // TODO: combine camera lag with damage shake.
}

void CSamusHud::ApplyClassicLag(const CUnitVector3f& lookDirection, CQuaternion& rotation,
                                const CStateManager& mgr, float dt, bool invert) {
  // TODO: recover camera lag through shared quaternion helpers.
}

void CSamusHud::SetMessage(const rstl::wstring& text, const CHUDMemoParms& info) {
  if (mMessagePane == nullptr) {
    return;
  }
  mMessageText = text;
  const bool visible = mMessageRoot->GetIsVisible();
  if (!visible || info.IsHintMemo()) {
    if (info.IsFadeOutOnly()) {
      mMessageTime = 1.f;
      if (info.IsHintMemo() && visible) {
        CSfxManager::SfxStart(0x12ac, 127, 64, CSfxManager::kAllAreas, false, false,
                              CSfxManager::kMedPriority);
      }
      return;
    }
    mMessageRoot->SetColor(CColor::White());
    mMessageRoot->SetVisibility(false, kTM_Children);
    CGuiWidget* pane = info.IsHintMemo() ? mMessageRoot : mMessagePane;
    if (!info.IsClearMemoWindow() || info.GetDisplayTime() != 0.f || mMessageTime != 0.f ||
        text.size() != 0) {
      pane->SetVisibility(true, kTM_Children);
    }
    mMessagePane->TextSupport().SetTypeWriteEffectOptions(info.GetFadeInText(), 0.1f, 40.f);
    if (info.IsClearMemoWindow()) {
      mLastMessageSoundChars = 0.f;
      mMessagePane->TextSupport().SetCurTime(0.f);
      mMessagePane->TextSupport().SetText(text);
    } else if (mMessagePane->TextSupport().GetText().size() == 0) {
      mLastMessageSoundChars = 0.f;
      mMessagePane->TextSupport().AddText(text);
    } else {
      mMessagePane->TextSupport().AddText(rstl::wstring_l(L"\n") + text);
    }
    mMessagePane->SetColor(CColor::White());
    mMessageRoot->SetColor(CColor::White());
    mMessageTime = info.GetDisplayTime();
    if (info.IsHintMemo()) {
      if (!visible) {
        mAButtonPulse = 0.f;
        CSfxManager::SfxStart(0x1286, 127, 64, CSfxManager::kAllAreas, false, false,
                              CSfxManager::kMedPriority);
      }
    } else {
      mMessageRoot->SetO2PTransform(mMessageRoot->GetIdleXform());
    }
  }
}

void CSamusHud::StopSounds(const CStateManager&) {
  StopSound(mDamageSound);
  StopSound(mStaticSoundLow);
  StopSound(mStaticSoundHigh);
  StopSound(mFreeLookSound);
}

void CSamusHud::PrepareScanDisplay(const CStateManager& mgr, int playerIndex) {
  if (!mScanInterface.null()) {
    mScanInterface->PrepareScanDisplay(mgr, playerIndex);
  }
}

void CSamusHud::UpdateBossLockOnWarning(float dt, const CStateManager& mgr) {
  // TODO: load and animate the boss-lock warning frame.
}

void CSamusHud::DrawBossLockOnWarning() const {
  if (mBossLockOnFrame.get() != nullptr) {
    mBossLockOnFrame->Draw(CGuiWidgetDrawParms::Default());
  }
}
