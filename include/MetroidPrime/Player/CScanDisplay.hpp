#ifndef _CSCANDISPLAY
#define _CSCANDISPLAY

#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CValidEntityPredicate.hpp"
#include "MetroidPrime/Factories/CScannableObjectInfo.hpp"
#include "MetroidPrime/HUD/CScanHistory.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"

class CFinalInput;
class CGuiFrame;
class CModel;
class CModelData;
class CStringTable;
class CTexture;

class CScanDisplay {
public:
  enum EScanState {
    kSS_Inactive,
    kSS_Downloading,
    kSS_DownloadComplete,
    kSS_ViewingScan,
    kSS_Done,
  };

  explicit CScanDisplay(const CGuiFrame* selHud);
  ~CScanDisplay();
  void StartScan(TUniqueId uid, const CScannableObjectInfo& info, CGuiTextPane* message,
                 CGuiTextPane* scrollMessage, CGuiWidget* textGroup, CGuiWidget* xMark,
                 CGuiWidget* aButton, CGuiWidget* dash, CGuiWidget* startButton,
                 CGuiTextPane* pressStart, CGuiWidget* historyRoot, CGuiWidget* historyRight,
                 const rstl::vector< SScanHierarchyNode >& history,
                 const rstl::vector< SScanHistoryWidgets >& widgets, float scanTime, bool showText,
                 const CStateManager& mgr);
  void StopScan();
  void Update(float dt, float scanningTime, const CStateManager& mgr);
  void ProcessInput(const CFinalInput& input);
  void Draw(const CStateManager& mgr) const;
  // Guessed name: resolves world geometry or the actor/scan model before drawing.
  void PrepareScanDisplay(const CStateManager& mgr, int playerIndex);

  EScanState GetScanState() const { return mState; }
  TUniqueId GetScanningObject() const { return mObject; }
  float GetBodyAlpha() const { return mBodyAlpha; }

private:
  // Guessed name
  class CScanTargetPredicate : public CValidEntityPredicate {
  public:
    explicit CScanTargetPredicate(TUniqueId object) : mObject(object) {}

    // CValidEntityPredicate
    ~CScanTargetPredicate();
    bool IsValid(const CStateManager& mgr, TUniqueId id) const override;

  private:
    TUniqueId mObject;
  };

  static void SetScanMessageTypeEffect(CGuiTextPane* pane, bool type);
  float GetDownloadStartTime(int historyIndex) const;
  float GetDownloadFraction(int historyIndex, float time) const;
  float GetTotalDownloadTime() const;
  void RequestScanDisplay();   // Guessed name
  void UpdateAPulse(float dt); // Guessed name

  TCachedToken< CTexture > mDataDotTexture;
  EScanState mState;
  TUniqueId mObject;
  rstl::optional_object< TCachedToken< CTexture > > mScanTexture;
  rstl::optional_object< CScannableObjectInfo > mScannableInfo;
  const CGuiFrame* mSelHud;
  CGuiWidget* mTextGroup;
  CGuiTextPane* mMessage;
  CGuiTextPane* mScrollMessage;
  CGuiWidget* mXMark;
  CGuiWidget* mAButton;
  CGuiWidget* mDash;
  CGuiWidget* mHistoryRoot;
  CGuiWidget* mHistoryRight;
  CGuiWidget* mStartButton;
  CGuiTextPane* mPressStart;
  rstl::vector< SScanHierarchyNode > mHistory;
  rstl::vector< SScanHistoryWidgets > mHistoryWidgets;
  rstl::optional_object< TCachedToken< CStringTable > > mScanString;
  rstl::optional_object< TCachedToken< CModel > > mScanModelToken;
  rstl::auto_ptr< CModelData > mScanModel;
  rstl::vector< TCachedToken< CStringTable > > mHistoryStrings;
  rstl::wstring mCategoryName;
  TUniqueId mModelObject;
  rstl::vector< rstl::pair< TAreaId, int > > mWorldModels;
  CAABox mModelBounds;
  CVector3f mStartPosition;
  CVector3f mEndPosition;
  CQuaternion mStartRotation;
  CQuaternion mEndRotation;
  CQuaternion mModelRotation;
  CVector3f mStartScale;
  CVector3f mEndScale;
  float x2ec_; // Camera parameter saved during preparation; use not established.
  float mStartFov;
  float mEndFov;
  float mModelTransition;
  float mXAlpha;
  float mBodyAlpha;
  int mPageCounter;
  float mAPulse;
  float mModelYaw;
  int mAPulseCount;
  bool mScanComplete : 1;
  bool x314_1_ : 1; // No initialized value or reader established.
  bool mHintsSuppressed : 1;
  bool mPreparePending : 1;
  bool mCanOpenLogbook : 1;
};
CHECK_SIZEOF(CScanDisplay, 0x318)

#endif // _CSCANDISPLAY
