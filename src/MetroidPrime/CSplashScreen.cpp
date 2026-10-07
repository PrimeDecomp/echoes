#include "MetroidPrime/CSplashScreen.hpp"

#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Input/IController.hpp"
#include "Kyoto/Text/CInstruction.hpp"
#include "Kyoto/Text/CTextExecuteBuffer.hpp"
#include "Kyoto/Text/CTextRenderBuffer.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CArchitectureQueue.hpp"
#include "MetroidPrime/CGameGlobalObjects.hpp"
#include "MetroidPrime/Decode.hpp"
#include "MetroidPrime/Player/CGameState.hpp"

#include "math.h"

extern bool sProgressiveModePrompt;

static const char* const skSplashScreenTextureNames[CSplashScreen::kSplashScreen_MAX] = {
    nullptr, "TXTR_HealthWarning", "TXTR_NintendoLogoNCL", "TXTR_RetroLogo", "TXTR_DolbyLogoNCL",
};

// Guessed names for the native prompt strings and optional health-warning texture.
static const wchar_t* const skProgressiveQuestion =
    L"Do you want to display in Progressive Mode?\n";
static const wchar_t* const skProgressiveYes = L"Yes     ";
static const wchar_t* const skProgressiveNo = L"No";
static const wchar_t* const skProgressiveEnabled = L"Screen has been set to\nProgressive Mode.";
static const wchar_t* const skProgressiveDisabled = L"Progressive Mode has\nbeen turned off.";
static const char* const skPressStartTextureName = "TXTR_HealthWarningPressStart";

// Guessed name; converts splash artwork coordinates to framebuffer coordinates.
static CVector2f SplashToScreen(const CVector2f& point, bool healthWarning) {
  const float videoWidth = CGraphics::Is50Hz() ? 720.f : 720.f;
  const float videoHeight = CGraphics::Is50Hz() ? 574.f : 480.f;
  const float borderX = (videoWidth - 666.f) * 0.5f;
  const float borderY = (videoHeight - 448.f) * 0.5f;
  const float imageHeight = CGraphics::Is50Hz() ? 528.f : 448.f;
  const GXRenderModeObj& mode = CGraphics::GetRenderMode();
  const float x = (borderX + 666.f * point.GetX() / 608.f - (videoWidth - 660.f) / 2.f) *
                  (float(int(mode.fbWidth)) / 660.f);
  const float healthWarningTop = -23.f;
  const float y = healthWarning ? 448.f * (point.GetY() - healthWarningTop) / 471.f : point.GetY();
  return CVector2f(x, (borderY + 448.f * (448.f - y) / imageHeight - borderY) *
                          (float(int(mode.xfbHeight)) / 448.f));
}

// Guessed name; the progressive-check stage deliberately has no texture.
static rstl::optional_object< TCachedToken< CTexture > > LoadSplashTexture(const char* name) {
  if (name == nullptr) {
    return rstl::optional_object< TCachedToken< CTexture > >();
  }
  return TCachedToken< CTexture >(gpSimplePool->GetObj(name));
}

CSplashScreen::CSplashScreen(ESplashScreen splash)
: CIOWin(rstl::string_l("SplashScreen"))
, mSplash(splash)
, mSplashTimeout(splash == kSplashScreen_HealthWarning ? 61.f : 2.f)
, mProgressiveSelectionTimeout(0.f)
, mProgressivePhase(kPP_Initial)
, mProgressiveMode(true)
, mTexturesLoaded(false)
, mSplashTexture(LoadSplashTexture(skSplashScreenTextureNames[mSplash]))
, mPressStartTexture(LoadSplashTexture(
      splash == kSplashScreen_HealthWarning ? skPressStartTextureName : nullptr)) {
  if (mSplashTexture.valid()) {
    mSplashTexture->Lock();
  }
  if (mPressStartTexture.valid()) {
    mPressStartTexture->Lock();
  }
}

CIOWin::EMessageReturn CSplashScreen::OnMessage(const CArchitectureMessage& message,
                                                CArchitectureQueue& queue) {
  switch (message.GetType()) {
  case kAM_TimerTick: {
    mTexturesLoaded = true;
    if ((mSplashTexture.valid() && !mSplashTexture->IsLoaded()) ||
        (mPressStartTexture.valid() && !mPressStartTexture->IsLoaded())) {
      mTexturesLoaded = false;
    }
    if (!mTexturesLoaded) {
      break;
    }

    if (mProgressivePhase == kPP_Initial) {
      mProgressivePhase = kPP_Complete;
      const bool isProgressiveCheck = mSplash == kSplashScreen_ProgressiveCheck;
      const bool bPressed = gpController->GetGamepadData(0).GetButton(kBU_B).GetIsPressed();
      const bool curProgressiveMode = CGraphics::GetProgressiveMode();
      const bool canSet = CGraphics::CanSetProgressiveMode();
      const bool defaultMode = CGraphics::GetProgressiveDefault();
      if (!sProgressiveModePrompt && isProgressiveCheck && canSet && !curProgressiveMode) {
        mProgressiveMode = canSet && defaultMode;
        CGraphics::SetProgressiveMode(mProgressiveMode);
      } else if (sProgressiveModePrompt && isProgressiveCheck && !curProgressiveMode) {
        if (canSet && (bPressed || defaultMode)) {
          mProgressiveSelectionTimeout = 10.f;
          mProgressivePhase = kPP_Selection;
          mProgressiveMode = true;
        } else {
          mProgressiveMode = false;
          CGraphics::SetProgressiveMode(false);
          sProgressiveModePrompt = false;
        }
      } else {
        mProgressiveMode = false;
        sProgressiveModePrompt = false;
      }
      if (isProgressiveCheck && mProgressivePhase == kPP_Complete) {
        mSplashTimeout = 0.f;
      }
    }

    const float delta = MakeMsg::GetParmTimerTick(message).GetReal();
    mSplashTimeout -= delta;
    if (mProgressiveSelectionTimeout > 0.f) {
      if (mProgressivePhase == kPP_Selection && mSplashTimeout < 0.5f) {
        mSplashTimeout = 0.5f;
        mProgressiveSelectionTimeout -= delta;
        if (mProgressiveSelectionTimeout <= 0.f) {
          mProgressivePhase = kPP_Confirmation;
          CGraphics::SetProgressiveMode(mProgressiveMode);
          mProgressiveSelectionTimeout = 5.f;
        }
      } else if (mProgressivePhase == kPP_Confirmation) {
        mSplashTimeout = 0.5f;
        mProgressiveSelectionTimeout -= delta;
      }
    }

    if (mSplashTimeout <= 0.f) {
      if (mSplash < kSplashScreen_Dolby) {
        int next = mSplash + 1;
        if (gpGameState->PreviousGameResults().mGameMode != 0 &&
            next == kSplashScreen_HealthWarning) {
          ++next;
        }
        queue.Push(
            MakeMsg::CreateCreateIOWin(kAMT_IOWinManager, 9999, 9999,
                                       rs_new CSplashScreen(static_cast< ESplashScreen >(next))));
      }
      return kMR_RemoveIOWinAndExit;
    }
    break;
  }
  case kAM_UserInput: {
    if (!mTexturesLoaded) {
      break;
    }
    const CFinalInput& input = MakeMsg::GetParmUserInput(message).GetUserInput();
    if (mProgressivePhase == kPP_Selection) {
      if (input.DLALeft() || input.DDPLeft()) {
        mProgressiveMode = true;
        mProgressiveSelectionTimeout = 10.f;
      } else if (input.DLARight() || input.DDPRight()) {
        mProgressiveMode = false;
        mProgressiveSelectionTimeout = 10.f;
      } else if (input.PA() || input.PStart()) {
        CGraphics::SetProgressiveMode(mProgressiveMode);
        mProgressiveSelectionTimeout = 5.f;
        mProgressivePhase = kPP_Confirmation;
      }
    } else if (mProgressivePhase == kPP_Confirmation && (input.PA() || input.PStart())) {
      mProgressiveSelectionTimeout = 0.f;
    }

    if (mSplash == kSplashScreen_HealthWarning && 59.f > mSplashTimeout && 0.5f < mSplashTimeout &&
        (input.PStart() || input.PA() || input.PB() || input.PX() || input.PY() || input.PZ() ||
         input.PL() || input.PR())) {
      mSplashTimeout = 0.5f;
    }
    break;
  }
  default:
    break;
  }
  return kMR_Exit;
}

static void DrawSplashTexture(const CTexture& texture, const CColor& color, float left, float top,
                              float right, float bottom);

void CSplashScreen::Draw() const {
  if (!mTexturesLoaded) {
    return;
  }
  const CColor tint = mSplash == kSplashScreen_Nintendo
                          ? CColor(uchar(220), uchar(0), uchar(0), uchar(255))
                          : CColor::White();
  const float alpha =
      mSplash == kSplashScreen_HealthWarning
          ? (mSplashTimeout > 60.5f ? 1.f - (mSplashTimeout - 60.5f) / 0.5f
             : mSplashTimeout > 0.5f ? 1.f
                                     : mSplashTimeout / 0.5f)
          : (mSplashTimeout > 1.5f ? 1.f - (mSplashTimeout - 1.5f) / 0.5f
             : mSplashTimeout > 0.5f ? 1.f
                                     : mSplashTimeout / 0.5f);

  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);
  gpRender->SetModelMatrix(CTransform4f::Identity());
  CGraphics::SetViewPointMatrix(CTransform4f::Identity());
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
  CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  gpRender->SetBlendMode_AlphaBlended();

  rstl::optional_object< TLockedToken< CTexture > > texture;
  const CTexture* tex = nullptr;
  if (mSplashTexture.valid()) {
    texture = TLockedToken< CTexture >(*mSplashTexture);
    tex = **texture;
    tex->Load(GX_TEXMAP0, CTexture::kCM_Clamp);
  }
  const CColor color = tint.WithAlphaOf(alpha);
  if (mSplash == kSplashScreen_ProgressiveCheck) {
    CGraphics::SetOrtho(-10.f, 650.f, -5.5f, 484.5f, -1.f, 1.f);
  } else if (mSplash == kSplashScreen_Nintendo) {
    gpRender->SetViewportOrtho(false, -4096.f, 4096.f);
    const CVector2f splashTopLeft(117.f, 258.f);
    const CVector2f splashBottomRight(493.f, 154.f);
    const CVector2f topLeft = SplashToScreen(splashTopLeft, false);
    const CVector2f bottomRight = SplashToScreen(splashBottomRight, false);
    DrawSplashTexture(*tex, color, topLeft.GetX(), topLeft.GetY(), bottomRight.GetX(),
                      bottomRight.GetY());
  } else if (mSplash == kSplashScreen_HealthWarning) {
    gpRender->SetViewportOrtho(false, -4096.f, 4096.f);
    const CViewport viewport = CGraphics::GetViewport();
    DrawSplashTexture(*tex, color, float(viewport.mLeft), float(viewport.mTop),
                      float(viewport.mLeft + viewport.mWidth),
                      float(viewport.mTop + viewport.mHeight));
    if (mSplashTimeout < 59.f) {
      const float blink = fmodf(59.f - mSplashTimeout, 1.f);
      const float blinkAlpha = blink < 0.5f ? blink / 0.5f : (1.f - blink) / 0.5f;
      const CTexture& pressStart = *mPressStartTexture->GetObject();
      const CVector2f topLeft =
          SplashToScreen(CVector2f(0.f, 359.f + pressStart.GetHeight()), true);
      const CVector2f bottomRight = SplashToScreen(CVector2f(608.f, 359.f), true);
      DrawSplashTexture(pressStart, color.WithAlphaModulatedBy(blinkAlpha), topLeft.GetX(),
                        topLeft.GetY(), bottomRight.GetX(), bottomRight.GetY());
    }
  } else if (mSplash == kSplashScreen_Dolby) {
    gpRender->SetViewportOrtho(false, -4096.f, 4096.f);
    const CVector2f splashTopLeft(189.f, 262.f);
    const CVector2f splashBottomRight(421.f, 150.f);
    const CVector2f topLeft = SplashToScreen(splashTopLeft, false);
    const CVector2f bottomRight = SplashToScreen(splashBottomRight, false);
    DrawSplashTexture(*tex, color, topLeft.GetX(), topLeft.GetY(), bottomRight.GetX(),
                      bottomRight.GetY());
  } else if (tex != nullptr) {
    const CViewport& viewport = CGraphics::GetViewport();
    CGraphics::Render2D(*tex, viewport.mLeft, viewport.mTop, viewport.mWidth, viewport.mHeight,
                        color);
  }

  const CViewport viewport = CGraphics::GetViewport();
  CTextExecuteBuffer text;
  text.AddWordWrapping(true);
  text.BeginBlock(0, 0, viewport.mWidth, viewport.mHeight - 208, false, kTD_Horizontal,
                  kJustification_Center, kVerticalJustification_Bottom);
  text.AddFont(TToken< CRasterFont >(*gpDefaultFont));
  const int bright = static_cast< int >(255.f * alpha);
  const int dim = static_cast< int >(96.f * alpha);
  const CTextColor selected(bright, bright, bright, 255);
  const CTextColor unselected(dim, dim, dim, 255);
  text.AddColor(kCT_Foreground, selected);
  if (mProgressivePhase == kPP_Selection && mSplashTimeout <= 0.5f) {
    const wchar_t* yes = skProgressiveYes;
    const wchar_t* no = skProgressiveNo;
    text.AddString(rstl::wstring_l(skProgressiveQuestion));
    text.AddColor(kCT_Foreground, mProgressiveMode ? selected : unselected);
    text.AddString(rstl::wstring_l(yes));
    text.AddColor(kCT_Foreground, mProgressiveMode ? unselected : selected);
    text.AddString(rstl::wstring_l(no));
  } else if (mProgressivePhase == kPP_Confirmation) {
    sProgressiveModePrompt = false;
    text.AddString(mProgressiveMode ? rstl::wstring_l(skProgressiveEnabled)
                                    : rstl::wstring_l(skProgressiveDisabled));
  }
  text.EndBlock();

  gpRender->SetViewportOrtho(false, -4096.f, 4096.f);
  CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
  CGraphics::SetCullMode(kCM_None);
  const CTransform4f xf =
      CTransform4f::FromColumns(CVector3f::Right(), CVector3f::Forward(), CVector3f::Down(),
                                CVector3f(0.f, 0.f, float(viewport.mHeight)));
  CGraphics::SetModelMatrix(xf);
  text.BuildRenderBuffer().Render(CColor::White(), 0.f);
  CGraphics::SetCullMode(kCM_Front);
}

// Guessed name; native rectangle helper shared by the splash artwork paths.
static void DrawSplashTexture(const CTexture& texture, const CColor& color, float left, float top,
                              float right, float bottom) {
  texture.Load(GX_TEXMAP0, CTexture::kCM_Clamp);
  CGraphics::StreamBegin(kP_TriangleStrip);
  CGraphics::StreamColor(color);
  CGraphics::StreamTexcoord(0.f, 1.f);
  CGraphics::StreamVertex(CVector3f(left, 0.f, bottom));
  CGraphics::StreamTexcoord(0.f, 0.f);
  CGraphics::StreamVertex(CVector3f(left, 0.f, top));
  CGraphics::StreamTexcoord(1.f, 1.f);
  CGraphics::StreamVertex(CVector3f(right, 0.f, bottom));
  CGraphics::StreamTexcoord(1.f, 0.f);
  CGraphics::StreamVertex(CVector3f(right, 0.f, top));
  CGraphics::StreamEnd();
}

CSplashScreen::~CSplashScreen() {}
