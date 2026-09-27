#include "MetroidPrime/CCredits.hpp"

#include "MetroidPrime/CArchMsgParmReal32.hpp"
#include "MetroidPrime/CArchMsgParmUserInput.hpp"
#include "MetroidPrime/CArchitectureMessage.hpp"
#include "MetroidPrime/CArchitectureQueue.hpp"
#include "MetroidPrime/CPlayMovie.hpp"
#include "MetroidPrime/Cameras/CCameraFilterPass.hpp"
#include "MetroidPrime/Decode.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"

#include "Kyoto/Audio/CStreamAudioManager.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Graphics/CMoviePlayer.hpp"
#include "Kyoto/Streams/CFilePreload.hpp"
#include "Kyoto/Text/CGuiTextSupport.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "rstl/math.hpp"

static rstl::string skMovieDirectory = rstl::string_l("Video/");
static rstl::string skMovieExtension = rstl::string_l(".thp");
static rstl::string skMovieNames[] = {
    rstl::string_l("gameEnding_Part2"), rstl::string_l("gameEnding_Part2B_75"),
    rstl::string_l("gameEnding_Part3"), rstl::string_l("Credits_Loop"),
    rstl::string_l("afterCredits_100"), rstl::string_l("Win_Movie"),
    rstl::string_l("Death_Movie")};

static void QueueIOWin(CArchitectureQueue& queue, CIOWin* win) {
  queue.Push(MakeMsg::CreateCreateIOWin(kAMT_IOWinManager, kFrontEndUIMsgPriority,
                                        kFrontEndUIDrawPriority, win));
}

static const char* const skCreditsAudio = "Audio/echoes-end1-32.dsp";

bool CCredits::GetIsContinueDraw() const { return false; }

void CCredits::DrawText(CGuiTextSupport& text, const CTransform4f& transform) {
  CGraphics::SetCullMode(kCM_None);
  gpRender->SetViewportOrtho(false, -4096.f, 4096.f);
  gpRender->SetModelMatrix(transform);
  CGraphics::SetDepthWriteMode(false, kE_Always, false);
  text.Render();
}

CCredits::CCredits()
: CIOWin(rstl::string_l("Credits"))
, mState(0)
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
  case 0: {
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
    mState = 1;
  }
  case 1: {
    if (mMoviePlayer.null()) {
      const rstl::string movieName = skMovieDirectory + skMovieNames[3] + skMovieExtension;
      mMoviePlayer = rs_new CMoviePlayer(movieName.data(), 0.f, true, true);
    }
    mState = 2;
  }
  case 2: {
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
    mState = 3;
  }
  case 3: {
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
  if (mState != 3) {
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
