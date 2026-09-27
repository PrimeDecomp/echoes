#include "MetroidPrime/CCredits.hpp"

#include "MetroidPrime/CArchMsgParmReal32.hpp"
#include "MetroidPrime/CArchMsgParmUserInput.hpp"
#include "MetroidPrime/CArchitectureMessage.hpp"
#include "MetroidPrime/CArchitectureQueue.hpp"
#include "MetroidPrime/CAutoSave.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CPlayMovie.hpp"
#include "MetroidPrime/CQuitGameScreen.hpp"
#include "MetroidPrime/CSaveGameScreen.hpp"
#include "MetroidPrime/Cameras/CCameraFilterPass.hpp"
#include "MetroidPrime/Decode.hpp"
#include "MetroidPrime/Player/CEnvironmentVariable.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"

#include "Kyoto/Audio/CAudioGroupSet.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Audio/CStreamAudioManager.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/CFrameDelayedKiller.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Graphics/CMoviePlayer.hpp"
#include "Kyoto/Streams/CFilePreload.hpp"
#include "Kyoto/Text/CGuiTextSupport.hpp"
#include "Kyoto/Text/CRasterFont.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "rstl/StringExtras.hpp"
#include "rstl/math.hpp"

static const char* const skJapaneseBodyFont = "FONT_DFSoGei-W5_18";
static rstl::string skMovieDirectory = rstl::string_l("Video/");
static rstl::string skMovieExtension = rstl::string_l(".thp");
static rstl::string skMovieNames[] = {
    rstl::string_l("gameEnding_Part2"), rstl::string_l("gameEnding_Part2B_75"),
    rstl::string_l("gameEnding_Part3"), rstl::string_l("Credits_Loop"),
    rstl::string_l("afterCredits_100"), rstl::string_l("Win_Movie"),
    rstl::string_l("Death_Movie")};
static rstl::string skLanguageSuffixes[] = {
    rstl::string_l(""), rstl::string_l(""), rstl::string_l(""), rstl::string_l(""),
    rstl::string_l(""), rstl::string_l(""), rstl::string_l("")};
static rstl::string skResultsMovieSuffixes[] = {rstl::string_l("_begin"), rstl::string_l("_loop"),
                                                rstl::string_l("_end")};
static const CColor skPulseStartColor(1.f, 1.f, 1.f, 1.f);
static const CColor skPulseEndColor(0.5f, 0.5f, 0.5f, 1.f);
static const CVector3f skTextOffset0 = CVector3f::Zero();
static const CVector3f skTextOffset1 = CVector3f::Zero();
static const CVector3f skTextOffset2 = CVector3f::Zero();
static const CVector3f skTextOffset3 = CVector3f::Zero();

static void QueueIOWin(CArchitectureQueue& queue, CIOWin* win) {
  queue.Push(MakeMsg::CreateCreateIOWin(kAMT_IOWinManager, kFrontEndUIMsgPriority,
                                        kFrontEndUIDrawPriority, win));
}

static const char* const skCreditsAudio = "Audio/echoes-end1-32.dsp";
static const char* const skCompletionAudio = "Audio/samusjak_edit.dsp";

bool CPlayMovie::GetIsContinueDraw() const { return false; }

bool CCredits::GetIsContinueDraw() const { return false; }

bool CMoviePlayer::DrawVideo() const {
  if (!CanDrawVideo()) {
    return false;
  }
  gpRender->SetDepthReadWrite(false, false);
  gpRender->SetViewportOrtho(false, -4096.f, 4096.f);
  const int left = CGraphics::GetViewport().mLeft;
  const int top = CGraphics::GetViewport().mTop;
  const int width = CGraphics::GetViewport().mWidth;
  const int height = CGraphics::GetViewport().mHeight;
  const int movieWidth = GetWidth();
  const int movieHeight = GetHeight();
  const int xMargin = (movieWidth - width) / 2;
  const int yMargin = (movieHeight - height) / 2;
  const_cast< CMoviePlayer* >(this)->DrawFrame(left - xMargin, left + width + xMargin,
                                               top - yMargin, top + height + yMargin);
  return true;
}

void CCredits::DrawText(CGuiTextSupport& text, const CTransform4f& transform) {
  CGraphics::SetCullMode(kCM_None);
  gpRender->SetViewportOrtho(false, -4096.f, 4096.f);
  gpRender->SetModelMatrix(transform);
  CGraphics::SetDepthWriteMode(false, kE_Always, false);
  text.Render();
}

CPlayMovie::CPlayMovie(int which)
: CIOWin(rstl::string_l("PlayMovie"))
, mState(kS_LoadText)
, mWhich(which)
, mMoviePlayer(nullptr)
, mMovieIndex(-1)
, mAudioFile(rstl::string_l("mem:") + skCompletionAudio)
, mCompletionScreenStrings(static_cast< CStringTable* >(nullptr))
, mLargeFont(static_cast< CRasterFont* >(nullptr))
, mTextDelay(gpTweakGui->GetCompletionScreenTextDelay())
, mContinueDelay(2.f)
, mContinueAlpha(0.f)
, mPulseTime(gpTweakGui->GetCompletionScreenPulseTime())
, mPrintedCharacters(0.f)
, mPulseStartColor(skPulseStartColor)
, mPulseEndColor(skPulseEndColor)
, mHideVideo(false)
, mFinished(false)
, mExit(false)
, mContinuePressed(false) {
  CMoviePlayer::SetSfxVolume(static_cast< uchar >(gpGameState->GameOptions().GetSfxVolume()));
  switch (mWhich) {
  case kWM_LoseGame: {
    mQuitScreen = rs_new CQuitGameScreen(kQT_ContinueFromLastSave, 0);
    const rstl::string name = skMovieDirectory + skMovieNames[which] +
                              skLanguageSuffixes[gpMain->GetLanguage()] + skMovieExtension;
    mMovies.push_back(
        rstl::auto_ptr< CMoviePlayer >(rs_new CMoviePlayer(name.data(), 0.f, false, true)));
    break;
  }
  case kWM_Results:
    mCompletionScreenStrings = gpSimplePool->GetObj(gpTweakGui->GetCompletionScreenTable().data());
    mCompletionScreenStrings.Lock();
    mLargeFont = gpSimplePool->GetObj(gpTweakGui->GetCompletionScreenTitleFont().data());
    mLargeFont.Lock();
    for (int i = 0; i < 3; ++i) {
      const rstl::string name =
          skMovieDirectory + skMovieNames[which] + skResultsMovieSuffixes[i] + skMovieExtension;
      const bool loop = i == 1;
      mMovies.push_back(
          rstl::auto_ptr< CMoviePlayer >(rs_new CMoviePlayer(name.data(), 0.1f, loop, false)));
      mMovies.back()->SetVolume(static_cast< uchar >(gpTweakGui->GetResultsMovieVolume()));
    }
    break;
  case kWM_EndingPart2:
  case kWM_EndingPart2B:
  case kWM_EndingPart3:
  case kWM_SpecialEnding: {
    const rstl::string name = skMovieDirectory + skMovieNames[which] + skMovieExtension;
    mMovies.push_back(
        rstl::auto_ptr< CMoviePlayer >(rs_new CMoviePlayer(name.data(), 0.f, false, false)));
    break;
  }
  default:
    break;
  }

  switch (mWhich) {
  case kWM_LoseGame:
    mMovies.back()->SetVolume(static_cast< uchar >(gpTweakGui->GetDeathMovieVolume()));
    break;
  case kWM_EndingPart2:
    mMovies.back()->SetVolume(static_cast< uchar >(gpTweakGui->GetEndingPart2Volume()));
    break;
  case kWM_EndingPart2B:
    mMovies.back()->SetVolume(static_cast< uchar >(gpTweakGui->GetEndingPart2BVolume()));
    break;
  case kWM_EndingPart3:
    mMovies.back()->SetVolume(static_cast< uchar >(gpTweakGui->GetEndingPart3Volume()));
    break;
  case kWM_SpecialEnding:
    mMovies.back()->SetVolume(static_cast< uchar >(gpTweakGui->GetSpecialEndingVolume()));
    break;
  default:
    break;
  }
}

CPlayMovie::~CPlayMovie() {}

CIOWin::EMessageReturn CPlayMovie::OnMessage(const CArchitectureMessage& msg,
                                             CArchitectureQueue& queue) {
  switch (msg.GetType()) {
  case kAM_TimerTick: {
    const float dt = MakeMsg::GetParmTimerTick(msg).GetReal();
    switch (mState) {
    case kS_LoadText:
      if (mWhich == kWM_Results) {
        if (!mCompletionScreenStrings.IsLoaded() || !mLargeFont.IsLoaded()) {
          return kMR_Exit;
        }
        const int width = CGraphics::GetViewport().mWidth;
        const int height = CGraphics::GetViewport().mHeight;
        const SObjectTag* font = gpResourceFactory->GetResourceIdByName(
            gpTweakGui->GetCompletionScreenTitleFont().data());
        mTitleText = rs_new CGuiTextSupport(
            font->GetId(), width, height,
            CGuiTextProperties(false, kJustification_Right, kVerticalJustification_Bottom),
            gpTweakGui->GetCompletionScreenTitleFontColor(),
            gpTweakGui->GetCompletionScreenTitleOutlineColor(), CColor::White(), gpSimplePool);
        mTitleText->SetTypeWriteEffectOptions(true, 1.f, 10.f);
        mTitleText->SetText(rstl::wstring(mCompletionScreenStrings->GetString("MainText")));

        font = gpResourceFactory->GetResourceIdByName(
            gpTweakGui->GetCompletionScreenBodyFont().data());
        mResultsText = rs_new CGuiTextSupport(
            font->GetId(), width, height,
            CGuiTextProperties(false, kJustification_Right, kVerticalJustification_Top),
            gpTweakGui->GetCompletionScreenStatsFontColor(),
            gpTweakGui->GetCompletionScreenStatsOutlineColor(), CColor::White(), gpSimplePool);
        mResultsText->SetTypeWriteEffectOptions(true, 1.f, 15.f);
        if (gpMain->GetRestartMode() != CMain::kRM_EndAutoSave) {
          const int percent = gpGameState->GetPlayerState()->GetItemPercentageRatio();
          mResultsText->AddText(
              rstl::wstring(mCompletionScreenStrings->GetString("PercentageComplete")));
          mResultsText->AddText(CStringExtras::ConvertToUNICODE(
              rstl::string(CBasics::Stringize(" %d%%\n", percent))));
        }
        const int totalMinutes = gpGameState->GetTotalPlayTime() / 60.0;
        int minutes;
        const int hours = totalMinutes / 60.f;
        minutes = totalMinutes - hours * 60;
        mResultsText->AddText(rstl::wstring(mCompletionScreenStrings->GetString("TotalTime")));
        mResultsText->AddText(CStringExtras::ConvertToUNICODE(
            rstl::string(CBasics::Stringize(" %02d:%02d\n", hours, minutes))));

        if (gpMain->GetRestartMode() != CMain::kRM_EndAutoSave) {
          mPulseStartColor = gpTweakGui->GetCompletionScreenUnlockFontColor();
          mPulseEndColor = skPulseEndColor;
          mUnlockText = rs_new CGuiTextSupport(
              font->GetId(), width, height,
              CGuiTextProperties(false, kJustification_Right, kVerticalJustification_Top),
              gpTweakGui->GetCompletionScreenUnlockFontColor(),
              gpTweakGui->GetCompletionScreenUnlockOutlineColor(), CColor::White(), gpSimplePool);
          mUnlockText->SetTypeWriteEffectOptions(true, 1.f, 15.f);
          if (!gpGameState->SystemOptions()
                   .FindEnvironmentVariable("NormalModeCompleted")
                   ->GetValue()) {
            gpGameState->SystemOptions().FindEnvironmentVariable("NormalModeCompleted")->Set(1);
            mUnlockText->AddText(
                rstl::wstring(mCompletionScreenStrings->GetString("HardModeUnlocked")));
            mUnlockText->AddText(CStringExtras::ConvertToUNICODE(rstl::string_l("\n")));
            mUnlockText->AddText(
                rstl::wstring(mCompletionScreenStrings->GetString("GalleryUnlocked")));
          } else if (!gpGameState->SystemOptions()
                          .FindEnvironmentVariable("HardModeCompleted")
                          ->GetValue() &&
                     gpGameState->GetHardModeEnabled()) {
            gpGameState->SystemOptions().FindEnvironmentVariable("HardModeCompleted")->Set(1);
            mUnlockText->AddText(
                rstl::wstring(mCompletionScreenStrings->GetString("HardModeGalleryUnlocked")));
          }
        }
        mContinueText = rs_new CGuiTextSupport(
            font->GetId(), width, height,
            CGuiTextProperties(false, kJustification_Center, kVerticalJustification_Bottom),
            gpTweakGui->GetCompletionScreenUnlockFontColor(),
            gpTweakGui->GetCompletionScreenUnlockOutlineColor(), CColor::White(), gpSimplePool);
        mContinueText->SetText(
            rstl::wstring(mCompletionScreenStrings->GetString("MessageContinue")));
      }
      mState = kS_LoadMovies;
    case kS_LoadMovies:
      if (mWhich == kWM_Results) {
        bool loaded = true;
        for (int i = 0; i < mMovies.size(); ++i) {
          if (!mMovies[i]->ContinueLoading()) {
            mMovies[i]->Update(dt);
            if (!mMovies[i]->GetIsFullyCached()) {
              loaded = false;
            }
          } else {
            loaded = false;
          }
        }
        if (!loaded) {
          break;
        }
      }
      SetMovieIndex(0);
      mState = kS_LoadAudio;
    case kS_LoadAudio:
      if (mWhich == kWM_Results) {
        if (mAudioPreload.null()) {
          mAudioPreload = rs_new CFilePreload(rstl::string_l(skCompletionAudio));
        }
        if (!mAudioPreload->IsReady()) {
          return kMR_Exit;
        }
        const int volume = gpTweakGui->GetCompletionScreenVolume();
        CStreamAudioManager::PlaySoftwareAudio(CStreamAudioManager::kSC_Default, mAudioFile, 0.f,
                                               2.f, static_cast< uchar >(volume), true);
      }
      mState = kS_Playing;
    case kS_Playing:
      if (mMoviePlayer->ContinueLoading()) {
        break;
      }
      mMoviePlayer->Update(dt);
      if (mWhich == kWM_Results) {
        UpdateText(dt);
        if (mMoviePlayer->GetIsMovieFinishedPlaying()) {
          switch (mMovieIndex) {
          case 0:
            SetMovieIndex(1);
            break;
          case 1:
            CStreamAudioManager::StopSoftwareAudio(CStreamAudioManager::kSC_Default, mAudioFile);
            SetMovieIndex(2);
            break;
          case 2:
            mFinished = true;
            break;
          }
        }
      } else {
        switch (mWhich) {
        case kWM_LoseGame:
          if (mMoviePlayer && mMoviePlayer->GetIsMovieFinishedPlaying()) {
            mDvdKeepAlive.Update();
          }
          break;
        default:
          if (mMoviePlayer->GetIsMovieFinishedPlaying()) {
            mFinished = true;
          }
          break;
        }
      }
      if (!mQuitScreen.null()) {
        const EQuitAction action = mQuitScreen->Update(dt);
        if (action == kQA_Yes) {
          gpMain->SetRestartMode(CMain::kRM_StateSetter);
          mFinished = true;
        } else if (action == kQA_No) {
          mFinished = true;
        }
      }
      if (mFinished && CStreamAudioManager::GetSoftwareAudioFileName(
                           CStreamAudioManager::kSC_Default) != mAudioFile) {
        mExit = true;
      }
      if (mExit) {
        CFrameDelayedKiller::StallAndFlushAllAllocations();
        switch (mWhich) {
        case kWM_EndingPart2:
          if (gpMain->GetRestartMode() == CMain::kRM_Credits1) {
            QueueIOWin(queue, rs_new CPlayMovie(kWM_EndingPart3));
          } else {
            QueueIOWin(queue, rs_new CPlayMovie(kWM_EndingPart2B));
          }
          break;
        case kWM_EndingPart2B:
          QueueIOWin(queue, rs_new CPlayMovie(kWM_EndingPart3));
          break;
        case kWM_EndingPart3:
          if (gpMain->GetRestartMode() == CMain::kRM_EndMovie1) {
            QueueIOWin(queue, rs_new CPlayMovie(kWM_SpecialEnding));
          } else {
            QueueIOWin(queue, rs_new CCredits());
          }
          break;
        case kWM_SpecialEnding:
          QueueIOWin(queue, rs_new CCredits());
          break;
        case kWM_Credits:
        case kWM_Results:
          break;
        default:
          break;
        }
        return kMR_RemoveIOWinAndExit;
      }
      break;
    }
    break;
  }
  case kAM_UserInput: {
    const CArchMsgParmUserInput parm = MakeMsg::GetParmUserInput(msg);
    const CFinalInput input = parm.GetUserInput();
    if (static_cast< int >(input.ControllerNumber()) == 0) {
      if (!mQuitScreen.null() && mMoviePlayer && mMoviePlayer->CanDrawVideo()) {
        mHideVideo = mHideVideo | mMoviePlayer->GetIsMovieFinishedPlaying();
        if (mHideVideo) {
          mQuitScreen->ProcessUserInput(input);
        } else if (input.PStart() || input.PA()) {
          mHideVideo = true;
          mMoviePlayer->SetPlayMode(CMoviePlayer::kPM_Stopped);
        }
      } else {
        ProcessUserInput(input);
      }
    }
    break;
  }
  default:
    break;
  }
  return kMR_Exit;
}

void CPlayMovie::Draw() const {
  if (mState == kS_Playing) {
    if (!mHideVideo) {
      DrawVideo();
    }
    if (!mQuitScreen.null()) {
      if (mHideVideo) {
        mQuitScreen->Draw();
      }
    } else if (mWhich == kWM_Results) {
      DrawText();
    }
  }
}

void CPlayMovie::DrawVideo() const {
  if (mMoviePlayer) {
    mMoviePlayer->DrawVideo();
  }
}

void CPlayMovie::DrawText() const {
  if (mMovieIndex == 2 && mMoviePlayer->GetPlayedSeconds() > 0.65f) {
    return;
  }
  gpRender->SetBlendMode_AdditiveAlpha();
  const int height = CGraphics::GetViewport().mHeight;
  const float y = 0.328f * height;
  if (!mTitleText.null()) {
    CTransform4f transform = CTransform4f::Scale(1.f, 1.f, 1.f);
    transform.SetTranslation(CVector3f(-32.f, 0.f, height + y) + skTextOffset0);
    CCredits::DrawText(*mTitleText.get(), transform);
  }
  if (!mResultsText.null()) {
    CTransform4f transform = CTransform4f::Scale(1.f, 1.f, 1.f);
    transform.SetTranslation(CVector3f(-32.f, 0.f, y) + skTextOffset1);
    CCredits::DrawText(*mResultsText.get(), transform);
  }
  if (!mUnlockText.null()) {
    const rstl::pair< CVector2i, CVector2i >& bounds = mResultsText->GetBounds();
    const int textHeight = bounds.second.GetY() - bounds.first.GetY() + 17;
    CTransform4f transform = CTransform4f::Scale(1.f, 1.f, 1.f);
    transform.SetTranslation(CVector3f(-32.f, 0.f, y - textHeight) + skTextOffset2);
    CCredits::DrawText(*mUnlockText.get(), transform);
  }
  if (!mContinueText.null() && mContinueAlpha > 0.f) {
    CTransform4f transform = CTransform4f::Scale(1.f, 1.f, 1.f);
    transform.SetTranslation(CVector3f(0.f, 0.f, height + 32.f) + skTextOffset3);
    CCredits::DrawText(*mContinueText.get(), transform);
  }
}

void CPlayMovie::UpdateText(float dt) {
  mTextDelay -= dt;
  if (!(mTextDelay > 0.f)) {
    mPulseTime += dt;
    if (!mTitleText.null()) {
      mTitleText->Update(dt);
    }
    if (!mResultsText.null()) {
      mResultsText->Update(dt);
      if (mResultsText->GetNumCharactersPrinted() >= mPrintedCharacters + 0.3f) {
        mPrintedCharacters += 0.3f;
        CSfxManager::SfxStart(0x618, 127, 64);
      }
    }
    if (!mUnlockText.null()) {
      float alpha = 0.5f * CMath::FastSinR(
                               1.57079637f *
                               (2.f * (mPulseTime / gpTweakGui->GetCompletionScreenPulseTime()))) +
                    0.5f;
      alpha = rstl::max_val(0.f, alpha);
      alpha = rstl::min_val(alpha, 1.f);
      const CColor fontColor = CColor::Lerp(mPulseStartColor, mPulseEndColor, alpha);
      const CColor outlineColor =
          CColor::Lerp(gpTweakGui->GetCompletionScreenUnlockOutlineColor(), CColor::Grey(), alpha);
      mUnlockText->SetFontColor(fontColor);
      mUnlockText->SetOutlineColor(outlineColor);
      mUnlockText->Update(dt);
    }
    if (!mContinueText.null()) {
      if (mContinuePressed) {
        mContinueAlpha -= dt / 0.5f;
        mContinueAlpha = rstl::max_val(mContinueAlpha, 0.f);
      } else {
        if (mMovieIndex == 1) {
          mContinueDelay -= dt;
          if (mContinueDelay < 0.f) {
            mContinueDelay = 0.f;
            mContinueAlpha += dt / 0.5f;
            mContinueAlpha = rstl::min_val(mContinueAlpha, 1.f);
          }
        }
      }
      mContinueText->SetGeometryColor(CColor(1.f, 1.f, 1.f, mContinueAlpha));
      mContinueText->Update(dt);
    }
  }
}

CIOWin::EMessageReturn CPlayMovie::ProcessUserInput(const CFinalInput& input) {
  if (mMovieIndex == 1 && input.PA() && !mContinuePressed) {
    mMoviePlayer->DisableLoop();
    CSfxManager::SfxStart(0x522, 127, 64);
    mContinuePressed = true;
  }
  return kMR_Exit;
}

void CPlayMovie::SetMovieIndex(int index) {
  mMovieIndex = index;
  if (mMovieIndex != -1) {
    mMoviePlayer = mMovies[mMovieIndex].get();
  } else {
    mMoviePlayer = nullptr;
  }
}

CCredits::CCredits()
: CIOWin(rstl::string_l("Credits"))
, mState(kS_LoadText)
, mCreditsTable(gpSimplePool->GetObj(gpTweakGui->GetCreditsTable().data()))
, mAudioFile(rstl::string_l("mem:") + skCreditsAudio)
, mScrollPosition(0.f)
, mTotalScrollDistance(0.f)
, mScrollSpeed(32.f)
, mTextFadeRemaining(gpTweakGui->GetCreditsTextFadeTime())
, mVideoFadeTime(0.f)
, mFinished(false)
, mVideoFaded(false)
, mTextFaded(false)
, mFadingIn(true)
, mFadingOut(false) {
  CGraphics::SetIsBeginSceneClearFb(true);
  mCreditsTable.Lock();
}

CCredits::~CCredits() {}

CIOWin::EMessageReturn CCredits::OnMessage(const CArchitectureMessage& msg,
                                           CArchitectureQueue& queue) {
  switch (msg.GetType()) {
  case kAM_TimerTick:
    return Update(MakeMsg::GetParmTimerTick(msg).GetReal(), queue);
  case kAM_UserInput: {
    const CArchMsgParmUserInput parm = MakeMsg::GetParmUserInput(msg);
    const CFinalInput input = parm.GetUserInput();
    return ProcessUserInput(input);
  }
  default:
    break;
  }
  return kMR_Exit;
}

CIOWin::EMessageReturn CCredits::Update(float dt, CArchitectureQueue& queue) {
  switch (mState) {
  case kS_LoadText: {
    if (!mCreditsTable.IsLoaded()) {
      return kMR_Exit;
    }
    int height = CGraphics::GetViewport().mHeight;
    int width = CGraphics::GetViewport().mWidth - 64;
    if (mText.empty()) {
      const CStringTable& table = **mCreditsTable;
      for (int i = 0; i < table.GetStringCount(); ++i) {
        rstl::ncrc_ptr< CGuiTextSupport > text = rs_new CGuiTextSupport(
            gpResourceFactory->GetResourceIdByName(gpTweakGui->GetCreditsFont().data())->GetId(),
            width, 0, CGuiTextProperties(true, kJustification_Center, kVerticalJustification_Top),
            gpTweakGui->GetCreditsFontColor(), gpTweakGui->GetCreditsOutlineColor(),
            CColor::White(), gpSimplePool);
        text->SetText(rstl::wstring_l(table.GetString(i)));
        mText.push_back(
            rstl::pair< rstl::ncrc_ptr< CGuiTextSupport >, CVector2i >(text, CVector2i()));
      }
    }

    for (TextList::iterator it = mText.begin(); it != mText.end(); ++it) {
      if (!it->first->GetIsTextSupportFinishedLoading()) {
        return kMR_Exit;
      }
    }
    int totalHeight = 0;
    for (TextList::iterator it = mText.begin(); it != mText.end(); ++it) {
      const rstl::pair< CVector2i, CVector2i >& bounds = it->first->GetBounds();
      const int textHeight = bounds.second.GetY() - bounds.first.GetY();
      it->second.SetY(textHeight);
      it->second.SetX(totalHeight);
      it->first->SetExtentX(width);
      it->first->SetExtentY(textHeight);
      totalHeight += textHeight;
    }
    const float halfHeight = height / 2.f;
    mTotalScrollDistance = totalHeight + halfHeight;
    mScrollSpeed = mTotalScrollDistance / (gpTweakGui->GetCreditsTotalTime() -
                                           rstl::max_val(gpTweakGui->GetCreditsMovieFadeTime(),
                                                         gpTweakGui->GetCreditsTextFadeTime()));
    mState = kS_LoadMovie;
  }
  case kS_LoadMovie: {
    if (mMoviePlayer.null()) {
      const rstl::string movieName = skMovieDirectory + skMovieNames[3] + skMovieExtension;
      mMoviePlayer = rs_new CMoviePlayer(movieName.data(), 0.f, true, true);
    }
    mState = kS_LoadAudio;
  }
  case kS_LoadAudio: {
    if (mAudioPreload.null()) {
      mAudioPreload = rs_new CFilePreload(rstl::string_l(skCreditsAudio));
    }
    if (!mAudioPreload->IsReady()) {
      return kMR_Exit;
    }
    const int volume = gpTweakGui->GetCreditsVolume();
    CStreamAudioManager::PlaySoftwareAudio(CStreamAudioManager::kSC_Default, mAudioFile, 0.f,
                                           gpTweakGui->GetCreditsMovieFadeTime(),
                                           static_cast< uchar >(volume), true);
    mState = kS_Playing;
  }
  case kS_Playing: {
    if (mMoviePlayer->ContinueLoading()) {
      break;
    }
    CGraphics::SetIsBeginSceneClearFb(true);
    mMoviePlayer->Update(dt);
    if (mFinished) {
      mFadingOut = true;
      if (mFadingIn) {
        mFadingIn = false;
        mVideoFadeTime = gpTweakGui->GetCreditsMovieFadeTime() - mVideoFadeTime;
      }
      CStreamAudioManager::StopSoftwareAudio(CStreamAudioManager::kSC_Default, mAudioFile);
    }
    if (mFadingIn || mFadingOut) {
      mVideoFadeTime =
          CMath::Clamp(0.f, mVideoFadeTime + dt, gpTweakGui->GetCreditsMovieFadeTime());
      if (mVideoFadeTime == gpTweakGui->GetCreditsMovieFadeTime()) {
        if (mFadingIn) {
          mFadingIn = false;
          mVideoFadeTime = 0.f;
        } else if (mFadingOut) {
          mVideoFaded = true;
        }
      }
    }
    mScrollPosition = rstl::min_val(mTotalScrollDistance, dt * mScrollSpeed + mScrollPosition);
    if (mScrollPosition >= mTotalScrollDistance || mFinished) {
      mFinished = true;
      mTextFadeRemaining = rstl::max_val(0.f, mTextFadeRemaining - dt);
      const float alpha = mTextFadeRemaining / gpTweakGui->GetCreditsTextFadeTime();
      for (TextList::iterator it = mText.begin(); it != mText.end(); ++it) {
        CGuiTextSupport& text = *it->first;
        text.SetGeometryColor(CColor::White().WithAlphaModulatedBy(alpha));
      }
      if (mTextFadeRemaining <= 0.f) {
        mTextFaded = true;
      }
    }
    if (mTextFaded && mVideoFaded &&
        CStreamAudioManager::GetSoftwareAudioFileName(CStreamAudioManager::kSC_Default) !=
            mAudioFile) {
      QueueIOWin(queue, rs_new CPlayMovie(5));
      return kMR_RemoveIOWinAndExit;
    }
    break;
  }
  default:
    break;
  }
  return kMR_Exit;
}

CIOWin::EMessageReturn CCredits::ProcessUserInput(const CFinalInput& input) { return kMR_Exit; }

void CCredits::Draw() const {
  if (mState != kS_Playing) {
    return;
  }
  DrawVideo();
  DrawText();
}

void CCredits::DrawText() const {
  const rstl::pair< CVector2f, CVector2f > region =
      gpRender->SetViewportOrtho(false, -4096.f, 4096.f);
  const float top = mScrollPosition;
  const float bottom = top - (region.second.GetY() - region.first.GetY());
  const float width = region.second.GetX();
  for (TextList::const_iterator it = mText.begin(); it != mText.end(); ++it) {
    const int offset = it->second.GetX();
    const int height = it->second.GetY();
    const int scaledOffset = static_cast< int >(static_cast< float >(offset));
    const int end = static_cast< int >(static_cast< float >(height) + scaledOffset);
    if (!(bottom > end) && !(top < scaledOffset)) {
      const int textWidth = it->first->GetTextBoundingWidth();
      const float scrollOffset = mScrollPosition - offset;
      CTransform4f transform = CTransform4f::Scale(1.f, 1.f, 1.f);
      transform.SetTranslation(CVector3f(0.5f * (width - textWidth), 0.f, scrollOffset));
      CGuiTextSupport& text = *it->first;
      gpRender->SetBlendMode_AdditiveAlpha();
      DrawText(text, transform);
    }
  }
  CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Multiply, CCameraFilterPass::kFS_CinemaBars,
                                CColor::Black(), nullptr, 1.9f);
}

void CCredits::DrawVideo() const {
  if (mMoviePlayer.get() && mMoviePlayer->DrawVideo() && (mFadingIn || mFadingOut)) {
    float alpha = mVideoFadeTime / gpTweakGui->GetCreditsMovieFadeTime();
    if (mFadingIn) {
      alpha = 1.f - alpha;
    }
    alpha = CMath::Clamp(0.f, alpha, 1.f);
    const CColor filterColor = CColor::Black().WithAlphaOf(alpha);
    CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Blend, CCameraFilterPass::kFS_Fullscreen,
                                  filterColor, nullptr, 1.f);
  }
}

CAutoSave::CAutoSave()
: CIOWin(rstl::string_l("AutoSave"))
, mState(kS_LoadAudio)
, mSaveGameScreen(rs_new CSaveGameScreen(kSC_InGame, gpGameState->GetCardSerial()))
, mAudioGroup(gpSimplePool->GetObj("UIMemory_AGSC")) {
  gpMain->StreamNewGameState(true);
  CGraphics::SetIsBeginSceneClearFb(true);
  mAudioGroup.Lock();
}

CAutoSave::~CAutoSave() {}

CIOWin::EMessageReturn CAutoSave::OnMessage(const CArchitectureMessage& msg,
                                            CArchitectureQueue& queue) {
  if (gpGameState->GetCardSerial() == 0) {
    return kMR_RemoveIOWinAndExit;
  }

  switch (msg.GetType()) {
  case kAM_TimerTick: {
    const float dt = MakeMsg::GetParmTimerTick(msg).GetReal();
    switch (mState) {
    case kS_LoadAudio:
      if (!mAudioGroup.IsLoaded()) {
        return kMR_Exit;
      }
      mState = kS_Saving;
    case kS_Saving: {
      const EMessageReturn result = mSaveGameScreen->Update(dt);
      mDvdKeepAlive.Update();
      if (result != kMR_Normal) {
        return kMR_RemoveIOWinAndExit;
      }
      break;
    }
    }
    break;
  }
  case kAM_UserInput: {
    const CArchMsgParmUserInput parm = MakeMsg::GetParmUserInput(msg);
    const CFinalInput input = parm.GetUserInput();
    if (mState == kS_Saving && static_cast< int >(input.ControllerNumber()) == 0) {
      mSaveGameScreen->ProcessUserInput(input);
    }
    break;
  }
  default:
    break;
  }
  return kMR_Exit;
}

void CAutoSave::Draw() const {
  if (mState == kS_Saving) {
    mSaveGameScreen->Draw();
  }
}

CDvdKeepAlive::CDvdKeepAlive()
: mFile("TestAnim.pak")
, mBuffer(static_cast< uchar* >(CMemory::Alloc(0x400, IAllocator::kHI_RoundUpLen)))
, mReadOffset(0) {}

CDvdKeepAlive::~CDvdKeepAlive() {
  if (!mRequest.null()) {
    mRequest->PostCancelRequest();
    mRequest->WaitUntilComplete();
  }
}

void CDvdKeepAlive::Update() {
  if (mRequest.null() || mRequest->IsComplete()) {
    const int length = mFile.GetFileSize();
    mReadOffset += 0x400;
    if (length - mReadOffset < 0x400) {
      mReadOffset = 0;
    }
    mRequest = mFile.AsyncSeekRead(mBuffer.get(), 0x400, kSO_Set, mReadOffset);
  }
}

bool CAutoSave::GetIsContinueDraw() const { return false; }
