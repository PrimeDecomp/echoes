#include "MetroidPrime/CPauseScreen.hpp"

#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CArchitectureQueue.hpp"
#include "MetroidPrime/Decode.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/CQuitGameScreen.hpp"
#include "MetroidPrime/Factories/CScannableObjectInfo.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScanTreeCategory.hpp"
#include "MetroidPrime/ScriptObjects/CScanTreeMenu.hpp"
#include "MetroidPrime/ScriptObjects/CScanTreeScan.hpp"
#include "MetroidPrime/ScriptObjects/CScanTreeSlider.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakGuiColors.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerRes.hpp"

#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CAuiBitmapMeter.hpp"
#include "GuiSys/CGuiFrameLoader.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "GuiSys/CGuiWidgetDrawParms.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/Animation/CAdvancementDeltas.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CDvdRequest.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CCubeModel.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Input/CFinalInput.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Text/CGuiTextSupport.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "dolphin/os/OSCache.h"

#include "rstl/math.hpp"
#include "rstl/StringExtras.hpp"
#include "rstl/algorithm.hpp"
#include <limits.h>
#include <stdio.h>
#include <string.h>

// Echoes combines the scan tree, options and model viewer in this screen.

CPauseScreen::CPauseScreen()
: mSelectedNodeTexture(gpSimplePool->GetObj("TXTR_ScanNetworkSelected"))
, mUnselectedNodeTexture(gpSimplePool->GetObj("TXTR_ScanNetworkUnselected"))
, mParentNodeTexture(gpSimplePool->GetObj("TXTR_ScanNetworkParent"))
, mSelectedCursorTexture(gpSimplePool->GetObj("TXTR_LogBookSelectedCursor"))
, mHighlightTexture(gpSimplePool->GetObj("TXTR_LogbookHighlight"))
, mScanSweepTexture(gpSimplePool->GetObj("TXTR_ScanSweepBar"))
, mStripedTexture(kTF_I4, 8, 8, 1)
, mFont(gpSimplePool->GetObj("FONT_Deface13B"))
, mSliderModel(gpSimplePool->GetObj("CMDL_OptionSlider"))
, mSliderEndModel(gpSimplePool->GetObj("CMDL_OptionSliderLeft"))
, mSliderCenterModel(gpSimplePool->GetObj("CMDL_OptionSliderCenter"))
, mMenuArrowModel(gpSimplePool->GetObj("CMDL_OptionsMenuArrow"))
, mOptionBackgroundModel(gpSimplePool->GetObj("CMDL_OptionBackground"))
, mNodeText(nullptr)
, mRotationInput(CVector2f::Zero())
, mRotationVelocity(CVector2f::Zero())
, mViewRotation(CQuaternion::NoRotation())
, x1f8_(-1)
, mFrameLoader(rs_new CGuiFrameLoader(gpResourceFactory->GetResourceIdByName("FRME_LogBook")->id,
                                      *gpResourceFactory, *gpSimplePool))
, mFrame(nullptr)
, mAdvanceButton(nullptr)
, mLoadState(kLS_ScanTree)
, mTransitionState(kTS_Loading)
, mAlpha(0.f)
, mScanInfo(nullptr)
, mScanStrings(nullptr)
, mHistoryRowExpanded(false)
, x494_(1)
, x498_(1)
, mPageCount(0)
, mPage(0)
, mPulseTime(0.f)
, mQuitScreen(nullptr)
, mLeftStickIcon(0)
, mRightStickIcon(0)
, mLegendHiddenAmount(0.f)
, x4bc_(0.f)
, x4c0_(0.f)
, mSelectionDelay(0.f)
, mSelectionHighlight(0.f)
, mModelPan(CVector3f::Zero())
, mModelCenterOffset(CVector3f::Zero())
, mModelScale(CVector3f::Zero())
, mModelPitch(CRelAngle::FromRadians(0.f))
, mModelYaw(CRelAngle::FromRadians(0.f))
, mModelZoomAmount(0.f)
, mModelFade(0.f)
, x508_(CVector3f::Zero())
, mPendingScanNode(-1)
, mActorLights(rs_new CActorLights(8, CVector3f::Zero(), 4, 4, 0.1f, false, false, false, false))
, mModelTransform(CTransform4f::Identity())
, mDone(false)
, x568_25_(true)
, mLegendVisible(false)
, mHistoryTextReady(false)
, mModelZoomed(false)
, mNodesTouched(false)
, x568_30_(false)
, mOpenedFromScan(false)
, mModelsReady(false) {
  const CEnvironmentVariable* legend =
      gpGameState->SystemOptions().FindEnvironmentVariable("LogbookLegendVisible");
  mLegendVisible = legend->GetMaximum() == legend->GetValue();
  InitializeStripedTexture();
  gpResourceFactory->GetResLoader().AddPakFileAsync(rstl::string("logbook"), false, false);

  mSelectedNodeTexture.Lock();
  mUnselectedNodeTexture.Lock();
  mParentNodeTexture.Lock();
  mFont.Lock();
  mSelectedCursorTexture.Lock();
  mHighlightTexture.Lock();
  mSliderModel.Lock();
  mSliderEndModel.Lock();
  mSliderCenterModel.Lock();
  mMenuArrowModel.Lock();
  mOptionBackgroundModel.Lock();
  mScanSweepTexture.Lock();

  const CViewport& viewport = CGraphics::GetViewport();
  mNodeText = rs_new CGuiTextSupport(
      gpResourceFactory->GetResourceIdByName("FONT_Deface13B")->id, viewport.mWidth,
      viewport.mHeight,
      CGuiTextProperties(false, kJustification_Center, kVerticalJustification_Top), CColor::White(),
      CColor::Black(), CColor::White(), gpSimplePool);

  for (int i = 0; i < 9; ++i) {
    mLeftStickIcons.push_back(
        gpSimplePool->GetObj(SObjectTag('TXTR', gpTweakPlayerRes->mLStick[i])));
    mRightStickIcons.push_back(
        gpSimplePool->GetObj(SObjectTag('TXTR', gpTweakPlayerRes->mCStick[i])));
  }
  for (int i = 0; i < 9; ++i) {
    mLeftStickIcons[i].Lock();
    mRightStickIcons[i].Lock();
  }
  CSfxManager::SfxStart(0x21d1, 0x7f, 0x3f, CSfxManager::kAllAreas, false, false,
                        CSfxManager::kMedPriority);
}

CPauseScreen::~CPauseScreen() {}

void CPauseScreen::InitializeFrameGlue() {
  mFrame = mFrameLoader->CreateFrame();
  if (mFrame.null()) {
    return;
  }

  mLights.reserve(2);
  mLights.push_back(CLight::BuildPoint(gpTweakGui->GetLogBookModelLight1Position(),
                                      gpTweakGui->GetLogBookModelLight1Color()));
  mLights[0].SetAttenuation(1.f, 0.2f, 0.f);
  mLights.push_back(CLight::BuildPoint(gpTweakGui->GetLogBookModelLight2Position(),
                                      gpTweakGui->GetLogBookModelLight2Color()));
  mLights[1].SetAttenuation(1.f, 0.2f, 0.f);
  mActorLights->BuildFakeLightList(mLights, gpTweakGui->GetLogBookModelAmbientLightColor());
  mMessage = static_cast< CGuiTextPane* >(mFrame->FindWidget("textpane_message"));
  mMessage->TextSupport().SetFontColor(gpTweakGui->GetLogBookScanTextWindowFontColor());
  mMessage->TextSupport().SetOutlineColor(gpTweakGuiColors->GetHUDMemoTextOutlineColor());
  mMessage->SetVisibility(false, kTM_Children);
  mHexWidgets.reserve(100);
  for (int i = 0; i < 100; ++i) {
    CGuiWidget* widget = mFrame->FindWidget(CBasics::Stringize("%s%d", "model_hex", i));
    if (widget != nullptr) {
      mHexWidgets.push_back(widget);
    }
  }
  if (CGuiWidget* widget = mFrame->FindWidget("model_topframe")) {
    widget->SetColor(gpTweakGui->GetLogBookMainWindowBorderColor());
  }
  if (CGuiWidget* widget = mFrame->FindWidget("model_label")) {
    widget->SetColor(gpTweakGui->GetLogBookMainWindowBorderColor());
  }
  if (CGuiWidget* widget = mFrame->FindWidget("model_frame_02")) {
    widget->SetColor(gpTweakGui->GetLogBookScanTextWindowBorderColor());
  }
  if (CGuiWidget* widget = mFrame->FindWidget("model_backdrop")) {
    widget->SetColor(gpTweakGui->GetLogBookScanTextWindowBackgroundColor());
  }
  if (CGuiWidget* widget = mFrame->FindWidget("model_frame_03")) {
    widget->SetColor(gpTweakGui->GetLogBookLegendWindowBorderColor());
  }
  if (CGuiWidget* widget = mFrame->FindWidget("model_backdrop2")) {
    widget->SetColor(gpTweakGui->GetLogBookLegendWindowBackgroundColor());
  }
  mBottomPane = mFrame->FindWidget("basewidget_bottomPane");
  if (!mLegendVisible) {
    mBottomPane->SetVisibility(false, kTM_Children);
    mLegendHiddenAmount = 1.f;
  }
  mScanInfoGroup = mFrame->FindWidget("basewidget_scaninfogroup");
  mScanInfoGroup->SetVisibility(false, kTM_Children);
  mAdvanceButton = mFrame->FindWidget("model_abutton");
  for (int i = 0; i < 6; ++i) {
    if (CGuiWidget* widget = mFrame->FindWidget(CBasics::Stringize("model_history%d_active", i + 1))) {
      mHistoryHighlights.push_back(widget);
      widget->SetVisibility(false, kTM_Children);
    }
    if (CGuiWidget* widget = mFrame->FindWidget(CBasics::Stringize("model_history%d_bottom", i + 1))) {
      mHistoryBackgrounds.push_back(widget);
      widget->SetVisibility(false, kTM_Children);
      widget->SetColor(gpTweakGui->GetLogBookHistorySelectedFrame());
    }
    if (CGuiTextPane* widget = static_cast< CGuiTextPane* >(
            mFrame->FindWidget(CBasics::Stringize("textpane_history%d", i + 1)))) {
      mHistoryLabels.push_back(widget);
      widget->TextSupport().SetFontColor(gpTweakGui->GetLogBookHistoryUnselectedTitle());
      widget->SetVisibility(false, kTM_Children);
      widget->TextSupport().SetWordWrap(true);
    }
    if (CAuiBitmapMeter* widget = static_cast< CAuiBitmapMeter* >(
            mFrame->FindWidget(CBasics::Stringize("barmeter_percent%d", i + 1)))) {
      mHistoryMeters.push_back(widget);
      widget->SetVisibility(false, kTM_Children);
      widget->SetColor(gpTweakGui->GetLogBookHistoryPercentBarUnselected());
      widget->SetTargetFraction(0.f);
      widget->SetCurrentFraction(0.f);
      widget->SetIncreaseSpeed(60.f);
      widget->SetDecreaseSpeed(60.f);
    }
    if (CGuiWidget* widget = mFrame->FindWidget(CBasics::Stringize("model_barmeterbg%d", i + 1))) {
      mHistoryMeterBackgrounds.push_back(widget);
      widget->SetVisibility(false, kTM_Children);
      widget->SetColor(gpTweakGui->GetLogBookHistoryPercentBarBackgroundUnselected());
    }
  }
  if (CGuiWidget* widget = mFrame->FindWidget("model_scanlines")) {
    widget->SetColor(gpTweakGui->GetLogBookScanlineColor());
  }
  if (CGuiWidget* widget = mFrame->FindWidget("model_frame")) {
    widget->SetColor(gpTweakGui->GetLogBookFrameColor());
  }
  if (CGuiWidget* widget = mFrame->FindWidget("model_legend_left")) {
    widget->SetColor(gpTweakGui->GetLogBookFrameColor());
  }
  if (CGuiWidget* widget = mFrame->FindWidget("model_legend_center")) {
    widget->SetColor(gpTweakGui->GetLogBookFrameColor());
  }
  if (CGuiWidget* widget = mFrame->FindWidget("model_legend_right")) {
    widget->SetColor(gpTweakGui->GetLogBookFrameColor());
  }
  if (CGuiWidget* widget = mFrame->FindWidget("model_black_right")) {
    widget->SetColor(gpTweakGui->GetLogBookLegendBackgroundColor());
  }
  if (CGuiWidget* widget = mFrame->FindWidget("model_black_left")) {
    widget->SetColor(gpTweakGui->GetLogBookLegendBackgroundColor());
  }
  if (CGuiWidget* widget = mFrame->FindWidget("model_black_center")) {
    widget->SetColor(gpTweakGui->GetLogBookLegendBackgroundColor());
  }
  if (CGuiWidget* widget = mFrame->FindWidget("model_rhs_bgnd")) {
    widget->SetColor(gpTweakGui->GetLogBookLegendBackgroundColor());
  }
  mInstructionLabel = static_cast< CGuiTextPane* >(mFrame->FindWidget("textpane_lable"));
  mInstructionLabel->TextSupport().SetText(
      rstl::wstring(gpStringTable->GetString("LogBookScreenInstructionPanelLabel")), false);
  mInstructionLabel->TextSupport().SetFontColor(gpTweakGui->GetLogBookLegendWindowFontColor());
  CGuiTextPane* left = static_cast< CGuiTextPane* >(mFrame->FindWidget("textpane_left"));
  left->TextSupport().SetText(rstl::wstring(gpStringTable->GetString("InstructionsLeft")), false);
  left->TextSupport().SetFontColor(gpTweakGui->GetLogBookLegendWindowFontColor());
  mRightInstructions = static_cast< CGuiTextPane* >(mFrame->FindWidget("textpane_right2"));
  mRightInstructions->TextSupport().SetText(rstl::wstring(gpStringTable->GetString("InstructionsMid")), false);
  mRightInstructions->TextSupport().SetFontColor(gpTweakGui->GetLogBookLegendWindowFontColor());
  CGuiTextPane* next = static_cast< CGuiTextPane* >(mFrame->FindWidget("textpane_instructions2"));
  next->TextSupport().SetText(rstl::wstring(gpStringTable->GetString("InstructionsNext")), false);
  next->TextSupport().SetFontColor(gpTweakGui->GetLogBookLegendWindowFontColor());
  CGuiTextPane* back = static_cast< CGuiTextPane* >(mFrame->FindWidget("textpane_instructions1"));
  back->TextSupport().SetText(rstl::wstring(gpStringTable->GetString("InstructionsBack")), false);
  back->TextSupport().SetFontColor(gpTweakGui->GetLogBookLegendWindowFontColor());
  CGuiTextPane* zoom = static_cast< CGuiTextPane* >(mFrame->FindWidget("textpane_right3"));
  zoom->TextSupport().SetText(rstl::wstring(gpStringTable->GetString("LogbookZoomInstructions")), false);
  zoom->TextSupport().SetFontColor(gpTweakGui->GetLogBookLegendWindowFontColor());
  mScanPercentage = static_cast< CGuiTextPane* >(mFrame->FindWidget("textpane_percent"));
  mScanPercentage->TextSupport().SetFontColor(gpTweakGui->GetLogBookLegendWindowFontColor());
  mItemPercentage = static_cast< CGuiTextPane* >(mFrame->FindWidget("textpane_percent1"));
  mItemPercentage->TextSupport().SetFontColor(gpTweakGui->GetLogBookLegendWindowFontColor());
  mLeftStickInstructions = static_cast< CGuiTextPane* >(mFrame->FindWidget("textpane_instructions"));
  mLeftStickInstructions->TextSupport().SetFontColor(gpTweakGui->GetLogBookLegendWindowFontColor());
  mRightStickInstructions = static_cast< CGuiTextPane* >(mFrame->FindWidget("textpane_right"));
  mRightStickInstructions->TextSupport().SetFontColor(gpTweakGui->GetLogBookLegendWindowFontColor());
  mFrameLoader = nullptr;
  mTransitionState = kTS_FadeIn;
  mAlpha = 0.f;
}

void CPauseScreen::Initialize(const CStateManager& mgr) {
  mScanTree.RefreshVisibility(const_cast< CStateManager& >(mgr));
  mScanTree.RefreshViewed(const_cast< CStateManager& >(mgr));
  const rstl::pair< uint, uint > scans = mScanTree.GetScanCounts();
  const int scanPercentage = int((100.f * int(scans.first)) / int(scans.second));
  char text[256];
  sprintf(text, "%d%%", scanPercentage);
  mScanPercentage->TextSupport().SetText(
      gpStringTable->GetString("ScansPercentage") +
          CStringExtras::ConvertToUNICODE(rstl::string(text)), false);
  sprintf(text, "%d%%", mgr.GetPlayerState(0)->GetItemPercentageRatio());
  mItemPercentage->TextSupport().SetText(
      gpStringTable->GetString("ItemsPercentage") +
          CStringExtras::ConvertToUNICODE(rstl::string(text)), false);

  mOpenedFromScan = false;
  const TUniqueId target = mgr.GetPlayer(0)->GetOrbitTargetId();
  if (mgr.GetPlayer(0)->GetPlayerScanState() == CPlayer::kSS_ScanComplete &&
      target != kInvalidUniqueId) {
    const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(target));
    if (actor != nullptr) {
      const CScannableObjectInfo* scan = actor->GetScannableObjectInfo();
      if (scan != nullptr) {
        mScanTree.SelectScan(scan->GetScannableObjectId(), 0.f);
        LoadScan(mScanTree.GetSelectedNode());
        UpdateHistoryText();
        mOpenedFromScan = true;
      }
    }
  }
}

void CPauseScreen::TrackTexture(const TToken< CTexture >& texture, bool restoreToARAM) {
  mTexturesToRestore.push_back(rstl::pair< bool, TToken< CTexture > >(restoreToARAM, texture));
}

bool CPauseScreen::EnsureTextureLoaded(const CToken& token) {
  if (token.GetTag().type == FourCC('TXTR')) {
    TToken< CTexture > texture(token);
    const int status = texture->GetBitmapDataStatus();
    if (status != 0) {
      if (status == 1) {
        TrackTexture(texture, true);
      }
      if (status == 2) {
        TrackTexture(texture, false);
      }
      if (!texture->TryReloadBitmapData(*gpResourceFactory)) {
        return false;
      }
    }
  }
  return true;
}

void CPauseScreen::RestoreTextures() {
  CTexture::sCurrentFrameCount = INT_MAX;
  for (rstl::list< rstl::pair< bool, TToken< CTexture > > >::iterator it =
           mTexturesToRestore.begin();
       it != mTexturesToRestore.end(); ++it) {
    CTexture& texture = **it->second;
    bool transferred = false;
    if (!texture.GetNoSwap() && it->first) {
      texture.LoadToARAM();
      if (texture.IsARAMTransferInProgress()) {
        while (texture.IsARAMTransferInProgress()) {
          CARAMToken::UpdateAllDMAs();
        }
      }
      transferred = true;
    }
    if (!transferred) {
      texture.UnloadBitmapData(it->second.GetTag().id);
    }
  }
  mTexturesToRestore.clear();
  CTexture::sCurrentFrameCount = 0;
}

bool CPauseScreen::CheckLoadComplete(const CStateManager& mgr) {
  if (!gpResourceFactory->GetResLoader().AreAllPaksLoaded()) {
    return false;
  }
  if (mScanTree.IsLoaded() && mScanTree.GetRootNode() == -1) {
    mScanTree.LoadAsync();
    return false;
  }
  if (mLoadState == kLS_ScanInfo) {
    if (!mScanInfo.null()) {
      if (!mScanInfo->IsLoaded()) {
        return true;
      }
      const CScannableObjectInfo& scan = *mScanInfo->GetObject();
      mModelPitch = CRelAngle::FromDegrees(scan.GetModelInitialPitch());
      mModelYaw = CRelAngle::FromDegrees(scan.GetModelInitialYaw());
      mModelTokens.clear();
      mModels.clear();
      mDependencies.clear();
      mModelsReady = false;
      mDependencies.reserve(scan.GetDependencies().size());
      for (rstl::vector< SObjectTag >::const_iterator it = scan.GetDependencies().begin();
           it != scan.GetDependencies().end(); ++it) {
        if (it->type != FourCC('AGSC')) {
          mDependencies.push_back(gpSimplePool->GetObj(*it));
          mDependencies.back().Lock();
        }
      }
      for (int i = 0; i < 11; ++i) {
        if (scan.GetAnimatedModelId(i) != kInvalidAssetId) {
          mModelTokens.push_back(gpSimplePool->GetObj(SObjectTag('ANCS', scan.GetAnimatedModelId(i))));
          mModelTokens[i]->Lock();
        } else if (scan.GetStaticModelId(i) != kInvalidAssetId) {
          mModelTokens.push_back(gpSimplePool->GetObj(SObjectTag('CMDL', scan.GetStaticModelId(i))));
          mModelTokens[i]->Lock();
        } else {
          mModelTokens.push_back(rstl::optional_object< CToken >());
        }
      }
      mScanStrings = rs_new TCachedToken< CStringTable >(
          gpSimplePool->GetObj(SObjectTag('STRG', scan.GetStringTableId())));
      mScanStrings->Lock();
    }
    mLoadState = kLS_ScanText;
  }
  if (mLoadState == kLS_ScanText) {
    if (!mScanStrings.null()) {
      if (!mScanStrings->IsLoaded()) {
        return true;
      }
      mMessage->TextSupport().SetText(rstl::wstring(mScanStrings->GetObject()->GetString(2)), true);
      if (!mMessage->TextSupport().GetIsTextSupportFinishedLoading()) {
        return true;
      }
      mMessage->TextSupport().SetPage(0);
      mPageCount = mMessage->TextSupport().GetTotalPageCount();
    }
    mLoadState = kLS_ScanModels;
  }
  if (mLoadState == kLS_ScanModels) {
    for (int i = 0; i < mModelTokens.size(); ++i) {
      if (mModelTokens[i].valid() && mModelTokens[i]->HasLock() && !mModelTokens[i]->IsLoaded()) {
        return true;
      }
    }
    for (rstl::vector< CToken >::const_iterator it = mDependencies.begin();
         it != mDependencies.end(); ++it) {
      if (!it->IsLoaded() || !EnsureTextureLoaded(*it)) {
        return true;
      }
    }
    if (mModels.empty()) {
      for (int i = 0; i < mModelTokens.size(); ++i) {
        if (mModelTokens[i].valid() && mModelTokens[i]->HasLock() && mModelTokens[i]->IsLoaded() &&
            !mScanInfo.null() && mScanInfo->GetObject() != nullptr) {
          mModels.push_back(mScanInfo->GetObject()->CreateModel(i));
        } else {
          mModels.push_back(rstl::auto_ptr< CModelData >(nullptr));
        }
      }
    }
    for (int i = 0; i < mModels.size(); ++i) {
      CModelData* model = mModels[i].get();
      if (model != nullptr) {
        if (!model->IsNull()) {
          model->Touch(CModelData::kWM_Normal, 0);
        }
        if (!model->IsLoaded(0)) {
          return true;
        }
        if (!model->HasAnimation()) {
          const CCubeModel* instance = model->PickStaticModel(CModelData::kWM_Normal)->GetModelInstance();
          if (instance != nullptr) {
            const rstl::vector< TCachedToken< CTexture > >& textures = instance->GetTextures();
            for (rstl::vector< TCachedToken< CTexture > >::const_iterator it = textures.begin();
                 it != textures.end(); ++it) {
              const TCachedToken< CTexture > texture = *it;
              if (!EnsureTextureLoaded(CToken(texture))) {
                return true;
              }
            }
          }
        }
      }
    }
    CAABox bounds = CAABox::MakeMaxInvertedBox();
    for (int i = 0; i < mModels.size(); ++i) {
      CModelData* model = mModels[i].get();
      if (model != nullptr && !model->IsNull()) {
        mModelFade = 0.f;
        model->Touch(CModelData::kWM_Normal, 0);
        model->EnableLooping(true);
        if (model->HasAnimation()) {
          CRandom16 random(0);
          model->AdvanceAnimation(0.02f, random, true);
          const CAABox modelBounds = model->AnimationData()->CalcBoundingBoxFromModelVerts();
          bounds.AccumulateBounds(modelBounds.GetMinPoint());
          bounds.AccumulateBounds(modelBounds.GetMaxPoint());
        } else {
          const CAABox modelBounds = model->GetBounds();
          bounds.AccumulateBounds(modelBounds.GetMinPoint());
          bounds.AccumulateBounds(modelBounds.GetMaxPoint());
        }
        mModelCenterOffset = -bounds.GetCenterPoint();
        const CVector3f extent = bounds.GetMaxPoint() - bounds.GetMinPoint();
        const float maxExtent = rstl::max_val(rstl::max_val(extent.GetX(), extent.GetZ()), extent.GetY());
        const float scale = (gpTweakGui->GetLogBookScanModelScale() *
                             mScanInfo->GetObject()->GetModelScale()) / maxExtent;
        mModelScale = CVector3f(scale, scale, scale);
      }
    }
    mModelsReady = true;
    mLoadState = kLS_Ready;
  }
  if (mLoadState == kLS_ScanTree) {
    if (!mScanTree.PollLoad()) {
      return false;
    }
    mLoadState = kLS_Ready;
  }
  if (!mFrameLoader.null() && !mFrameLoader->IsFinishedLoading()) {
    return false;
  }
  if (mFrame.null() && !mFrameLoader.null()) {
    InitializeFrameGlue();
    Initialize(mgr);
  }
  if (mFrame.null()) {
    return false;
  }
  mFont.IsLoaded();
  mUnselectedNodeTexture.IsLoaded();
  mSelectedNodeTexture.IsLoaded();
  mParentNodeTexture.IsLoaded();
  mSelectedCursorTexture.IsLoaded();
  mHighlightTexture.IsLoaded();
  mSliderModel.IsLoaded();
  mSliderEndModel.IsLoaded();
  mSliderCenterModel.IsLoaded();
  mMenuArrowModel.IsLoaded();
  mOptionBackgroundModel.IsLoaded();
  mScanSweepTexture.IsLoaded();
  if (!mNodesTouched) {
    TouchVisibleNodes();
    mNodesTouched = true;
  }
  return true;
}

void CPauseScreen::UpdatePulse(float dt) {
  mPulseTime += dt / gpTweakGui->GetMapBackgroundCycleTime();
  if (mPulseTime >= 1.f) {
    mPulseTime = 0.f;
  }
  const float width = gpTweakGui->GetMapBackgroundPulseWidth();
  const float pulse =
      1.f - ((1.f - mPulseTime) * (mPulseTime - width) + mPulseTime * (mPulseTime + width));
  const float count = mHexWidgets.size();
  int index = 0;
  for (rstl::vector< CGuiWidget* >::const_iterator it = mHexWidgets.begin();
       it != mHexWidgets.end(); ++it, ++index) {
    const float brightness = 1.f - rstl::min_val(1.f, CMath::AbsF(pulse - index / count) / width);
    (*it)->SetColor(CColor::Modulate(gpTweakGui->GetMapBackgroundColor(),
                                     CColor(brightness, brightness, brightness, 1.f)));
  }
}

void CPauseScreen::Update(float dt, const CStateManager& mgr, CArchitectureQueue& queue) {
  if (mDone || !CheckLoadComplete(mgr)) {
    return;
  }
  if (!mHistoryTextReady) {
    UpdateHistoryText();
  }
  if (!mLegendVisible) {
    mLegendHiddenAmount += dt / gpTweakGui->GetLogBookLegendHideTime();
  } else {
    mLegendHiddenAmount -= dt / gpTweakGui->GetLogBookLegendHideTime();
  }
  mLegendHiddenAmount = CMath::Clamp(0.f, mLegendHiddenAmount, 1.f);
  if (mBottomPane != nullptr) {
    mBottomPane->SetColor(CColor::White().WithAlphaModulatedBy(1.f - mLegendHiddenAmount));
    mBottomPane->SetVisibility(!close_enough(mLegendHiddenAmount, 1.f), kTM_Children);
  }
  if (mScanInfoGroup != nullptr) {
    mScanInfoGroup->SetO2PTransform(mScanInfoGroup->GetIdleXform() *
                                  CTransform4f::Translate(0.f, 0.f, -4.3f * mLegendHiddenAmount));
  }
  if (mModelZoomed) {
    mModelZoomAmount = 2.f * dt + mModelZoomAmount;
  } else {
    mModelZoomAmount = -(2.f * dt - mModelZoomAmount);
  }
  mModelZoomAmount = CMath::Clamp(0.f, mModelZoomAmount, 1.f);
  mSelectionDelay = rstl::max_val(mSelectionDelay - dt, 0.f);
  mSelectionHighlight = rstl::max_val(-(3.f * dt - mSelectionHighlight), 0.f);
  mModelFade = rstl::min_val(mModelFade + dt / gpTweakGui->GetLogBookScanObjectFadeInTime(), 1.f);
  mFrame->Update(dt);
  if (mQuitScreen.get() == nullptr) {
    mScanTree.Update(dt);
  } else {
    const EQuitAction action = mQuitScreen->Update(dt);
    if (action == kQA_No) {
      mQuitScreen = rstl::auto_ptr< CQuitGameScreen >(nullptr);
    } else if (action == kQA_Yes) {
      queue.Push(MakeMsg::CreateQuitGameplay(kAMT_Game));
    }
  }
  UpdateHistoryColors();
  UpdatePulse(dt);
  if (mPendingScanNode != -1) {
    mScanTree.MarkViewed(const_cast< CStateManager& >(mgr), mPendingScanNode);
    mPendingScanNode = -1;
  }
  const float transition = mScanTree.GetTransition();
  const rstl::rc_ptr< CScanTreeNode > node = mScanTree.GetNode(mScanTree.GetSelectedNode());
  const CVector3f position = node->GetDisplayPosition();
  mRotationVelocity += mRotationInput * dt;
  mRotationVelocity *= 0.97f;
  mRotationInput = CVector2f::Zero();
  if (mRotationVelocity.MagSquared() > 1166400.f) {
    mRotationVelocity = mRotationVelocity.AsNormalized() * 1080.f;
  }
  const CVector2f rotation = mRotationVelocity * dt;
  const CTransform4f view = mViewRotation.BuildTransform4f();
  const CVector3f selectionOffset = -2.f * view.GetColumn(kDY);
  mViewRotation = mViewRotation * CQuaternion::ZRotation(CRelAngle::FromDegrees(rotation.GetX())) *
                  CQuaternion::XRotation(CRelAngle::FromDegrees(rotation.GetY()));
  if (close_enough(transition, 0.f) && close_enough(mSelectionDelay, 0.f)) {
    float bestDistance = 10000.f;
    if (node->GetNodeType() == CScanTreeNode::kNT_Category) {
      const rstl::rc_ptr< CScanTreeNode > categoryNode(node);
      CScanTreeCategory& category = static_cast< CScanTreeCategory& >(*categoryNode);
      const int count = category.GetChildCount();
      const int selected = category.GetSelectedChild();
      for (int i = 0; i < count; ++i) {
        const int id = category.GetChild(i);
        const rstl::rc_ptr< CScanTreeNode > child = mScanTree.GetNode(id);
        if (!child->IsVisible()) {
          continue;
        }
        const float distance = (id == selected ? 1.f : 1.5f) *
            (child->GetDisplayPosition() - position - selectionOffset).MagSquared();
        if (distance < bestDistance) {
          category.SetSelectedChild(id);
          bestDistance = distance;
        }
      }
      if (selected != category.GetSelectedChild()) {
        CSfxManager::SfxStart(0x21ce, 0x7f, 0x3f, CSfxManager::kAllAreas, false, false,
                              CSfxManager::kMedPriority);
        mSelectionHighlight = 1.f;
      }
    }
  }
  if (!mModels.empty()) {
    const CVector3f lightPositions[] = {gpTweakGui->GetLogBookModelLight1Position(),
                                      gpTweakGui->GetLogBookModelLight2Position()};
    const CVector3f* lightPosition = lightPositions;
    for (rstl::vector< CLight >::iterator it = mLights.begin(); it != mLights.end(); ++it) {
      it->SetPosition(mViewRotation.BuildTransform4f() *
                     (*lightPosition + (GetModelPosition() - GetDefaultModelPosition())));
      ++lightPosition;
    }
    mActorLights->BuildFakeLightList(mLights, gpTweakGui->GetLogBookModelAmbientLightColor());
    const rstl::rc_ptr< CScanTreeNode > current = mScanTree.GetNode(mScanTree.GetSelectedNode());
    const CVector3f scale = (1.f + 0.5f * mModelZoomAmount) * mModelScale;
    mModelTransform = view.GetRotation() * CTransform4f::Translate(GetModelPosition()) *
                      CTransform4f::RotateX(mModelPitch) * CTransform4f::RotateZ(mModelYaw) *
                      CTransform4f::Scale(scale) * CTransform4f::Translate(mModelCenterOffset);
    if (!close_enough(mScanTree.GetTransition(), 0.f) ||
        current->GetNodeType() == CScanTreeNode::kNT_Scan ||
        current->GetNodeType() == CScanTreeNode::kNT_Inventory) {
      for (int i = 0; i < mModels.size(); ++i) {
        if (mModels[i].get() != nullptr && mModels[i]->HasAnimation()) {
          CRandom16 random(0);
          mModels[i]->AdvanceAnimation(dt, random, true);
          mModels[i]->AnimationData()->AdvanceParticles(mModelTransform, dt, CVector3f::One(), nullptr);
        }
      }
    } else {
      RestoreTextures();
      mModels.clear();
      mModelTokens.clear();
      mDependencies.clear();
      mModelTokens = rstl::reserved_vector< rstl::optional_object< CToken >, 11 >();
      mModels = rstl::reserved_vector< rstl::auto_ptr< CModelData >, 11 >();
      mDependencies = rstl::vector< CToken >();
    }
  }
  float scanAlpha = 0.f;
  if (node->GetNodeType() == CScanTreeNode::kNT_Scan ||
      node->GetNodeType() == CScanTreeNode::kNT_Inventory) {
    scanAlpha = 1.f - transition;
  }
  const int previousId = mScanTree.GetPreviousNode();
  if (previousId != -1) {
    const rstl::rc_ptr< CScanTreeNode > previous = mScanTree.GetNode(previousId);
    if (previous->GetNodeType() == CScanTreeNode::kNT_Scan ||
        previous->GetNodeType() == CScanTreeNode::kNT_Inventory) {
      scanAlpha = transition;
    }
  }
  const float visibleAlpha = (1.f - mModelZoomAmount) * scanAlpha;
  if (mScanInfoGroup != nullptr) {
    mScanInfoGroup->SetColor(CColor::Modulate(CColor::White(),
        CColor(visibleAlpha, visibleAlpha, visibleAlpha, 1.f)).WithAlphaModulatedBy(visibleAlpha));
    mScanInfoGroup->SetVisibility(!close_enough(visibleAlpha, 0.f), kTM_Children);
    if (mPage < mPageCount - 1) {
      mAdvanceButton->SetColor(CColor::White().WithAlphaModulatedBy(
          0.5f * (1.f + CMath::FastCosR(5.f * CGraphics::GetSecondsMod900()))));
    } else {
      mAdvanceButton->SetColor(CColor::White().WithAlphaModulatedBy(0.f));
    }
  }
  mRightInstructions->SetColor(CColor::White().WithAlphaOf(
      rstl::max_val(1.f - mLegendHiddenAmount, (1.f - scanAlpha) * (1.f - scanAlpha))));
  const wchar_t imagePrefix[] = L"&image=";
  const wchar_t separator[] = L";";
  rstl::wstring text;
  text.reserve(256);
  text.append(imagePrefix, -1);
  text.append(CStringExtras::ConvertToUNICODE(rstl::string(
      CBasics::Stringize("SI,0.6,1.0,%8.8X", gpTweakPlayerRes->mLStick[mLeftStickIcon]))));
  text.append(separator, -1);
  text.append(gpStringTable->GetString("InstructionRotate"), -1);
  mLeftStickInstructions->TextSupport().SetText(text, false);
  text.assign(imagePrefix, -1);
  text.append(CStringExtras::ConvertToUNICODE(rstl::string(
      CBasics::Stringize("SI,0.6,1.0,%8.8X", gpTweakPlayerRes->mCStick[mRightStickIcon]))));
  text.append(separator, -1);
  text.append(gpStringTable->GetString("InstructionMove"), -1);
  mRightStickInstructions->TextSupport().SetText(text, false);
  if (mTransitionState == kTS_FadeIn) {
    mAlpha = rstl::min_val(mAlpha + dt / 0.4f, 1.f);
    if (mAlpha == 1.f) {
      mTransitionState = kTS_Active;
    }
  } else if (mTransitionState == kTS_FadeOut) {
    mAlpha = rstl::max_val(mAlpha - dt / 0.4f, 0.f);
    if (mAlpha == 0.f) {
      mDone = true;
      gpResourceFactory->GetResLoader().RemovePakFile("logbook");
      mFrame = rstl::auto_ptr< CGuiFrame >(nullptr);
      gpGameState->RecordCompressedGameOptions(gpGameState->SystemOptions().GetSaveIdx());
      CSfxManager::SfxStop(mRotateSfx);
      CSfxManager::SfxStop(mPanSfx);
      CSfxManager::SfxStop(mZoomSfx);
    }
  }
}

void CPauseScreen::TouchVisibleNodes() {
  int selected = mScanTree.GetSelectedNode();
  if (selected == -1) {
    selected = mScanTree.GetRootNode();
  }
  rstl::rc_ptr< CScanTreeNode > node = mScanTree.GetNode(selected);
  node->LockResources();
  if (node->GetNodeType() == CScanTreeNode::kNT_Category) {
    const CScanTreeCategory& category = static_cast< const CScanTreeCategory& >(*node);
    const int count = category.GetChildCount();
    for (int i = 0; i < count; ++i) {
      rstl::rc_ptr< CScanTreeNode > child = mScanTree.GetNode(category.GetChild(i));
      child->LockResources();
    }
  }
}

void CPauseScreen::ProcessControllerInput(const CFinalInput& input) {
  if (!mQuitScreen.null()) {
    mQuitScreen->ProcessUserInput(input);
  } else if (mLoadState != kLS_ScanTree && mTransitionState == kTS_Active) {
    ProcessRotationInput(input);
    ProcessSelectionInput(input);
    ProcessButtonInput(input);
    UpdateStickIcons(input);
  }
}

void CPauseScreen::LoadScan(int nodeId) {
  rstl::rc_ptr< CScanTreeNode > node = mScanTree.GetNode(nodeId);
  if (node->GetNodeType() == CScanTreeNode::kNT_Scan ||
      node->GetNodeType() == CScanTreeNode::kNT_Inventory) {
    const CScanTreeScan& scan = static_cast< const CScanTreeScan& >(*node);
    mPendingScanNode = nodeId;
    mLoadState = kLS_ScanInfo;
    mScanInfo = rs_new TCachedToken< CScannableObjectInfo >(
        gpSimplePool->GetObj(SObjectTag('SCAN', scan.GetScannableInfo())));
    mScanInfo->Lock();
    mPage = 0;
    mPageCount = 0;
    mMessage->TextSupport().SetText(rstl::string(""), false);
    mModelPan = CVector3f::Zero();
    CSfxManager::SfxStart(0x56c, 0x7f, 0x3f, CSfxManager::kAllAreas, false, false,
                          CSfxManager::kMedPriority);
  }
}

void CPauseScreen::SelectNode(int nodeId) {
  if (nodeId != -1) {
    bool select = true;
    rstl::rc_ptr< CScanTreeNode > node = mScanTree.GetNode(nodeId);
    if (node->GetNodeType() == CScanTreeNode::kNT_Scan ||
        node->GetNodeType() == CScanTreeNode::kNT_Inventory) {
      LoadScan(nodeId);
    } else if (node->GetNodeType() == CScanTreeNode::kNT_Menu ||
               node->GetNodeType() == CScanTreeNode::kNT_Slider) {
      if (node->GetNodeType() == CScanTreeNode::kNT_Slider) {
        CScanTreeSlider& slider = static_cast< CScanTreeSlider& >(*node);
        slider.RefreshNormalizedValue();
        slider.SaveValue();
      } else {
        CScanTreeMenu& menu = static_cast< CScanTreeMenu& >(*node);
        menu.RefreshSelectedOption();
        if (menu.GetSetting() == 7) {
          mQuitScreen = rs_new CQuitGameScreen(kQT_QuitGame, 0);
          select = false;
          CSfxManager::SfxStart(600, 0x7f, 0x3f, CSfxManager::kAllAreas, false, false,
                                CSfxManager::kMedPriority);
          CSfxManager::SfxStop(mRotateSfx);
          mRotateSfx = CSfxHandle();
        }
      }
      mRotationInput = CVector2f::Zero();
      mRotationVelocity = CVector2f::Zero();
    } else {
      mScanTree.InitializeNodePositions(nodeId);
      mScanTree.RandomizeChildPositions(nodeId);
    }
    if (select) {
      if (node->GetNodeType() == CScanTreeNode::kNT_Category) {
        CSfxManager::SfxStart(0x21cf, 0x7f, 0x3f, CSfxManager::kAllAreas, false, false,
                              CSfxManager::kMedPriority);
      } else if (node->GetNodeType() == CScanTreeNode::kNT_Menu ||
                 node->GetNodeType() == CScanTreeNode::kNT_Slider) {
        CSfxManager::SfxStart(600, 0x7f, 0x3f, CSfxManager::kAllAreas, false, false,
                              CSfxManager::kMedPriority);
      } else {
        CSfxManager::SfxStart(0x255, 0x7f, 0x3f, CSfxManager::kAllAreas, false, false,
                              CSfxManager::kMedPriority);
      }
      mScanTree.SelectNode(nodeId);
    }
  }
  UpdateHistoryText();
}

void CPauseScreen::AdvancePage() {
  rstl::rc_ptr< CScanTreeNode > node = mScanTree.GetNode(mScanTree.GetSelectedNode());
  if (mPage == mPageCount - 1) {
    const int parent = node->GetParentNode();
    if (parent == -1) {
      mTransitionState = kTS_FadeOut;
      CSfxManager::SfxStart(0x21d0, 0x7f, 0x3f, CSfxManager::kAllAreas, false, false,
                            CSfxManager::kMedPriority);
    } else {
      CSfxManager::SfxStart(0x21cf, 0x7f, 0x3f, CSfxManager::kAllAreas, false, false,
                            CSfxManager::kMedPriority);
      mScanTree.SelectNode(parent);
    }
  } else {
    CSfxManager::SfxStart(0x21d4, 0x7f, 0x3f, CSfxManager::kAllAreas, false, false,
                          CSfxManager::kMedPriority);
    mPage = rstl::min_val(mPageCount - 1, mPage + 1);
    mMessage->TextSupport().SetPage(mPage);
  }
}

void CPauseScreen::FinishOptionEdit(bool accept) {
  rstl::rc_ptr< CScanTreeNode > node = mScanTree.GetNode(mScanTree.GetSelectedNode());
  if (node->GetNodeType() == CScanTreeNode::kNT_Slider) {
    CScanTreeSlider& slider = static_cast< CScanTreeSlider& >(*node);
    if (accept) {
      if (!close_enough(slider.GetSavedNormalizedValue(), slider.GetNormalizedValue())) {
        CSfxManager::SfxStart(0xbfd, 0x7f, 0x3f, CSfxManager::kAllAreas, false, false,
                              CSfxManager::kMedPriority);
      }
    } else {
      slider.RestoreSavedValue();
    }
  } else {
    CScanTreeMenu& menu = static_cast< CScanTreeMenu& >(*node);
    if (!accept) {
      menu.ApplySelectedOption();
    } else {
      if (menu.GetCurrentOptionIndex() == 1) {
        CGameOptions& options = gpGameState->GameOptions();
        switch (menu.GetSetting()) {
        case 8:
          options.ResetSoundToDefaults();
          break;
        case 9:
          options.ResetScreenToDefaults();
          break;
        case 10:
          options.ResetExtraFlagsToDefaults();
          break;
        case 11:
          options.ResetVisorToDefaults();
          break;
        }
        if (menu.GetSetting() >= 8 && menu.GetSetting() < 12) {
          menu.ApplyOption(0);
          CSfxManager::SfxStart(0x5e0, 0x3c, 0x3f, CSfxManager::kAllAreas, false, false,
                                CSfxManager::kMedPriority);
        }
      }
      if (menu.GetSetting() < 8 || menu.GetSetting() > 11) {
        if (menu.GetSelectedOption() != menu.GetCurrentOptionIndex()) {
          CSfxManager::SfxStart(0xbfd, 0x7f, 0x3f, CSfxManager::kAllAreas, false, false,
                                CSfxManager::kMedPriority);
        }
      }
    }
  }
  mScanTree.SelectNode(node->GetParentNode());
  UpdateHistoryText();
  CSfxManager::SfxStart(599, 0x7f, 0x3f, CSfxManager::kAllAreas, false, false,
                        CSfxManager::kMedPriority);
}

void CPauseScreen::ProcessButtonInput(const CFinalInput& input) {
  if (!close_enough(mScanTree.GetTransition(), 0.f)) {
    return;
  }
  if (input.PA()) {
    rstl::rc_ptr< CScanTreeNode > node = mScanTree.GetNode(mScanTree.GetSelectedNode());
    if (node->GetNodeType() == CScanTreeNode::kNT_Category) {
      SelectNode(static_cast< CScanTreeCategory& >(*node).GetSelectedChild());
    } else if ((node->GetNodeType() == CScanTreeNode::kNT_Scan ||
                node->GetNodeType() == CScanTreeNode::kNT_Inventory) &&
               close_enough(mModelZoomAmount, 0.f)) {
      if (mMessage != nullptr) {
        AdvancePage();
      }
    } else if (node->GetNodeType() == CScanTreeNode::kNT_Menu ||
               node->GetNodeType() == CScanTreeNode::kNT_Slider) {
      FinishOptionEdit(true);
    }
  } else if (input.PB()) {
    rstl::rc_ptr< CScanTreeNode > node = mScanTree.GetNode(mScanTree.GetSelectedNode());
    const int parent = node->GetParentNode();
    if (mModelZoomed) {
      mModelZoomed = false;
      CSfxManager::SfxStart(0x10cb, 0x7f, 0x3f, CSfxManager::kAllAreas, false, false,
                            CSfxManager::kMedPriority);
    } else if (node->GetNodeType() == CScanTreeNode::kNT_Menu ||
               node->GetNodeType() == CScanTreeNode::kNT_Slider) {
      FinishOptionEdit(false);
    } else if (parent == -1 || mOpenedFromScan) {
      mTransitionState = kTS_FadeOut;
      CSfxManager::SfxStart(0x21d0, 0x7f, 0x3f, CSfxManager::kAllAreas, false, false,
                            CSfxManager::kMedPriority);
    } else {
      mScanTree.SelectNode(parent);
      UpdateHistoryText();
      CSfxManager::SfxStart(0x21cd, 0x7f, 0x3f, CSfxManager::kAllAreas, false, false,
                            CSfxManager::kMedPriority);
    }
  } else if (input.PStart()) {
    mTransitionState = kTS_FadeOut;
    CSfxManager::SfxStart(0x21d0, 0x7f, 0x3f, CSfxManager::kAllAreas, false, false,
                          CSfxManager::kMedPriority);
  } else if (input.PY()) {
    mLegendVisible = !mLegendVisible;
    gpGameState->SystemOptions().FindEnvironmentVariable("LogbookLegendVisible")->Set(mLegendVisible);
    if (!close_enough(mModelZoomAmount, 1.f)) {
      if (mLegendVisible) {
        CSfxManager::SfxStart(0x13b7, 0x7f, 0x3f, CSfxManager::kAllAreas, false, false,
                              CSfxManager::kMedPriority);
      } else {
        CSfxManager::SfxStart(0x13b6, 0x7f, 0x3f, CSfxManager::kAllAreas, false, false,
                              CSfxManager::kMedPriority);
      }
    }
  } else if (input.PX()) {
    rstl::rc_ptr< CScanTreeNode > node = mScanTree.GetNode(mScanTree.GetSelectedNode());
    if (node->GetNodeType() == CScanTreeNode::kNT_Scan ||
        node->GetNodeType() == CScanTreeNode::kNT_Inventory) {
      mModelZoomed = !mModelZoomed;
      if (mModelZoomed) {
        CSfxManager::SfxStart(0x10ca, 0x7f, 0x3f, CSfxManager::kAllAreas, false, false,
                              CSfxManager::kMedPriority);
      } else {
        CSfxManager::SfxStart(0x10cb, 0x7f, 0x3f, CSfxManager::kAllAreas, false, false,
                              CSfxManager::kMedPriority);
      }
      if (!mModelZoomed) {
        SetZoomSound(false);
        SetPanSound(false);
      }
    }
  }
}

void CPauseScreen::ProcessSelectionInput(const CFinalInput& input) {
  const float amount = 2.f * input.DeltaTime();
  float x = amount * (input.GetAnalogRightX() * gpTweakGui->GetLogBookRotationSpeed());
  float y = amount * (input.GetAnalogRightY() * gpTweakGui->GetLogBookRotationSpeed());
  if (CMath::AbsF(x) < 0.01f) {
    x = 0.f;
  }
  if (CMath::AbsF(y) < 0.01f) {
    y = 0.f;
  }
  const CTransform4f view = mViewRotation.BuildTransform4f();
  rstl::rc_ptr< CScanTreeNode > node = mScanTree.GetNode(mScanTree.GetSelectedNode());
  const CVector3f direction(x, 0.f, y);
  float best = 10000.f;
  if (!direction.CanBeNormalized()) {
    return;
  }
  const CVector3f normalized = direction.AsNormalized();
  if (node->GetNodeType() == CScanTreeNode::kNT_Category) {
    CScanTreeCategory& category = static_cast< CScanTreeCategory& >(*node);
    const int count = category.GetChildCount();
    const int selected = category.GetSelectedChild();
    rstl::rc_ptr< CScanTreeNode > selectedNode = mScanTree.GetNode(selected);
    const CVector3f position = node->GetDisplayPosition();
    for (int i = 0; i < count; ++i) {
      const int childId = category.GetChild(i);
      rstl::rc_ptr< CScanTreeNode > child = mScanTree.GetNode(childId);
      if (!child->IsVisible()) {
        continue;
      }
      const CVector3f delta = child->GetDisplayPosition() - position;
      float score = delta.MagSquared();
      const CVector3f projected = view.TransposeRotate(delta);
      const CVector3f flat(projected.GetX(), 0.f, projected.GetZ());
      if (flat.CanBeNormalized()) {
        const float dot = CVector3f::Dot(normalized, flat.AsNormalized());
        if (dot < 0.85f) {
          continue;
        }
        score *= 1.f / (1.1f + dot);
      }
      if (score < best) {
        category.SetSelectedChild(childId);
        best = score;
      }
    }
    if (selected != category.GetSelectedChild()) {
      CSfxManager::SfxStart(0x21ce, 0x7f, 0x3f, CSfxManager::kAllAreas, false, false,
                            CSfxManager::kMedPriority);
      mSelectionDelay = 1.f;
      mSelectionHighlight = 1.f;
    }
  }
}

void CPauseScreen::SetPanSound(bool playing) {
  if (playing) {
    if (mPanSfx == CSfxHandle()) {
      mPanSfx = CSfxManager::SfxStart(300, 0x7f, 0x3f, CSfxManager::kAllAreas, false, true,
                                      CSfxManager::kMedPriority);
    }
  } else if (mPanSfx != CSfxHandle()) {
    CSfxManager::SfxStop(mPanSfx);
    mPanSfx = CSfxHandle();
  }
}

void CPauseScreen::SetZoomSound(bool playing) {
  if (playing) {
    if (mZoomSfx == CSfxHandle()) {
      mZoomSfx = CSfxManager::SfxStart(0x78, 0x7f, 0x3f, CSfxManager::kAllAreas, false, true,
                                       CSfxManager::kMedPriority);
    }
  } else if (mZoomSfx != CSfxHandle()) {
    CSfxManager::SfxStop(mZoomSfx);
    mZoomSfx = CSfxHandle();
  }
}

void CPauseScreen::ProcessRotationInput(const CFinalInput& input) {
  const float amount = 100.f * input.DeltaTime();
  float x = amount * (-input.GetAnalogLeftX() * gpTweakGui->GetLogBookRotationSpeed());
  float y = amount * (input.GetAnalogLeftY() * gpTweakGui->GetLogBookRotationSpeed());
  if (CMath::AbsF(x) < 0.01f) {
    x = 0.f;
  }
  if (CMath::AbsF(y) < 0.01f) {
    y = 0.f;
  }
  if (!close_enough(x, 0.f) || !close_enough(y, 0.f)) {
    if (mRotateSfx == CSfxHandle()) {
      rstl::rc_ptr< CScanTreeNode > node = mScanTree.GetNode(mScanTree.GetSelectedNode());
      if (node->GetNodeType() == CScanTreeNode::kNT_Scan ||
          node->GetNodeType() == CScanTreeNode::kNT_Inventory) {
        mRotateSfx = CSfxManager::SfxStart(0x22bf, 0x7f, 0x3f, CSfxManager::kAllAreas, false, true,
                                          CSfxManager::kMedPriority);
      } else if (node->GetNodeType() == CScanTreeNode::kNT_Category) {
        mRotateSfx = CSfxManager::SfxStart(0x21d3, 0x7f, 0x3f, CSfxManager::kAllAreas, false, true,
                                          CSfxManager::kMedPriority);
      }
    }
  } else if (mRotateSfx != CSfxHandle()) {
    CSfxManager::SfxStop(mRotateSfx);
    mRotateSfx = CSfxHandle();
  }
  rstl::rc_ptr< CScanTreeNode > node = mScanTree.GetNode(mScanTree.GetSelectedNode());
  if (node->GetNodeType() == CScanTreeNode::kNT_Scan ||
      node->GetNodeType() == CScanTreeNode::kNT_Inventory) {
    ProcessModelInput(input, !close_enough(mModelZoomAmount, 0.f));
  } else if (node->GetNodeType() == CScanTreeNode::kNT_Menu) {
    CScanTreeMenu& menu = static_cast< CScanTreeMenu& >(*node);
    const int oldOption = menu.GetCurrentOptionIndex();
    const int count = menu.GetOptionCount();
    const bool left = input.DLALeft();
    const bool right = input.DLARight();
    int option = oldOption;
    if (mLeftRepeat.Update(input.DeltaTime(), left) && left && oldOption - 1 >= 0) {
      option = oldOption - 1;
    }
    if (mRightRepeat.Update(input.DeltaTime(), right) && right && oldOption + 1 < count) {
      option = oldOption + 1;
    }
    if (option != oldOption) {
      menu.ApplyOption(option);
      CSfxManager::SfxStart(0x5a8, 0x5f, 0x3f, CSfxManager::kAllAreas, false, false,
                            CSfxManager::kMedPriority);
    }
  } else if (node->GetNodeType() == CScanTreeNode::kNT_Slider) {
    CScanTreeSlider& slider = static_cast< CScanTreeSlider& >(*node);
    const float current = slider.GetNormalizedValue();
    float delta = input.DeltaTime() *
                  (input.GetAnalogLeftX() * gpTweakGui->GetLogBookSliderSpeed());
    if (close_enough(delta, 0.f)) {
      if (input.PDPLeft()) {
        delta -= 0.01f;
      } else if (input.PDPRight()) {
        delta += 0.01f;
      }
    }
    const float value = rstl::min_val(1.f, rstl::max_val(0.f, current + delta));
    if (close_enough(value, current)) {
      if (mRotateSfx != CSfxHandle()) {
        CSfxManager::SfxStop(mRotateSfx);
        mRotateSfx = CSfxHandle();
      }
    } else {
      slider.SetNormalizedValue(value);
      slider.ApplyNormalizedValue();
      if (mRotateSfx == CSfxHandle()) {
        mRotateSfx = CSfxManager::SfxStart(0x256, 0x7f, 0x3f, CSfxManager::kAllAreas, false, true,
                                          CSfxManager::kMedPriority);
      }
    }
  } else {
    mRotationInput = CVector2f(x, y) * 120.f;
  }
}

void CPauseScreen::ProcessModelInput(const CFinalInput& input, bool allowTranslation) {
  const CControlMapper& mapper = gpGameState->ControlMapper();
  const float motionAmt = 6.f * input.DeltaTime();
  const float circleUp = mapper.GetAnalogInput(CControlMapper::kC_MapCircleUp, input);
  const float circleDown = mapper.GetAnalogInput(CControlMapper::kC_MapCircleDown, input);
  const float circleLeft = mapper.GetAnalogInput(CControlMapper::kC_MapCircleLeft, input);
  const float circleRight = mapper.GetAnalogInput(CControlMapper::kC_MapCircleRight, input);
  const float moveForward = mapper.GetAnalogInput(CControlMapper::kC_MapMoveForward, input);
  const float moveBack = mapper.GetAnalogInput(CControlMapper::kC_MapMoveBack, input);
  const float moveLeft = mapper.GetAnalogInput(CControlMapper::kC_MapMoveLeft, input);
  const float moveRight = mapper.GetAnalogInput(CControlMapper::kC_MapMoveRight, input);
  const float zoomIn = mapper.GetAnalogInput(CControlMapper::kC_MapZoomIn, input);
  const float zoomOut = mapper.GetAnalogInput(CControlMapper::kC_MapZoomOut, input);
  const CVector3f oldPan = mModelPan;
  const float yaw = 0.5f * motionAmt * (circleLeft - circleRight);
  const float pitch = 0.5f * motionAmt * (circleUp - circleDown);
  if (allowTranslation) {
    mModelPan +=
        CVector3f(0.25f * motionAmt * (moveRight - moveLeft), 0.5f * motionAmt * (zoomOut - zoomIn),
                  0.25f * motionAmt * (moveForward - moveBack));
    if (mModelPan.MagSquared() > 9.f) {
      mModelPan = 3.f * mModelPan.AsNormalized();
    }
  }
  mModelPitch = CRelAngle::FromDegrees(
      CMath::Clamp(gpTweakGui->GetLogBookModelRotationClampLowerLimit(),
                   CRelAngle::FromRadians(pitch).AsDegrees() + mModelPitch.AsDegrees(),
                   gpTweakGui->GetLogBookModelRotationClampUpperLimit()));
  mModelYaw += CRelAngle::FromRadians(yaw);
  const CVector3f movement = mModelPan - oldPan;
  const bool zoomInput = !close_enough(zoomIn, 0.f) || !close_enough(zoomOut, 0.f);
  const bool panInput = !close_enough(moveForward, 0.f) || !close_enough(moveBack, 0.f) ||
                        !close_enough(moveLeft, 0.f) || !close_enough(moveRight, 0.f);
  SetZoomSound(!close_enough(movement.GetY(), 0.f) && zoomInput);
  SetPanSound((!close_enough(movement.GetX(), 0.f) || !close_enough(movement.GetZ(), 0.f)) &&
              panInput);
}

void CPauseScreen::UpdateStickIcons(const CFinalInput& input) {
  const CControlMapper& mapper = gpGameState->ControlMapper();
  const float up = mapper.GetAnalogInput(CControlMapper::kC_MapCircleUp, input);
  const float down = mapper.GetAnalogInput(CControlMapper::kC_MapCircleDown, input);
  const float left = mapper.GetAnalogInput(CControlMapper::kC_MapCircleLeft, input);
  const float right = mapper.GetAnalogInput(CControlMapper::kC_MapCircleRight, input);
  int direction = 0;
  if (up > 0.f)
    direction += 2;
  if (down > 0.f)
    direction += 1;
  if (left > 0.f)
    direction += 4;
  if (right > 0.f)
    direction += 8;
  switch (direction) {
  case 1:
    mLeftStickIcon = 1;
    break;
  case 2:
    mLeftStickIcon = 5;
    break;
  case 4:
    mLeftStickIcon = 3;
    break;
  case 5:
    mLeftStickIcon = 2;
    break;
  case 6:
    mLeftStickIcon = 4;
    break;
  case 8:
    mLeftStickIcon = 7;
    break;
  case 9:
    mLeftStickIcon = 8;
    break;
  case 10:
    mLeftStickIcon = 6;
    break;
  default:
    mLeftStickIcon = 0;
    break;
  }
  const float forward = mapper.GetAnalogInput(CControlMapper::kC_MapMoveForward, input);
  const float back = mapper.GetAnalogInput(CControlMapper::kC_MapMoveBack, input);
  const float moveLeft = mapper.GetAnalogInput(CControlMapper::kC_MapMoveLeft, input);
  const float moveRight = mapper.GetAnalogInput(CControlMapper::kC_MapMoveRight, input);
  uchar move = forward > 0.f;
  if (back > 0.f)
    move += 2;
  if (moveLeft > 0.f)
    move += 4;
  if (moveRight > 0.f)
    move += 8;
  switch (move) {
  case 1:
    mRightStickIcon = 1;
    break;
  case 2:
    mRightStickIcon = 5;
    break;
  case 4:
    mRightStickIcon = 3;
    break;
  case 5:
    mRightStickIcon = 2;
    break;
  case 6:
    mRightStickIcon = 4;
    break;
  case 8:
    mRightStickIcon = 7;
    break;
  case 9:
    mRightStickIcon = 8;
    break;
  case 10:
    mRightStickIcon = 6;
    break;
  default:
    mRightStickIcon = 0;
    break;
  }
}

void CPauseScreen::SetFog(bool enabled) const {
  if (enabled) {
    CGraphics::SetFog(kRFM_PerspLin, gpTweakGui->GetLogBookFogNear(),
                      gpTweakGui->GetLogBookFogFar(), gpTweakGui->GetLogBookFogColor());
  } else {
    CGraphics::SetFog(kRFM_None, 0.f, 0.f, CColor::Black());
  }
}

void CPauseScreen::Draw() const {
  if (mDone || !mScanTree.IsLoaded() || !mFont.IsLoaded() || mFrame.null()) {
    return;
  }
  if (!mQuitScreen.null()) {
    mQuitScreen->Draw();
    return;
  }
  float frameAlpha = mAlpha;
  if (!close_enough(mModelZoomAmount, 0.f)) {
    frameAlpha = 1.f - mModelZoomAmount;
  }
  if (!close_enough(frameAlpha, 0.f)) {
    CGraphics::SetDepthRange(0.f, 0.f);
    mFrame->Draw(CGuiWidgetDrawParms(frameAlpha, CVector3f::Zero()));
  }
  CGraphics::SetDepthRange(0.125f, 1.f);
  const CViewport& viewport = CGraphics::GetViewport();
  gpRender->SetPerspective(30.f, viewport.mWidth, viewport.mHeight, 0.2f, 4096.f);
  CGraphics::SetModelMatrix(CTransform4f::Identity());
  const CTransform4f view = mViewRotation.BuildTransform4f();
  const CVector3f camera(gpTweakGui->GetLogBookCameraXOffset(),
                         -gpTweakGui->GetLogBookCameraDistance(),
                         gpTweakGui->GetLogBookCameraZOffset());
  const CVector3f zoomCamera(0.f, -gpTweakGui->GetLogBookCameraDistance(), 0.f);
  const CVector3f cameraPosition = (1.f - mModelZoomAmount) * camera + mModelZoomAmount * zoomCamera;
  CGraphics::SetViewPointMatrix(view * CTransform4f::Translate(cameraPosition));
  CGraphics::SetDepthRange(0.125f, 1.f);
  CGraphics::SetCullMode(kCM_None);
  CGraphics::SetLineWidth(2.f, kTO_One);
  SetFog(true);

  const float transition = mScanTree.GetTransition();
  const int selectedId = mScanTree.GetSelectedNode();
  rstl::rc_ptr< CScanTreeNode > node = mScanTree.GetNode(selectedId);
  CVector3f origin = node->GetDisplayPosition();
  const bool option = node->GetNodeType() == CScanTreeNode::kNT_Slider ||
                      node->GetNodeType() == CScanTreeNode::kNT_Menu;
  rstl::vector< SNodeDraw > nodes;
  if (!close_enough(transition, 0.f) || option) {
    const int previousId = mScanTree.GetPreviousNode();
    rstl::rc_ptr< CScanTreeNode > previous = mScanTree.GetNode(previousId);
    const bool previousOption = previous->GetNodeType() == CScanTreeNode::kNT_Slider ||
                                previous->GetNodeType() == CScanTreeNode::kNT_Menu;
    origin = previous->GetDisplayPosition();
    const float previousAlpha = mAlpha * (option || previousOption ? 1.f : transition);
    switch (previous->GetNodeType()) {
    case CScanTreeNode::kNT_Category:
      DrawScanTree(view, origin, previousId, !option, nodes);
      break;
    case CScanTreeNode::kNT_Scan:
    case CScanTreeNode::kNT_Inventory:
      DrawModels(previousAlpha);
      break;
    }
    if (!option) {
      origin = node->GetDisplayPosition();
    }
  }
  if (node->GetNodeType() == CScanTreeNode::kNT_Category) {
    DrawScanTree(view, origin, selectedId, false, nodes);
  }
  DrawNodes(view, nodes);
  const float alpha = (1.f - transition) * mAlpha;
  switch (node->GetNodeType()) {
  case CScanTreeNode::kNT_Menu:
    DrawMenuNode(view, origin, selectedId, alpha);
    break;
  case CScanTreeNode::kNT_Scan:
  case CScanTreeNode::kNT_Inventory:
    DrawModels(alpha);
    break;
  case CScanTreeNode::kNT_Slider:
    DrawSliderNode(view, origin, selectedId, alpha);
    break;
  }
  CGraphics::SetCullMode(kCM_Front);
  CGraphics::SetDepthWriteMode(true, kE_LEqual, true);
  SetFog(false);
}

void CPauseScreen::DrawScanTree(const CTransform4f& view, const CVector3f& origin, int nodeId,
                                bool skipParent, rstl::vector< SNodeDraw >& nodes) const {
  nodes.reserve(nodes.size() + 2);
  rstl::rc_ptr< CScanTreeNode > node = mScanTree.GetNode(nodeId);
  const CVector3f position = node->GetDisplayPosition() - origin;
  gpRender->SetBlendMode_AdditiveAlpha();
  gpRender->SetModelMatrix(CTransform4f::Identity());
  if (!skipParent) {
    SNodeDraw draw;
    draw.mNode = node;
    draw.mPosition = position;
    draw.mDepth = 0.f;
    draw.mStyle = 1;
    draw.mAlpha = mScanTree.GetLayoutProgress();
    nodes.push_back_unsafe(draw);
  }
  if (node->GetNodeType() == CScanTreeNode::kNT_Category) {
    const CScanTreeCategory& category = static_cast< const CScanTreeCategory& >(*node);
    const int count = category.GetChildCount();
    nodes.reserve(nodes.size() + count);
    for (int i = 0; i < count; ++i) {
      const int childId = category.GetChild(i);
      rstl::rc_ptr< CScanTreeNode > child = mScanTree.GetNode(childId);
      if (!child->IsVisible()) {
        continue;
      }
      const bool selected = category.GetSelectedChild() == childId;
      const bool option = child->GetNodeType() == CScanTreeNode::kNT_Menu ||
                          child->GetNodeType() == CScanTreeNode::kNT_Slider;
      const bool activeOption = option && childId == mScanTree.GetSelectedNode();
      int style = 2;
      if (selected) {
        style = activeOption ? 4 : 3;
      }
      const float alpha = rstl::min_val(1.f, rstl::max_val(0.f, child->GetOpacity()));
      const CColor brightness(alpha, alpha, alpha, 1.f);
      const CVector3f childPosition = child->GetDisplayPosition() - origin;
      SNodeDraw draw;
      draw.mNode = child;
      draw.mPosition = childPosition;
      draw.mDepth = 0.f;
      draw.mStyle = style;
      draw.mAlpha = alpha;
      nodes.push_back_unsafe(draw);
      DrawConnection(view, position, childPosition,
                     CColor::Modulate(gpTweakGui->GetLogBookNodeColor(), brightness), 1.f);
    }
  }
}

void CPauseScreen::DrawNodes(const CTransform4f& view, rstl::vector< SNodeDraw >& nodes) const {
  CTexture* parent = mParentNodeTexture.GetObject();
  CTexture* unselected = mUnselectedNodeTexture.GetObject();
  CTexture* selected = mSelectedNodeTexture.GetObject();
  CTexture* highlight = mHighlightTexture.GetObject();
  if (parent == nullptr || unselected == nullptr || selected == nullptr || highlight == nullptr) {
    return;
  }
  gpRender->SetDepthReadWrite(false, false);
  for (rstl::vector< SNodeDraw >::iterator it = nodes.begin(); it != nodes.end(); ++it) {
    it->mDepth = view.TransposeRotate(it->mPosition).GetY();
  }
  struct SDepthCompare {
    bool operator()(const SNodeDraw& a, const SNodeDraw& b) const { return a.mDepth > b.mDepth; }
  };
  rstl::sort(nodes.begin(), nodes.end(), SDepthCompare());

  for (rstl::vector< SNodeDraw >::const_iterator it = nodes.begin(); it != nodes.end(); ++it) {
    const float alpha = rstl::min_val(1.f, rstl::max_val(0.f, it->mAlpha));
    const CColor brightness(alpha, alpha, alpha, 1.f);
    const CColor faded = brightness.WithAlphaOf(alpha);
    const CColor* textColor;
    const CColor* selectedTextColor;
    if (it->mNode->IsViewed()) {
      textColor = &gpTweakGui->GetLogBookMainWindowTextColor();
      selectedTextColor = &gpTweakGui->GetLogBookMainWindowSelectedTextColor();
    } else {
      textColor = &gpTweakGui->GetLogBookMainWindowUnviewedColor();
      selectedTextColor = &gpTweakGui->GetLogBookMainWindowUnviewedSelectedColor();
    }
    switch (it->mStyle) {
    case 0:
      unselected->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
      DrawNodeIcon(view, it->mPosition,
                   CColor::Modulate(gpTweakGui->GetLogBookNodeBackgroundColor(), brightness),
                   gpTweakGui->GetLogBookNodeScale(), true);
      DrawNodeLabel(view, it->mPosition, it->mNode, CColor::Modulate(*textColor, brightness),
                    gpTweakGui->GetLogBookSelectedNodeScale(), gpTweakGui->GetLogBookTextScale());
      break;
    case 1:
      parent->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
      DrawNodeIcon(view, it->mPosition, CColor::White().WithAlphaOf(alpha),
                   gpTweakGui->GetLogBookNodeScale(), false);
      break;
    case 2:
      unselected->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
      DrawNodeIcon(view, it->mPosition,
                   CColor::Modulate(gpTweakGui->GetLogBookNodeColor(), brightness),
                   gpTweakGui->GetLogBookNodeScale(), true);
      DrawNodeLabel(view, it->mPosition, it->mNode, CColor::Modulate(*textColor, brightness),
                    gpTweakGui->GetLogBookSelectedNodeScale(), gpTweakGui->GetLogBookTextScale());
      break;
    case 3:
      unselected->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
      DrawNodeIcon(view, it->mPosition,
                   CColor::Modulate(gpTweakGui->GetLogBookSelectedNodeColor(), faded),
                   gpTweakGui->GetLogBookSelectedNodeScale(), true);
      selected->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
      DrawNodeIcon(view, it->mPosition,
                   CColor::Modulate(gpTweakGui->GetLogBookSelectedNodeColor(), faded),
                   gpTweakGui->GetLogBookSelectedNodeScale(), false);
      if (mSelectionHighlight > 0.f) {
        highlight->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
        DrawNodeIcon(view, it->mPosition,
                     CColor::Lerp(CColor(0.f, 0.f, 0.f, 0.f), CColor::White(), mSelectionHighlight),
                     gpTweakGui->GetLogBookSelectedNodeScale(), true);
      }
      DrawNodeLabel(view, it->mPosition, it->mNode, CColor::Modulate(*selectedTextColor, brightness),
                    gpTweakGui->GetLogBookSelectedNodeScale(), gpTweakGui->GetLogBookSelectedTextScale());
      break;
    case 4:
      unselected->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
      DrawNodeIcon(view, it->mPosition,
                   CColor::Modulate(gpTweakGui->GetLogBookSelectedNodeColor(), faded),
                   gpTweakGui->GetLogBookSelectedNodeScale(), true);
      selected->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
      DrawNodeIcon(view, it->mPosition,
                   CColor::Modulate(gpTweakGui->GetLogBookSelectedNodeColor(), faded),
                   gpTweakGui->GetLogBookSelectedNodeScale(), false);
      DrawOptionBackground(view, it->mPosition, alpha);
      DrawNodeLabel(view, it->mPosition, it->mNode, CColor::Modulate(*selectedTextColor, brightness),
                    gpTweakGui->GetLogBookSelectedNodeScale(), gpTweakGui->GetLogBookSelectedTextScale());
      break;
    }
  }
}

void CPauseScreen::DrawConnection(const CTransform4f& view, const CVector3f& from,
                                  const CVector3f& to, const CColor& color, float progress) const {
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvPassthru);
  const CVector3f delta = to - from;
  if (delta.CanBeNormalized()) {
    const float length = delta.Magnitude();
    const CVector3f direction = (1.f / length) * delta;
    const CVector3f forward = view.GetColumn(kDY);
    const CVector3f projected = delta - (length * CVector3f::Dot(direction, forward)) * forward;
    const float projectedLength = projected.Magnitude();
    if (projectedLength > 0.34f) {
      const CVector3f trim = ((0.17f * length) / projectedLength) * direction;
      const CVector3f start = from + trim;
      const CVector3f end = to - trim;
      const CVector3f current = (1.f - progress) * start + progress * end;
      for (int i = 2; i != 0; --i) {
        CGraphics::SetLineWidth(i + 1, kTO_Zero);
        CGraphics::StreamBegin(kP_Lines);
        CGraphics::StreamColor(color.WithAlphaModulatedBy(1.f / 3.f));
        CGraphics::StreamVertex(start);
        CGraphics::StreamVertex(current);
        CGraphics::StreamEnd();
      }
    }
  }
}

void CPauseScreen::DrawNodeIcon(const CTransform4f& view, const CVector3f& position,
                                const CColor& color, float scale, bool additive) const {
  if (additive) {
    gpRender->SetBlendMode_AdditiveAlpha();
  } else {
    gpRender->SetBlendMode_AlphaBlended();
  }
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
  CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  CGraphics::SetModelMatrix(CTransform4f::Identity());
  const CVector3f right = view.GetColumn(kDX);
  const CVector3f up = view.GetColumn(kDZ);
  const float size = 0.2f * scale;
  CGraphics::StreamBegin(kP_Quads);
  CGraphics::StreamColor(color);
  CGraphics::StreamTexcoord(0.f, 0.f);
  CGraphics::StreamVertex(position + size * (-up - right));
  CGraphics::StreamTexcoord(1.f, 0.f);
  CGraphics::StreamVertex(position + size * (-up + right));
  CGraphics::StreamTexcoord(1.f, 1.f);
  CGraphics::StreamVertex(position + size * (up + right));
  CGraphics::StreamTexcoord(0.f, 1.f);
  CGraphics::StreamVertex(position + size * (up - right));
  CGraphics::StreamEnd();
}

void CPauseScreen::DrawNodeLabel(const CTransform4f& view, const CVector3f& position,
                                 const rstl::rc_ptr< CScanTreeNode >& node, const CColor& color,
                                 float iconScale, float textScale) const {
  if (node->AreResourcesLoaded()) {
    gpRender->SetBlendMode_AdditiveAlpha();
    mNodeText->SetText(node->GetName(), false);
    mNodeText->SetGeometryColor(color);
    const float scale = gpTweakGui->GetLogBookTextScale();
    const CVector3f offset(-mNodeText->GetTextBoundingWidth() * 0.5f, 0.f,
                           -(1.2f * (0.2f * iconScale) * 0.5f) / (0.02f * scale));
    const CTransform4f textXf = CTransform4f::Scale(0.02f * textScale) * view.GetRotation() *
                                CTransform4f::Translate(offset);
    CGraphics::SetModelMatrix(CTransform4f::Translate(position) * textXf);
    mNodeText->Render();
  }
}

void CPauseScreen::DrawOptionBackground(const CTransform4f& view, const CVector3f& position,
                                        float alpha) const {
  const float scale = gpTweakGui->GetLogBookSelectedNodeScale();
  if (mOptionBackgroundModel.GetObject() != nullptr) {
    const CVector3f offset(0.f, 0.01f, -(-0.05f + ((0.2f * scale) * 0.5f + 0.62136f)));
    const CTransform4f background =
        view.GetRotation() * CTransform4f::Translate(offset) * CTransform4f::Scale(0.18f);
    CGraphics::SetModelMatrix(CTransform4f::Translate(position) * background);
    mOptionBackgroundModel.GetObject()->Draw(
        CModelFlags(CModelFlags::kT_Blend, CColor::White().WithAlphaOf(alpha)));
  }
}

void CPauseScreen::DrawSliderNode(const CTransform4f& view, const CVector3f& origin, int nodeId,
                                  float alpha) const {
  gpRender->SetDepthReadWrite(false, false);
  rstl::rc_ptr< CScanTreeNode > node = mScanTree.GetNode(nodeId);
  gpTweakGui->GetLogBookSelectedNodeScale();
  const float scale = gpTweakGui->GetLogBookSelectedTextScale();
  const CVector3f position = node->GetDisplayPosition() - origin;
  if (node->AreResourcesLoaded()) {
    gpRender->SetBlendMode_AdditiveAlpha();
    mNodeText->SetText(node->GetName(), false);
    gpTweakGui->GetLogBookTextScale();
    const float textScale = 0.02f * scale;
    const float textOffset = -mNodeText->GetTextBoundingWidth() * 0.5f;
    const CScanTreeSlider& slider = static_cast< const CScanTreeSlider& >(*node);
    const float value = slider.GetNormalizedValue();
    const float defaultValue = slider.GetNormalizedDefaultValue();
    static const float widthScale = gpTweakGui->GetLogBookSliderTextWidthScale();
    static const float heightScale = gpTweakGui->GetLogBookSliderTextHeightScale();
    const rstl::pair< CVector2i, CVector2i >& bounds = mNodeText->GetBounds();
    DrawSlider(view, position, scale, widthScale * (bounds.second.GetX() - bounds.first.GetX()),
               value, defaultValue,
               textScale * textOffset - heightScale * (bounds.second.GetY() - bounds.first.GetY()),
               alpha);
  }
}

void CPauseScreen::DrawSlider(const CTransform4f& view, const CVector3f& position, float scale,
                              float width, float value, float defaultValue, float textOffset,
                              float alpha) const {
  const float sliderScale = gpTweakGui->GetLogBookSliderScale();
  const float centerWidth = width - 4.094f;
  const float halfWidth = centerWidth * 0.5f;
  const CTransform4f local = CTransform4f::Scale(scale * sliderScale) * view.GetRotation() *
                            CTransform4f::Translate(0.f, 0.f, textOffset);
  const CTransform4f world = CTransform4f::Translate(position) * local;
  const CColor selectionColor = gpTweakGui->GetLogBookSliderSelectionColor().WithAlphaModulatedBy(alpha);
  const CColor backgroundColor = gpTweakGui->GetLogBookSliderBackgroundColor().WithAlphaModulatedBy(alpha);
  static const CTransform4f flip = CTransform4f::Scale(CVector3f(-1.f, 1.f, 1.f));
  const CVector3f endPosition(-(halfWidth - 0.5f), 0.f, 0.f);
  if (mSliderEndModel.GetObject() != nullptr) {
    CGraphics::SetModelMatrix(world * CTransform4f::Translate(endPosition));
    mSliderEndModel.GetObject()->Draw(CModelFlags(CModelFlags::kT_Additive, backgroundColor));
    CGraphics::SetModelMatrix(world * flip * CTransform4f::Translate(endPosition));
    mSliderEndModel.GetObject()->Draw(CModelFlags(CModelFlags::kT_Blend, backgroundColor));
  }
  CGraphics::SetModelMatrix(world * CTransform4f::Scale(CVector3f(centerWidth, 1.f, 1.f)));
  if (mSliderCenterModel.GetObject() != nullptr) {
    mSliderCenterModel.GetObject()->Draw(CModelFlags(CModelFlags::kT_Blend, backgroundColor));
  }
  CGraphics::SetModelMatrix(
      world * CTransform4f::Translate(CVector3f((defaultValue - 0.5f) * (width - 2.f), 0.f, 0.f)));
  if (mSliderModel.GetObject() != nullptr) {
    const CColor dim = CColor::Modulate(selectionColor, CColor(0.5f, 0.5f, 0.5f, 0.5f));
    mSliderModel.GetObject()->Draw(CModelFlags(CModelFlags::kT_Additive, dim));
  }
  CGraphics::SetModelMatrix(
      world * CTransform4f::Translate(CVector3f((value - 0.5f) * (width - 2.f), 0.f, 0.f)));
  if (mSliderModel.GetObject() != nullptr) {
    mSliderModel.GetObject()->Draw(CModelFlags(CModelFlags::kT_Additive, selectionColor));
  }

  char text[16];
  sprintf(text, "%02d", int(100.f * value));
  mNodeText->SetText(rstl::string(text), false);
  const float heightScale = gpTweakGui->GetLogBookSliderTextHeightScale();
  const CVector3f offset(-mNodeText->GetTextBoundingWidth() * 0.5f, 0.f,
                         -5.f + textOffset / heightScale);
  const CTransform4f textXf = CTransform4f::Scale(0.02f * scale) * view.GetRotation() *
                             CTransform4f::Translate(offset);
  CGraphics::SetModelMatrix(CTransform4f::Translate(position) * textXf);
  mNodeText->Render();
}

void CPauseScreen::DrawMenuNode(const CTransform4f& view, const CVector3f& origin, int nodeId,
                                float alpha) const {
  gpRender->SetDepthReadWrite(false, false);
  rstl::rc_ptr< CScanTreeNode > node = mScanTree.GetNode(nodeId);
  const float nodeScale = gpTweakGui->GetLogBookSelectedNodeScale();
  const float textScale = gpTweakGui->GetLogBookSelectedTextScale();
  gpTweakGui->GetLogBookMenuOptionColor();
  const CVector3f position = node->GetDisplayPosition() - origin;
  if (node->AreResourcesLoaded()) {
    gpRender->SetBlendMode_AdditiveAlpha();
    const float labelScale = gpTweakGui->GetLogBookTextScale();
    const float scaledText = 0.02f * textScale;
    CVector3f offset(-mNodeText->GetTextBoundingWidth() * 0.5f, 0.f,
                     -(1.2f * (0.2f * nodeScale) * 0.5f) / (0.02f * labelScale));
    const float optionScale = 0.02f * gpTweakGui->GetLogBookMenuOptionScale();
    mNodeText->SetText(node->GetName(), false);
    const CScanTreeMenu& menu = static_cast< const CScanTreeMenu& >(*node);
    const rstl::pair< CVector2i, CVector2i >& labelBounds = mNodeText->GetBounds();
    offset.SetZ(scaledText *
                (offset.GetZ() - (labelBounds.second.GetY() - labelBounds.first.GetY()) - 1.5f) /
                optionScale);
    const CTransform4f optionXf = CTransform4f::Scale(optionScale) * view.GetRotation() *
                                 CTransform4f::Translate(offset);
    CGraphics::SetModelMatrix(CTransform4f::Translate(position) * optionXf);
    mNodeText->SetGeometryColor(gpTweakGui->GetLogBookMenuOptionColor());
    mNodeText->SetText(menu.GetOptionName(menu.GetCurrentOptionIndex()), false);
    mNodeText->Render();

    const CColor enabled = gpTweakGui->GetLogBookMenuOptionEnabledArrowColor().WithAlphaModulatedBy(alpha);
    const CColor disabled = gpTweakGui->GetLogBookMenuOptionDisabledArrowColor().WithAlphaModulatedBy(alpha);
    CModel* arrow = mMenuArrowModel.GetObject();
    if (arrow != nullptr) {
      static const CVector3f flip(-1.f, 1.f, 1.f);
      const float arrowScale = gpTweakGui->GetLogBookMenuOptionArrowScale();
      const rstl::pair< CVector2i, CVector2i >& bounds = mNodeText->GetBounds();
      const CVector3f arrowOffset(
          optionScale * -(bounds.second.GetX() - bounds.first.GetX()) * 0.5f - 0.1f, 0.f,
          optionScale * -(bounds.second.GetY() - bounds.first.GetY()) * 0.5f +
              optionScale * (offset.GetZ() - bounds.first.GetY()));
      const bool rightEnabled = menu.GetCurrentOptionIndex() < menu.GetOptionCount() - 1;
      const bool leftEnabled = menu.GetCurrentOptionIndex() > 0;
      const CTransform4f left = view.GetRotation() * CTransform4f::Translate(arrowOffset) *
                               CTransform4f::Scale(arrowScale);
      CGraphics::SetModelMatrix(CTransform4f::Translate(position) * left);
      arrow->Draw(CModelFlags(CModelFlags::kT_One, leftEnabled ? enabled : disabled));
      const CTransform4f right = view.GetRotation() * CTransform4f::Scale(flip) *
                                CTransform4f::Translate(arrowOffset) * CTransform4f::Scale(arrowScale);
      CGraphics::SetModelMatrix(CTransform4f::Translate(position) * right);
      arrow->Draw(CModelFlags(CModelFlags::kT_One, rightEnabled ? enabled : disabled));
    }
  }
}

bool CPauseScreen::IsDone() const { return mDone; }

void CPauseScreen::DrawModels(float alpha) const {
  if (!mModels.empty() && mModelsReady) {
    for (int i = 0; i < mModels.size(); ++i) {
      CModelData* model = mModels[i].get();
      if (model != nullptr && !model->IsNull()) {
        if (!model->IsLoaded(0)) {
          return;
        }
        model->Touch(CModelData::kWM_Normal, 0);
        if (model->HasAnimation()) {
          model->AnimationData()->PreRender();
        }
      }
    }
    SetFog(false);
    DrawModelView(mModelTransform, alpha);
    SetFog(true);
  }
}

void CPauseScreen::InitializeStripedTexture() {
  uchar* data = static_cast< uchar* >(mStripedTexture.Lock());
  memset(data, 0xff, 8);
  memset(data + 8, 0x66, 8);
  memset(data + 16, 0xff, 8);
  memset(data + 24, 0x66, 8);
  DCFlushRange(data, 32);
  mStripedTexture.UnLock();
}

void CPauseScreen::RenderModels(const CTransform4f& xf, const CModelFlags& flags,
                                bool particles) const {
  if (mModels[0].get() != nullptr) {
    if (particles && mModels[0]->HasAnimation()) {
      mModels[0]->AnimationData()->GetParticleDB().SetModulationColorAllActiveEffects(
          flags.GetColorRef());
      mModels[0]->AnimationData()->GetParticleDB().RenderSystemsToBeDrawnFirst();
    }
    mModels[0]->Render(CModelData::kWM_Normal, xf, mActorLights.get(), flags);
    CModelData* model = mModels[0].get();
    if (model->HasAnimation()) {
      if (mModels[1].get() != nullptr && mModels[1]->HasAnimation()) {
        TLockedToken< CSkinnedModel > original = model->AnimationData()->GetModelData();
        model->AnimationData()->SetSkinnedModel(mModels[1]->AnimationData()->GetModelData());
        model->Render(CModelData::kWM_Normal, xf, mActorLights.get(), flags);
        model->AnimationData()->SetSkinnedModel(original);
      }
      CAnimData& animation = *mModels[0]->AnimationData();
      const CCharLayoutInfo* layout = animation.GetCharLayoutInfo();
      for (int i = 2; i < 11; ++i) {
        if (mModels[i].get() != nullptr && !mScanInfo.null()) {
          const rstl::string& locator = (*mScanInfo)->GetModelLocator(i - 2);
          if (locator.size() != 0) {
            const CSegId id = animation.GetLocatorSegId(locator);
            const CTransform4f locatorXf = animation.GetLocatorTransform(id, nullptr);
            if (locator.find("LCTR") == -1) {
              const CTransform4f attachment =
                  locatorXf * layout->GetLinearRotations()[id.val()].BuildTransform4f();
              mModels[i]->Render(CModelData::kWM_Normal, xf * attachment, mActorLights.get(), flags);
            } else {
              mModels[i]->Render(CModelData::kWM_Normal, xf * locatorXf, mActorLights.get(), flags);
            }
          }
        }
      }
    }
    if (particles && mModels[0]->HasAnimation()) {
      mModels[0]->AnimationData()->GetParticleDB().RenderSystemsToBeDrawnLast();
      mModels[0]->AnimationData()->GetParticleDB().RenderSystemsNormallyAddedToRenderer();
    }
  }
}

void CPauseScreen::DrawModelView(const CTransform4f& xf, float alpha) const {
  const float flash = CMath::Clamp(
      0.f, const_cast< CMayaSpline& >(gpTweakGui->GetLogBookScanObjectFadeInSpline()).EvaluateAt(mModelFade), 1.f);
  if (!close_enough(mModelFade, 1.f)) {
    CCubeRenderer::That()->SetRequestRGBA6(true);
    GXSetColorUpdate(GX_FALSE);
    CCubeModel::SetRenderModelBlack(true);
    gpRender->SetDepthReadWrite(false, false);
    static const int maskAlpha[] = {64, 128};
    static const float radii[] = {0.075f, 0.025f};
    static const float phases[] = {2.0734513f, 103.67256f};
    for (int layer = 0; layer < 2; ++layer) {
      CGX::SetDstAlpha(true, maskAlpha[layer]);
      const float radius = radii[layer];
      for (int i = 0; i < 3; ++i) {
        const float angle = 6.2831855f * i / 3.f + phases[layer];
        const float x = radius * CMath::FastCosR(angle);
        const float z = radius * CMath::FastSinR(angle);
        RenderModels(CTransform4f::Translate(x, 0.f, z) * xf,
                     CModelFlags(CModelFlags::kT_Opaque, 1.f), false);
      }
    }
    CGX::SetDstAlpha(true, 255);
    RenderModels(CTransform4f(xf), CModelFlags(CModelFlags::kT_Opaque, 1.f), false);
    CCubeModel::SetRenderModelBlack(false);
    GXSetColorUpdate(GX_TRUE);
    CGX::SetDstAlpha(false, 0);
  }
  CGraphics::SetCullMode(kCM_Front);
  RenderModels(CTransform4f(xf), CModelFlags(CModelFlags::kT_Blend, alpha * mAlpha), true);
  CGraphics::SetCullMode(kCM_None);
  if (!close_enough(mModelFade, 1.f)) {
    GXSetAlphaUpdate(GX_FALSE);
    CGX::SetDstAlpha(true, 0);
    CTexture* sweep = mScanSweepTexture.GetObject();
    if (sweep != nullptr) {
      const CGraphics::CProjectionState projection = CGraphics::GetProjectionState();
      const CTransform4f view = CGraphics::GetViewMatrix();
      const rstl::pair< CVector2f, CVector2f > viewport =
          gpRender->SetViewportOrtho(true, -4096.f, 4096.f);
      CGX::SetChanCtrl(CGX::Channel0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL,
                       GX_DF_NONE, GX_AF_NONE);
      mStripedTexture.Load(GX_TEXMAP0, CTexture::kCM_Repeat);
      CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO);
      CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
      CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
      CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE,
                          GX_PTIDENTITY);
      CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
      CGX::SetTevDirect(GX_TEVSTAGE0);
      const CColor color = CColor::Modulate(gpTweakGui->GetLogBookScanObjectFadeInFlashColor(),
                                            CColor(flash, flash, flash, 1.f));
      CGX::SetTevKColor(GX_KCOLOR0, color.GetGXColor());
      CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
      static const GXVtxDescList descriptors[] = {
          {GX_VA_POS, GX_DIRECT}, {GX_VA_TEX0, GX_DIRECT}, {GX_VA_NULL, GX_NONE}};
      CGX::SetVtxDescv(descriptors);
      CGX::SetNumTexGens(1);
      CGX::SetNumTevStages(1);
      CGX::SetNumChans(0);
      CGX::SetNumIndStages(0);
      gpRender->SetDepthReadWrite(false, false);
      CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
      CGraphics::SetBlendMode(kBM_Blend, kBF_DstAlpha, kBF_One, kLO_Clear);
      const float left = viewport.first.GetX();
      const float bottom = viewport.first.GetY();
      const float right = viewport.second.GetX();
      const float top = viewport.second.GetY();
      const float texCoord = (top - bottom) / mStripedTexture.GetHeight();
      CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
      GXPosition3f32(left, 0.f, bottom);
      GXTexCoord2f32(0.f, texCoord);
      GXPosition3f32(right, 0.f, bottom);
      GXTexCoord2f32(texCoord, texCoord);
      GXPosition3f32(left, 0.f, top);
      GXTexCoord2f32(0.f, 0.f);
      GXPosition3f32(right, 0.f, top);
      GXTexCoord2f32(texCoord, 0.f);
      CGX::End();
      sweep->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
      static const float speeds[] = {1.5f, 0.38f, -2.f, 0.6f, -0.6f};
      static const float scales[] = {1.f, 0.41f, 0.21f, 0.13f, 0.21f};
      for (int i = 0; i < 5; ++i) {
        const float phase = mModelFade * speeds[i];
        const float position = (phase - CMath::FloorF(phase)) * (top - bottom);
        const CVector2f low(left - 1.f, top - position);
        const CVector2f high(right + 1.f, top - -(scales[i] * sweep->GetHeight() - position));
        CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
        GXPosition3f32(low.GetX(), 0.f, low.GetY());
        GXTexCoord2f32(0.f, 0.f);
        GXPosition3f32(low.GetX(), 0.f, high.GetY());
        GXTexCoord2f32(0.f, 1.f);
        GXPosition3f32(high.GetX(), 0.f, low.GetY());
        GXTexCoord2f32(1.f, 0.f);
        GXPosition3f32(high.GetX(), 0.f, high.GetY());
        GXTexCoord2f32(1.f, 1.f);
        CGX::End();
      }
      CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
      CGraphics::SetDepthWriteMode(true, kE_LEqual, true);
      CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
      CGraphics::SetProjectionState(projection);
      CGraphics::SetViewPointMatrix(view);
      CGraphics::SetCullMode(kCM_Front);
    }
  }
}

void CPauseScreen::UpdateHistoryText() {
  rstl::reserved_vector< rstl::wstring, 7 > names;
  rstl::reserved_vector< float, 7 > completion;
  int nodeId = mScanTree.GetSelectedNode();
  const rstl::wstring instruction(gpStringTable->GetString("LogBookScreenInstructionPanelLabel"));
  const rstl::wstring spacing(gpStringTable->GetString("LogbookLineSpacing"));
  while (nodeId != -1) {
    rstl::rc_ptr< CScanTreeNode > node = mScanTree.GetNode(nodeId);
    if (node.IsNull()) {
      break;
    }
    if (!node->AreResourcesLoaded()) {
      return;
    }
    names.push_back(node->GetName());
    completion.push_back(float(node->GetVisibleDescendantCount()) / float(node->GetDescendantCount()));
    nodeId = node->GetParentNode();
  }
  if (!names.empty()) {
    names.pop_back();
    completion.pop_back();
    for (int i = 0; i < names.size(); ++i) {
      const float fraction = completion[completion.size() - i - 1];
      mHistoryLabels[i]->TextSupport().SetText(spacing + names[names.size() - i - 1], false);
      static_cast< CAuiBitmapMeter* >(mHistoryMeters[i])->SetTargetFraction(fraction);
      static_cast< CAuiBitmapMeter* >(mHistoryMeters[i])->SetCurrentFraction(fraction);
      mHistoryRowExpanded[i] = false;
    }
  }
  mHistoryTextReady = true;
}

void CPauseScreen::UpdateHistoryColors() {
  rstl::reserved_vector< int, 6 > rows;
  for (int i = 0; i < 6; ++i) {
    if (mHistoryRowExpanded[i]) {
      rows.push_back(i + 6);
      ++i;
    } else {
      rows.push_back(i);
    }
  }

  float transition = mScanTree.GetTransition();
  const CColor& unselectedTitle = gpTweakGui->GetLogBookHistoryUnselectedTitle();
  const CColor& selectedTitle = gpTweakGui->GetLogBookHistorySelectedTitle();
  const CColor clear(0.f, 0.f, 0.f, 0.f);
  const CColor& selectedFrame = gpTweakGui->GetLogBookHistorySelectedFrame();
  const CColor& unselectedFrame = gpTweakGui->GetLogBookHistoryUnselectedFrame();
  const CColor& cursor = gpTweakGui->GetLogBookHistoryCursorColor();
  const CColor& unselectedBar = gpTweakGui->GetLogBookHistoryPercentBarUnselected();
  const CColor& selectedBar = gpTweakGui->GetLogBookHistoryPercentBarSelected();
  const CColor& unselectedBackground = gpTweakGui->GetLogBookHistoryPercentBarBackgroundUnselected();
  const CColor& selectedBackground = gpTweakGui->GetLogBookHistoryPercentBarBackgroundSelected();

  int depth = 0;
  int nodeId = mScanTree.GetSelectedNode();
  rstl::rc_ptr< CScanTreeNode > selected = mScanTree.GetNode(nodeId);
  const bool fromParent = selected->GetParentNode() == mScanTree.GetPreviousNode();
  while (nodeId != -1) {
    rstl::rc_ptr< CScanTreeNode > node = mScanTree.GetNode(nodeId);
    if (node.IsNull()) {
      break;
    }
    ++depth;
    nodeId = node->GetParentNode();
  }

  int count = depth - 1;
  if (!close_enough(transition, 0.f) && !fromParent) {
    count = depth;
  }
  count = CMath::Clamp(0, count, mHistoryBackgrounds.size());
  int i = 0;
  for (; i < count; ++i) {
    const int row = rows[i];
    mHistoryLabels[row]->SetVisibility(true, kTM_Children);
    mHistoryBackgrounds[row]->SetVisibility(true, kTM_Children);
    mHistoryHighlights[row]->SetVisibility(true, kTM_Children);
    mHistoryMeters[row]->SetVisibility(true, kTM_Children);
    mHistoryMeterBackgrounds[row]->SetVisibility(true, kTM_Children);
    mHistoryHighlights[row]->SetColor(clear);
  }
  for (; i < rows.size(); ++i) {
    const int row = rows[i];
    mHistoryLabels[row]->SetVisibility(false, kTM_Children);
    mHistoryBackgrounds[row]->SetVisibility(false, kTM_Children);
    mHistoryMeters[row]->SetVisibility(false, kTM_Children);
    mHistoryMeterBackgrounds[row]->SetVisibility(false, kTM_Children);
    mHistoryHighlights[row]->SetVisibility(false, kTM_Children);
  }

  if (close_enough(transition, 0.f)) {
    if (count > 0) {
      const int row = rows[count - 1];
      mHistoryLabels[row]->TextSupport().SetFontColor(selectedTitle);
      mHistoryBackgrounds[row]->SetColor(unselectedFrame);
      mHistoryHighlights[row]->SetColor(cursor);
      mHistoryMeters[row]->SetColor(selectedBar);
      mHistoryMeterBackgrounds[row]->SetColor(selectedBackground);
    }
  } else {
    if (fromParent) {
      transition = 1.f - transition;
    }
    if (count > 1) {
      const int row = rows[count - 2];
      mHistoryLabels[row]->TextSupport().SetFontColor(
          CColor::Lerp(selectedTitle, unselectedTitle, transition));
      mHistoryBackgrounds[row]->SetColor(CColor::Lerp(unselectedFrame, selectedFrame, transition));
      mHistoryHighlights[row]->SetColor(CColor::Lerp(cursor, clear, transition));
      mHistoryMeters[row]->SetColor(CColor::Lerp(selectedBar, unselectedBar, transition));
      mHistoryMeterBackgrounds[row]->SetColor(
          CColor::Lerp(selectedBackground, unselectedBackground, transition));
    }
    if (count > 0) {
      const int row = rows[count - 1];
      const float fade = 1.f - transition;
      mHistoryLabels[row]->TextSupport().SetFontColor(CColor::Lerp(selectedTitle, clear, fade));
      mHistoryBackgrounds[row]->SetColor(CColor::Lerp(unselectedFrame, clear, fade));
      mHistoryHighlights[row]->SetColor(CColor::Lerp(cursor, clear, fade));
      mHistoryMeters[row]->SetColor(CColor::Lerp(selectedBar, clear, fade));
      mHistoryMeterBackgrounds[row]->SetColor(CColor::Lerp(selectedBackground, clear, fade));
    }
  }
}

CVector3f CPauseScreen::GetDefaultModelPosition() {
  return CVector3f(gpTweakGui->GetLogBookModelXOffset(), 0.f, gpTweakGui->GetLogBookModelZOffset());
}

CVector3f CPauseScreen::GetModelPosition() const {
  const CVector3f position = GetDefaultModelPosition();
  const CVector3f hiddenPosition = position + CVector3f(0.f, -0.6f, -0.5f);
  const CVector3f legendPosition =
      (1.f - mLegendHiddenAmount) * position + mLegendHiddenAmount * hiddenPosition;
  return (1.f - mModelZoomAmount) * legendPosition + mModelZoomAmount * mModelPan;
}

CScanTreeMenu::~CScanTreeMenu() {}

CScanTreeSlider::~CScanTreeSlider() {}

CScanTreeScan::~CScanTreeScan() {}

CScanTreeCategory::~CScanTreeCategory() {}

CScanTreeNode::~CScanTreeNode() {}

CScanTree::~CScanTree() {}
