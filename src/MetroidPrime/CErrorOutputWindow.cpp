#include "MetroidPrime/CErrorOutputWindow.hpp"

#include "MetroidPrime/CArchitectureMessage.hpp"
#include "MetroidPrime/CGameGlobalObjects.hpp"

#include "Kyoto/Audio/CStreamAudioManager.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/CMemoryCardSys.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CMoviePlayer.hpp"
#include "Kyoto/Input/IController.hpp"
#include "Kyoto/Text/CInstruction.hpp"
#include "Kyoto/Text/CTextExecuteBuffer.hpp"
#include "Kyoto/Text/CTextRenderBuffer.hpp"
#include "MetaRender/CCubeRenderer.hpp"

#include "dolphin/dvd.h"

CErrorOutputWindow::CErrorOutputWindow(EFlag flag)
: CIOWin(rstl::string_l("Error output window"))
, mState(kS_Zero)
, x18_25_(true)
, x18_26_(true)
, x18_27_(true)
, x18_28_(flag == kF_Zero)
, mMsg(nullptr) {}

bool CErrorOutputWindow::GetIsContinueDraw() const { return mState != kS_One; }

CIOWin::EMessageReturn CErrorOutputWindow::OnMessage(const CArchitectureMessage& msg,
                                                     CArchitectureQueue&) {
  switch (msg.GetType()) {
  case kAM_UserInput:
    return mState != kS_Zero ? kMR_Exit : kMR_Normal;

  case kAM_FrameBegin:
    UpdateWindow();
    // fallthrough

  case kAM_TimerTick:
  case kAM_FrameEnd:
    return mState != kS_Zero ? kMR_Exit : kMR_Normal;
  default:
    break;
  }
  return kMR_Normal;
}

void CErrorOutputWindow::UpdateWindow() {
  int driveStatus = DVDGetDriveStatus();
  const wchar_t* errMsg = nullptr;
  bool flagThing = mState != kS_Zero;
  if (CMemoryCardSys::mIsCardBusy) {
    driveStatus = 0;
  }
  static int sLastDvdStatus = 0;
  if (driveStatus != sLastDvdStatus) {
    sLastDvdStatus = driveStatus;
  }
  switch (driveStatus) {
  case 5:
    errMsg = L"The Disc Cover is open.\nIf you want to continue the game,\nplease close the Disc "
             L"Cover.";
    break;
  case 4:
  case 6:
    errMsg = L"Please insert the\nMetroid Prime 2 Echoes Game Disc.";
    break;
  case 0xb:
    errMsg = L"The Game Disc could not be read.\nPlease read the Nintendo GameCube\nInstruction "
             L"Booklet\nfor more information.";
    break;
  default:
    break;
  }
  if (driveStatus != 2 && driveStatus != 1) {
    flagThing = errMsg != nullptr;
    if (errMsg != nullptr) {
      mMsg = errMsg;
    }
  }
  if (!flagThing) {
    if (mState != kS_Zero) {
      SetState(kS_Zero);
    }
  } else {
    SetState(kS_One);
  }
}

void CErrorOutputWindow::Draw() const {
  switch (mState) {
  case kS_Zero:
    break;
  case kS_One:
    DrawError();
    if (gpRender != nullptr) {
      gpRender->SetRequestRGBA6(true);
    }
    break;
  }
}

void CErrorOutputWindow::DrawError() const {
  if (!mMsg) {
    return;
  }

  CViewport viewport = CGraphics::GetViewport();
  CTextExecuteBuffer execBuffer;
  execBuffer.AddWordWrapping(true);
  execBuffer.BeginBlock(0, 0, viewport.mWidth, viewport.mHeight, false, kTD_Horizontal,
                        kJustification_Center, kVerticalJustification_Center);
  execBuffer.AddFont(TToken< CRasterFont >(*gpDefaultFont));
  execBuffer.AddString(rstl::wstring_l(mMsg));
  execBuffer.EndBlock();

  if (x18_28_) {
    gpRender->SetBlendMode_AlphaBlended();
    rstl::pair< CVector2f, CVector2f > vp = gpRender->SetViewportOrtho(true, -4096.f, 4096.f);
    const CVector2f& lt = vp.first;
    const CVector2f& rb = vp.second;
    gpRender->SetDepthReadWrite(false, false);
    gpRender->BeginTriangleStrip(4);
    gpRender->PrimColor(CColor::Black().WithAlphaOf(1.f));
    gpRender->PrimVertex(CVector3f(lt.GetX() - 1.f, 0.f, 1.f + rb.GetY()));
    gpRender->PrimVertex(CVector3f(lt.GetX() - 1.f, 0.f, lt.GetY() - 1.f));
    gpRender->PrimVertex(CVector3f(1.f + rb.GetX(), 0.f, 1.f + rb.GetY()));
    gpRender->PrimVertex(CVector3f(1.f + rb.GetX(), 0.f, lt.GetY() - 1.f));
    gpRender->EndPrimitive();
    gpRender->SetBlendMode_AlphaBlended();
  }

  const float top = CCast::LtoF(viewport.mTop);
  CGraphics::SetViewPointMatrix(CTransform4f::Identity());
  CGraphics::SetOrtho(viewport.mLeft, viewport.mLeft + viewport.mWidth,
                      viewport.mTop + viewport.mHeight, top, -4096.f, 4096.f);
  CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
  CGraphics::SetCullMode(kCM_None);
  CGraphics::SetDepthWriteMode(true, kE_Always, false);
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);
  const CTransform4f xf =
      CTransform4f::FromColumns(CVector3f::Right(), CVector3f::Forward(), CVector3f::Down(),
                                CVector3f(0.f, 0.f, viewport.mHeight));
  CGraphics::SetModelMatrix(xf);
  execBuffer.BuildRenderBuffer().Render(CColor::White(), 0.f);
  CGraphics::SetCullMode(kCM_Front);
}

void CErrorOutputWindow::SetState(EState state) {
  if (state != kS_Zero && gpController != nullptr) {
    for (int i = 0; i < 4; ++i) {
      gpController->SetMotorState(static_cast< EIOPort >(i), kMS_Stop);
    }
  }

  if (state != mState) {
    if (state != kS_Zero) {
      if (gpRender != nullptr) {
        gpRender->SetRequestRGBA6(true);
      }
      if (x18_28_) {
        x18_26_ = CStreamAudioManager::GetMusicUnmute();
        x18_27_ = CStreamAudioManager::GetSfxUnmute();
        x18_25_ = CMoviePlayer::GetAudioEnabled();
        CStreamAudioManager::SetMusicUnmute(false);
        CStreamAudioManager::SetSfxUnmute(false);
        CMoviePlayer::SetAudioEnabled(false);
      }
    } else if (x18_28_) {
      CStreamAudioManager::SetMusicUnmute(x18_26_);
      CStreamAudioManager::SetSfxUnmute(x18_27_);
      CMoviePlayer::SetAudioEnabled(x18_25_);
    }
    mState = state;
  }
}

void CErrorOutputWindow::Update() { UpdateWindow(); }

void CErrorOutputWindow::ShowMessage() const {
  if (!mMsg) {
    return;
  }

  DrawError();
}
