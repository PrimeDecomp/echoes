#include "MetroidPrime/CPauseScreen.hpp"

#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CQuitGameScreen.hpp"

#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiFrameLoader.hpp"
#include "GuiSys/CGuiTextSupport.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "dolphin/os/OSCache.h"

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

  // TODO: read LogbookLegendVisible and populate/lock the two stick-icon sets from player tweaks.
  CSfxManager::SfxStart(0x21d1, 0x7f, 0x3f, false, CSfxManager::kMedPriority, false,
                        CSfxManager::kAllAreas);
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

bool CPauseScreen::EnsureTextureLoaded(const CToken&) {
  // TODO: track TXTR residency and request its bitmap before the model is rendered.
  return false;
}

void CPauseScreen::RestoreTextures() {
  // TODO: restore each tracked texture's previous residency and drain outstanding ARAM transfers.
}

bool CPauseScreen::CheckLoadComplete(const CStateManager&) {
  // TODO: advance scan-tree, SCAN, STRG, model/dependency and frame-loading states.
  // Model loading may continue after the surrounding GUI becomes ready.
  return false;
}

void CPauseScreen::UpdatePulse(float dt) {
  mPulseTime += dt;
  // TODO: animate the hex widgets' staggered color pulse.
}

void CPauseScreen::Update(float, const CStateManager&, CArchitectureQueue&) {
  // TODO: update transitions, tree rotation/selection, model animation, GUI text and quit messages.
}

void CPauseScreen::TouchVisibleNodes() {
  // TODO: touch the current node (or root before selection) and its visible children.
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

void CPauseScreen::LoadScan(int) {
  // TODO: load the selected SCAN token, reset pagination/model pan, and schedule its resource load.
}

void CPauseScreen::SelectNode(int) {
  // TODO: enter a category, scan, menu or slider; the quit option creates CQuitGameScreen.
}

void CPauseScreen::AdvancePage() {
  // TODO: advance multipage scan text or return to the tree/game after the final page.
}

void CPauseScreen::FinishOptionEdit(bool) {
  // TODO: accept/cancel menu or slider edits, handle reset-to-default, and select the parent node.
}

void CPauseScreen::ProcessButtonInput(const CFinalInput&) {
  // TODO: handle accept/back, legend visibility, model zoom and exit while the tree is stationary.
}

void CPauseScreen::ProcessSelectionInput(const CFinalInput&) {
  // TODO: select a visible child in the right-stick direction using view-space positions.
}

void CPauseScreen::SetPanSound(bool playing) {
  if (playing) {
    if (mPanSfx == CSfxHandle()) {
      mPanSfx = CSfxManager::SfxStart(300, 0x7f, 0x3f, false, CSfxManager::kMedPriority, true,
                                      CSfxManager::kAllAreas);
    }
  } else if (mPanSfx != CSfxHandle()) {
    CSfxManager::SfxStop(mPanSfx);
    mPanSfx = CSfxHandle();
  }
}

void CPauseScreen::SetZoomSound(bool playing) {
  if (playing) {
    if (mZoomSfx == CSfxHandle()) {
      mZoomSfx = CSfxManager::SfxStart(0x78, 0x7f, 0x3f, false, CSfxManager::kMedPriority, true,
                                       CSfxManager::kAllAreas);
    }
  } else if (mZoomSfx != CSfxHandle()) {
    CSfxManager::SfxStop(mZoomSfx);
    mZoomSfx = CSfxHandle();
  }
}

void CPauseScreen::ProcessRotationInput(const CFinalInput&) {
  // TODO: rotate the tree/model or change the active option, with GUI key-repeat and sound.
}

void CPauseScreen::ProcessModelInput(const CFinalInput&, bool) {
  // TODO: pan/zoom the model, clamp pitch and translation, and update movement sounds.
}

void CPauseScreen::UpdateStickIcons(const CFinalInput&) {
  // TODO: choose one of nine direction icons for each stick from the mapped controls.
}

void CPauseScreen::SetFog(bool) const {
  // TODO: configure the scan network's fog through the shared renderer.
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

void CPauseScreen::DrawNodeIcon(const CTransform4f&, const CVector3f&, const CColor&, float,
                                bool) const {
  // TODO: draw a camera-facing textured quad with the requested blend mode.
}

void CPauseScreen::DrawNodeLabel(const CTransform4f&, const CVector3f&,
                                 const rstl::rc_ptr< CScanTreeNode >&, const CColor&, float,
                                 float) const {
  // TODO: set the node's localized name/color and draw the shared text support below its icon.
}

void CPauseScreen::DrawOptionBackground(const CTransform4f&, const CVector3f&, float) const {
  // TODO: draw the option backdrop model under the selected node.
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

void CPauseScreen::DrawModels(float) const {
  // TODO: pre-render loaded animated models and draw the model view with scan-network fog disabled.
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

void CPauseScreen::RenderModels(const CTransform4f&, const CModelFlags&, bool) const {
  // TODO: render the main model, alternate skin and locator-attached models, including particles.
}

void CPauseScreen::DrawModelView(const CTransform4f&, float) const {
  // TODO: draw the scan sweep, fading model and selected-cursor overlay.
}

void CPauseScreen::UpdateHistoryText() {
  // TODO: walk ancestors to populate the history labels and completion meters.
}

void CPauseScreen::UpdateHistoryColors() {
  // TODO: animate history-row visibility and selection colors during node transitions.
}

CVector3f CPauseScreen::GetDefaultModelPosition() {
  // TODO: use the logbook model-position components from CTweakGui.
  return CVector3f::Zero();
}

CVector3f CPauseScreen::GetModelPosition() const {
  // TODO: interpolate the default, legend-hidden and zoomed/panned model positions.
  return CVector3f::Zero();
}
