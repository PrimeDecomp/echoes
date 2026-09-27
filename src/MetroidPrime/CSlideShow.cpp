#include "MetroidPrime/CSlideShow.hpp"

#include "GuiSys/CGuiTextSupport.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Audio/CStaticAudioPlayer.hpp"
#include "Kyoto/CDependencyGroup.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CEnvironmentVariable.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Tweaks/CTweakContents.hpp"
#include "rstl/math.hpp"

static CVector2f sZeroVector(0.f, 0.f);

static int GetStickDirection(float up, float down, float left, float right) {
  uchar direction = 0;
  if (up > 0.f) {
    direction |= 1;
  }
  if (down > 0.f) {
    direction |= 2;
  }
  if (left > 0.f) {
    direction |= 4;
  }
  if (right > 0.f) {
    direction |= 8;
  }
  switch (direction) {
  case 1:
    return 1;
  case 2:
    return 5;
  case 4:
    return 3;
  case 5:
    return 2;
  case 6:
    return 4;
  case 8:
    return 7;
  case 9:
    return 8;
  case 10:
    return 6;
  default:
    return 0;
  }
}

static void DrawTexture(const rstl::auto_ptr< TToken< CTexture > >& token, const CVector3f& offset,
                        const CColor& color, const CVector2f* viewportOffset,
                        const CVector2f* viewportSize) {
  // TODO: Set the orthographic tiled viewport and stream the textured quad.
}

CSlideShow::CSlideShow()
: CIOWin(rstl::string_l("SlideShow"))
, mPhase(0)
, x38_(0)
, mTotalSlides(0)
, mGallery(0)
, mSlide(-1)
, mCrossfadeTimer(0.f)
, mRepeatTimer(0.f)
, mIdleTimer(0.f)
, mSlideNumberTimer(0.f)
, mLStick(0)
, mCStick(0)
, mLTrigger(0)
, mRTrigger(0)
, mSlideNumberOffset(32.f)
, mFadeTimer(gpTweakContents->TweakSlideShow.fadeInTime)
, mControlsAlpha(1.f)
, mShowControls(true)
, mShowSlideNumber(true)
, mDisableInput(false)
, mExit(false)
, mIntroFade(true)
, mOutroFade(false)
, mGalleryChanged(true)
, mLoadMusic(true) {
  mSlideA.mParent = this;
  mSlideB.mParent = this;
  // TODO: Request the pak, create the three text supports, and preload controller icon tokens.
}

CSlideShow::~CSlideShow() {
  // TODO: Unload the slideshow pak; owned members already perform their own cleanup.
}

uchar CSlideShow::GetGalleriesUnlocked() {
  uchar flags = 0;
  if (gpGameState != nullptr) {
    CPersistentOptions& options = gpGameState->SystemOptions();
    const int percent = options.FindEnvironmentVariable("PercentScans")->GetValue();
    if (percent >= 40) {
      flags |= 1;
    }
    if (percent >= 60) {
      flags |= 2;
    }
    if (percent >= 80) {
      flags |= 4;
    }
    if (percent >= 100) {
      flags |= 8;
    }
    if (options.FindEnvironmentVariable("NormalModeCompleted")->GetValue() != 0) {
      flags |= 16;
    }
    if (options.FindEnvironmentVariable("HardModeCompleted")->GetValue() != 0) {
      flags |= 32;
    }
  }
  return flags;
}

void CSlideShow::BuildGalleryLists(uint flags) {
  // TODO: Filter dependencies/labels by unlocked galleries and recover each slide's tile grid.
}

bool CSlideShow::LoadTXTRDep(const char* name) {
  const SObjectTag* tag = gpResourceFactory->GetResourceIdByName(name);
  if (tag == nullptr || tag->type != 'DGRP') {
    return false;
  }
  mGalleryTXTRDeps.reserve(mGalleryTXTRDeps.size() + 1);
  mGalleryTXTRDeps.push_back(TToken< CDependencyGroup >(gpSimplePool->GetObj(*tag)));
  return true;
}

CIOWin::EMessageReturn CSlideShow::OnMessage(const CArchitectureMessage& msg,
                                             CArchitectureQueue& queue) {
  // TODO: Restore the asynchronous pak/dependency/string/audio phases and timer/input dispatch.
  return kMR_Exit;
}

void CSlideShow::Draw() const {
  // TODO: Draw both crossfading slides, text, and the intro/outro fullscreen filter.
}

bool IsDataLoreResearchScan(CAssetId id) { return true; }

rstl::pair< int, int > CStateManager::CalculateScanCompletionRate() const {
  // TODO: Count completed player scans whose logbook ancestors include the research category.
  return rstl::pair< int, int >(0, 0);
}

CAssetId UpdatePersistentScanPercent(int previous, int current, int total) {
  // TODO: Update PercentScans and choose the unlock-message resource at each new threshold.
  return kInvalidAssetId;
}

CIOWin::EMessageReturn CSlideShow::ProcessUserInput(const CFinalInput& input) {
  // TODO: Handle exit, controls visibility, gallery selection, slide repeat, and viewport input.
  return kMR_Exit;
}

CIOWin::EMessageReturn CSlideShow::AdvanceSlide(bool forward) {
  if (!mGalleries.empty()) {
    CSfxManager::SfxStart(0x5b6, 127, 64);
    mSlide += forward ? 1 : -1;
    const int gallery = mGallery;
    if (mSlide < 0) {
      --mGallery;
      mGalleryChanged = true;
    } else if (mSlide >= mGalleries[mGallery].mSlides.size()) {
      ++mGallery;
      mGalleryChanged = true;
    }
    if (mGallery < 0) {
      mGallery = mGalleries.size() - 1;
      mSlide = mGalleries[mGallery].mSlides.size() - 1;
    } else if (mGallery >= mGalleries.size()) {
      mSlide = 0;
      mGallery = 0;
    } else if (mGallery > gallery) {
      mSlide = 0;
    } else if (mGallery < gallery) {
      mSlide = mGalleries[mGallery].mSlides.size() - 1;
    }
  }
  return kMR_Exit;
}

void CSlideShow::LoadSlide() {
  // TODO: Queue the selected gallery's tile range in mSlideB and initialize its viewport.
}

void CSlideShow::SetShowControls(bool show) { mShowControls = show; }

bool CSlideShow::IsControlsAnimating() const { return mControlsAlpha > 0.f; }

void CSlideShow::UpdateControls(float dt) {
  if (!mControlsText.null()) {
    mControlsAlpha = CMath::Clamp(0.f, mControlsAlpha + 2.f * (mShowControls ? dt : -dt), 1.f);
    mControlsText->Update(dt);
  }
}

void CSlideShow::UpdateControlsText(const CFinalInput& input) {
  if (mControlsText.null()) {
    return;
  }
  const CControlMapper& mapper = gpGameState->ControlMapper();
  mCStick = GetStickDirection(mapper.GetAnalogInput(CControlMapper::kC_MapMoveForward, input),
                              mapper.GetAnalogInput(CControlMapper::kC_MapMoveBack, input),
                              mapper.GetAnalogInput(CControlMapper::kC_MapMoveLeft, input),
                              mapper.GetAnalogInput(CControlMapper::kC_MapMoveRight, input));
  mLStick = GetStickDirection(mapper.GetAnalogInput(CControlMapper::kC_MapCircleDown, input),
                              mapper.GetAnalogInput(CControlMapper::kC_MapCircleUp, input),
                              mapper.GetAnalogInput(CControlMapper::kC_MapCircleLeft, input),
                              mapper.GetAnalogInput(CControlMapper::kC_MapCircleRight, input));
  mRTrigger = mapper.GetAnalogInput(CControlMapper::kC_MapZoomIn, input) > 0.f ? 1 : 0;
  mLTrigger = mapper.GetAnalogInput(CControlMapper::kC_MapZoomOut, input) > 0.f ? 1 : 0;
  // TODO: Assemble image tags and named strings from mGalleryNames, then set the controls text.
}

void CSlideShow::SetPanSfx(bool active) {
  if (active) {
    if (!mPanSfx) {
      mPanSfx = CSfxManager::SfxStart(0x5b4, 127, 64, CSfxManager::kAllAreas, false, true);
    }
  } else {
    CSfxManager::SfxStop(mPanSfx);
    mPanSfx.Clear();
  }
}

void CSlideShow::SetZoomSfx(bool active) {
  if (active) {
    if (!mZoomSfx) {
      mZoomSfx = CSfxManager::SfxStart(0x5b5, 127, 64, CSfxManager::kAllAreas, false, true);
    }
  } else {
    CSfxManager::SfxStop(mZoomSfx);
    mZoomSfx.Clear();
  }
}

void CSlideShow::UpdateMusicVolume(float time, float fadeTime) {
  // TODO: Scale the music preference by the clamped fade ratio and start mixing the audio.
}

void CSlideShow::SetTexturesLocked(rstl::vector< CToken >& textures, bool locked) {
  for (int i = 0; i < textures.size(); ++i) {
    if (locked) {
      textures[i].Lock();
    } else {
      textures[i].Unlock();
    }
  }
}

void CSlideShow::SetDependenciesLocked(rstl::vector< TToken< CDependencyGroup > >& deps,
                                       bool locked) {
  for (int i = 0; i < deps.size(); ++i) {
    if (locked) {
      deps[i].Lock();
    } else {
      deps[i].Unlock();
    }
  }
}

bool CSlideShow::AreAllDepsLoaded(const rstl::vector< TToken< CDependencyGroup > >& deps) const {
  for (int i = 0; i < deps.size(); ++i) {
    if (!deps[i].IsLoaded()) {
      return false;
    }
  }
  return true;
}

void CSlideShow::DrawSlideNumber() const {
  // TODO: Position and fade the slide number and the newly selected gallery's label.
}

void CSlideShow::UpdateSlideNumber(float dt) {
  // TODO: Format the cumulative slide number and update its text support.
}

void CSlideShow::DrawControls() const {
  // TODO: Render the controls text using the controls and screen-fade alpha values.
}

CIOWin::EMessageReturn CSlideShow::SSlideData::ProcessUserInput(const CFinalInput& input) {
  // TODO: Apply mapped pan/zoom input, constrain the tiled viewport, and update sound effects.
  return kMR_Exit;
}

void CSlideShow::SSlideData::InitializeViewport() {
  if (!IsLoaded()) {
    return;
  }
  const float width = (**mTextures.front().mToken)->GetWidth();
  const float height = (**mTextures.front().mToken)->GetHeight();
  mTextureWidth = width * mColumns;
  mTextureHeight = height * (mTextures.size() / mColumns);
  const float textureAspect = mTextureWidth / mTextureHeight;
  const CViewport& viewport = CGraphics::GetViewport();
  const float aspect = float(viewport.mWidth) / viewport.mHeight;
  mVpOffset = sZeroVector;
  if (textureAspect != aspect) {
    if (textureAspect > aspect) {
      mVpSize = CVector2f(mTextureWidth, mTextureWidth / aspect);
    } else {
      mVpSize = CVector2f(mTextureHeight * aspect, mTextureHeight);
    }
  }
  for (int i = 0; i < mTextures.size(); ++i) {
    const float x = width * (i % mColumns);
    const float y = height * (i / mColumns);
    mTextures[i].mRightTop = CVector2f(x + width, y);
    mTextures[i].mLeftBottom = CVector2f(x, y + height);
  }
  mCanvasSize = mVpSize;
  mReady = true;
}

const bool CSlideShow::SSlideData::IsLoaded() const {
  if (mTextures.empty()) {
    return false;
  }
  const bool loaded = !mTextures.front().mToken.null() && mTextures.front().mToken->IsLoaded();
  if (!mStopLoading) {
    for (int i = 0; i < mTextures.size(); ++i) {
      if (!mTextures[i].mToken.null()) {
        mTextures[i].mToken->Lock();
        if (!mTextures[i].mToken->IsLoaded()) {
          break;
        }
        mTextures[i].mAlpha = rstl::min_val(mTextures[i].mAlpha + 0.01f, 1.f);
      }
    }
  } else {
    for (int i = 0; i < mTextures.size(); ++i) {
      if (!mTextures[i].mToken.null() && mTextures[i].mToken->HasLock() &&
          !mTextures[i].mToken->IsLoaded()) {
        mTextures[i].mToken->Unlock();
        break;
      }
    }
  }
  return loaded;
}

void CSlideShow::SSlideData::Draw() const {
  if (!IsReady()) {
    return;
  }
  const CVector2f leftBottom(mVpOffset.GetX(), mVpOffset.GetY() + mVpSize.GetY());
  const CVector2f rightTop(mVpOffset.GetX() + mVpSize.GetX(), mVpOffset.GetY());
  const float x = (mCanvasSize.GetX() - mTextureWidth) / 2.f;
  const float y = (mCanvasSize.GetY() - mTextureHeight) / 2.f;
  for (int i = 0; i < mTextures.size(); ++i) {
    const STexture& texture = mTextures[i];
    if (texture.mAlpha > 0.f && !(leftBottom.GetX() > x + texture.mRightTop.GetX()) &&
        !(leftBottom.GetY() < y + texture.mRightTop.GetY()) &&
        !(rightTop.GetX() < x + texture.mLeftBottom.GetX()) &&
        !(rightTop.GetY() > y + texture.mLeftBottom.GetY())) {
      DrawTexture(texture.mToken,
                  CVector3f(x + texture.mLeftBottom.GetX(), 0.f, y + texture.mRightTop.GetY()),
                  mMulColor.WithAlphaModulatedBy(texture.mAlpha), &mVpOffset, &mVpSize);
    }
  }
}

void CSlideShow::SSlideData::Reset() {
  mGallery = -1;
  mSlide = -1;
  mTextures = rstl::vector< STexture >();
  mColumns = 0;
  mReady = false;
  mVpOffset = sZeroVector;
  mVpSize = sZeroVector;
  mCanvasSize = sZeroVector;
  mMulColor = CColor::White().WithAlphaOf(0.f);
}

bool CSlideShow::GetIsContinueDraw() const { return false; }
