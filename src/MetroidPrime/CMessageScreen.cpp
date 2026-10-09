#include "MetroidPrime/CMessageScreen.hpp"

#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiModel.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "GuiSys/CGuiWidgetDrawParms.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Input/CFinalInput.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetroidPrime/CGameGlobalObjects.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/SFX/UI.h"
#include "MetroidPrime/Tweaks/CTweakGuiColors.hpp"
#include "rstl/math.hpp"

CMessageScreen::CMessageScreen(CAssetId msg, float time)
: mMsg(gpSimplePool->GetObj(SObjectTag('STRG', msg)))
, mMsgScreen(gpSimplePool->GetObj("FRME_MsgScreen"))
, mLoadedMsgScreen(nullptr)
, x44_(CVector3f::Zero())
, mBottomPos(CVector3f::Zero())
, x5c_(CVector3f::Zero())
, mVideoBandOffset(10.f)
, mPage(0)
, mBlurAmt(0.f)
, mDelayTime(time)
, mExit(false) {
  mMsgScreen.Lock();
  mMsg.Lock();
}

CMessageScreen::~CMessageScreen() {}

bool CMessageScreen::Update(float dt, float blurAmt) {
  mBlurAmt = blurAmt;
  if (!mLoadedMsgScreen) {
    const bool ready = mMsgScreen.TryCache() && mMsg.TryCache();
    if (ready) {
      mLoadedMsgScreen = mMsgScreen.GetObject();
      mTextpane_message =
          static_cast< CGuiTextPane* >(mLoadedMsgScreen->FindWidget("textpane_message"));
      mBasewidget_center = mLoadedMsgScreen->FindWidget("basewidget_center");
      mBasewidget_bottom = mLoadedMsgScreen->FindWidget("basewidget_bottom");
      mModel_abutton = static_cast< CGuiModel* >(mLoadedMsgScreen->FindWidget("model_abutton"));
      mModel_bottom = static_cast< CGuiModel* >(mLoadedMsgScreen->FindWidget("model_bottom"));
      mModel_center = static_cast< CGuiModel* >(mLoadedMsgScreen->FindWidget("model_center"));
      mModel_bg = static_cast< CGuiModel* >(mLoadedMsgScreen->FindWidget("model_bg"));
      mBottomPos = mBasewidget_bottom->GetLocalPosition();

      if (CGuiWidget* widget = mLoadedMsgScreen->FindWidget("basewidget_top")) {
        widget->SetColor(gpTweakGuiColors->GetHUDMemoTextForegroundColor());
      }
      if (CGuiWidget* widget = mLoadedMsgScreen->FindWidget("basewidget_centerdeco")) {
        widget->SetColor(gpTweakGuiColors->GetHUDMemoTextForegroundColor());
      }
      if (CGuiWidget* widget = mLoadedMsgScreen->FindWidget("model_bottom")) {
        widget->SetColor(gpTweakGuiColors->GetHUDMemoTextForegroundColor());
      }

      mModel_bottom->SetDepthWrite(true);
      mModel_center->SetDepthWrite(true);
      mModel_bg->SetDepthWrite(true);

      if (mMsg.GetObject()->GetStringCount() > 0) {
        CGuiTextSupport& text = mTextpane_message->TextSupport();
        text.SetTypeWriteEffectOptions(false, 0.1f, 30.f);
        text.SetText(mMsg.GetObject()->GetString(0));
        text.SetFontColor(gpTweakGuiColors->GetHUDMemoTextForegroundColor());
        text.SetControlTXTRMap(&gpGameState->GameOptions().GetControlTXTRMap());
      }
    }
  }

  if (mLoadedMsgScreen) {
    if (mDelayTime > 0.f) {
      mDelayTime -= dt;
    }

    float alpha = rstl::max_val(0.f, (mBlurAmt - 0.7f) / 0.3f);
    mBasewidget_bottom->SetColor(CColor::White().WithAlphaOf(alpha));

    const float pulse =
        mDelayTime <= 0.f
            ? CMath::Clamp(
                  0.f,
                  0.5f * (1.f + CMath::FastSinR(5.f * CGraphics::GetSecondsMod900() - M_PIF / 2.f)),
                  1.f)
            : 0.f;
    mModel_abutton->SetColor(CColor::White().WithAlphaOf(pulse));

    mVideoBandOffset += 12.f * dt;
    if (mVideoBandOffset > 10.f) {
      mVideoBandOffset -= 20.f;
    }
    mLoadedMsgScreen->Update(dt);
  }

  return !mExit;
}

void CMessageScreen::ProcessControllerInput(const CFinalInput& input) {
  if (mLoadedMsgScreen && mDelayTime <= 0.f && input.PA()) {
    CGuiTextSupport& text = mTextpane_message->TextSupport();
    if (text.GetCurTime() < text.GetTotalAnimationTime()) {
      text.SetCurTime(text.GetTotalAnimationTime());
      return;
    }

    ++mPage;
    if (mPage >= mMsg.GetObject()->GetStringCount()) {
      mExit = true;
      return;
    }

    text.SetTypeWriteEffectOptions(false, 0.1f, 30.f);
    text.SetText(mMsg.GetObject()->GetString(mPage));
    CSfxManager::SfxStart(SFXui_x_override_02_oneshot, 127, 64);
    mDelayTime = 0.8f;
  }
}

void CMessageScreen::Draw() const {
  if (mLoadedMsgScreen) {
    mLoadedMsgScreen->Draw(CGuiWidgetDrawParms(mBlurAmt, CVector3f::Zero()));
  }
}
