#include "MetroidPrime/CPauseScreenBlur.hpp"

#include "MetroidPrime/CAutoMapper.hpp"
#include "MetroidPrime/CInGameGuiManager.hpp"
#include "MetroidPrime/Cameras/CCameraFilterPass.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakGuiColors.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"

#include "float.h"
#include "rstl/math.hpp"

CPauseScreenBlur::CPauseScreenBlur()
: mMapLightQuarter(gpSimplePool->GetObj("TXTR_MapLightQuarter"))
, mPrevState(kS_InGame)
, mNextState(kS_InGame)
, mBlurAmt(0.f)
, mBlurring(false)
, mGameDraw(true) {}

CPauseScreenBlur::~CPauseScreenBlur() {}

void CPauseScreenBlur::OnNewInGameGuiState(EInGameGuiState state, const CStateManager& mgr,
                                           const CInGameGuiManager& guiMgr) {
  if (InGameGuiStates::IsGameplayState(state)) {
    SetState(kS_InGame, guiMgr);
    return;
  }

  switch (state) {
  case kIGGS_MapScreen:
    SetState(kS_MapScreen, guiMgr);
    break;
  case kIGGS_PauseSaveGame:
    SetState(kS_SaveGame, guiMgr);
    break;
  case kIGGS_PauseHUDMessage:
    SetState(kS_HUDMessage, guiMgr);
    break;
  case kIGGS_PauseGame:
  case kIGGS_PauseLogBook:
    SetState(kS_Pause, guiMgr);
    break;
  }
}

void CPauseScreenBlur::Update(float dt, const CStateManager& mgr, bool b) {
  if (mPrevState == mNextState) {
    return;
  }

  if (mBlurAmt < 0.f) {
    mBlurAmt = rstl::min_val(0.f, mBlurAmt + dt);
  } else if (mBlurAmt > 0.f) {
    mBlurAmt = rstl::min_val(1.f, 2.f * dt + mBlurAmt);
  }

  if (mBlurAmt == 0.f || mBlurAmt == 1.f) {
    OnBlurComplete(b);
  }

  if (mBlurAmt == 0.f && b) {
    mCamBlur.DisableBlur(0.f);
  } else {
    mCamBlur.SetBlur(CCameraBlurPass::kBT_HiBlur,
                     GetBlurAmtInline() * gpTweakGui->GetPauseBlurFactor(), 0.f, true);
    mBlurring = true;
  }
}

void CPauseScreenBlur::Draw(const CStateManager& mgr) {
  mCamBlur.Draw();
  CGraphics::DisableAllLights();
  CGraphics::SetAmbientColor(CColor(0xffffffff));
  const float t = fabs(mBlurAmt);
  const float ease = 2.f * t - t * t;
  if (mCamBlur.GetCurrType() != CCameraBlurPass::kBT_NoBlur) {
    CCameraFilterPass::DrawFilter(
        CCameraFilterPass::kFT_Multiply, CCameraFilterPass::kFS_FullscreenQuarters,
        CColor::Lerp(CColor::White(), gpTweakGuiColors->GetPauseScreenBGModulateColor(), ease),
        *mMapLightQuarter, ease);
    CCameraFilterPass::DrawFilter(
        CCameraFilterPass::kFT_Multiply, CCameraFilterPass::kFS_ScanLinesEven,
        CColor::Lerp(CColor::White(), CColor(0xc0c0c0ff), ease), nullptr, ease);
  }

  if (mBlurring && mCamBlur.GetNoPersistentCopy()) {
    mBlurring = false;
    mGameDraw = false;
  }
}

void CPauseScreenBlur::OnBlurComplete(bool b) {
  if (mNextState == kS_InGame && !b) {
    return;
  }
  mPrevState = mNextState;
  if (mPrevState == kS_InGame) {
    mGameDraw = true;
  }
}

void CPauseScreenBlur::SetState(EState state, const CInGameGuiManager& guiMgr) {
  switch (mPrevState) {
  case kS_InGame:
    if (state != kS_InGame) {
      CSfxManager::SetChannel(CSfxManager::kSC_PauseScreen);
      if (state == kS_HUDMessage) {
        CSfxManager::SfxStart(0x3fb, 0x7f, 0x40);
      } else if (state == kS_MapScreen) {
        const CAutoMapper* mapper = guiMgr.GetAutoMapper();
        if (mapper != nullptr && mapper->GetMapMode() == CAutoMapper::kMM_Teleport) {
          CSfxManager::SfxStart(0x274a, 0x7f, 0x40);
        }
        CSfxManager::SfxStart(0x7a, 0x7f, 0x40);
      }
      mBlurAmt = FLT_EPSILON;
    }
    break;
  case kS_MapScreen:
  case kS_SaveGame:
  case kS_HUDMessage:
  case kS_Pause:
    break;
  }

  switch (state) {
  case kS_InGame: {
    bool valid = true;
    if (mPrevState == kS_InGame && mNextState == kS_InGame) {
      valid = false;
    }
    if (valid) {
      CSfxManager::SetChannel(CSfxManager::kSC_Game);

      if (mPrevState == kS_HUDMessage) {
        CSfxManager::SfxStart(0x3fd, 0x7f, 0x40);
      } else if (mPrevState == kS_MapScreen) {
        CSfxManager::SfxStart(0x7c, 0x7f, 0x40);
      }

      mBlurAmt = -1.f;
    }
    break;
  }
  case kS_MapScreen:
  case kS_SaveGame:
  case kS_HUDMessage:
  case kS_Pause:
    break;
  }

  mNextState = state;
}

float CPauseScreenBlur::GetBlurAmt() const { return GetBlurAmtInline(); }
