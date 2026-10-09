#include "MetroidPrime/CSlideShow.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Audio/DolphinCRSFAudio.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/CDependencyGroup.hpp"
#include "Kyoto/CDvdFile.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Text/CGuiTextSupport.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CArchMsgParmUserInput.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCameraFilterPass.hpp"
#include "MetroidPrime/Decode.hpp"
#include "MetroidPrime/HUD/CHudDecoInterfaceScan.hpp"
#include "MetroidPrime/HUD/CSamusHud.hpp"
#include "MetroidPrime/Player/CEnvironmentVariable.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/Tweaks/CTweakContents.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerRes.hpp"
#include "MetroidPrime/Tweaks/CTweakSlideShow.hpp"
#include "rstl/StringExtras.hpp"
#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"

// Unreferenced; only the pooled strings survive in the final binary.
static const char* skSlideShowPrefix = "slideshow";
static const char* const skGalleryPrefix = "Gallery";
static const char* skGalleryBorderModel = "CMDL_GalleryBorder";
static const char skImageTag[] = "&image=";
static const char skImageTagEnd[] = ";";
static const char* const skLogbookName = "Logbook";

static CVector2f sZeroVector(0.f, 0.f);
static rstl::string sSlideShowMusic = rstl::string_l("");

static int GetStickDirection(float up, float down, float left, float right) {
  uint direction = 0;
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
  case 5:
    return 2;
  case 4:
    return 3;
  case 6:
    return 4;
  case 2:
    return 5;
  case 10:
    return 6;
  case 8:
    return 7;
  case 9:
    return 8;
  default:
    return 0;
  }
}

static inline int GetStickDirection(const CFinalInput& input, CControlMapper::ECommands up,
                                    CControlMapper::ECommands down, CControlMapper::ECommands left,
                                    CControlMapper::ECommands right) {
  const CControlMapper& mapper = gpGameState->ControlMapper();
  return GetStickDirection(mapper.GetAnalogInput(up, input), mapper.GetAnalogInput(down, input),
                           mapper.GetAnalogInput(left, input), mapper.GetAnalogInput(right, input));
}

static void DrawTexture(const rstl::auto_ptr< TToken< CTexture > >& token, const CVector3f& offset,
                        const CColor& color, const CVector2f* viewportOffset,
                        const CVector2f* viewportSize) {
  if (token.null()) {
    return;
  }
  const CTexture& texture = ***token;
  const float width = texture.GetWidth();
  const float height = texture.GetHeight();
  const CViewport& viewport = CGraphics::GetViewport();
  int left = viewport.mLeft;
  int top = viewport.mTop;
  int vpWidth = viewport.mWidth;
  int vpHeight = viewport.mHeight;
  if (viewportOffset != nullptr) {
    left = int(viewportOffset->GetX());
    top = int(viewportOffset->GetY());
  }
  if (viewportSize != nullptr) {
    vpWidth = int(viewportSize->GetX());
    vpHeight = int(viewportSize->GetY());
  }
  CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
  CGraphics::SetOrtho(left, left + vpWidth, top + vpHeight, top, -1.f, 1.f);
  CGraphics::SetViewPointMatrix(CTransform4f::Identity());
  CGraphics::SetModelMatrix(CTransform4f::Translate(offset));
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
  CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  texture.Load(GX_TEXMAP0, CTexture::kCM_Clamp);
  CGraphics::StreamBegin(kP_Quads);
  CGraphics::StreamColor(color);
  CGraphics::StreamTexcoord(0.f, 0.f);
  CGraphics::StreamVertex(CVector3f::Zero());
  CGraphics::StreamTexcoord(1.f, 0.f);
  CGraphics::StreamVertex(CVector3f(width, 0.f, 0.f));
  CGraphics::StreamTexcoord(1.f, 1.f);
  CGraphics::StreamVertex(CVector3f(width, 0.f, height));
  CGraphics::StreamTexcoord(0.f, 1.f);
  CGraphics::StreamVertex(CVector3f(0.f, 0.f, height));
  CGraphics::StreamEnd();
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
  gpResourceFactory->GetResLoader().AddPakFileAsync(gpTweakSlideShow->GetPakFile(), false, false);
  const CViewport& viewport = CGraphics::GetViewport();
  const CColor fontColor = gpTweakSlideShow->GetFontColor();
  const CColor outlineColor = gpTweakSlideShow->GetFontOutlineColor();
  const SObjectTag* font =
      gpResourceFactory->GetResourceIdByName(gpTweakSlideShow->GetFont().data());
  mControlsText = rs_new CGuiTextSupport(
      font->GetId(), viewport.mWidth, viewport.mHeight,
      CGuiTextProperties(false, kJustification_Center, kVerticalJustification_Bottom), fontColor,
      outlineColor, CColor::White(), gpSimplePool);
  mGalleryNameText = rs_new CGuiTextSupport(
      font->GetId(), viewport.mWidth, viewport.mHeight,
      CGuiTextProperties(false, kJustification_Left, kVerticalJustification_Bottom), fontColor,
      outlineColor, CColor::White(), gpSimplePool);
  mSlideNumberText = rs_new CGuiTextSupport(
      font->GetId(), viewport.mWidth, viewport.mHeight,
      CGuiTextProperties(false, kJustification_Right, kVerticalJustification_Bottom), fontColor,
      outlineColor, CColor::White(), gpSimplePool);

  const rstl::reserved_vector< CAssetId, 9 >* sticks[] = {&gpTweakPlayerRes->mLStick,
                                                          &gpTweakPlayerRes->mCStick};
  mStickTextures.reserve(18);
  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 9; ++j) {
      mStickTextures.push_back(gpSimplePool->GetObj(SObjectTag('TXTR', (*sticks[i])[j])));
    }
  }
  SetTexturesLocked(mStickTextures, true);
  const rstl::reserved_vector< CAssetId, 2 >* buttons[] = {
      &gpTweakPlayerRes->mLTrigger, &gpTweakPlayerRes->mRTrigger, &gpTweakPlayerRes->mBButton,
      &gpTweakPlayerRes->mYButton};
  mButtonTextures.reserve(8);
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 2; ++j) {
      mButtonTextures.push_back(gpSimplePool->GetObj(SObjectTag('TXTR', (*buttons[i])[j])));
    }
  }
  SetTexturesLocked(mButtonTextures, true);
}

CSlideShow::~CSlideShow() {
  gpResourceFactory->GetResLoader().RemovePakFile(gpTweakSlideShow->GetPakFile());
}

uint CSlideShow::GetGalleriesUnlocked() {
  uint flags = 0;
  if (gpGameState != nullptr) {
    const int percent =
        gpGameState->SystemOptions().EnvVars().FindEnvironmentVariable("PercentScans")->GetValue();
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
    if (gpGameState->SystemOptions()
            .EnvVars()
            .FindEnvironmentVariable("NormalModeCompleted")
            ->GetValue() != 0) {
      flags |= 16;
    }
    if (gpGameState->SystemOptions()
            .EnvVars()
            .FindEnvironmentVariable("HardModeCompleted")
            ->GetValue() != 0) {
      flags |= 32;
    }
  }
  return flags;
}

void CSlideShow::BuildGalleryLists(uint flags) {
  const int count = mGalleryTXTRDeps.size();
  mGalleries.reserve(count);
  rstl::vector< TToken< CDependencyGroup > >::iterator dep = mGalleryTXTRDeps.begin();
  rstl::vector< rstl::wstring >::iterator label = mGalleryLabels.begin();
  for (int i = 0; dep != mGalleryTXTRDeps.end() && i < count; ++i) {
    if ((flags & (1 << i)) == 0) {
      dep = mGalleryTXTRDeps.erase(dep);
      label = mGalleryLabels.erase(label);
      continue;
    }

    const int textureCount = dep->GetT()->GetObjectTagVector().size();
    mGalleries.push_back_unsafe(SGalleryData(i));
    SGalleryData& gallery = mGalleries.back();
    gallery.mTextures.reserve(textureCount);
    gallery.mSlides.reserve(textureCount);
    int slide = 0;
    int row = 0;
    int column = 0;
    int tiles = 0;
    int columns = 0;
    int missingRows = 0;
    while (slide < textureCount) {
      rstl::string name = CBasics::Stringize("%s_%02d_%03d", "slideshow", i, slide);
      const SObjectTag* tag = gpResourceFactory->GetResourceIdByName(name.data());
      if (tag != nullptr) {
        gallery.mTextures.push_back_unsafe(tag);
        column = 1;
        ++tiles;
        columns = 1;
        ++missingRows;
        tag = nullptr;
      } else {
        name.append(CBasics::Stringize("_%02d%02d", column, row), -1);
        tag = gpResourceFactory->GetResourceIdByName(name.data());
        if (tag != nullptr) {
          gallery.mTextures.push_back_unsafe(tag);
          ++column;
          ++tiles;
          columns = column;
          missingRows = 0;
        }
      }
      if (tag == nullptr) {
        if (missingRows == 1 && tiles > 0) {
          gallery.mSlides.push_back_unsafe(
              rstl::pair< int, int >(gallery.mTextures.size(), columns));
          ++slide;
          row = 0;
          missingRows = 0;
          tiles = 0;
        } else {
          if (missingRows > 1) {
            break;
          }
          ++missingRows;
          ++row;
        }
        column = 0;
      }
    }
    ++dep;
    ++label;
  }
}

bool CSlideShow::LoadTXTRDep(const char* name) {
  const SObjectTag* tag = gpResourceFactory->GetResourceIdByName(name);
  if (tag != nullptr && tag->type == 'DGRP') {
    if (mGalleryTXTRDeps.size() + 1 > mGalleryTXTRDeps.capacity()) {
      mGalleryTXTRDeps.reserve(mGalleryTXTRDeps.size() + 1);
    }
    mGalleryTXTRDeps.push_back_unsafe(TToken< CDependencyGroup >(gpSimplePool->GetObj(*tag)));
  } else {
    return false;
  }
  return true;
}

CIOWin::EMessageReturn CSlideShow::OnMessage(const CArchitectureMessage& msg,
                                             CArchitectureQueue& queue) {
  if (msg.GetType() == kAM_UserInput) {
    const CArchMsgParmUserInput input = MakeMsg::GetParmUserInput(msg);
    if (input.GetUserInput().ControllerNumber() == 0) {
      return ProcessUserInput(input.GetUserInput());
    }
    return kMR_Exit;
  }
  if (msg.GetType() != kAM_TimerTick) {
    return kMR_Exit;
  }
  if (mExit) {
    return kMR_RemoveIOWinAndExit;
  }
  const float dt = MakeMsg::GetParmTimerTick(msg).GetReal();
  if (mPhase == 0) {
    if (!gpResourceFactory->GetResLoader().AreAllPaksLoaded()) {
      gpResourceFactory->GetResLoader().AsyncIdlePakLoading();
      return kMR_Exit;
    }
    mPhase = 1;
  }
  if (mPhase == 1) {
    if (mGalleryTXTRDeps.empty()) {
      mGalleryTXTRDeps.reserve(8);
      for (int i = 1;; ++i) {
        const rstl::string name = CBasics::Stringize("%s%02d_DGRP", skGalleryPrefix, i);
        if (!LoadTXTRDep(name.data())) {
          break;
        }
      }
      SetDependenciesLocked(mGalleryTXTRDeps, true);
    }
    if (!AreAllDepsLoaded(mGalleryTXTRDeps)) {
      return kMR_Exit;
    }
    mPhase = 2;
  }
  if (mPhase == 2) {
    if (mGalleryNames.null()) {
      mGalleryNames = rs_new TToken< CStringTable >(
          gpSimplePool->GetObj(gpTweakSlideShow->GetStringResName().data()));
      mGalleryNames->Lock();
    }
    if (!mGalleryNames->IsLoaded()) {
      return kMR_Exit;
    }
    const CStringTable& strings = ***mGalleryNames;
    mGalleryLabels.reserve(mGalleryTXTRDeps.size());
    for (int i = 0; i < mGalleryTXTRDeps.size(); ++i) {
      mGalleryLabels.push_back(
          rstl::wstring(strings.GetString(CBasics::Stringize("GalleryName%d", i + 1))));
    }
    mPhase = 3;
  }
  if (mPhase == 3) {
    BuildGalleryLists(GetGalleriesUnlocked());
    for (int i = 0; i < mGalleries.size(); ++i) {
      mTotalSlides += mGalleries[i].mSlides.size();
    }
    AdvanceSlide(true);
    mPhase = 4;
  }
  if (mPhase == 4) {
    if (mLoadMusic && mAudio.null()) {
      if (sSlideShowMusic.size() != 0 && CDvdFile::FileExists(sSlideShowMusic.data())) {
        mAudio = rs_new CRSFAudio(sSlideShowMusic, 0, 0);
      }
      mLoadMusic = false;
    }
    if (!mAudio.null()) {
      if (!mAudio->IsFullyLoaded()) {
        return kMR_Exit;
      }
      UpdateMusicVolume(mFadeTimer, gpTweakSlideShow->GetFadeInTime());
      mAudio->StartMixOut();
    }
    mPhase = 5;
  }
  if (mPhase == 5) {
    if (mIntroFade || mOutroFade) {
      mFadeTimer = rstl::max_val(0.f, mFadeTimer - dt);
      if (mFadeTimer <= 0.f) {
        if (mDisableInput) {
          mExit = true;
        } else {
          mIntroFade = false;
          mOutroFade = false;
          mFadeTimer = gpTweakSlideShow->GetFadeOutTime();
        }
      }
    }
    LoadSlide();
    if (mSlideB.IsReady()) {
      const float blendTime = gpTweakSlideShow->GetSlideBlendTime();
      if (mCrossfadeTimer > blendTime) {
        mSlideA = mSlideB;
        SetPanSfx(false);
        SetZoomSfx(false);
        mSlideA.mMulColor.SetAlpha(1.f);
        mSlideB.Reset();
        mCrossfadeTimer = 0.f;
      } else {
        const float alpha = CMath::Clamp(0.f, mCrossfadeTimer / blendTime, 1.f);
        mSlideA.mMulColor.SetAlpha(1.f - alpha);
        mSlideB.mMulColor.SetAlpha(alpha);
        mCrossfadeTimer += dt;
      }
    } else if (CMath::AbsF(mRepeatTimer) > gpTweakSlideShow->GetSlideShowDelay()) {
      AdvanceSlide(mRepeatTimer > 0.f);
      mRepeatTimer = 0.f;
    }
    const float idleTime =
        IsControlsAnimating() ? 0.f : gpTweakSlideShow->GetSlideNumberHideDelay();
    mIdleTimer = CMath::Clamp(0.f, mIdleTimer + dt, idleTime);
    if (mIdleTimer >= gpTweakSlideShow->GetSlideNumberHideDelay()) {
      mSlideNumberTimer += dt;
    } else {
      mSlideNumberTimer -= dt;
    }
    const float transitionTime = gpTweakSlideShow->GetSlideNumberTransitionTime();
    mSlideNumberTimer = CMath::Clamp(0.f, mSlideNumberTimer, transitionTime);
    if (mShowSlideNumber && mSlideNumberTimer == transitionTime) {
      mShowSlideNumber = false;
      mGalleryChanged = false;
    } else if (mSlideNumberTimer < transitionTime) {
      mShowSlideNumber = true;
    }
    UpdateControls(dt);
    UpdateSlideNumber(dt);
    if (mOutroFade) {
      UpdateMusicVolume(mFadeTimer, gpTweakSlideShow->GetFadeOutTime());
    }
  }
  return kMR_Exit;
}

void CSlideShow::Draw() const {
  if (mPhase != 5) {
    return;
  }
  gpRender->SetDepthReadWrite(false, false);
  if (mSlideA.IsReady()) {
    mSlideA.Draw();
  }
  if (mSlideB.IsReady()) {
    mSlideB.Draw();
  }
  if (!mSlideNumberText.null() && mShowSlideNumber) {
    DrawSlideNumber();
  }
  if (IsControlsAnimating()) {
    DrawControls();
  }
  if (mIntroFade || mOutroFade) {
    float alpha = CMath::Clamp(0.f,
                               mFadeTimer / (mIntroFade ? gpTweakSlideShow->GetFadeInTime()
                                                        : gpTweakSlideShow->GetFadeOutTime()),
                               1.f);
    if (mOutroFade) {
      alpha = 1.f - alpha;
    }
    CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Blend, CCameraFilterPass::kFS_Fullscreen,
                                  CColor::Black().WithAlphaOf(alpha), nullptr, 1.f);
  }
}

bool IsDataLoreResearchScan(CAssetId id) { return true; }

struct SlideShowScanIdLess {
  bool operator()(const CPlayerState::SPersistentState::SScanState& scan, CAssetId id) const {
    return scan.mAssetId < id;
  }
  bool operator()(CAssetId id, const CPlayerState::SPersistentState::SScanState& scan) const {
    return id < scan.mAssetId;
  }
};

rstl::pair< int, int > CStateManager::CalculateScanCompletionRate() const {
  int complete = 0;
  int total = 0;
  const CHudDecoInterfaceScan* scanInterface = CSamusHud::GetScanInterface(0);
  if (scanInterface == nullptr) {
    return rstl::pair< int, int >(complete, total);
  }
  const rstl::vector< SScanHierarchyNode >& hierarchy = scanInterface->GetHierarchy();
  const rstl::vector< CPlayerState::SPersistentState::SScanState >& scanStates =
      GetPlayerState(0)->ScanStates();
  int logbook = -1;
  for (int i = 0; i < hierarchy.size(); ++i) {
    if (hierarchy[i].mName == skLogbookName) {
      logbook = i;
    }
  }
  for (int i = 0; i < hierarchy.size(); ++i) {
    const SScanHierarchyNode& node = hierarchy[i];
    if (node.mScan == kInvalidAssetId) {
      continue;
    }
    rstl::vector< CPlayerState::SPersistentState::SScanState >::const_iterator state =
        rstl::binary_find(scanStates.begin(), scanStates.end(), node.mScan, SlideShowScanIdLess());
    const bool finished = state != scanStates.end() && state->mProgress == 0xff;
    for (int parent = node.mParent; parent != -1; parent = hierarchy[parent].mParent) {
      if (parent == logbook) {
        ++total;
        if (finished) {
          ++complete;
        }
      }
    }
  }
  return rstl::pair< int, int >(complete, total);
}

CAssetId UpdatePersistentScanPercent(int previous, int current, int total) {
  if (previous != current) {
    static const char* const unlockMessages[] = {
        "STRG_SlideShow_Unlock1_", "STRG_SlideShow_Unlock2_", "STRG_SlideShow_Unlock3_",
        "STRG_SlideShow_Unlock4_"};
    const float interval = gpTweakSlideShow->GetScanPercentInterval();
    const float previousPercent = 100.f * (float(previous) / total);
    const float currentPercent = 100.f * (float(current) / total);
    const int saved =
        gpGameState->SystemOptions().EnvVars().FindEnvironmentVariable("PercentScans")->GetValue();
    const int scanPercent = int(currentPercent);
    const int previousStep = int(rstl::max_val(0.f, previousPercent - 20.f) / interval);
    const int step = int(rstl::max_val(0.f, currentPercent - 20.f) / interval);
    const bool firstTime = scanPercent > saved;
    if (firstTime) {
      gpGameState->SystemOptions()
          .EnvVars()
          .FindEnvironmentVariable("PercentScans")
          ->Set(scanPercent);
    }
    if (step > previousStep) {
      const int message = CMath::Clamp(0, step - 1, 3);
      const rstl::string name = rstl::string(unlockMessages[message]) + (firstTime ? "1" : "2");
      return gpResourceFactory->GetResourceIdByName(name.data())->GetId();
    }
  }
  return kInvalidAssetId;
}

CIOWin::EMessageReturn CSlideShow::ProcessUserInput(const CFinalInput& input) {
  if (!mDisableInput && mPhase == 5) {
    if (IsControlsAnimating()) {
      UpdateControlsText(input);
    }
    if (input.PB()) {
      SetPanSfx(false);
      SetZoomSfx(false);
      CSfxManager::SfxStart(0x5b7, 127, 64);
      mDisableInput = true;
      mOutroFade = true;
      return kMR_Exit;
    }
    if (input.PY()) {
      SetShowControls(!mShowControls);
      if (mShowControls) {
        mGalleryChanged = true;
        CSfxManager::SfxStart(0x5b3, 127, 64);
      } else {
        CSfxManager::SfxStart(0x5b2, 127, 64);
      }
    }
    bool changed = false;
    if (input.PDPRight()) {
      mGallery = (mGallery + 1) % mGalleries.size();
      mSlide = -1;
      mGalleryChanged = true;
      changed = true;
      AdvanceSlide(true);
    } else if (input.PDPLeft()) {
      --mGallery;
      if (mGallery < 0) {
        mGallery = mGalleries.size() - 1;
      }
      mSlide = -1;
      mGalleryChanged = true;
      changed = true;
      AdvanceSlide(true);
    } else if (gpGameState->ControlMapper().GetPressInput(CControlMapper::kC_MapCircleLeft,
                                                          input) ||
               input.PA()) {
      changed = true;
      AdvanceSlide(true);
    } else if (gpGameState->ControlMapper().GetPressInput(CControlMapper::kC_MapCircleRight,
                                                          input)) {
      changed = true;
      AdvanceSlide(false);
    } else if (gpGameState->ControlMapper().GetAnalogInput(CControlMapper::kC_MapCircleLeft,
                                                           input) ||
               input.DA()) {
      mRepeatTimer = rstl::max_val(0.f, mRepeatTimer);
      mRepeatTimer += input.DeltaTime();
    } else if (gpGameState->ControlMapper().GetAnalogInput(CControlMapper::kC_MapCircleRight,
                                                           input)) {
      mRepeatTimer = rstl::min_val(0.f, mRepeatTimer);
      mRepeatTimer -= input.DeltaTime();
    } else {
      mRepeatTimer = 0.f;
    }
    if (changed) {
      mRepeatTimer = 0.f;
      mIdleTimer = 0.f;
    }
    if (mSlideA.IsReady()) {
      return mSlideA.ProcessUserInput(input);
    }
  }
  return kMR_Exit;
}

CIOWin::EMessageReturn CSlideShow::AdvanceSlide(bool forward) {
  if (!mGalleries.empty()) {
    CSfxManager::SfxStart(0x5b6, 127, 64);
    if (forward) {
      ++mSlide;
    } else {
      --mSlide;
    }
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
  if (mSlideB.mTextures.empty() && (mSlideA.mGallery != mGallery || mSlideA.mSlide != mSlide)) {
    const SGalleryData& gallery = mGalleries[mGallery];
    mSlideA.mStopLoading = true;
    const int first = mSlide == 0 ? 0 : gallery.mSlides[mSlide - 1].first;
    const int end = gallery.mSlides[mSlide].first;
    mSlideB.mTextures.reserve(end - first);
    for (int i = first; i < end; ++i) {
      const SObjectTag* tag = gallery.mTextures[i];
      if (tag != nullptr && gpResourceFactory->GetResourceTypeById(tag->GetId()) == 'TXTR') {
        mSlideB.mTextures.push_back_unsafe(STexture());
        mSlideB.mTextures.back().mToken = rs_new TToken< CTexture >(gpSimplePool->GetObj(*tag));
      }
    }
    mSlideB.mGallery = mGallery;
    mSlideB.mSlide = mSlide;
    mSlideB.mColumns = gallery.mSlides[mSlide].second;
    mSlideB.InitializeViewport();
  }
  if (mSlideB.IsLoaded() && !mSlideB.mReady) {
    mSlideB.InitializeViewport();
  }
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
  mCStick =
      GetStickDirection(input, CControlMapper::kC_MapMoveForward, CControlMapper::kC_MapMoveBack,
                        CControlMapper::kC_MapMoveLeft, CControlMapper::kC_MapMoveRight);
  mLStick =
      GetStickDirection(input, CControlMapper::kC_MapCircleDown, CControlMapper::kC_MapCircleUp,
                        CControlMapper::kC_MapCircleLeft, CControlMapper::kC_MapCircleRight);
  const float zoomIn =
      gpGameState->ControlMapper().GetAnalogInput(CControlMapper::kC_MapZoomIn, input);
  const float zoomOut =
      gpGameState->ControlMapper().GetAnalogInput(CControlMapper::kC_MapZoomOut, input);
  mRTrigger = zoomIn > 0.f ? 1 : 0;
  mLTrigger = zoomOut > 0.f ? 1 : 0;
  rstl::wstring text;
  text.reserve(256);
  const CStringTable& strings = ***mGalleryNames;
  text.append(CStringExtras::ConvertToUNICODE(CBasics::Stringize(
      "%sSI,0.6,1.0,%8.8X%s", skImageTag, gpTweakPlayerRes->mLStick[mLStick], skImageTagEnd)));
  text.append(strings.GetString("Browse"), -1);
  text.append(CStringExtras::ConvertToUNICODE(rstl::string_l("   ")));
  text.append(CStringExtras::ConvertToUNICODE(CBasics::Stringize(
      "%s%8.8X%s", skImageTag, gpTweakPlayerRes->mLTrigger[mLTrigger], skImageTagEnd)));
  text.append(CStringExtras::ConvertToUNICODE(rstl::string_l(" ")));
  text.append(strings.GetString("Zoom"), -1);
  text.append(CStringExtras::ConvertToUNICODE(rstl::string_l(" ")));
  text.append(CStringExtras::ConvertToUNICODE(CBasics::Stringize(
      "%s%8.8X%s", skImageTag, gpTweakPlayerRes->mRTrigger[mRTrigger], skImageTagEnd)));
  text.append(CStringExtras::ConvertToUNICODE(rstl::string_l("  ")));
  text.append(CStringExtras::ConvertToUNICODE(CBasics::Stringize(
      "%sSI,0.6,1.0,%8.8X%s", skImageTag, gpTweakPlayerRes->mCStick[mCStick], skImageTagEnd)));
  text.append(strings.GetString("Pan"), -1);
  text.append(CStringExtras::ConvertToUNICODE(rstl::string_l("   ")));
  text.append(CStringExtras::ConvertToUNICODE(CBasics::Stringize(
      "%sSI,1.0,1.0,%8.8X%s", skImageTag, gpTweakPlayerRes->mYButton[0], skImageTagEnd)));
  text.append(CStringExtras::ConvertToUNICODE(rstl::string_l(" ")));
  text.append(strings.GetString("Instructions"), -1);
  text.append(CStringExtras::ConvertToUNICODE(rstl::string_l("   ")));
  text.append(CStringExtras::ConvertToUNICODE(CBasics::Stringize(
      "%sSI,0.6,1.0,%8.8X%s", skImageTag, gpTweakPlayerRes->mBButton[0], skImageTagEnd)));
  text.append(CStringExtras::ConvertToUNICODE(rstl::string_l(" ")));
  text.append(strings.GetString("Quit"), -1);
  mControlsText->SetText(text);
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
  if (!mAudio.null()) {
    const float volume = CMath::Clamp(0.f, time / fadeTime, 1.f);
    const uchar musicVolume =
        CCast::ToUint8(0.7421875f * volume * int(gpGameState->GameOptions().GetMusicVolume()));
    mAudio->SetVolume(musicVolume);
    mAudio->StartMixOut();
  }
}

void CSlideShow::SetTexturesLocked(rstl::vector< CToken >& textures, bool locked) {
  for (rstl::vector< CToken >::iterator it = textures.begin(); it != textures.end(); ++it) {
    if (locked) {
      it->Lock();
    } else {
      it->Unlock();
    }
  }
}

void CSlideShow::SetDependenciesLocked(rstl::vector< TToken< CDependencyGroup > >& deps,
                                       bool locked) {
  for (rstl::vector< TToken< CDependencyGroup > >::iterator it = deps.begin(); it != deps.end();
       ++it) {
    if (locked) {
      it->Lock();
    } else {
      it->Unlock();
    }
  }
}

bool CSlideShow::AreAllDepsLoaded(const rstl::vector< TToken< CDependencyGroup > >& deps) const {
  for (rstl::vector< TToken< CDependencyGroup > >::const_iterator it = deps.begin();
       it != deps.end(); ++it) {
    if (!it->IsLoaded()) {
      return false;
    }
  }
  return true;
}

void CSlideShow::DrawSlideNumber() const {
  if (mSlideNumberText.null()) {
    return;
  }
  const int height = CGraphics::GetViewport().mHeight;
  const float fadeTime = gpTweakSlideShow->GetSlideNumberTransitionTime();
  const float alpha = CMath::Clamp(
      0.f, (fadeTime - mSlideNumberTimer) / gpTweakSlideShow->GetSlideNumberTransitionTime(), 1.f);
  const rstl::pair< CVector2i, CVector2i >& bounds = mSlideNumberText->GetBounds();
  const float textHeight = bounds.second.GetY() - bounds.first.GetY();
  const float y = 2.f * height - mSlideNumberOffset - textHeight;
  CGraphics::SetCullMode(kCM_None);
  gpRender->SetViewportOrtho(false, -4096.f, 4096.f);
  gpRender->SetDepthReadWrite(false, false);
  gpRender->SetModelMatrix(CTransform4f::Translate(-32.f, 0.f, y));
  mSlideNumberText->SetGeometryColor(CColor::White().WithAlphaModulatedBy(alpha));
  mSlideNumberText->Render();
  if (!mGalleryNameText.null() && mGalleryChanged) {
    gpRender->SetModelMatrix(CTransform4f::Translate(32.f, 0.f, y));
    mGalleryNameText->SetGeometryColor(CColor::White().WithAlphaModulatedBy(alpha));
    mGalleryNameText->SetText(mGalleryLabels[mGallery]);
    mGalleryNameText->Render();
  }
}

void CSlideShow::UpdateSlideNumber(float dt) {
  if (!mSlideNumberText.null() && !mGalleries.empty()) {
    int slide = mSlide;
    for (int i = 0; i < mGallery; ++i) {
      slide += mGalleries[i].mSlides.size();
    }
    rstl::string text = CBasics::Stringize("%d/%d", slide + 1, mTotalSlides);
    mSlideNumberText->SetText(text);
    mSlideNumberText->Update(dt);
  }
}

void CSlideShow::DrawControls() const {
  if (CGuiTextSupport* text = mControlsText.get()) {
    const float fadeTime =
        mIntroFade ? gpTweakSlideShow->GetFadeInTime() : gpTweakSlideShow->GetFadeOutTime();
    text->SetGeometryColor(CColor::White().WithAlphaOf(mControlsAlpha * fadeTime));
    const int height = CGraphics::GetViewport().mHeight;
    gpRender->SetViewportOrtho(false, -4096.f, 4096.f);
    gpRender->SetModelMatrix(CTransform4f::Translate(0.f, 0.f, 32.f + height));
    CGraphics::SetCullMode(kCM_None);
    gpRender->SetDepthReadWrite(false, false);
    mControlsText->Render();
  }
}

CIOWin::EMessageReturn CSlideShow::SSlideData::ProcessUserInput(const CFinalInput& input) {
  if (IsReady()) {
    const CViewport& viewport = CGraphics::GetViewport();
    const int vpWidth = viewport.mWidth;
    const int vpHeight = viewport.mHeight;
    const CControlMapper& mapper = gpGameState->ControlMapper();
    const float aspect = float(vpWidth) / vpHeight;
    const float textureHeight = mTextureHeight;
    const float textureWidth = mTextureWidth;
    const float textureSize = rstl::max_val(textureWidth, textureHeight);
    const float zoomIn = mapper.GetAnalogInput(CControlMapper::kC_MapZoomIn, input);
    const float zoomOut = mapper.GetAnalogInput(CControlMapper::kC_MapZoomOut, input);
    const float zoom = textureSize * (zoomOut - zoomIn) / 1024.f;
    const CVector2f oldSize = mVpSize;
    if (zoom != 0.f) {
      CVector2f offset = mVpSize;
      const float delta = zoom * gpTweakSlideShow->GetScaleMultiplier();
      mVpSize[0] += aspect * delta;
      mVpSize[1] += delta;
      const float minWidth = vpWidth;
      mVpSize[0] = CMath::Clamp(minWidth, mVpSize.GetX(), mCanvasSize.GetX());
      const float minHeight = vpHeight;
      mVpSize[1] = CMath::Clamp(minHeight, mVpSize.GetY(), mCanvasSize.GetY());
      offset -= mVpSize;
      offset /= 2.f;
      mVpOffset += offset;
    }
    mParent->SetZoomSfx(!(oldSize == mVpSize));

    const CVector2f oldOffset = mVpOffset;
    const float forward =
        gpGameState->ControlMapper().GetAnalogInput(CControlMapper::kC_MapMoveForward, input);
    const float back =
        gpGameState->ControlMapper().GetAnalogInput(CControlMapper::kC_MapMoveBack, input);
    const float left =
        gpGameState->ControlMapper().GetAnalogInput(CControlMapper::kC_MapMoveLeft, input);
    const float right =
        gpGameState->ControlMapper().GetAnalogInput(CControlMapper::kC_MapMoveRight, input);
    const float speed = rstl::max_val(mTextureWidth, mTextureHeight) *
                        gpTweakSlideShow->GetTranslationMultiplier() / 1024.f;
    mVpOffset[0] -= speed * left;
    mVpOffset[0] += speed * right;
    mVpOffset[1] += speed * forward;
    mVpOffset[1] -= speed * back;
    mVpOffset[0] = CMath::Clamp(0.f, mVpOffset.GetX(), mCanvasSize.GetX() - mVpSize.GetX());
    mVpOffset[1] = CMath::Clamp(0.f, mVpOffset.GetY(), mCanvasSize.GetY() - mVpSize.GetY());

    const float availableX = textureWidth / 2.f - mVpSize.GetX() / 2.f;
    const float availableY = textureHeight / 2.f - mVpSize.GetY() / 2.f;
    const float halfX = rstl::max_val(0.f, availableX);
    const float halfY = rstl::max_val(0.f, availableY);
    mVpOffset[0] =
        CMath::Clamp(mCanvasSize.GetX() / 2.f - halfX, mVpSize.GetX() / 2.f + mVpOffset.GetX(),
                     mCanvasSize.GetX() / 2.f + halfX) -
        mVpSize.GetX() / 2.f;
    mVpOffset[1] =
        CMath::Clamp(mCanvasSize.GetY() / 2.f - halfY, mVpSize.GetY() / 2.f + mVpOffset.GetY(),
                     mCanvasSize.GetY() / 2.f + halfY) -
        mVpSize.GetY() / 2.f;
    mParent->SetPanSfx(!(oldOffset == mVpOffset));
  }
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
  bool loaded = true;
  if (mTextures.front().mToken.null() || !mTextures.front().mToken->IsLoaded()) {
    loaded = false;
  }
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
  mMulColor = CColor::White();
  mMulColor.SetAlpha(0.f);
}

bool CSlideShow::GetIsContinueDraw() const { return false; }
