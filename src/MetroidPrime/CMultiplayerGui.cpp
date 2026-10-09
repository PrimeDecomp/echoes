#include "MetroidPrime/CMultiplayerGui.hpp"

#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiFrameLoader.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "GuiSys/CGuiWidgetDrawParms.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CGameMode.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/SFX/TimersMultiplayer.h"
#include "MetroidPrime/Tweaks/CTweakGuiColors.hpp"
#include "rstl/StringExtras.hpp"
#include "rstl/math.hpp"
#include <math.h>

static const char* const skScoreWidgetName = "basewidget_score";
static const char* const skScoreTextName = "textpane_score";
static const char* const skScoreBackgroundName = "model_bg";
static const char* const skScoreFlashName = "model_flash";
static const char* const skScoreFillName = "model_bgfill";
static const char* const skTimeWidgetName = "basewidget_time";
static const char* const skTimeTextName = "textpane_time";
static const char* const skTimeBackgroundName = "model_timebg";
static const char* const skTimeFlashName = "model_timeflash";
static const char* const skTimeFillName = "model_timebgfill";

static const char* const skScoreboardFrameNames[] = {"FRME_Scoreboard4", "FRME_Scoreboard2",
                                                     "FRME_Scoreboard4"};

CMultiplayerGui::CMultiplayerGui(const CStateManager& mgr)
: mFrameLoader(rs_new CGuiFrameLoader(
      gpResourceFactory->GetResourceIdByName(skScoreboardFrameNames[mgr.GetViewportLayoutIndex()])
          ->id,
      *gpResourceFactory, *gpSimplePool))
, mReadyFrame(nullptr)
, mTimeWarningSfxCooldown(0.f) {}

void CMultiplayerGui::Update(float dt, const CStateManager& mgr) {
  if (mReadyFrame == nullptr) {
    if (mFrameLoader.get() == nullptr) {
      if (mFrame->GetIsFinishedLoading()) {
        mReadyFrame = mFrame.get();
        BindWidgets(mgr);
      }
    } else {
      CGuiFrame* frame = mFrameLoader->CreateFrame();
      if (frame != nullptr) {
        mFrame = rstl::auto_ptr< CGuiFrame >(frame);
        mFrameLoader = rstl::auto_ptr< CGuiFrameLoader >();
      }
    }
    return;
  }

  mReadyFrame->Update(dt);
  const uint numPlayers = mgr.GetNumPlayers();
  const CGameMode& gameMode = gpGameState->GetGameMode();
  const float flashDelta = 0.5f * dt;
  for (uint i = 0; i < numPlayers; ++i) {
    const uint widgetIndex = numPlayers > 2 ? mgr.GetPlayerState(i)->GetPlayerSelection() : i;
    const int score = gameMode.GetItemAmount(mgr, i);
    const int deaths = mgr.GetPlayerState(i)->GetItemAmount(CPlayerState::kIT_DiedCount);
    mScoreTextPanes[widgetIndex]->TextSupport().SetText(
        CStringExtras::ConvertToUNICODE(rstl::string(CBasics::Stringize("%02d", score))));
    if (gameMode.IsNearScoreLimit(mgr, i)) {
      mScoreTextPanes[widgetIndex]->TextSupport().SetFontColor(
          gpTweakGuiColors->GetMultiplayerWinningScoreTextColor());
    } else {
      mScoreTextPanes[widgetIndex]->TextSupport().SetFontColor(
          gpTweakGuiColors->GetMultiplayerScoreTextColor());
    }
    if (score > mPreviousScores[i]) {
      mScoreIncreaseFlashTimes[i] = 1.f;
      mPreviousScores[i] = score;
    }
    if (score < mPreviousScores[i]) {
      mScoreDecreaseFlashTimes[i] = 1.f;
      mPreviousScores[i] = score;
    }
    if (deaths != mPreviousDeathCounts[i]) {
      mScoreDecreaseFlashTimes[i] = 1.f;
      mPreviousDeathCounts[i] = deaths;
    }
    mScoreDecreaseFlashTimes[i] = rstl::max_val(0.f, mScoreDecreaseFlashTimes[i] - flashDelta);
    mScoreIncreaseFlashTimes[i] = rstl::max_val(0.f, mScoreIncreaseFlashTimes[i] - flashDelta);
    if (CMath::IsEpsilon(mScoreDecreaseFlashTimes[i], 0.f, 1.e-5f) &&
        CMath::IsEpsilon(mScoreIncreaseFlashTimes[i], 0.f, 1.e-5f)) {
      mScoreFlashModels[widgetIndex]->SetVisibility(false, kTM_Children);
    } else {
      CColor color(0.f, 0.f, 0.f, 0.f);
      if (mScoreDecreaseFlashTimes[i] > mScoreIncreaseFlashTimes[i]) {
        if (int(8.f * mScoreDecreaseFlashTimes[i]) & 1) {
          color = gpTweakGuiColors->GetMultiplayerScoreLossColor();
        }
      } else if (int(8.f * mScoreIncreaseFlashTimes[i]) & 1) {
        color = gpTweakGuiColors->GetMultiplayerScoreGainColor();
      }
      mScoreFlashModels[widgetIndex]->SetColor(color);
      mScoreFlashModels[widgetIndex]->SetVisibility(true, kTM_Children);
    }
  }

  if (gameMode.GetMatchTimeLimit() > 0.f) {
    const float elapsedTime = gameMode.GetElapsedTime();
    const float timeLimit = gameMode.GetMatchTimeLimit();
    const float timeRemaining = timeLimit - elapsedTime;
    const int secondsRemaining = int(timeRemaining);
    const rstl::string timeString(
        CBasics::Stringize("%02d:%02d", secondsRemaining / 60, secondsRemaining % 60));
    mTimeTextPane->TextSupport().SetText(CStringExtras::ConvertToUNICODE(timeString));
    const bool periodicWarning = float(fmod(double(timeRemaining - 1.f), 300.0)) > 298.f;
    bool minuteWarning = secondsRemaining <= 60 && secondsRemaining > 58;
    const bool justStarted = gameMode.GetElapsedTime() < 3.f;
    if (justStarted) {
      minuteWarning = false;
    }
    mTimeWarningSfxCooldown = rstl::max_val(0.f, mTimeWarningSfxCooldown - dt);
    if (((!justStarted && periodicWarning) || minuteWarning) && mTimeWarningSfxCooldown <= 0.f) {
      CSfxManager::SfxStart(SFXti2_x_oneminute_00_oneshot, 127, 63, CSfxManager::kAllAreas, false,
                            false, CSfxManager::kMedPriority);
      mTimeWarningSfxCooldown = 2.5f;
    }
    if (secondsRemaining < 11 && mTimeWarningSfxCooldown <= 0.f) {
      CSfxManager::SfxStart(SFXti2_x_countdown_00_oneshot, 127, 63, CSfxManager::kAllAreas, false,
                            false, CSfxManager::kMedPriority);
      mTimeWarningSfxCooldown = 1.f;
    }
    if ((!justStarted && periodicWarning) || minuteWarning || secondsRemaining < 11) {
      mTimeFlash->SetVisibility((int(4.f * timeRemaining) & 1) != 0, kTM_Children);
    } else {
      mTimeFlash->SetVisibility(false, kTM_Children);
    }
  }
}

void CMultiplayerGui::Draw() const {
  if (mReadyFrame != nullptr) {
    mReadyFrame->Draw(CGuiWidgetDrawParms::Default());
  }
}

void CMultiplayerGui::BindWidgets(const CStateManager& mgr) {
  const int numPlayers = mgr.GetNumPlayers();
  const CGameMode& gameMode = gpGameState->GetGameMode();
  static const char* noTimeSuffix = "notime";
  static const char* timeSuffix = "";
  const bool noTimer = gameMode.GetMatchTimeLimit() <= 0.f && numPlayers == 2;
  const char* visibleSuffix = noTimer ? noTimeSuffix : timeSuffix;
  const char* hiddenSuffix = !noTimer ? noTimeSuffix : timeSuffix;
  for (int i = 0; i < 4; ++i) {
    CGuiWidget* widget = mReadyFrame->FindWidget(
        CBasics::Stringize("%s%s%d", skScoreWidgetName, hiddenSuffix, i + 1));
    if (widget != nullptr) {
      widget->SetVisibility(false, kTM_Children);
    }
  }
  for (int i = 0; i < 4; ++i) {
    const int n = i + 1;
    CGuiWidget* widget =
        mReadyFrame->FindWidget(CBasics::Stringize("%s%s%d", skScoreWidgetName, visibleSuffix, n));
    if (widget != nullptr) {
      mScoreWidgets.push_back(widget);
      widget->SetVisibility(false, kTM_Children);
      CGuiTextPane* textPane = static_cast< CGuiTextPane* >(
          mReadyFrame->FindWidget(CBasics::Stringize("%s%s%d", skScoreTextName, visibleSuffix, n)));
      mScoreTextPanes.push_back(textPane);
      textPane->TextSupport().SetFontColor(gpTweakGuiColors->GetMultiplayerScoreTextColor());
      textPane->SetDepthTest(false);
      widget = mReadyFrame->FindWidget(
          CBasics::Stringize("%s%s%d", skScoreBackgroundName, visibleSuffix, n));
      widget->SetColor(gpTweakGuiColors->GetMultiplayerScoreboardDecoColor());
      widget->SetDepthTest(false);
      widget =
          mReadyFrame->FindWidget(CBasics::Stringize("%s%s%d", skScoreFillName, visibleSuffix, n));
      widget->SetColor(gpTweakGuiColors->GetMultiplayerScoreboardBackgroundColor());
      widget->SetDepthTest(false);
      widget =
          mReadyFrame->FindWidget(CBasics::Stringize("%s%s%d", skScoreFlashName, visibleSuffix, n));
      widget->SetVisibility(false, kTM_Children);
      mScoreFlashModels.push_back(widget);
      widget->SetDepthTest(false);
    }
  }
  for (int i = 0; i < int(numPlayers); ++i) {
    const int widgetIndex = numPlayers > 2 ? int(mgr.GetPlayerState(i)->GetPlayerSelection()) : i;
    mScoreWidgets[widgetIndex]->SetVisibility(true, kTM_Children);
    mPreviousScores.push_back(0);
    mPreviousDeathCounts.push_back(0);
    mScoreIncreaseFlashTimes.push_back(0.f);
    mScoreDecreaseFlashTimes.push_back(0.f);
  }
  CGuiWidget* timeBase = mReadyFrame->FindWidget(skTimeWidgetName);
  timeBase->SetVisibility(gameMode.GetMatchTimeLimit() > 0.f, kTM_Children);
  mTimeTextPane = static_cast< CGuiTextPane* >(mReadyFrame->FindWidget(skTimeTextName));
  mTimeTextPane->TextSupport().SetFontColor(gpTweakGuiColors->GetMultiplayerTimerTextColor());
  mTimeTextPane->SetDepthTest(false);
  mTimeBackground = mReadyFrame->FindWidget(skTimeBackgroundName);
  mTimeBackground->SetColor(gpTweakGuiColors->GetMultiplayerScoreboardDecoColor());
  mTimeBackground->SetDepthTest(false);
  mTimeFlash = mReadyFrame->FindWidget(skTimeFlashName);
  mTimeFlash->SetColor(gpTweakGuiColors->GetMultiplayerTimerTextBlinkColor());
  mTimeFlash->SetVisibility(false, kTM_Children);
  mTimeFlash->SetDepthTest(false);
  CGuiWidget* timeFill = mReadyFrame->FindWidget(skTimeFillName);
  if (timeFill != nullptr) {
    timeFill->SetColor(gpTweakGuiColors->GetMultiplayerScoreboardBackgroundColor());
    timeFill->SetDepthTest(false);
  }
}
