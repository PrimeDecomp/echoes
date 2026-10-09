#ifndef _CSLIDESHOW
#define _CSLIDESHOW

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CIOWin.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/pair.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CDependencyGroup;
class CFinalInput;
class CGuiTextSupport;
class CRSFAudio;
class CStringTable;
class CTexture;

class CSlideShow : public CIOWin {
public:
  CSlideShow();

  // CIOWin
  ~CSlideShow() override;
  EMessageReturn OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue) override;
  bool GetIsContinueDraw() const override;
  void Draw() const override;

  static uint GetGalleriesUnlocked();

private:
  struct SGalleryData {
    int mGallery;
    rstl::vector< const SObjectTag* > mTextures;
    rstl::vector< rstl::pair< int, int > > mSlides;

    explicit SGalleryData(int gallery) : mGallery(gallery) {}
  };

  struct STexture {
    rstl::auto_ptr< TToken< CTexture > > mToken;
    CVector2f mLeftBottom;
    CVector2f mRightTop;
    mutable float mAlpha;

    STexture() : mLeftBottom(0.f, 0.f), mRightTop(0.f, 0.f), mAlpha(0.f) {}
  };

  struct SSlideData {
    CSlideShow* mParent;
    int mGallery;
    int mSlide;
    rstl::vector< STexture > mTextures;
    int mColumns;
    float mTextureWidth;
    float mTextureHeight;
    bool mReady;
    bool mStopLoading;
    CVector2f mVpOffset;
    CVector2f mVpSize;
    CVector2f mCanvasSize;
    CColor mMulColor;

    SSlideData()
    : mParent(nullptr)
    , mGallery(-1)
    , mSlide(-1)
    , mColumns(0)
    , mReady(false)
    , mStopLoading(false)
    , mVpOffset(0.f, 0.f)
    , mVpSize(0.f, 0.f)
    , mCanvasSize(0.f, 0.f)
    , mMulColor(CColor::White().WithAlphaOf(0.f)) {}

    const bool IsLoaded() const;
    bool IsReady() const { return IsLoaded() && mReady; }
    void Reset();
    void Draw() const;
    void InitializeViewport();
    EMessageReturn ProcessUserInput(const CFinalInput& input);
  };

  friend struct SSlideData;

  bool LoadTXTRDep(const char* name);
  void BuildGalleryLists(uint flags);
  EMessageReturn ProcessUserInput(const CFinalInput& input);
  EMessageReturn AdvanceSlide(bool forward);
  void LoadSlide();
  bool IsControlsAnimating() const;
  void SetShowControls(bool show);
  void UpdateControls(float dt);
  void UpdateControlsText(const CFinalInput& input);
  void UpdateSlideNumber(float dt);
  void DrawSlideNumber() const;
  void DrawControls() const;
  bool AreAllDepsLoaded(const rstl::vector< TToken< CDependencyGroup > >& deps) const;
  void SetDependenciesLocked(rstl::vector< TToken< CDependencyGroup > >& deps, bool locked);
  void SetTexturesLocked(rstl::vector< CToken >& textures, bool locked);
  void UpdateMusicVolume(float time, float fadeTime);
  void SetZoomSfx(bool active);
  void SetPanSfx(bool active);

  int mPhase;
  rstl::vector< TToken< CDependencyGroup > > mGalleryTXTRDeps;
  rstl::vector< SGalleryData > mGalleries;
  int x38_; // Initialized to zero; no other use found in this TU.
  int mTotalSlides;
  int mGallery;
  int mSlide;
  float mCrossfadeTimer;
  float mRepeatTimer;
  float mIdleTimer;
  float mSlideNumberTimer;
  SSlideData mSlideA;
  SSlideData mSlideB;
  rstl::single_ptr< CGuiTextSupport > mControlsText;
  rstl::single_ptr< CGuiTextSupport > mGalleryNameText;
  rstl::single_ptr< CGuiTextSupport > mSlideNumberText;
  rstl::single_ptr< CRSFAudio > mAudio;
  rstl::single_ptr< TToken< CStringTable > > mGalleryNames;
  rstl::vector< rstl::wstring > mGalleryLabels;
  CSfxHandle mPanSfx;
  CSfxHandle mZoomSfx;
  int mLStick;
  int mCStick;
  int mLTrigger;
  int mRTrigger;
  rstl::vector< CToken > mStickTextures;
  rstl::vector< CToken > mButtonTextures;
  rstl::vector< CToken > mTextures;
  float mSlideNumberOffset; // Guessed name
  float mFadeTimer;
  float mControlsAlpha; // Guessed name
  bool mShowControls : 1;
  bool mShowSlideNumber : 1; // Guessed name
  bool mDisableInput : 1;
  bool mExit : 1;
  bool mIntroFade : 1;
  bool mOutroFade : 1;
  bool mGalleryChanged : 1;
  bool mLoadMusic : 1; // Guessed name
};
CHECK_SIZEOF(CSlideShow, 0x164)

bool IsDataLoreResearchScan(CAssetId id);
CAssetId UpdatePersistentScanPercent(int previous, int current, int total);

#endif // _CSLIDESHOW
