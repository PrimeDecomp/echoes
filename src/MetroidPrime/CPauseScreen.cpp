#include "MetroidPrime/CPauseScreen.hpp"

#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CQuitGameScreen.hpp"
#include "MetroidPrime/Factories/CScannableObjectInfo.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/ScriptObjects/CScanTreeCategory.hpp"
#include "MetroidPrime/ScriptObjects/CScanTreeMenu.hpp"
#include "MetroidPrime/ScriptObjects/CScanTreeScan.hpp"
#include "MetroidPrime/ScriptObjects/CScanTreeSlider.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerRes.hpp"

#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CAuiBitmapMeter.hpp"
#include "GuiSys/CGuiFrameLoader.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Input/CFinalInput.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Text/CGuiTextSupport.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "dolphin/os/OSCache.h"

#include "rstl/math.hpp"
#include <limits.h>
#include <string.h>

// Structure-first scaffold. The scan-tree, GUI and model-viewer bodies remain incomplete.
// Native definitions follow the target's deferred-emission order.

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
  // TODO: create the frame, bind its widgets, initialize lights/history, and begin fading in.
}

void CPauseScreen::Initialize(const CStateManager&) {
  // TODO: initialize scan-tree completion counts and open the player's just-completed scan.
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

bool CPauseScreen::CheckLoadComplete(const CStateManager&) {
  // TODO: advance scan-tree, SCAN, STRG, model/dependency and frame-loading states.
  // Model loading may continue after the surrounding GUI becomes ready.
  return false;
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

void CPauseScreen::Update(float, const CStateManager&, CArchitectureQueue&) {
  // TODO: update transitions, tree rotation/selection, model animation, GUI text and quit messages.
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
  // TODO: draw the loaded GUI, scan network, active node's model/menu/slider, or quit dialog.
}

void CPauseScreen::DrawScanTree(const CTransform4f&, const CVector3f&, int, bool,
                                rstl::vector< SNodeDraw >&) const {
  // TODO: gather parent/child node records and draw their connecting lines.
}

void CPauseScreen::DrawNodes(const CTransform4f&, rstl::vector< SNodeDraw >&) const {
  // TODO: depth-sort the records and draw the appropriate node textures, labels and highlights.
}

void CPauseScreen::DrawConnection(const CTransform4f&, const CVector3f&, const CVector3f&,
                                  const CColor&, float) const {
  // TODO: project and trim the connection, then draw its layered line segments.
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

void CPauseScreen::DrawSliderNode(const CTransform4f&, const CVector3f&, int, float) const {
  // TODO: measure the slider node's label and draw its current/previous values.
}

void CPauseScreen::DrawSlider(const CTransform4f&, const CVector3f&, float, float, float, float,
                              float, float) const {
  // TODO: draw end caps, center, old/new slider positions and the percentage label.
}

void CPauseScreen::DrawMenuNode(const CTransform4f&, const CVector3f&, int, float) const {
  // TODO: draw the current choice and dim unavailable left/right menu arrows.
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
            if (locator.find(rstl::string("LCTR")) == -1) {
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

void CPauseScreen::DrawModelView(const CTransform4f&, float) const {
  // TODO: draw the scan sweep, fading model and selected-cursor overlay.
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
  // TODO: animate history-row visibility and selection colors during node transitions.
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
