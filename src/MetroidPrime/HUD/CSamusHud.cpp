#include "MetroidPrime/HUD/CSamusHud.hpp"
#include "Collision/CollisionUtil.hpp"
#include "GuiSys/CAuiBitmapMeter.hpp"
#include "GuiSys/CAuiEnergyBarT01.hpp"
#include "GuiSys/CGuiCamera.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiFrameLoader.hpp"
#include "GuiSys/CGuiHeadWidget.hpp"
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
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CSphere.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CCinematicCamera.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerCameraBob.hpp"
#include "MetroidPrime/Player/CPlayerGun.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDynamicLight.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakGuiColors.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "rstl/StringExtras.hpp"

#include <stdio.h>
#include <stdlib.h>

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

static const char* const sBossLockOnRings[] = {"model_ring", "model_ring1", "model_ring2"};

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
  const float angle = 0.5294118f * t + -0.20262942f;
  const float x = 17.f * CMath::FastSinR(angle);
  const float y = 0.2f + (17.f * CMath::FastCosR(angle) + -17.f);
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
    const CPlayerGun& gun = *player.mGun;
    CPlayerState::EBeamId beam;
    if (player.GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
      beam = playerState.GetCurrentBeam();
    } else {
      beam = gun.GetPrimaryWeaponId();
    }
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
    if (info.EnabledForPlayer(i) && gpSamusHud[i] != nullptr) {
      gpSamusHud[i]->InternalDisplayHudMemo(text, info);
    }
  }
}

void CSamusHud::DeferHintMemo(CAssetId stringTable, uint index, const CHUDMemoParms& info) {
  for (int i = 0; i < 4; ++i) {
    if (info.EnabledForPlayer(i) && gpSamusHud[i] != nullptr) {
      gpSamusHud[i]->InternalDeferHintMemo(stringTable, index, info);
    }
  }
}

bool CSamusHud::IsHudMemoVisible(int playerIndex) {
  if (gpSamusHud[playerIndex] == nullptr) {
    return false;
  }
  if (gpSamusHud[playerIndex]->mMessageRoot == nullptr ||
      gpSamusHud[playerIndex]->mMessagePane == nullptr) {
    return false;
  }
  return gpSamusHud[playerIndex]->mMessageRoot->GetIsVisible() ||
         gpSamusHud[playerIndex]->mMessagePane->GetIsVisible();
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
, mDamageSectorRemaining(12, 0.f)
, mDamageSectorDurations(12, 0.f)
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
, mThreatAnimationTime(1.f)
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
  if (gpSamusHud[playerIndex] != nullptr) {
    const rstl::reserved_vector< bool, 4 > enables =
        gpSamusHud[playerIndex]->BuildPlayerHasBeams(mgr);
    CHudVisorBeamMenu* menu = gpSamusHud[playerIndex]->mBeamMenu.get();
    const CPlayerState::EBeamId beam = mgr.GetPlayerState(playerIndex)->GetCurrentBeam();
    if (menu != nullptr) {
      menu->SetPlayerHas(enables, beam);
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
          *mgr.GetCameraManager(mPlayerIndex)->GetCurrentCamera(mgr, true)) != nullptr;
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
    mEnergyWarning->TextSupport().SetFontColor(
        gpTweakGuiColors->GetEnergyWarningColor().WithAlphaOf(mEnergyLowPulse * mEnergyLowFade));
    mEnergyWarning->TextSupport().SetOutlineColor(
        gpTweakGuiColors->GetEnergyWarningOutlineColor().WithAlphaOf(mEnergyLowPulse *
                                                                     mEnergyLowFade));
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
    if (mTargetingManager.CheckLoadComplete()) {
      mLoadPhase = kLP_Frames;
      InitializeFrameGlueMutable(mgr);
      UpdateEnergy(0.f, mgr, true);
      UpdateMissile(0.f, mgr, true);
      UpdateBeamAmmo(mgr, true);
      UpdateBallMode(mgr, true);
      fn_8006653c(mgr, true);
      ResolveLockOnTexture();
    } else {
      return false;
    }
    // Fall through.
  case kLP_Frames:
    if (mLoadedHudFrame->GetIsFinishedLoading() &&
        (mLoadedHelmetFrame == nullptr || mLoadedHelmetFrame->GetIsFinishedLoading())) {
      mLoadPhase = kLP_Complete;
    } else {
      return false;
    }
    // Fall through.
  case kLP_Complete:
    return true;
  default:
    return false;
  }
}

void CSamusHud::UpdateVisorAndBeamMenus(float dt, const CStateManager& mgr) {
  const CPlayerState& state = *mgr.GetPlayerState(mPlayerIndex);
  const CPlayer& player = *mgr.GetPlayer(mPlayerIndex);
  const CPlayerGun& gun = *player.mGun;
  if (player.GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
    const CPlayerState::EBeamId currentBeam = state.GetCurrentBeam();
    if (currentBeam != mMenuBeam) {
      mBallBeamTransition = 0.6f - mBallBeamTransition;
      mPreviousBallBeam = mMenuBeam;
    }
    mBallBeamTransition = rstl::max_val(mBallBeamTransition - dt, 0.f);
    const float transition = (2.f * mBallBeamTransition - 0.6f) / 0.6f;
    const CPlayerState::EBeamId pending = transition > 0.f ? mPreviousBallBeam : currentBeam;
    const CPlayerState::EBeamId selected = transition > 0.f ? mPreviousBallBeam : currentBeam;
    mBeamMenuTransition = CMath::Clamp(0.f, CMath::AbsF(transition), 1.f);
    if (mBeamMenu.get() != nullptr) {
      mBeamMenu->SetSelection(selected, pending, mBeamMenuTransition);
      mBeamMenu->SetPlayerHas(BuildPlayerHasBeams(mgr));
      mMenuBeam = currentBeam;
    }
  } else {
    const CPlayerState::EBeamId currentBeam = gun.GetPrimaryWeaponId();
    const CPlayerState::EBeamId nextBeam = gun.GetPrimaryDestWeaponId();
    mBeamMenuTransition = CMath::Clamp(0.f, gun.GetHoloTransitionFactor(), 1.f);
    const CPlayerState::EPlayerVisor visor = state.GetCurrentVisor();
    const CPlayerState::EPlayerVisor nextVisor = state.GetTransitioningVisor();
    const float visorInterp = state.GetVisorTransitionFactor();
    if (mBeamMenu.get() != nullptr) {
      mBeamMenu->SetSelection(currentBeam, nextBeam, mBeamMenuTransition);
      mBeamMenu->SetPlayerHas(BuildPlayerHasBeams(mgr));
      mMenuBeam = currentBeam;
    }
    if (mVisorMenu.get() != nullptr) {
      mVisorMenu->SetSelection(visor, nextVisor, visorInterp);
      mVisorMenu->SetPlayerHas(BuildPlayerHasVisors(mgr));
    }
  }
}

void CSamusHud::UpdateFreeLook(float dt, const CStateManager& mgr) {
  const CPlayer& player = *mgr.GetPlayer(mPlayerIndex);
  const CGameCamera* const camera = CCameraManager::CastGameCameratoFirstPersonCamera(
      player.GetCameraManager()->GetCurrentCamera(mgr, true));
  const bool inFreeLook = player.IsInFreeLook() && camera != nullptr &&
                          player.GetPlayerScanState() == CPlayer::kSS_NotScanning;
  const bool lookHeld = player.GetFreeLookStickState();
  if (mInFreeLook != inFreeLook) {
    if (inFreeLook) {
      CSfxManager::SfxStart(0x1b3, 127, player.GetSoundPan(CPlayer::kMSP_4), CSfxManager::kAllAreas,
                            false, false, CSfxManager::kMedPriority);
    } else {
      CSfxManager::SfxStart(0x1b2, 127, player.GetSoundPan(CPlayer::kMSP_4), CSfxManager::kAllAreas,
                            false, false, CSfxManager::kMedPriority);
    }
    mInFreeLook = inFreeLook;
  }
  const float threshold = 1.f - 60.f * (0.00001001358f * dt);
  const float oldDot = mFreeLookDirectionDot;
  const CVector3f direction =
      camera == nullptr ? mPreviousFreeLookDirection : camera->GetTransform().GetForward();
  mFreeLookDirectionDot =
      inFreeLook && lookHeld
          ? CMath::Limit(CVector3f::Dot(direction, mPreviousFreeLookDirection), 1.f)
          : 1.f;
  mPreviousFreeLookDirection = direction;
  const bool crossed = (oldDot >= threshold && mFreeLookDirectionDot < threshold) ||
                       (oldDot < threshold && mFreeLookDirectionDot >= threshold);
  if (inFreeLook) {
    mFreeLookFade = rstl::min_val(0.5f, mFreeLookFade + dt);
  } else {
    mFreeLookFade = rstl::max_val(0.f, mFreeLookFade - dt);
  }
  if (!close_enough(mFreeLookFade, 0.f)) {
    const CVector3f scale(0.5f / mFreeLookFade, 0.5f / mFreeLookFade, 0.5f / mFreeLookFade);
    if (mFreeLookLeft != nullptr) {
      mFreeLookLeft->SetO2WTransform(mFreeLookLeftTransform * CTransform4f::Scale(scale));
      mFreeLookLeft->SetIsVisible(true);
    }
    if (mFreeLookRight != nullptr) {
      mFreeLookRight->SetO2WTransform(mFreeLookRightTransform * CTransform4f::Scale(scale));
      mFreeLookRight->SetIsVisible(true);
    }
  } else {
    if (mFreeLookLeft != nullptr) {
      mFreeLookLeft->SetIsVisible(false);
    }
    if (mFreeLookRight != nullptr) {
      mFreeLookRight->SetIsVisible(false);
    }
  }
  if (crossed) {
    mFreeLookSoundCycle = 0.f;
  } else if (mFreeLookSoundCycle < 0.05f) {
    mFreeLookSoundCycle = rstl::min_val(0.05f, mFreeLookSoundCycle + dt);
    if (mFreeLookSoundCycle == 0.05f) {
      if (mFreeLookDirectionDot < threshold) {
        if (!mFreeLookSound) {
          mFreeLookSound =
              CSfxManager::SfxStart(0x19b, 127, player.GetSoundPan(CPlayer::kMSP_4),
                                    CSfxManager::kAllAreas, true, true, CSfxManager::kMedPriority);
        }
      } else {
        CSfxManager::SfxStop(mFreeLookSound);
        mFreeLookSound.Clear();
      }
    }
  }
  if (camera != nullptr) {
    const CMatrix3f cameraRotation = camera->GetTransform().BuildMatrix3f();
    const CUnitVector3f cameraDirection(cameraRotation.GetColumn(kDY));
    CVector3f horizonDirection(cameraDirection.GetX(), cameraDirection.GetY(), 0.f);
    horizonDirection.Normalize();
    const float dot = CMath::Limit(CVector3f::Dot(cameraDirection, horizonDirection), 1.f);
    float angle = CMath::AbsF(acosf(dot));
    if (cameraDirection.GetZ() < 0.f) {
      angle = -angle;
    }
    if (mFreeLookSound) {
      float pitch = angle * gpTweakGui->GetFreeLookSfxPitchScale() / (M_PIF / 2.f);
      if (!gpTweakGui->GetNoAbsoluteFreeLookSfxPitch()) {
        pitch = CMath::AbsF(pitch);
      }
      CSfxManager::PitchBend(mFreeLookSound, int(8192.f + pitch));
    }
  }
}

void CSamusHud::UpdateStaticInterference(float dt, const CStateManager& mgr) {
  float interference =
      mgr.GetPlayerState(mPlayerIndex)->StaticInterference().GetTotalInterference();
  const float oldInterference = mStaticInterference;
  if (mgr.IsMultiplayer() &&
      mgr.GetPlayerState(mPlayerIndex)->GetItemCapacity(CPlayerState::kIT_HackedEffect) > 0) {
    interference += 0.2f;
  }
  if (mgr.GetPlayer(mPlayerIndex)->GetMorphballTransitionState() != CPlayer::kMS_Unmorphed) {
    interference = 0.f;
  }
  if (mStaticInterference < interference) {
    mStaticInterference = rstl::min_val(interference, mStaticInterference + dt);
  } else if (mStaticInterference > interference) {
    mStaticInterference = rstl::max_val(interference, mStaticInterference - dt);
  }
  UpdateStaticSfx(mgr, mStaticSoundLow, mStaticCycleLow,
                  mgr.ReturnFirstIfSingleElseSecond(0x275, 0x265c), dt, oldInterference, 0.1f);
  UpdateStaticSfx(mgr, mStaticSoundHigh, mStaticCycleHigh,
                  mgr.ReturnFirstIfSingleElseSecond(0x275, 0x265d), dt, oldInterference, 0.5f);
  if (mStaticInterference > 0.f) {
    mStaticFilter.SetFilter(CCameraFilterPass::kFT_Blend, CCameraFilterPass::kFS_RandomStatic, 0.f,
                            CColor::White().WithAlphaOf(mStaticInterference), kInvalidAssetId);
  } else {
    mStaticFilter.DisableFilter(0.f);
  }
}

void CSamusHud::UpdateStaticSfx(const CStateManager& mgr, CSfxHandle& sound, float& cycle,
                                const ushort soundId, float dt, float previousInterference,
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
  const CPlayer& player = *mgr.GetPlayer(mPlayerIndex);
  const CPlayerState& state = *mgr.GetPlayerState(mPlayerIndex);
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
      if (mFilledEnergyTanks[i] != nullptr && mEmptyEnergyTanks[i] != nullptr) {
        if (i < numEnergyTanks) {
          const bool full = i < filledTanks;
          mFilledEnergyTanks[i]->SetVisibility(full, kTM_Children);
          mEmptyEnergyTanks[i]->SetVisibility(!full, kTM_Children);
        } else {
          mFilledEnergyTanks[i]->SetVisibility(false, kTM_Children);
          mEmptyEnergyTanks[i]->SetVisibility(false, kTM_Children);
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
      const CColor red(1.f, 0.f, 0.f, 1.f);
      damageColor = CColor::Lerp(damageColor, red, mEnergyLowTimer);
    }
    mEnergyBar->SetFilledColor(damageColor);
    mEnergyBar->SetShadowColor(finalShadow);
    mEnergyBar->SetEmptyColor(finalEmpty);
    if (mEnergyDigits != nullptr) {
      mEnergyDigits->SetColor(damageColor);
    }
  }
  if (mBossEnergy.get() != nullptr) {
    const TUniqueId bossId = mgr.GetBossId();
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(bossId))) {
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
  const CPlayerState& state = *mgr.GetPlayerState(mPlayerIndex);
  const CPlayerGun& gun = *mgr.GetPlayer(mPlayerIndex)->mGun;
  const int enabled = gun.GetMissileMode() ? 0 : 1;
  const int missiles = state.GetItemAmount(CPlayerState::kIT_Missile, true);
  const int capacity = state.GetItemCapacity(CPlayerState::kIT_Missile);
  if (init || missiles != mMissileAmount || enabled != mMissileEnabled ||
      capacity != mMissileCapacity || !close_enough(mMissilePickupPulse, 0.f) ||
      !close_enough(mMissileModeTransition, 0.f)) {
    if (GetNextState() != kHS_Scan) {
      if (missiles > mMissileAmount) {
        mMissilePickupPulse = 0.5f;
      }
      mMissilePickupPulse = rstl::max_val(mMissilePickupPulse - dt, 0.f);
      const float pickup = CMath::FastSinR(M_PIF * (mMissilePickupPulse / 0.5f));
      const CColor flash =
          CColor::Lerp(CColor::Black(), gpTweakGuiColors->GetMissileGroupChangeFlash(), pickup);
      mMissileModeTransition = rstl::max_val(mMissileModeTransition - 3.f * dt, 0.f);
      if (mMissileEnabled != enabled) {
        mMissileModeTransition = 1.f;
      }
      const float transition =
          gun.GetMissileMode() ? mMissileModeTransition : 1.f - mMissileModeTransition;
      const CColor& active =
          CColor::Add(ModulateColor(gpTweakGuiColors->GetMissileGroupActiveColor()), flash);
      const CColor& inactive =
          CColor::Add(ModulateColor(gpTweakGuiColors->GetMissileGroupInactiveColor()), flash);
      const CColor& depletion = gpTweakGuiColors->GetMissileDepletionColor();
      const CColor iconColor =
          missiles == 0 ? depletion : CColor::Lerp(active, inactive, transition);
      const bool visible = mgr.IsMultiplayer() ? missiles != 0 : capacity != 0;
      if (mMissileIcon != nullptr) {
        mMissileIcon->SetColor(iconColor);
        mMissileIcon->SetVisibility(visible, kTM_Children);
      }
      const CColor& activeText =
          CColor::Add(ModulateColor(gpTweakGuiColors->GetActiveTextForegroundColor()), flash);
      const CColor& inactiveText =
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
        (1.f + CMath::FastCosR(CMath::WrapPi(M_2PIF * CGraphics::GetSecondsMod900() / 1.5f))) / 2.f;
    if (mMissileIcon != nullptr) {
      const CColor& activeColor = ModulateColor(gpTweakGuiColors->GetMissileGroupActiveColor());
      const CColor& inactiveColor = ModulateColor(gpTweakGuiColors->GetMissileGroupInactiveColor());
      const CColor base = CColor::Lerp(activeColor, inactiveColor, transition);
      const CColor color = CColor::Lerp(base, gpTweakGuiColors->GetMissileWarningColor(), pulse);
      mMissileIcon->SetColor(color);
    }
    if (mMissileDigits != nullptr) {
      const CColor& activeColor = ModulateColor(gpTweakGuiColors->GetActiveTextForegroundColor());
      const CColor& inactiveColor =
          ModulateColor(gpTweakGuiColors->GetInactiveTextForegroundColor());
      const CColor base = CColor::Lerp(activeColor, inactiveColor, transition);
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
  const CPlayerGun& gun = *player.mGun;
  CPlayerState::EBeamId beam;
  if (player.GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
    beam = mBallBeamTransition > 0.3f ? mPreviousBallBeam : state.GetCurrentBeam();
  } else {
    beam = gun.GetPrimaryWeaponId();
  }
  const float beamFactor = CMath::Clamp(0.f, mBeamMenuTransition, 1.f);
  const int darkAmmo = state.GetItemAmount(CPlayerState::kIT_DarkAmmo, true);
  const int lightAmmo = state.GetItemAmount(CPlayerState::kIT_LightAmmo, true);
  if (init || mDarkAmmo != darkAmmo || beam != mAmmoBeam ||
      float(darkAmmo) <= float(state.GetItemCapacity(CPlayerState::kIT_DarkAmmo)) *
                             gpTweakGui->GetMissileWarningThreshold() ||
      !close_enough(mDarkAmmoPickupPulse, 0.f) ||
      (!close_enough(beamFactor, 0.f) && !close_enough(beamFactor, 1.f))) {
    if (state.GetItemAmount(CPlayerState::kIT_DarkBeam, true) > 0 ||
        state.GetItemAmount(CPlayerState::kIT_AnnihilatorBeam, true) > 0) {
      if (darkAmmo > mDarkAmmo) {
        mDarkAmmoPickupPulse = 0.5f;
      }
      mDarkAmmoPickupPulse = rstl::max_val(mDarkAmmoPickupPulse - 0.0166f, 0.f);
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
            (1.f + CMath::FastCosR(CMath::WrapPi(M_2PIF * CGraphics::GetSecondsMod900() / 1.5f))) /
            2.f;
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
            darkAmmo != 0 ? digits : gpTweakGuiColors->GetDarkAmmoDepletionColor());
      }
      if (mDarkAmmoSegments.size() != 0) {
        const int perTank =
            CPlayerState::GetPowerUpMaxValue(CPlayerState::kIT_DarkAmmo) / mDarkAmmoSegments.size();
        const int capacity = state.GetItemCapacity(CPlayerState::kIT_DarkAmmo);
        const int capacityTanks = (capacity + perTank - 1) / perTank;
        const int activeTanks = darkAmmo / perTank + 1;
        const int remainder = darkAmmo % perTank;
        for (int i = 0; i < mDarkAmmoSegments.size(); ++i) {
          mDarkAmmoSegments[i]->SetVisibility(capacity != 0, kTM_Children);
          mDarkAmmoMeters[i]->SetVisibility(capacity != 0, kTM_Children);
          mDarkAmmoMeters[i]->SetColor(fill);
          mDarkAmmoMeters[i]->SetShadowColor(shadow);
          if (i < capacityTanks) {
            mDarkAmmoSegments[i]->SetColor(full);
          } else {
            mDarkAmmoSegments[i]->SetColor(empty);
          }
          if (i < activeTanks) {
            mDarkAmmoMeters[i]->SetTargetFraction(1.f);
          } else {
            mDarkAmmoMeters[i]->SetTargetFraction(0.f);
          }
        }
        if (activeTanks > 0 && activeTanks <= mDarkAmmoSegments.size()) {
          mDarkAmmoMeters[activeTanks - 1]->SetTargetFraction(float(remainder) / float(perTank));
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
        if (darkAmmo > 0) {
          mDarkAmmoIcon->SetColor(full);
        } else {
          mDarkAmmoIcon->SetColor(empty);
        }
      }
    } else {
      if (mDarkAmmoIcon != nullptr) {
        mDarkAmmoIcon->SetIsVisible(false);
      }
      for (int i = 0; i < mDarkAmmoSegments.size(); ++i) {
        mDarkAmmoSegments[i]->SetColor(gpTweakGuiColors->GetDarkAmmoTankEmptyUnselectedColor());
        mDarkAmmoMeters[i]->SetVisibility(false, kTM_Children);
        mDarkAmmoSegments[i]->SetVisibility(false, kTM_Children);
      }
    }
  }
  if (init || mLightAmmo != lightAmmo || beam != mAmmoBeam ||
      float(lightAmmo) <= float(state.GetItemCapacity(CPlayerState::kIT_LightAmmo)) *
                              gpTweakGui->GetMissileWarningThreshold() ||
      !close_enough(mLightAmmoPickupPulse, 0.f) ||
      (!close_enough(beamFactor, 0.f) && !close_enough(beamFactor, 1.f))) {
    if (state.GetItemAmount(CPlayerState::kIT_LightBeam, true) > 0 ||
        state.GetItemAmount(CPlayerState::kIT_AnnihilatorBeam, true) > 0) {
      if (lightAmmo > mLightAmmo) {
        mLightAmmoPickupPulse = 0.5f;
      }
      mLightAmmoPickupPulse = rstl::max_val(mLightAmmoPickupPulse - 0.0166f, 0.f);
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
            (1.f + CMath::FastCosR(CMath::WrapPi(M_2PIF * CGraphics::GetSecondsMod900() / 1.5f))) /
            2.f;
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
            lightAmmo != 0 ? digits : gpTweakGuiColors->GetLightAmmoDepletionColor());
      }
      if (mLightAmmoMeters.size() != 0) {
        const int perTank = CPlayerState::GetPowerUpMaxValue(CPlayerState::kIT_LightAmmo) /
                            mLightAmmoSegments.size();
        const int capacity = state.GetItemCapacity(CPlayerState::kIT_LightAmmo);
        const int capacityTanks = (capacity + perTank - 1) / perTank;
        const int activeTanks = lightAmmo / perTank + 1;
        const int remainder = lightAmmo % perTank;
        for (int i = 0; i < mLightAmmoSegments.size(); ++i) {
          mLightAmmoSegments[i]->SetVisibility(capacity != 0, kTM_Children);
          mLightAmmoMeters[i]->SetVisibility(capacity != 0, kTM_Children);
          mLightAmmoMeters[i]->SetColor(fill);
          mLightAmmoMeters[i]->SetShadowColor(shadow);
          if (i < capacityTanks) {
            mLightAmmoSegments[i]->SetColor(full);
          } else {
            mLightAmmoSegments[i]->SetColor(empty);
          }
          if (i < activeTanks) {
            mLightAmmoMeters[i]->SetTargetFraction(1.f);
          } else {
            mLightAmmoMeters[i]->SetTargetFraction(0.f);
          }
        }
        if (activeTanks > 0 && activeTanks <= mLightAmmoSegments.size()) {
          mLightAmmoMeters[activeTanks - 1]->SetTargetFraction(float(remainder) / float(perTank));
        }
      }
      char buffer[16];
      sprintf(buffer, "%02d", lightAmmo);
      mLightAmmoDigits->TextSupport().SetText(
          CStringExtras::ConvertToUNICODE(rstl::string_l(buffer)));
      mLightAmmo = lightAmmo;
      if (mLightAmmoIcon != nullptr) {
        mLightAmmoIcon->SetIsVisible(true);
        if (lightAmmo > 0) {
          mLightAmmoIcon->SetColor(full);
        } else {
          mLightAmmoIcon->SetColor(empty);
        }
      }
    } else {
      if (mLightAmmoIcon != nullptr) {
        mLightAmmoIcon->SetIsVisible(false);
      }
      for (int i = 0; i < mLightAmmoSegments.size(); ++i) {
        mLightAmmoSegments[i]->SetColor(gpTweakGuiColors->GetLightAmmoTankEmptyUnselectedColor());
        mLightAmmoMeters[i]->SetVisibility(false, kTM_Children);
        mLightAmmoSegments[i]->SetVisibility(false, kTM_Children);
      }
      if (mLightAmmoDigits != nullptr) {
        mLightAmmoDigits->TextSupport().SetFontColor(gpTweakGuiColors->GetMissileDepletionColor());
      }
    }
  }
  if (mNextState != kHS_Scan) {
    if (mLightAmmoDigits != nullptr) {
      mLightAmmoDigits->SetIsVisible(state.GetItemAmount(CPlayerState::kIT_LightBeam, true) > 0 ||
                                     state.GetItemAmount(CPlayerState::kIT_AnnihilatorBeam, true) >
                                         0);
    }
    if (mDarkAmmoDigits != nullptr) {
      mDarkAmmoDigits->SetIsVisible(state.GetItemAmount(CPlayerState::kIT_DarkBeam, true) > 0 ||
                                    state.GetItemAmount(CPlayerState::kIT_AnnihilatorBeam, true) >
                                        0);
    }
  }
  mAmmoBeam = beam;
}

void CSamusHud::UpdateBallMode(const CStateManager& mgr, bool) {
  if (mPowerBombDigits == nullptr && mPowerBombIcon == nullptr && mBombIndicators.size() != 3) {
    return;
  }

  const CPlayerState& playerState = *mgr.GetPlayerState(mPlayerIndex);
  const CPlayerGun& gun = *mgr.GetPlayer(mPlayerIndex)->mGun;
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
    mLockedOnIndicator->TryCache();
  }
}

void CSamusHud::UpdateThreatAssessment(float dt, const CStateManager& mgr) {
  if (mgr.GetNumPlayers() > 1) {
    return;
  }
  const CPlayer& player = *mgr.GetPlayer(mPlayerIndex);
  const CVector3f position = player.GetTranslation();
  CAABox playerBounds = CAABox::MakeNullBox();
  const rstl::optional_object< CAABox > playerTouch = player.GetTouchBounds();
  if (playerTouch.valid()) {
    playerBounds = *playerTouch;
  }
  CAABox bounds = CAABox::MakeMaxInvertedBox();
  const float range = gpTweakGui->GetThreatWorldRadius();
  bounds.AccumulateBounds(position + CVector3f(-range, -range, -range));
  bounds.AccumulateBounds(position + CVector3f(range, range, range));
  const CObjectList& triggers = mgr.GetObjectListById(kOL_Trigger);
  float threatDistance = 9999.f;
  for (int i = triggers.GetFirstObjectIndex(); i != -1; i = triggers.GetNextObjectIndex(i)) {
    const CScriptTrigger* trigger = static_cast< const CScriptTrigger* >(triggers[i]);
    if (trigger->GetActive() &&
        (trigger->GetTriggerFlags() &
         (kTFL_DetectMorphedPlayer | kTFL_DetectUnmorphedPlayer | kTFL_DetectScrewAttack)) != 0) {
      const rstl::optional_object< CAABox > touch = trigger->GetTouchBounds();
      if (!(touch.valid() && touch->DoBoundsOverlap(bounds))) {
        continue;
      }
      const CDamageVulnerability* vulnerability =
          player.GetDamageVulnerability(CVector3f::Zero(), CVector3f::Up(),
                                        CDamageInfo(CWeaponMode(kWT_Power), 0.f, 0.f, 0.f, true));
      if (trigger->GetDamageInfo().GetDamage(*vulnerability) == 0.f) {
        continue;
      }
      if (touch.valid()) {
        const CAABox triggerBounds = *touch;
        const float distance = CAABox::DistanceBetween(playerBounds, triggerBounds);
        if (distance < threatDistance) {
          threatDistance = distance;
        }
      }
    }
  }
  if (player.WasDamaged() && player.GetDamageWeaponType() == kWT_Dark) {
    threatDistance = 0.f;
  }
  const float exposure = player.GetDarkWorldDamageExposureFraction();
  float environmentThreat = mgr.GetIsDarkWorld() && exposure > 0.08f ? exposure : 0.f;
  if (mThreatGauge != nullptr) {
    if (!close_enough(environmentThreat, 0.f) || threatDistance < range) {
      mThreatAmount = rstl::max_val(environmentThreat, 1.f - threatDistance / range);
      if (mThreatIcon != nullptr) {
        mThreatIcon->SetVisibility(true, kTM_Children);
      }
      mThreatGauge->SetVisibility(true, kTM_Children);
    } else {
      mThreatAmount = 0.f;
    }
    mThreatGauge->SetTargetFraction(mThreatAmount);
  }
  const CColor active = ModulateColor(gpTweakGuiColors->GetThreatGroupActiveColor());
  const float amount = mThreatGauge != nullptr ? rstl::max_val(mThreatGauge->GetShadowFraction(),
                                                               mThreatGauge->GetCurrentFraction())
                                               : mThreatAmount;
  const CColor iconColor =
      CColor::Lerp(active, gpTweakGuiColors->GetThreatGroupDamageColor(), amount);
  const CColor gaugeColor =
      CColor::Lerp(active, gpTweakGuiColors->GetThreatBarFilledColor(), amount);
  if (!close_enough(environmentThreat, 0.f) || threatDistance <= range ||
      !close_enough(amount, 0.f)) {
    if (!close_enough(environmentThreat, 0.f) || threatDistance <= range) {
      mThreatAnimationTime += 3.f * dt;
    }
    const float animAlpha = rstl::min_val(1.f, mThreatAnimationTime);
    const float pulse =
        amount < 1.f ? 0.f : (1.f - CMath::FastCosR(3.f * mThreatAnimationTime)) / 2.f;
    const CColor warning =
        CColor::Lerp(iconColor, gpTweakGuiColors->GetThreatWarningColor(), pulse);
    const float alpha =
        animAlpha * (!mScanInterface.null() ? mScanInterface->GetMessageTextAlpha() : 1.f);
    if (mThreatRoot != nullptr) {
      mThreatRoot->SetVisibility(true, kTM_Children);
    }
    if (mThreatIcon != nullptr) {
      mThreatIcon->SetColor(warning.WithAlphaModulatedBy(alpha));
    }
    if (mThreatGauge != nullptr) {
      mThreatGauge->SetColor(gaugeColor.WithAlphaModulatedBy(alpha));
    }
    if (mThreatBar != nullptr) {
      mThreatBar->SetColor(
          ModulateColor(gpTweakGuiColors->GetHUDDecorativeColor()).WithAlphaModulatedBy(alpha));
    }
  } else {
    if (mNextState == kHS_Scan) {
      mThreatAnimationTime = CMath::Clamp(0.f, mThreatAnimationTime - 3.f * dt, 1.f);
    } else {
      mThreatAnimationTime += 3.f * dt;
    }
    const float alpha = rstl::min_val(1.f, mThreatAnimationTime);
    if (mThreatRoot != nullptr) {
      if (close_enough(alpha, 0.f)) {
        mThreatRoot->SetVisibility(false, kTM_Children);
      } else {
        mThreatRoot->SetVisibility(true, kTM_Children);
      }
    }
    if (mThreatIcon != nullptr) {
      mThreatIcon->SetColor(ModulateColor(gpTweakGuiColors->GetThreatGroupInactiveColor())
                                .WithAlphaModulatedBy(alpha));
    }
    if (mThreatGauge != nullptr) {
      mThreatGauge->SetColor(ModulateColor(gpTweakGuiColors->GetThreatGroupInactiveColor())
                                 .WithAlphaModulatedBy(alpha));
    }
    if (mThreatBar != nullptr) {
      mThreatBar->SetColor(
          ModulateColor(gpTweakGuiColors->GetHUDDecorativeColor()).WithAlphaModulatedBy(alpha));
    }
  }
}

void CSamusHud::fn_8006653c(const CStateManager&, bool) {}

bool CSamusHud::IsCachedLightInAreaLights(const SCachedHudLight& light,
                                          const CActorLights& lights) const {
  const CColor color = light.mColor;
  const uint count = lights.GetActiveAreaLightCount();
  for (uint i = 0; i < count; ++i) {
    const CLight& areaLight = lights.GetLight(i);
    if (areaLight.GetColor() == color && areaLight.GetPosition() == light.mPosition) {
      return true;
    }
  }
  return false;
}

bool CSamusHud::IsAreaLightInCachedLights(const CLight& light) const {
  for (int i = 0; i < 3; ++i) {
    const SCachedHudLight& cached = mHudLights[i];
    if (cached.mFade != 0.f && light.GetColor() == cached.mColor &&
        light.GetPosition() == cached.mPosition) {
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
  if (mgr.GetViewportLayoutIndex() != 0) {
    return;
  }
  const CGameCamera* const camera = CCameraManager::CastGameCameratoFirstPersonCamera(
      mgr.GetCameraManager(mPlayerIndex)->GetCurrentCamera(mgr, true));
  if (camera == nullptr) {
    return;
  }
  const CVector3f position = camera->GetTranslation();
  const CVector3f lookDirection = camera->GetTransform().GetForward();
  const CAABox bounds(position - CVector3f(0.125f, 0.125f, 0.125f),
                      position + CVector3f(0.125f, 0.125f, 0.125f));
  CActorLights& lights = *mLights;
  const TAreaId area = mgr.GetPlayer(mPlayerIndex)->GetCurrentAreaId();
  if (area == kInvalidAreaId || mgr.GetPendingDockArea() != kInvalidAreaId) {
    return;
  }
  lights.BuildAreaLightList(mgr, mgr.GetWorld()->GetAreaAlways(area), bounds);
  for (int i = 0; i < 3; ++i) {
    SCachedHudLight& light = mHudLights[i];
    const CVector3f direction = (light.mPosition - position).AsNormalized();
    if (light.mFade > 0.f && (CVector3f::Dot(lookDirection, direction) <= 0.15707964f ||
                              !IsCachedLightInAreaLights(light, lights))) {
      light.mFade *= -1.f;
    }
  }
  int available = 0;
  for (int i = 0; i < 3; ++i) {
    if (mHudLights[i].mFade <= 0.f) {
      ++available;
    }
  }
  --available;
  for (uint i = 0; i < lights.GetActiveAreaLightCount(); ++i) {
    if (available < 1) {
      break;
    }
    const CLight& light = lights.GetLight(i);
    const CVector3f direction = (light.GetPosition() - position).AsNormalized();
    if (!IsAreaLightInCachedLights(light) &&
        CVector3f::Dot(lookDirection, direction) > 0.15707964f) {
      const int slot = FindEmptyHudLightSlot(light);
      if (slot != -1) {
        --available;
        mHudLights[slot] =
            SCachedHudLight(light.GetPosition(), light.GetColor(), light.GetAttenuationConstant(),
                            light.GetAttenuationLinear(), light.GetAttenuationQuadratic(), 0.001f);
      }
    }
  }
  for (int i = 0; i < 3; ++i) {
    SCachedHudLight& light = mHudLights[i];
    if (light.mFade < 0.f) {
      light.mFade = rstl::min_val(light.mFade + 2.f * dt, 0.f);
    } else if (light.mFade < 1.f && light.mFade != 0.f) {
      light.mFade = rstl::min_val(light.mFade + 2.f * dt, 1.f);
    }
  }

  const CPlayerState& state = *mgr.GetPlayerState(mPlayerIndex);
  const CPlayerState::EPlayerVisor visor = state.GetCurrentVisor();
  const float visorTransition = state.GetVisorTransitionFactor();
  const CColor lightWorldAdd[4] = {gpTweakGui->GetHelmetBaseAmbientColorCombatLightWorld(),
                                   gpTweakGui->GetHelmetBaseAmbientColorEchoLightWorld(),
                                   gpTweakGui->GetHelmetBaseAmbientColorScanLightWorld(),
                                   gpTweakGui->GetHelmetBaseAmbientColorDarkLightWorld()};
  const CColor darkWorldAdd[4] = {gpTweakGui->GetHelmetBaseAmbientColorCombatDarkWorld(),
                                  gpTweakGui->GetHelmetBaseAmbientColorEchoDarkWorld(),
                                  gpTweakGui->GetHelmetBaseAmbientColorScanDarkWorld(),
                                  gpTweakGui->GetHelmetBaseAmbientColorDarkDarkWorld()};
  const CColor lightWorldMultiply[4] = {gpTweakGui->GetHelmetLightAmbientModCombatLightWorld(),
                                        gpTweakGui->GetHelmetLightAmbientModEchoLightWorld(),
                                        gpTweakGui->GetHelmetLightAmbientModScanLightWorld(),
                                        gpTweakGui->GetHelmetLightAmbientModDarkLightWorld()};
  const CColor darkWorldMultiply[4] = {gpTweakGui->GetHelmetLightAmbientModCombatDarkWorld(),
                                       gpTweakGui->GetHelmetLightAmbientModEchoDarkWorld(),
                                       gpTweakGui->GetHelmetLightAmbientModScanDarkWorld(),
                                       gpTweakGui->GetHelmetLightAmbientModDarkDarkWorld()};
  const CColor* addColors = mgr.GetIsDarkWorld() ? darkWorldAdd : lightWorldAdd;
  const CColor* multiplyColors = mgr.GetIsDarkWorld() ? darkWorldMultiply : lightWorldMultiply;
  CColor lightAdd = CColor::Lerp(addColors[0], addColors[visor], visorTransition);
  const CColor lightMultiply =
      CColor::Lerp(multiplyColors[0], multiplyColors[visor], visorTransition);
  for (int i = 0; i < mHudLights.size(); ++i) {
    const SCachedHudLight& light = mHudLights[i];
    const CVector3f toCamera = position - light.mPosition;
    const CVector3f direction =
        camera->GetTransform().BuildMatrix3f().GetTranspose() * toCamera.AsNormalized();
    const float distance = rstl::max_val(toCamera.Magnitude(), FLT_EPSILON);
    const float falloff = rstl::min_val(
        1.f,
        1.f / (light.mAttenuationConstant * gpTweakGui->GetExplosionLightFalloffMultConstant() +
               distance *
                   (light.mAttenuationLinear * gpTweakGui->GetExplosionLightFalloffMultLinear()) +
               distance * (distance * (light.mAttenuationQuadratic *
                                       gpTweakGui->GetExplosionLightFalloffMultQuadratic()))));
    const float intensity = CMath::Clamp(0.f,
                                         (falloff * CMath::AbsF(light.mFade)) *
                                             CVector3f::Dot(CVector3f::Forward(), -1.f * direction),
                                         1.f);
    CColor color = CColor::Modulate(light.mColor, CColor(intensity, intensity, intensity, 1.f));
    color = GetVisorHudLightColor(color, mgr);
    lightAdd = CColor::Add(lightAdd, CColor::Modulate(color, lightMultiply));
  }

  const CObjectList& gameLights = mgr.GetObjectListById(kOL_GameLight);
  for (int i = gameLights.GetFirstObjectIndex(); i != -1; i = gameLights.GetNextObjectIndex(i)) {
    const CEntity* entity = gameLights[i];
    if (entity == nullptr || !entity->GetActive()) {
      continue;
    }
    const CScriptDynamicLight* dynamicLight = TCastToConstPtr< CScriptDynamicLight >(entity);
    if (dynamicLight != nullptr && !dynamicLight->UsesWorld()) {
      continue;
    }
    const CGameLight* light = static_cast< const CGameLight* >(entity);
    if (TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(light->GetParentId()))) {
      continue;
    }
    const CLight& candidate = light->GetLight();
    if (candidate.GetType() == kLT_Hard) {
      const float distanceSquared = (candidate.GetPosition() - position).MagSquared();
      const float radius = candidate.GetRadius();
      if (distanceSquared < radius * radius && radius > 0.01f) {
        const float intensity = 0.2f * (1.f - CMath::SqrtF(distanceSquared) / radius);
        lightAdd = CColor::Add(lightAdd, CColor(intensity, intensity, intensity, 1.f));
      }
    } else if (candidate.GetIntensity() > FLT_EPSILON &&
               CollisionUtil::AABoxSphereIntersection(
                   bounds, CSphere(candidate.GetPosition(), candidate.GetRadius()))) {
      const CVector3f toCamera = position - candidate.GetPosition();
      const float distance = rstl::max_val(toCamera.Magnitude(), FLT_EPSILON);
      float falloff = rstl::min_val(
          1.f, 1.f / (gpTweakGui->GetExplosionLightFalloffMultConstant() *
                          candidate.GetAttenuationConstant() +
                      distance * (gpTweakGui->GetExplosionLightFalloffMultLinear() *
                                  candidate.GetAttenuationLinear()) +
                      distance * (distance * (gpTweakGui->GetExplosionLightFalloffMultQuadratic() *
                                              candidate.GetAttenuationQuadratic()))));
      if (candidate.GetType() == kLT_Spot) {
        const float dot = rstl::max_val(
            0.f, CVector3f::Dot(camera->GetTransform().GetForward(), candidate.GetDirection()));
        const float factor = CMath::Clamp(0.f, (2.f / M_PIF) * float(asin(dot)), 1.f);
        falloff *= factor;
      }
      CColor color = CColor::Modulate(candidate.GetColor(), CColor(falloff, falloff, falloff, 1.f));
      color = GetVisorHudLightColor(color, mgr);
      lightAdd = CColor::Add(lightAdd, color);
    }
  }
  const CColor ambientScale(uchar(64), uchar(64), uchar(64), uchar(255));
  lightAdd = CColor::Add(lightAdd, CColor::Modulate(lights.GetAmbientColor(), ambientScale));
  mHelmetLightingWidget->SetColor(lightAdd);
}

CColor CSamusHud::GetVisorHudLightColor(const CColor& color, const CStateManager& mgr) const {
  const CPlayerState& state = *mgr.GetPlayerState(mPlayerIndex);
  const CPlayerState::EPlayerVisor visor = state.GetCurrentVisor();
  const float t = state.GetVisorTransitionFactor();
  CColor result = color;
  switch (visor) {
  case CPlayerState::kPV_Combat:
    break;
  case CPlayerState::kPV_Scan: {
    const CColor multiplier =
        CColor::Lerp(CColor::White(), gpTweakGuiColors->GetScanVisorHUDLightMultiply(), t);
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

void CSamusHud::UpdateHudDamage(float dt, const CStateManager& mgr, uint) {
  const CPlayer& player = *mgr.GetPlayer(mPlayerIndex);
  if (player.GetHealthInfo()->GetHP() <= 0.f) {
    mDamageFilterDuration = FLT_EPSILON;
    mDamageFilterRemaining = FLT_EPSILON;
  }
  if (player.WasDamaged() && mgr.GetGameState() == CStateManager::kGS_Running &&
      (player.GetDamageWeaponType() != kWT_AreaDark ||
       mgr.GetPlayerState(mPlayerIndex)->GetCurrentSuitRaw() == CPlayerState::kPS_Varia)) {
    mDamageTime += dt;
  } else {
    mDamageTime = 0.f;
  }
  const float pulseDuration = gpTweakGui->GetHudDamagePulseDuration();
  const float pulseTime = CMath::AbsF(fmodf(mDamageTime, pulseDuration));
  mDamagePulse = pulseTime < 0.5f * pulseDuration
                     ? pulseTime / (0.5f * pulseDuration)
                     : (pulseDuration - pulseTime) / (0.5f * pulseDuration);
  mDamagePulse = CMath::Clamp(0.f,
                              (mDamagePulse * rstl::min_val(0.3f, player.GetDamageAmount())) *
                                  gpTweakGui->GetHudDamageColorGain(),
                              1.f);

  if (mDamageFilterRemaining > 0.f) {
    mDamageFilterRemaining = rstl::max_val(0.f, mDamageFilterRemaining - dt);
    if (mDamageFilterRemaining == 0.f) {
      CSfxManager::RemoveEmitter(mDamageSound);
      mDamageSound.Clear();
    }
  }
  const CColor& ambientColor = gpTweakGuiColors->GetFlashPassColor();
  const float peak = mDamageFilterDuration * gpTweakGui->GetHudDamagePeakFactor();
  float colorGain =
      mDamageFilterRemaining > peak
          ? (mDamageFilterDuration - mDamageFilterRemaining) / (mDamageFilterDuration - peak)
          : mDamageFilterRemaining / peak;
  colorGain = CMath::Clamp(0.f, colorGain * mDamageFilterGain, 1.f);
  const CColor color0 = ambientColor.WithAlphaModulatedBy(colorGain);
  const CColor color1 =
      gpTweakGuiColors->GetDamageAmbientPulseColor().WithAlphaModulatedBy(mDamagePulse);
  CColor filterColor = CColor::Add(color0, color1);
  if (mNextState != kHS_Scan) {
    for (uint i = 0; i < 4; ++i) {
      if (CGuiWidget* widget = mLoadedHudFrame->FindWidget(sDecorativeWidgets[i])) {
        const CColor color = ModulateColor(gpTweakGuiColors->GetHUDDecorativeColor());
        widget->SetColor(
            CColor::Lerp(color, gpTweakGuiColors->GetHUDDamageModulateColor(), colorGain));
      }
    }
  }
  if (filterColor.GetAlphau8()) {
    if (player.GetMorphballTransitionState() != CPlayer::kMS_Unmorphed) {
      filterColor = filterColor.WithAlphaModulatedBy(0.75f);
    }
    mDamageFilter.SetFilter(CCameraFilterPass::kFT_Add, CCameraFilterPass::kFS_Fullscreen, 0.f,
                            filterColor, kInvalidAssetId);
  } else {
    mDamageFilter.DisableFilter(0.f);
  }
  if (mDamageSound) {
    CSfxManager::UpdateEmitter(mDamageSound, player.GetTransform().GetTranslation(),
                               player.GetTransform().GetForward(), 127);
  }

  for (int i = 0; i < mDamageSectorRemaining.size(); ++i) {
    if (mDamageSectorRemaining[i] > 0.f) {
      mDamageSectorRemaining[i] = rstl::max_val(0.f, mDamageSectorRemaining[i] - dt);
      const float ratio = mDamageSectorRemaining[i] / mDamageSectorDurations[i];
      const float intensity = mDamageSectorIntensity[i];
      mDamageSectorIntensity[i] = rstl::min_val(1.f, ratio * intensity);
    }
  }
  if (mDamageHighlightRemaining > 0.f) {
    mDamageHighlightRemaining = rstl::max_val(0.f, mDamageHighlightRemaining - dt);
  }
  if (mgr.IsMultiplayer()) {
    return;
  }

  bool updateTransform = false;
  if (mShakeTranslationAmount > 0.f) {
    const float deceleration = (60.f * dt) * gpTweakGui->GetHUDDamageJostleReturnAcceleration();
    mShakeTranslationVelocity -= deceleration;
    mShakeTranslationAmount =
        rstl::max_val(0.f, mShakeTranslationAmount + mShakeTranslationVelocity);
    updateTransform = true;
  }
  if (mShakeRemaining > 0.f) {
    mShakeRemaining = rstl::max_val(0.f, mShakeRemaining - dt);
    const float rotateFraction = mShakeRemaining / mShakeDuration;
    const float rotate = rstl::min_val(rotateFraction * mShakeGain,
                                       gpTweakGui->GetHUDDamageDistortionMaxMagnitude());
    const int xRandom = rand();
    const float xAngle = (2.f * M_PIF / 10.f) * ((xRandom / float(RAND_MAX)) * rotate);
    const CQuaternion xRotation = CQuaternion::XRotation(CRelAngle::FromRadians(xAngle));
    const int zRandom = rand();
    const float zAngle = (2.f * M_PIF / 10.f) * ((zRandom / float(RAND_MAX)) * rotate);
    const CQuaternion zRotation = CQuaternion::ZRotation(CRelAngle::FromRadians(zAngle));
    mHudLagShake = xRotation * zRotation;
    CVector3f vectors[3] = {CVector3f::Right(), CVector3f::Forward(), CVector3f::Up()};
    for (int i = 0; i < 4; ++i) {
      const int random = rand();
      const int component = rand() % 9;
      const float amount = (random / float(RAND_MAX) - 0.5f) * rotate;
      vectors[component % 3][component / 3] += amount;
    }
    mShakeRotation = CMatrix3f(vectors[0], vectors[1], vectors[2]);
    updateTransform = true;
  }
  if (updateTransform) {
    mShakeTranslation =
        rstl::min_val(mShakeTranslationAmount, gpTweakGui->GetHUDDamageJostleMaxOffset()) *
        mDamagerToPlayer;
    if (mDecorationRoot != nullptr) {
      const CTransform4f& idle = mDecorationRoot->GetIdleXform();
      const CVector3f& translation =
          idle.GetTranslation() + gpTweakGui->GetHudDecoShakeTranslateGain() * mShakeTranslation;
      const CTransform4f xf(idle.BuildMatrix3f() * mShakeRotation, translation);
      mDecorationRoot->SetO2PTransform(xf);
    }
  }
}

void CSamusHud::UpdateStateTransition(float dt, const CStateManager& mgr) {
  const EHudState desired = GetDesiredHudState(mgr);
  if (desired != mDesiredState) {
    mDesiredState = desired;
    const bool ballTransition = desired == kHS_Ball || mNextState == kHS_Ball;
    mTransitionFactor = ballTransition ? FLT_EPSILON : mTransitionFactor;
    UpdateHudColor();
    mTransitionState = kTS_FadeOut;
  }
  switch (mTransitionState) {
  case kTS_FadeOut:
    mTransitionFactor = rstl::max_val(mTransitionFactor - 5.f * dt, 0.f);
    UpdateHudColor();
    if (mTransitionFactor == 0.f) {
      mBossEnergy = nullptr;
      if (mDesiredState == kHS_Ball) {
        UninitializeFrameGlueMutable();
        mPendingHudFrame = rs_new CGuiFrameLoader(
            gpResourceFactory->GetResourceIdByName(sBallHudNames[mgr.GetViewportLayoutIndex()])
                ->GetId(),
            *gpResourceFactory, *gpSimplePool);
      } else if (mNextState == kHS_Ball) {
        UninitializeFrameGlueMutable();
        mPendingHudFrame = rs_new CGuiFrameLoader(
            gpResourceFactory->GetResourceIdByName(sCombatHudNames[mgr.GetViewportLayoutIndex()])
                ->GetId(),
            *gpResourceFactory, *gpSimplePool);
      }
      mTransitionState = kTS_Loading;
    }
    if (mTransitionState != kTS_Loading) {
      return;
    }
  case kTS_Loading:
    if (CGuiFrameLoader* loader = mPendingHudFrame.get()) {
      if (!loader->IsFinishedLoading()) {
        return;
      }
      mHudFrame = loader->CreateFrame();
      mLoadedHudFrame = mHudFrame.get();
      mPendingHudFrame = nullptr;
      mPreviousState = mNextState;
      mNextState = mDesiredState;
      UpdateHudColor();
      mTransitionState = kTS_FadeIn;
      InitializeFrameGlueMutable(mgr);
    } else {
      mPreviousState = mNextState;
      mNextState = mDesiredState;
      mTransitionState = kTS_FadeIn;
      UpdateHudColor();
    }
    if (mNextState == kHS_Scan) {
      mScanInterface =
          rs_new CHudDecoInterfaceScan(mgr, *mLoadedHudFrame, mHudStringTable, mPlayerIndex);
      if (!mBeamMenu.null()) {
        mBeamMenu->SetIsVisibleGame(false);
      }
    }
    if (mPreviousState == kHS_Scan) {
      mScanInterface = nullptr;
      if (!mBeamMenu.null()) {
        mBeamMenu->SetIsVisibleGame(true);
      }
    }
    UpdateHelmetWidgets();
    UpdateHudWidgetColors();
    UpdateEnergy(0.f, mgr, true);
    fn_8006653c(mgr, true);
    ResolveLockOnTexture();
    if (mScanInterface.null()) {
      UpdateMissile(0.f, mgr, true);
      UpdateBeamAmmo(mgr, true);
    }
  case kTS_FadeIn:
    mTransitionFactor = rstl::min_val(5.f * dt + mTransitionFactor, 1.f);
    UpdateHudColor();
    if (mTransitionFactor == 1.f) {
      mTransitionState = kTS_Idle;
    }
    break;
  case kTS_Idle:
    break;
  }
}

void CSamusHud::UpdateHudFrame(float dt, const CStateManager& mgr) {
  UpdateStateTransition(dt, mgr);
  if (mLoadedHudFrame != nullptr) {
    mLoadedHudFrame->Update(dt);
  }
}

void CSamusHud::UpdateBootSequence(float dt, const CStateManager& mgr) {
  const CPlayer& player = *mgr.GetPlayer(mPlayerIndex);
  if (player.GetRezbitState() == CPlayer::kRS_Recovering && mBootTimer <= 0.f) {
    mBootText.SetFontColor(gpTweakGuiColors->GetHUDMemoTextForegroundColor().WithAlphaOf(0.5f));
    mBootTimer = 4.f;
    mHudBootAlpha = 0.f;
    mBootText.SetText(rstl::wstring(), false);
    mBooting = true;
    mBootTextFade = 0.5f;
    mCorruptTextTimer = 0.f;
  } else if (player.GetRezbitState() == CPlayer::kRS_Recovered && mBootTimer <= 0.f) {
    mBootText.SetFontColor(gpTweakGuiColors->GetHUDMemoTextForegroundColor().WithAlphaOf(0.5f));
    mBootTimer = 4.f;
    mHudBootAlpha = 1.f;
    mBootText.SetText(rstl::wstring(), false);
    mBooting = false;
    mBootTextFade = 8.f;
    mCorruptTextTimer = 0.f;
  } else if (!close_enough(mHudBootAlpha, 1.f)) {
    mBootText.SetFontColor(gpTweakGuiColors->GetHUDMemoTextForegroundColor().WithAlphaOf(0.5f));
    mHudBootAlpha = rstl::min_val(1.f, mHudBootAlpha + 4.f * dt);
    mCorruptTextTimer = 0.f;
  }

  const float oldBootTimer = mBootTimer;
  if (oldBootTimer > 0.f) {
    mBootTimer -= dt;
    const char* tableName = mBooting ? "BootText" : "GearLostText";
    const float lineCount = mBooting ? 11.f : 10.f;
    const int nextLine = int(lineCount * (4.f - mBootTimer) * 0.25f);
    const int oldLine = int(lineCount * (4.f - oldBootTimer) * 0.25f);
    if (nextLine != oldLine) {
      const int firstString = gpStringTable->GetStringIndex(tableName);
      rstl::wstring text;
      for (int i = rstl::max_val(0, nextLine - 12); i < nextLine; ++i) {
        text = text + gpStringTable->GetString(firstString + i) + L"\n";
      }
      mBootText.SetText(text, false);
    }
  } else if (player.GetRezbitState() == CPlayer::kRS_Infected) {
    mCorruptTextTimer -= dt;
    if (mCorruptTextTimer <= 0.f) {
      mBootText.SetFontColor(gpTweakGuiColors->GetHUDMemoTextForegroundColor().WithAlphaOf(0.5f));
      mCorruptTextTimer = 0.25f;
      char corruptText[14] = {0};
      for (int i = 0; i < 13; ++i) {
        corruptText[i] = char(int(123.f * (float(i) + CGraphics::GetSecondsMod900())) % 96 + ' ');
        if (corruptText[i] == '&') {
          corruptText[i] = '\'';
        }
      }
      rstl::wstring text =
          mBootText.GetText() + CStringExtras::ConvertToUNICODE(rstl::string(corruptText));
      if (text.size() > 720) {
        text = text.substr(text.size() - 700);
      }
      mBootText.SetText(text, false);
    }
  } else {
    mBootTextFade = rstl::max_val(0.f, mBootTextFade - dt);
    mBootText.SetFontColor(gpTweakGuiColors->GetHUDMemoTextForegroundColor().WithAlphaOf(
        rstl::min_val(mBootTextFade, 0.5f)));
    if (close_enough(mBootTextFade, 0.f)) {
      mBootText.SetText(rstl::wstring(), false);
      mBootTimer = 0.f;
    }
  }
}

void CSamusHud::UpdateHudMemo(float dt, const CStateManager& mgr) {
  if (mMessageAButton != nullptr) {
    const float oldPulse = mAButtonPulse;
    if (mHudMemoIndex == 0) {
      mAButtonPulse += 2.f * dt;
      if (mAButtonPulse > 1.f) {
        mAButtonPulse -= 2.f;
      }
    }
    const float a = CMath::AbsF(mAButtonPulse);
    mMessageAButton->SetColor(CColor::White().WithAlphaOf(a));
    const bool pulseSound = !mgr.GetCameraManager(mPlayerIndex)->IsInCinematicCamera() &&
                            oldPulse < 0.f && mAButtonPulse >= 0.f &&
                            mMessageRoot->GetIsVisible() &&
                            (mMessageTime == 0.f || mMessageTime >= 1.f);
    if (pulseSound) {
      CSfxManager::SfxStart(0x111f, 127, 64, CSfxManager::kAllAreas, false, false,
                            CSfxManager::kMedPriority);
    }
  }
  float messageAlpha = 1.f;
  if (mMessageTime > 0.f) {
    messageAlpha = rstl::min_val(1.f, mMessageTime);
  } else if (mMessagePane == nullptr || mMessageRoot == nullptr ||
             (!mMessagePane->GetIsVisible() && !mMessageRoot->GetIsVisible())) {
    messageAlpha = 0.f;
  }
  if (!mBossEnergy.null()) {
    mBossEnergy->SetAlpha(1.f - messageAlpha);
  }
  if (!mHudMemoString.null() && mHudMemoString->IsLoaded()) {
    SetMessage(rstl::wstring((**mHudMemoString)->GetString(mHudMemoIndex)), mHudMemoParms);
    mHudMemoString = nullptr;
  }
  if (mMessagePane != nullptr && mMessageRoot != nullptr) {
    if (mMessageTime > 0.f) {
      mMessageTime = rstl::max_val(0.f, mMessageTime - dt);
      if (mMessageTime == 0.f) {
        mMessagePane->TextSupport().SetTypeWriteEffectOptions(false, 0.f, 1.f);
        mMessageRoot->SetVisibility(false, kTM_Children);
        mMessageText = rstl::wstring_l(L"");
      }
    }
    const float rootAlpha = rstl::min_val(messageAlpha, 1.f);
    mMessageRoot->SetColor(CColor::White().WithAlphaOf(rootAlpha));
  }
  const float printed = mMessagePane->TextSupport().GetNumCharactersPrinted();
  const float charsPerSound = gpTweakGui->GetWorldTransManagerCharsPerSfx();
  if (printed >= mLastMessageSoundChars + charsPerSound) {
    mLastMessageSoundChars += charsPerSound;
    if ((mMessageRoot->GetIsVisible() || mMessagePane->GetIsVisible()) &&
        !mgr.GetCameraManager(mPlayerIndex)->IsInCinematicCamera()) {
      CSfxManager::SfxStart(0x3fe, 127, 64, CSfxManager::kAllAreas, false, false,
                            CSfxManager::kMedPriority);
    }
  }
  if (mLoadedMemoFrame != nullptr) {
    mLoadedMemoFrame->Update(dt);
  }
  if (mCounter != nullptr) {
    const float escapeTime = mgr.GetEscapeSequenceTimer();
    if (escapeTime > 0.f) {
      const int seconds = int(escapeTime);
      const int hundredths = int(100.f * escapeTime);
      char text[16];
      sprintf(text, "%02d:%02d:%02d", seconds / 60, seconds % 60, hundredths % 100);
      mCounter->TextSupport().SetText(rstl::string(text), false);
      mCounter->SetIsVisible(true);
      const float counterAlpha = rstl::min_val(1.f, 1.f - rstl::min_val(1.f, mMessageTime));
      mCounter->SetColor(CColor::White().WithAlphaOf(CMath::Clamp(0.f, counterAlpha, 1.f)));
    } else {
      mCounter->SetIsVisible(false);
    }
  }
}

void CSamusHud::Update(float dt, const CStateManager& mgr, uint helmetVisibility, bool hudVisible,
                       bool targetingVisible) {
  UpdateStateTransition(dt, mgr);
  if (mLoadedHudFrame == nullptr) {
    return;
  }

  const CPlayer& player = *mgr.GetPlayer(mPlayerIndex);
  if (mHudCamera != nullptr) {
    float morphFactor = 0.f;
    switch (player.GetMorphballTransitionState()) {
    case CPlayer::kMS_Morphed:
      morphFactor = 1.f;
      break;
    case CPlayer::kMS_Unmorphed:
      morphFactor = 0.f;
      break;
    case CPlayer::kMS_Morphing:
      morphFactor = player.GetMorphBallTransitionFactor();
      break;
    case CPlayer::kMS_Unmorphing:
      morphFactor = 1.f - player.GetMorphBallTransitionFactor();
      break;
    }
    mViewportScaleY = 1.f - morphFactor * gpTweakGui->GetBallViewportYReduction();
    const float xfbHeight = float(int(CGraphics::GetRenderMode().xfbHeight));
    const float halfReduction = 0.5f * (xfbHeight * gpTweakGui->GetBallViewportYReduction());
    const CTransform4f& idle = mHudCamera->GetIdleXform();
    const float zOffset = ((1.f - morphFactor) * halfReduction - halfReduction) * 0.01f;
    const CVector3f translation(idle.Get03(), idle.Get13(), idle.Get23() + zOffset);
    mHudCamera->SetO2PTransform(CTransform4f::Translate(translation));
  }

  const bool englishOnly = gpGameState->GameOptions().GetIsHudEnglish();
  if (englishOnly != mEnglishOnly) {
    mEnglishOnly = englishOnly;
    RefreshHudStringTable();
    if (!mVisorMenu.null()) {
      mVisorMenu->RefreshText();
    }
    if (!mBeamMenu.null()) {
      mBeamMenu->RefreshText();
    }
  }
  const bool helmetVisible = helmetVisibility != 0;
  const bool firstPerson = player.GetMorphballTransitionState() == CPlayer::kMS_Unmorphed &&
                           !player.GetCameraManager()->IsInCinematicCamera();
  if (firstPerson != mFirstPerson) {
    if (firstPerson) {
      EnterFirstPerson(mgr);
    } else {
      LeaveFirstPerson(mgr);
    }
    mFirstPerson = firstPerson;
  }
  mBootText.Update(dt);
  UpdateEnergyLow(dt, mgr);
  for (int i = 0; i < 17; ++i) {
    mProfileInfo[i].mUpdateTime = 0;
  }

  UpdateBootSequence(dt, mgr);
  UpdateBossLockOnWarning(dt, mgr);
  UpdateHudLag(dt, mgr);
  UpdateHudDynamicLights(dt, mgr);
  if (targetingVisible) {
    mTargetingManager.Update(dt, mgr);
  }
  UpdateStaticInterference(dt, mgr);
  if (helmetVisible) {
    if (mNextState != kHS_None) {
      UpdateEnergy(dt, mgr, false);
      UpdateFreeLook(dt, mgr);
    }
    UpdateThreatAssessment(dt, mgr);
    UpdateBeamAmmo(mgr, false);
    UpdateMissile(dt, mgr, false);
    UpdateVisorAndBeamMenus(dt, mgr);
    UpdateBallMode(mgr, false);
    ResolveLockOnTexture();
    if (!mRadar.null()) {
      mRadar->SetColor(ModulateColor(gpTweakGuiColors->GetRadarWidgetColor()));
      mRadar->Update(dt, mgr);
    }
  }
  UpdateHudMemo(dt, mgr);
  mLoadedHudFrame->Update(dt);
  if (!mBossEnergy.null()) {
    mBossEnergy->Update(dt);
  }
  if (!mScanInterface.null()) {
    mScanInterface->Update(dt, mgr);
  }
  if (!mVisorMenu.null()) {
    const float alpha = mScanInterface.null() ? 1.f : mScanInterface->GetMessageTextAlpha();
    mVisorMenu->UpdateHudAlpha(alpha);
    if (mVisorBracket != nullptr) {
      mVisorBracket->SetColor(
          ModulateColor(gpTweakGuiColors->GetHUDDecorativeColor()).WithAlphaOf(alpha));
    }
    mVisorMenu->Update(dt, false);
  }
  if (!mBeamMenu.null()) {
    mBeamMenu->Update(dt, false);
  }
  if (mDarkVisor != nullptr && mDarkVisorBacking != nullptr) {
    const CPlayerState::EPlayerVisor visor = player.GetPlayerState()->GetCurrentVisor();
    mDarkVisor->SetIsVisible(visor == CPlayerState::kPV_Dark);
    mDarkVisorBacking->SetIsVisible(visor == CPlayerState::kPV_Dark);
  }
  if (player.WasDamaged() && mgr.GetGameState() == CStateManager::kGS_Running) {
    const CVector3f position = player.GetDamageLocationWR();
    const float damage = player.GetDamageAmount();
    const float previousDamage = player.GetPrevDamageAmount();
    ShowDamage(position, damage, previousDamage, mgr);
  }
  UpdateHudDamage(dt, mgr, helmetVisibility);
}

rstl::reserved_vector< bool, 4 > CSamusHud::BuildPlayerHasVisors(const CStateManager& mgr) const {
  rstl::reserved_vector< bool, 4 > result;
  const CPlayerState& state = *mgr.GetPlayerState(mPlayerIndex);
  result.push_back(state.HasPowerUp(CPlayerState::kIT_CombatVisor));
  result.push_back(state.HasPowerUp(CPlayerState::kIT_EchoVisor));
  result.push_back(state.HasPowerUp(CPlayerState::kIT_ScanVisor));
  result.push_back(state.HasPowerUp(CPlayerState::kIT_DarkVisor));
  return result;
}

rstl::reserved_vector< bool, 4 > CSamusHud::BuildPlayerHasBeams(const CStateManager& mgr) const {
  rstl::reserved_vector< bool, 4 > result;
  const CPlayerState& state = *mgr.GetPlayerState(mPlayerIndex);
  result.push_back(state.HasPowerUp(CPlayerState::kIT_PowerBeam));
  result.push_back(state.HasPowerUp(CPlayerState::kIT_DarkBeam));
  result.push_back(state.HasPowerUp(CPlayerState::kIT_LightBeam));
  result.push_back(state.HasPowerUp(CPlayerState::kIT_AnnihilatorBeam));
  return result;
}

void CSamusHud::DrawLockOnIndicators(const CStateManager& mgr,
                                     const rstl::reserved_vector< TUniqueId, 12 >& targets) const {
  if (targets.empty() || mLockedOnIndicator->GetObject() == nullptr) {
    return;
  }
  mLockedOnIndicator->GetObject()->LoadMipLevel(0, GX_TEXMAP0, CTexture::kCM_Clamp);
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
  CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  CGraphics::SetCullMode(kCM_None);
  gpRender->SetBlendMode_AdditiveAlpha();
  CGraphics::SetDepthWriteMode(false, kE_LEqual, false);
  const rstl::pair< CVector2f, CVector2f > bounds =
      gpRender->SetViewportOrtho(true, -4096.f, 4096.f);
  const float extent = bounds.second.GetX() - bounds.first.GetX();
  for (rstl::reserved_vector< TUniqueId, 12 >::const_iterator it = targets.begin();
       it != targets.end(); ++it) {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(*it))) {
      const CRelAngle direction = GetRelativeDirection(actor->GetTranslation(), mgr);
      const float scale = gpTweakGui->GetLockOnIndicatorScale() * extent / 100.f;
      gpRender->SetModelMatrix(CTransform4f(CMatrix3f::Scale(scale), CVector3f::Zero()) *
                               CTransform4f::RotateY(direction));
      static const float verticalOffset = gpTweakGui->GetLockOnIndicatorVerticalOffset();
      CGraphics::StreamBegin(kP_Quads);
      if (const CPlayer* player = TCastToConstPtr< CPlayer >(*actor)) {
        CGraphics::StreamColor(gpTweakGui->GetPlayerLockOnIndicatorColor(
            player->GetPlayerState()->GetPlayerSelection()));
      } else {
        CGraphics::StreamColor(gpTweakGui->GetLockOnIndicatorColor());
      }
      for (int i = 0; i < 13; ++i) {
        CGraphics::StreamTexcoord(1.f, 1.f);
        CGraphics::StreamVertex(1.f, 0.f, 1.f + verticalOffset);
        CGraphics::StreamTexcoord(0.f, 1.f);
        CGraphics::StreamVertex(1.f, 0.f, verticalOffset - 1.f);
        CGraphics::StreamTexcoord(0.f, 0.f);
        CGraphics::StreamVertex(-1.f, 0.f, verticalOffset - 1.f);
        CGraphics::StreamTexcoord(1.f, 0.f);
        CGraphics::StreamVertex(-1.f, 0.f, 1.f + verticalOffset);
      }
      CGraphics::StreamEnd();
    }
  }
  CGraphics::SetCullMode(kCM_Front);
  CGraphics::SetDepthWriteMode(true, kE_LEqual, true);
}

void CSamusHud::DrawLockOnIndicators(const CStateManager& mgr) const {
  if (!mLockedOnIndicator) {
    return;
  }
  rstl::reserved_vector< TUniqueId, 12 > targets;
  const CPlayer& player = *mgr.GetPlayer(mPlayerIndex);
  if (mgr.IsMultiplayer()) {
    const TUniqueId playerId = player.GetUniqueId();
    for (uint i = 0; i < mgr.GetNumPlayers(); ++i) {
      if (i != mPlayerIndex) {
        const CPlayer& other = *mgr.GetPlayer(i);
        if (other.GetPlayerState()->IsPlayerAlive() && other.GetOrbitTargetId() == playerId) {
          targets.push_back(other.GetUniqueId());
        }
      }
    }
  } else if (player.GetEnemyLockOnActorId() != kInvalidUniqueId) {
    targets.push_back(player.GetEnemyLockOnActorId());
  }
  DrawLockOnIndicators(mgr, targets);
}

void CSamusHud::DrawAttachedEnemyEffect(const CStateManager& mgr) const {
  const float drainTime = mgr.GetPlayer(mPlayerIndex)->GetEnergyDrain().GetEnergyDrainTime();
  if (drainTime > 0.f) {
    const float period = gpTweakGui->GetEnergyDrainModPeriod();
    const float phaseOffset = -0.25f * period;
    const CColor& filterColor = gpTweakGuiColors->GetMetroidSuckPulseColor();
    float alpha;
    if (gpTweakGui->GetEnergyDrainSinusoidalPulse()) {
      alpha = 0.5f * (1.f + CMath::FastSinR(phaseOffset + 2.f * M_PIF * drainTime / period));
    } else {
      float phase = CMath::AbsF(fmodf(drainTime, period));
      const float halfPeriod = 0.5f * period;
      if (phase < halfPeriod) {
        phase /= halfPeriod;
      } else {
        phase = (period - phase) / halfPeriod;
      }
      alpha = phase;
    }
    const CColor color = filterColor.WithAlphaModulatedBy(alpha);
    CCameraFilterPass::DrawFilter(gpTweakGui->GetEnergyDrainFilterAdditive()
                                      ? CCameraFilterPass::kFT_Add
                                      : CCameraFilterPass::kFT_Blend,
                                  CCameraFilterPass::kFS_Fullscreen, color, nullptr, 1.f);
  }
}

void CSamusHud::DrawPlayerFilter(const CStateManager& mgr) const {
  const CPlayer& player = *mgr.GetPlayer(mPlayerIndex);
  if (player.GetScreenFilterColor().GetAlpha() > 0.f) {
    CColor color = player.GetScreenFilterColor();
    color = CColor::Lerp(CColor::Black(), color, color.GetAlpha());
    CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Subtract,
                                  CCameraFilterPass::kFS_Fullscreen, color, nullptr, 1.f);
  }
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
  const CPlayer& player = *mgr.GetPlayer(mPlayerIndex);
  const CPlayer::EPlayerMorphBallState ballState = player.GetMorphballTransitionState();
  if (mNextState == kHS_None || player.GetSpawnedMorphballState() == CPlayer::kMS_Morphed ||
      ballState == CPlayer::kMS_Morphing || ballState == CPlayer::kMS_Unmorphing ||
      mLoadedHudFrame == nullptr || mGuiState == 1) {
    return;
  }

  if (mBootTimer > 0.f || mBootTextFade > 0.f ||
      (player.GetRezbitState() == CPlayer::kRS_Infected && ballState != CPlayer::kMS_Morphed)) {
    gpRender->SetBlendMode_AlphaBlended();
    gpRender->SetViewportOrtho(true, -4096.f, 4096.f);
    CGraphics::SetModelMatrix(CTransform4f::Translate(-210.f, 0.f, 200.f) *
                              CTransform4f::Scale(1.f, 1.f, 1.f));
    CGraphics::SetCullMode(kCM_None);
    gpRender->SetDepthReadWrite(false, false);
    mBootText.Render();
    CGraphics::SetCullMode(kCM_Front);
  }
  if (player.GetRezbitState() == CPlayer::kRS_Recovering) {
    return;
  }
  mDamageFilter.Draw();
  if (ballState == CPlayer::kMS_Unmorphed) {
    DrawAttachedEnemyEffect(mgr);
    DrawPlayerFilter(mgr);
    mStaticFilter.Draw();
    if (targetingVisible) {
      mTargetingManager.Draw(mgr, false);
    }
  }
  const float hudAlpha = (mPreviousState == kHS_Ball ? mTransitionFactor : mHudBootAlpha) *
                         gpGameState->GameOptions().GetHudAlpha();
  if (helmetVisibility != 0 && helmetVisibility < 5) {
    if (alpha < 1.f) {
      CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_NoColor,
                                    CCameraFilterPass::kFS_CookieCutterDepthRandomStatic,
                                    CColor::White(), nullptr, 1.f - alpha);
    }
    mLoadedHudFrame->Draw(CGuiWidgetDrawParms(hudAlpha, CVector3f::Zero()));
  }
  if (!mScanInterface.null()) {
    mScanInterface->Draw(mgr);
  }
  if (hudVisible && helmetVisibility != 0 && helmetVisibility < 5 && !mRadar.null()) {
    mRadar->Draw(mgr, alpha);
  }
  gpRender->SetDepthReadWrite(true, true);
  if (mNextState != kHS_Combat && mNextState != kHS_Dark && mNextState != kHS_Echo) {
    return;
  }
  DrawBossLockOnWarning();
  DrawLockOnIndicators(mgr);
  bool drawDamage = false;
  for (int i = 0; i < mDamageSectorIntensity.size(); ++i) {
    if (!close_enough(mDamageSectorIntensity[i], 0.f)) {
      drawDamage = true;
      break;
    }
  }
  if (!drawDamage || !mDamageRingTexture.TryCache()) {
    return;
  }

  mDamageRingTexture.GetObject()->LoadMipLevel(0, GX_TEXMAP0, CTexture::kCM_Clamp);
  const rstl::pair< CVector2f, CVector2f > bounds =
      gpRender->SetViewportOrtho(true, -4096.f, 4096.f);
  const float scale = (bounds.second.GetX() - bounds.first.GetX()) * 0.125f;
  gpRender->SetModelMatrix(CTransform4f(CMatrix3f::Scale(scale), CVector3f::Zero()));
  const float innerRadius = 0.75f * gpTweakGui->GetHUDDamageIndicatorRadius();
  const float outerRadius = 1.25f * gpTweakGui->GetHUDDamageIndicatorRadius();
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
  CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  CGraphics::SetCullMode(kCM_None);
  gpRender->SetBlendMode_AdditiveAlpha();
  CGraphics::SetDepthWriteMode(false, kE_LEqual, false);
  CGraphics::StreamBegin(kP_TriangleStrip);
  for (int i = 0; i < 13; ++i) {
    const float angle = 2.f * M_PIF * float(i) / 12.f;
    const float textureAngle = (i & 1) ? 0.5335988f : 0.01f;
    const float innerU = fabsf(0.6f * CMath::FastSinR(textureAngle));
    const float innerV = fabsf(2.f * (0.6f * CMath::FastCosR(textureAngle)));
    const float outerU = fabsf(CMath::FastSinR(textureAngle));
    const float outerV = fabsf(CMath::FastCosR(textureAngle));
    CColor color = gpTweakGuiColors->GetDamageIndicatorColor();
    color.SetAlpha(mDamageSectorIntensity[i % mDamageSectorIntensity.size()] * color.GetAlpha());
    CGraphics::StreamColor(color);
    CGraphics::StreamTexcoord(2.f * innerU, innerV);
    CGraphics::StreamVertex(innerRadius * CMath::FastSinR(angle), 0.f,
                            innerRadius * CMath::FastCosR(angle));
    CGraphics::StreamTexcoord(2.f * outerU, 2.f * outerV);
    CGraphics::StreamVertex(outerRadius * CMath::FastSinR(angle), 0.f,
                            outerRadius * CMath::FastCosR(angle));
  }
  CGraphics::StreamEnd();
  CGraphics::SetCullMode(kCM_Front);
  CGraphics::SetDepthWriteMode(true, kE_LEqual, true);
}

void CSamusHud::DrawHelmet(const CStateManager& mgr, float cameraYOffset) const {
  if (mLoadedHelmetFrame != nullptr && !mgr.GetPlayer(mPlayerIndex)->IsInTurret()) {
    const bool unmorphed =
        mgr.GetPlayer(mPlayerIndex)->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed;
    if (mLoadedHelmetFrame != nullptr && unmorphed && mNextState != kHS_Ball) {
      const CGameOptions& options = gpGameState->GameOptions();
      const float alpha = mPreviousState == kHS_Ball ? mTransitionFactor : 1.f;
      const CGuiWidgetDrawParms parms(alpha * options.GetHelmetAlpha(),
                                      CVector3f(0.f, 15.f * cameraYOffset, 0.f));
      mLoadedHelmetFrame->Draw(parms);
    }
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
  const CGameCamera* const camera = CCameraManager::CastGameCameratoFirstPersonCamera(
      mgr.GetCameraManager(mPlayerIndex)->GetCurrentCamera(mgr, true));
  if (camera == nullptr) {
    return CRelAngle::FromRadians(0.f);
  }
  const CVector3f localPosition = camera->GetTransform().TransposeMultiply(position);
  const CVector3f forward(0.f, 1.f, 0.f);
  const CVector3f flatPosition(localPosition.GetX(), localPosition.GetY(), 0.f);
  if (!flatPosition.CanBeNormalized()) {
    return CRelAngle::FromRadians(0.f);
  }
  const CVector3f direction = flatPosition.AsNormalized();
  const float angle = acosf(CVector3f::Dot(forward, direction));
  const CVector3f cross = CVector3f::Cross(forward, direction);
  if (cross.GetZ() > 0.f) {
    return CRelAngle::FromRadians(2.f * M_PIF - angle);
  }
  return CRelAngle::FromRadians(angle);
}

void CSamusHud::ShowDamage(CVector3f position, float damage, float previousDamage,
                           const CStateManager& mgr) {
  if (position.IsNonZero() && !close_enough(damage, 0.f)) {
    const CRelAngle angle = GetRelativeDirection(position, mgr);
    const int sector = CMath::Clamp(0, int(12.f * (angle.AsRadians() / (2.f * M_PIF))), 11);
    mDamageSectorIntensity[sector] = rstl::max_val(
        mDamageSectorIntensity[sector], damage * gpTweakGui->GetHUDFlashMagnitudeLinear() +
                                            gpTweakGui->GetHUDFlashMagnitudeConstant());
    const float duration =
        rstl::max_val(FLT_EPSILON, damage * gpTweakGui->GetHUDFlashTimeScaleLinear() +
                                       gpTweakGui->GetHUDFlashTimeConstant());
    mDamageSectorDurations[sector] = rstl::max_val(mDamageSectorDurations[sector], duration);
    mDamageSectorRemaining[sector] = mDamageSectorDurations[sector];
    mDamageHighlightDuration = mDamageSectorDurations[sector];
  }
  mDamageHighlightRemaining = mDamageHighlightDuration;

  const CPlayer& player = *mgr.GetPlayer(mPlayerIndex);
  if (!player.GetFrozenState()) {
    const float duration =
        damage * gpTweakGui->GetFlashPassTimerLinear() + gpTweakGui->GetFlashPassTimerConstant();
    if (duration > mDamageFilterRemaining) {
      mDamageFilterGain = damage * gpTweakGui->GetFlashPassMagnitudeLinear() +
                          gpTweakGui->GetFlashPassMagnitudeConstant();
      mDamageFilterDuration = duration;
      mDamageFilterRemaining = mDamageFilterDuration;
      if (!mDamageSound && mgr.GetPendingDockArea() == kInvalidAreaId &&
          player.GetDamageWeaponType() != kWT_AreaDark) {
        mDamageSound =
            CSfxManager::AddEmitter(mgr.ReturnFirstIfSingleElseSecond(0x955, 0x264d),
                                    player.GetTransform().GetTranslation(), CSfxManager::kAllAreas,
                                    false, true, CSfxManager::kMaxPriority);
      }
    }
  }
  if (position.IsNonZero()) {
    const CGameCamera* const camera = CCameraManager::CastGameCameratoFirstPersonCamera(
        mgr.GetCameraManager(mPlayerIndex)->GetCurrentCamera(mgr, true));
    if (camera != nullptr) {
      const CVector3f cameraToDamage = camera->GetTransform().GetQuickInverse() * position;
      mShakeTranslationVelocity = previousDamage * gpTweakGui->GetHUDDamageJostleMagnitudeLinear() +
                                  gpTweakGui->GetHUDDamageJostleMagnitudeConstant();
      mShakeTranslationAmount = mShakeTranslationVelocity;
      const CVector3f& direction = cameraToDamage.CanBeNormalized()
                                       ? cameraToDamage.AsNormalized()
                                       : static_cast< const CVector3f& >(CVector3f::Forward());
      mDamagerToPlayer = -1.f * direction;
      mShakeGain = previousDamage * gpTweakGui->GetHUDDamageDistortionMagnitudeLinear() +
                   gpTweakGui->GetHUDDamageDistortionMagnitudeConstant();
      mShakeDuration = previousDamage * gpTweakGui->GetHUDDamageDistortionTimeLinear() +
                       gpTweakGui->GetHUDDamageDistortionTimeConstant();
      mShakeRemaining = mShakeDuration;
    }
  }
}

void CSamusHud::UpdateHudLag(float dt, const CStateManager& mgr) {
  if (mgr.GetViewportLayoutIndex() != 0) {
    return;
  }
  if (!gpGameState->GameOptions().GetHUDLag()) {
    CGuiWidget* root = mLoadedHelmetFrame->GetRootWidget();
    root->SetO2PTransform(root->GetIdleXform());
    CGuiCamera* camera = mLoadedHudFrame->GetFrameCamera();
    camera->SetO2WTransform(camera->GetIdleXform());
    mTargetingManager.CompoundTargetReticle().SetLeadingOrientation(CQuaternion::NoRotation());
    return;
  }

  CUnitVector3f cameraDirection(mPreviousCameraDirection, CUnitVector3f::kN_No);
  const CGameCamera* const camera = CCameraManager::CastGameCameratoFirstPersonCamera(
      mgr.GetCameraManager(mPlayerIndex)->GetCurrentCamera(mgr, true));
  if (camera == nullptr) {
    mHudLag = CQuaternion::NoRotation();
    mInverseHudLag = CQuaternion::NoRotation();
  } else {
    const CMatrix3f rotation = camera->GetTransform().BuildMatrix3f();
    cameraDirection = CUnitVector3f(rotation.GetColumn(1), CUnitVector3f::kN_No);
    ApplyClassicLag(cameraDirection, mHudLag, mgr, dt, false);
    ApplyClassicLag(cameraDirection, mInverseHudLag, mgr, dt, true);
    CQuaternion leadingRotation =
        CQuaternion::LookAt(CUnitVector3f(mPreviousCameraDirection), cameraDirection,
                            CRelAngle::FromRadians(2.f * M_PIF));
    leadingRotation *= leadingRotation;
    leadingRotation *= leadingRotation;
    mTargetingManager.CompoundTargetReticle().SetLeadingOrientation(leadingRotation);
  }
  const CVector3f bob = mgr.GetPlayer(mPlayerIndex)->CameraBobObject()->GetHelmetBobTranslation();
  const CVector3f lagOffset =
      CVector3f(0.f, 0.f, bob.GetZ()) + gpTweakGui->GetHudLagOffsetScale() * mShakeTranslation;
  const CQuaternion lagRotation = mHudLagShake * mHudLag;
  CGuiCamera* helmetCamera = mLoadedHelmetFrame->GetFrameCamera();
  const CVector3f helmetPosition = helmetCamera->GetIdleXform().GetTranslation();
  const CVector3f helmetPivot =
      mLoadedHelmetFrame->GetRootWidget()->GetIdleXform().GetTranslation();
  CTransform4f helmetTransform =
      BuildFinalCameraTransform(lagRotation, helmetPivot + lagOffset, helmetPosition);
  helmetTransform.SetTranslation(helmetTransform.GetTranslation() + lagOffset);
  helmetCamera->SetO2WTransform(helmetTransform);
  const CVector3f hudPosition = mHudCamera->GetIdleXform().GetTranslation();
  const CVector3f hudPivot = mLoadedHudFrame->GetRootWidget()->GetIdleXform().GetTranslation();
  CTransform4f hudTransform =
      BuildFinalCameraTransform(lagRotation, hudPivot + lagOffset, hudPosition);
  if (mNextState != kHS_Ball) {
    hudTransform.SetTranslation(hudTransform.GetTranslation() + lagOffset);
  }
  mLoadedHudFrame->GetFrameCamera()->SetO2WTransform(hudTransform);
  mPreviousCameraDirection = cameraDirection;
}

void CSamusHud::ApplyClassicLag(const CUnitVector3f& lookDirection, CQuaternion& rotation,
                                const CStateManager& mgr, float dt, bool invert) {
  const CQuaternion lookRotation =
      CQuaternion::LookAt(lookDirection, CVector3f::Forward(), CRelAngle::FromRadians(2.f * M_PIF));
  CQuaternion targetRotation =
      invert ? CQuaternion::LookAt(CUnitVector3f(lookRotation.Transform(mPreviousCameraDirection)),
                                   CVector3f::Forward(), CRelAngle::FromRadians(2.f * M_PIF))
             : CQuaternion::LookAt(CVector3f::Forward(),
                                   CUnitVector3f(lookRotation.Transform(mPreviousCameraDirection)),
                                   CRelAngle::FromRadians(2.f * M_PIF));
  targetRotation *= targetRotation;
  const CVector3f targetDirection = targetRotation.BuildTransform().GetColumn(1);
  const CVector3f currentDirection = rotation.BuildTransform().GetColumn(1);
  const float angularStep = 0.5f * (dt * gpTweakPlayerA->GetFreeLookSpeed());
  const float angle = acosf(CMath::Limit(CVector3f::Dot(currentDirection, targetDirection), 1.f));
  const float step = angle > 0.f ? angularStep / angle : 0.f;
  const float t = CMath::Clamp(0.f, (18.f * dt) * step, 1.f);
  targetRotation = CQuaternion::SlerpLocal(rotation, targetRotation, t);
  rotation = targetRotation;
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
        text.length() != 0) {
      pane->SetVisibility(true, kTM_Children);
    }
    mMessagePane->TextSupport().SetTypeWriteEffectOptions(info.GetFadeInText(), 0.1f, 40.f);
    if (info.IsClearMemoWindow()) {
      mLastMessageSoundChars = 0.f;
      mMessagePane->TextSupport().SetCurTime(0.f);
      mMessagePane->TextSupport().SetText(text);
    } else if (mMessagePane->TextSupport().GetText().length() == 0) {
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
  if (mgr.IsMultiplayer()) {
    return;
  }
  const CPlayer& player = *mgr.GetPlayer(mPlayerIndex);
  if (player.GetEnemyLockOnCount() == 0) {
    if (!mBossLockOnFrameLoader.null()) {
      mBossLockOnFrameLoader = rstl::auto_ptr< CGuiFrameLoader >();
    }
    if (!mBossLockOnFrame.null()) {
      mBossLockOnFrame = rstl::auto_ptr< CGuiFrame >();
    }
    if (mLockedOnIndicator) {
      mLockedOnIndicator.clear();
    }
    return;
  }
  if (mBossLockOnFrame.get() == nullptr) {
    if (!mLockedOnIndicator) {
      mLockedOnIndicator = TCachedToken< CTexture >(gpSimplePool->GetObj("TXTR_LockedOnIndicator"));
      mLockedOnIndicator->Lock();
    }
    if (mBossLockOnFrameLoader.get() == nullptr) {
      mBossLockOnFrameLoader =
          rs_new CGuiFrameLoader(gpResourceFactory->GetResourceIdByName("FRME_BossLockon")->GetId(),
                                 *gpResourceFactory, *gpSimplePool);
    }
    if (mBossLockOnFrameLoader->IsFinishedLoading()) {
      mBossLockOnFrame = mBossLockOnFrameLoader->CreateFrame();
      mBossLockOnFrameLoader = nullptr;
      CGuiTextPane* warning =
          static_cast< CGuiTextPane* >(mBossLockOnFrame->FindWidget("textpane_warning"));
      if (warning != nullptr) {
        warning->TextSupport().SetText(
            rstl::wstring(gpStringTable->GetString("EnemyLockedOnWarning")), false);
        warning->TextSupport().SetFontColor(gpTweakGui->GetLockOnIndicatorColor());
      }
    }
  }
  if (mBossLockOnFrame.get() != nullptr) {
    mBossLockOnFrame->Update(dt);
    const int ringCount = rstl::min_val(3, int(player.GetEnemyLockOnCount()));
    for (int i = 0; i < 3; ++i) {
      if (CGuiWidget* ring = mBossLockOnFrame->FindWidget(sBossLockOnRings[i])) {
        const float intensity = 1.f - float(i) / 3.f;
        ring->SetColor(CColor::Modulate(gpTweakGui->GetLockOnIndicatorColor(),
                                        CColor(intensity, intensity, intensity, 1.f)));
        ring->SetVisibility(i < ringCount, kTM_Children);
      }
    }
    if (CGuiWidget* warning = mBossLockOnFrame->FindWidget("textpane_warning")) {
      warning->SetVisibility(ringCount == 3, kTM_Children);
    }
  }
}

void CSamusHud::DrawBossLockOnWarning() const {
  if (mBossLockOnFrame.get() != nullptr) {
    mBossLockOnFrame->Draw(CGuiWidgetDrawParms::Default());
  }
}
