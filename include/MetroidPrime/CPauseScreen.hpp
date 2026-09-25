#ifndef _CPAUSESCREEN
#define _CPAUSESCREEN

#include "GuiSys/CRepeatState.hpp"
#include "MetroidPrime/CScanTree.hpp"

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/list.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

class CActorLights;
class CArchitectureQueue;
class CFinalInput;
class CGuiFrame;
class CGuiFrameLoader;
class CGuiTextPane;
class CGuiTextSupport;
class CGuiWidget;
class CModel;
class CModelData;
class CModelFlags;
class CQuitGameScreen;
class CRasterFont;
class CScannableObjectInfo;
class CStateManager;
class CStringTable;

// Echoes combines the logbook, inventory and options in one scan-tree-driven screen.
// Class correspondence is inferred from CInGameGuiManager, not an original GC symbol.
class CPauseScreen {
public:
  CPauseScreen();
  ~CPauseScreen();

  void InitializeFrameGlue();
  bool CheckLoadComplete(const CStateManager& mgr);
  void Update(float dt, const CStateManager& mgr, CArchitectureQueue& queue);
  void ProcessControllerInput(const CFinalInput& input);
  void Draw() const;
  bool IsDone() const;

private:
  // Guessed names for target-derived states and drawing records.
  enum ELoadState {
    kLS_Ready,
    kLS_ScanTree,
    kLS_ScanText,
    kLS_ScanModels,
    kLS_ScanInfo,
  };
  enum ETransitionState { kTS_Loading, kTS_FadeIn, kTS_Active, kTS_FadeOut };
  struct SNodeDraw {
    rstl::rc_ptr< CScanTreeNode > mNode;
    CVector3f mPosition;
    float mDepth;
    int mStyle;
    float mAlpha;
  };

  // Guessed names; exact original private method names are not available.
  void Initialize(const CStateManager& mgr);
  void TrackTexture(const TToken< CTexture >& texture, bool restoreToARAM);
  bool EnsureTextureLoaded(const CToken& token);
  void RestoreTextures();
  void UpdatePulse(float dt);
  void TouchVisibleNodes();
  void LoadScan(int nodeId);
  void SelectNode(int nodeId);
  void AdvancePage();
  void FinishOptionEdit(bool accept);
  void ProcessButtonInput(const CFinalInput& input);
  void ProcessSelectionInput(const CFinalInput& input);
  void SetPanSound(bool playing);
  void SetZoomSound(bool playing);
  void ProcessRotationInput(const CFinalInput& input);
  void ProcessModelInput(const CFinalInput& input, bool allowTranslation);
  void UpdateStickIcons(const CFinalInput& input);
  void SetFog(bool enabled) const;
  void DrawScanTree(const CTransform4f& view, const CVector3f& origin, int nodeId, bool skipParent,
                    rstl::vector< SNodeDraw >& nodes) const;
  void DrawNodes(const CTransform4f& view, rstl::vector< SNodeDraw >& nodes) const;
  void DrawConnection(const CTransform4f& view, const CVector3f& from, const CVector3f& to,
                      const CColor& color, float progress) const;
  void DrawNodeIcon(const CTransform4f& view, const CVector3f& position, const CColor& color,
                    float scale, bool additive) const;
  void DrawNodeLabel(const CTransform4f& view, const CVector3f& position,
                     const rstl::rc_ptr< CScanTreeNode >& node, const CColor& color,
                     float iconScale, float textScale) const;
  void DrawOptionBackground(const CTransform4f& view, const CVector3f& position, float alpha) const;
  void DrawSliderNode(const CTransform4f& view, const CVector3f& origin, int nodeId,
                      float alpha) const;
  void DrawSlider(const CTransform4f& view, const CVector3f& position, float scale, float width,
                  float value, float previousValue, float textOffset, float alpha) const;
  void DrawMenuNode(const CTransform4f& view, const CVector3f& origin, int nodeId,
                    float alpha) const;
  void DrawModels(float alpha) const;
  void InitializeStripedTexture();
  void RenderModels(const CTransform4f& xf, const CModelFlags& flags, bool particles) const;
  void DrawModelView(const CTransform4f& xf, float alpha) const;
  void UpdateHistoryText();
  void UpdateHistoryColors();
  static CVector3f GetDefaultModelPosition();
  CVector3f GetModelPosition() const;

  rstl::reserved_vector< TToken< CTexture >, 9 > mLeftStickIcons;
  rstl::reserved_vector< TToken< CTexture >, 9 > mRightStickIcons;
  TCachedToken< CTexture > mSelectedNodeTexture;
  TCachedToken< CTexture > mUnselectedNodeTexture;
  TCachedToken< CTexture > mParentNodeTexture;
  TCachedToken< CTexture > mSelectedCursorTexture;
  TCachedToken< CTexture > mHighlightTexture;
  TCachedToken< CTexture > mScanSweepTexture;
  CTexture mStripedTexture;
  TCachedToken< CRasterFont > mFont;
  TCachedToken< CModel > mSliderModel;
  TCachedToken< CModel > mSliderEndModel;
  TCachedToken< CModel > mSliderCenterModel;
  TCachedToken< CModel > mMenuArrowModel;
  TCachedToken< CModel > mOptionBackgroundModel;
  rstl::auto_ptr< CGuiTextSupport > mNodeText;
  CVector2f mRotationInput;
  CVector2f mRotationVelocity;
  CQuaternion mViewRotation;
  CScanTree mScanTree;
  int x1f8_;
  rstl::vector< CAssetId > x1fc_;
  rstl::auto_ptr< CGuiFrameLoader > mFrameLoader;
  rstl::auto_ptr< CGuiFrame > mFrame;
  CGuiTextPane* mMessage;
  CGuiWidget* mScanInfoGroup;
  uint x224_;
  uint x228_;
  CGuiTextPane* mScanPercentage;
  CGuiTextPane* mItemPercentage;
  CGuiTextPane* mLeftStickInstructions;
  CGuiTextPane* mRightStickInstructions;
  CGuiTextPane* mRightInstructions;
  CGuiWidget* mBottomPane;
  CGuiWidget* mAdvanceButton;
  ELoadState mLoadState;
  ETransitionState mTransitionState;
  float mAlpha;
  rstl::reserved_vector< rstl::optional_object< CToken >, 11 > mModelTokens;
  rstl::reserved_vector< rstl::auto_ptr< CModelData >, 11 > mModels;
  rstl::vector< CToken > mDependencies;
  rstl::list< rstl::pair< bool, TToken< CTexture > > > mTexturesToRestore;
  rstl::auto_ptr< TCachedToken< CScannableObjectInfo > > mScanInfo;
  rstl::auto_ptr< TCachedToken< CStringTable > > mScanStrings;
  rstl::vector< CGuiWidget* > mHexWidgets;
  rstl::reserved_vector< CGuiWidget*, 12 > mHistoryBackgrounds;
  rstl::reserved_vector< CGuiWidget*, 12 > mHistoryHighlights;
  rstl::reserved_vector< CGuiTextPane*, 12 > mHistoryLabels;
  rstl::reserved_vector< CGuiWidget*, 12 > mHistoryMeters;
  rstl::reserved_vector< CGuiWidget*, 12 > mHistoryMeterBackgrounds;
  rstl::reserved_vector< bool, 6 > mHistoryRowExpanded;
  CGuiTextPane* mInstructionLabel;
  int x494_;
  int x498_;
  int mPageCount;
  int mPage;
  float mPulseTime;
  rstl::auto_ptr< CQuitGameScreen > mQuitScreen;
  int mLeftStickIcon;
  int mRightStickIcon;
  float mLegendHiddenAmount;
  float x4bc_;
  float x4c0_;
  float mSelectionDelay;
  float mSelectionHighlight;
  CRepeatState mLeftRepeat;
  CRepeatState mRightRepeat;
  CVector3f mModelPan;
  CVector3f mModelCenterOffset;
  CVector3f mModelScale;
  CRelAngle mModelPitch;
  CRelAngle mModelYaw;
  float mModelZoomAmount;
  float mModelFade;
  CVector3f x508_;
  CSfxHandle mRotateSfx;
  CSfxHandle mPanSfx;
  CSfxHandle mZoomSfx;
  int mPendingScanNode;
  rstl::vector< CLight > mLights;
  rstl::single_ptr< CActorLights > mActorLights;
  CTransform4f mModelTransform;
  bool mDone : 1;
  bool x568_25_ : 1;
  bool mLegendVisible : 1;
  bool mHistoryTextReady : 1;
  bool mModelZoomed : 1;
  bool mNodesTouched : 1;
  bool x568_30_ : 1;
  bool mOpenedFromScan : 1;
  bool mModelsReady : 1;
};
CHECK_SIZEOF(CPauseScreen, 0x56c)

#endif // _CPAUSESCREEN
