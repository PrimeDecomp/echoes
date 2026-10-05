#include "MetroidPrime/HUD/CHudBossEnergyInterface.hpp"

#include "GuiSys/CAuiEnergyBarT01.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "MetroidPrime/Tweaks/CTweakGuiColors.hpp"
#include "rstl/math.hpp"

static const char* const skEnemyEnergyGroupWidgetNames[] = {
    "basewidget_bossenergy", "basewidget_bossenergy", "basewidget_bossenergy",
    "basewidget_bossdark", "basewidget_bossenergy"};
static const char* const skEnemyEnergyBarWidgetNames[] = {
    "energybart01_bossbar", "energybart01_bossbar", "energybart01_bossbar",
    "energybart01_bossbardark", "energybart01_bossbar"};
static const char* const skEnemyEnergyTextWidgetNames[] = {
    "textpane_boss", "textpane_boss", "textpane_boss", "textpane_bossdark", "textpane_boss"};

rstl::pair< CVector3f, CVector3f > CHudBossEnergyInterface::BossEnergyCoordFunc(float t) {
  const float theta = -0.32352942f + 0.64705884f * t;
  const float x = 17.f * CMath::FastSinR(theta);
  const float y = 17.f * CMath::FastCosR(theta) + -17.f;
  return rstl::pair< CVector3f, CVector3f >(CVector3f(x, y, 0.3f), CVector3f(x, y, 0.f));
}

rstl::pair< CVector3f, CVector3f > CHudBossEnergyInterface::BallBossEnergyCoordFunc(float t) {
  const float x = 8.5f * t;
  return rstl::pair< CVector3f, CVector3f >(CVector3f(x, 0.f, 0.f), CVector3f(x, 0.f, 0.4f));
}

CHudBossEnergyInterface::CHudBossEnergyInterface(CGuiFrame& frame, int hudState)
: mAlpha(1.f), mFade(0.f), mCurrentEnergy(0.f), mMaximumEnergy(0.f), mVisible(false) {
  mRoot = frame.FindWidget(skEnemyEnergyGroupWidgetNames[hudState]);
  mEnergyBar =
      static_cast< CAuiEnergyBarT01* >(frame.FindWidget(skEnemyEnergyBarWidgetNames[hudState]));
  mName = static_cast< CGuiTextPane* >(frame.FindWidget(skEnemyEnergyTextWidgetNames[hudState]));

  if (hudState == CSamusHud::kHS_Ball) {
    mEnergyBar->SetCoordFunc(BallBossEnergyCoordFunc);
  } else {
    mEnergyBar->SetCoordFunc(BossEnergyCoordFunc);
  }
  mEnergyBar->SetTesselation(0.2f);

  const CTweakGuiColors::SVisorColorScheme colors =
      gpTweakGuiColors->GetVisorColorScheme(static_cast< CSamusHud::EHudState >(hudState));
  mEnergyBar->SetFilledColor(
      CColor::Modulate(colors.GetHUDHue(), gpTweakGuiColors->GetEnergyBarFilledColor()));
  mEnergyBar->SetShadowColor(
      CColor::Modulate(colors.GetHUDHue(), gpTweakGuiColors->GetEnergyBarShadowColor()));
  mEnergyBar->SetEmptyColor(
      CColor::Modulate(colors.GetHUDHue(), gpTweakGuiColors->GetEnergyBarEmptyColor()));
  mName->TextSupport().SetFontColor(
      CColor::Modulate(colors.GetHUDHue(), gpTweakGuiColors->GetActiveTextForegroundColor()));
  mName->TextSupport().SetOutlineColor(
      CColor::Modulate(colors.GetHUDHue(), gpTweakGuiColors->GetTextShadowOutlineColor()));
}

CHudBossEnergyInterface::~CHudBossEnergyInterface() { mRoot->SetVisibility(false, kTM_Children); }

void CHudBossEnergyInterface::SetBossParams(const bool visible, const rstl::wstring& name,
                                            float energy, float maxEnergy) {
  mVisible = visible;
  if (visible) {
    mEnergyBar->SetFilledDrainSpeed(1000.f * (0.001f * maxEnergy));
    mEnergyBar->SetCurrEnergy(energy, CAuiEnergyBarT01::kSM_Normal);
    mEnergyBar->SetMaxEnergy(maxEnergy);
    mName->TextSupport().SetText(name);
  }
  mCurrentEnergy = energy;
  mMaximumEnergy = maxEnergy;
}

void CHudBossEnergyInterface::SetAlpha(float alpha) { mAlpha = alpha; }

void CHudBossEnergyInterface::Update(float dt) {
  if (mVisible) {
    mFade = rstl::min_val(1.f, mFade + dt);
  } else {
    mFade = rstl::max_val(0.f, mFade - dt);
  }

  if (mFade > 0.f) {
    mRoot->SetColor(CColor::White().WithAlphaOf(mAlpha * mFade));
    mRoot->SetVisibility(true, kTM_Children);
  } else {
    mRoot->SetVisibility(false, kTM_Children);
  }
}
