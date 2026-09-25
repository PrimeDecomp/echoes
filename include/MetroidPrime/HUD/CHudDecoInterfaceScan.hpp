#ifndef _CHUDDECOINTERFACESCAN
#define _CHUDDECOINTERFACESCAN
#include "Kyoto/CDvdRequest.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"
class CGuiFrame;
class CGuiFrameLoader;
class CGuiWidget;
class CGuiTextPane;
class CGuiCamera;
class CAuiEnergyBarT01;
class CAuiMeter;
class CScanDisplay;
class CStringTable;
class CStateManager;
class CFinalInput;
class CScannableObjectInfo;
// Guessed name; unlike Prime's decoration interface, this has no virtual base.
class CHudDecoInterfaceScan {
public:
  CHudDecoInterfaceScan(const CStateManager& mgr, CGuiFrame& frame,
                        const TLockedToken< CStringTable >& strings, int playerIndex);
  ~CHudDecoInterfaceScan();
  void ProcessControllerInput(const CFinalInput& input);
  void PrepareScanDisplay(const CStateManager& mgr, int playerIndex);
  void Draw(const CStateManager& mgr) const;
  void Update(float dt, const CStateManager& mgr);
  float GetMessageTextAlpha() const;

private:
  // Guessed name
  struct SScanHierarchyNode {
    uint x0;
    rstl::string mName;
    CAssetId mScan;
    int mParent;
    int mTotalScans;
    int mCompletedScans;
  };
  // Guessed name
  struct SScanHistoryWidgets {
    CGuiWidget* mRoot;
    CGuiTextPane* mHistory;
    CGuiTextPane* mNumber;
    CAuiMeter* mPercent;
    CGuiWidget* mFlash;
    CGuiWidget* mDouble;
  };
  void InitializeFlatFrame(const CStateManager& mgr);
  void UpdateScanDisplay(const CStateManager& mgr, float dt);
  const CScannableObjectInfo* GetCurrScanInfo(const CStateManager& mgr) const;
  // Guessed names for Echoes' scan-history support.
  void StartHierarchyLoad();
  bool CheckHierarchyLoadComplete();
  void ReadHierarchy(CInputStream& in);
  void ClearHierarchy();
  void UpdateHierarchyProgress(const CStateManager& mgr);
  void BuildScanHistory(CAssetId scan, rstl::vector< SScanHierarchyNode >& history) const;
  int mPlayerIndex;
  rstl::auto_ptr< CGuiFrameLoader > mFrameLoader;
  rstl::auto_ptr< CGuiFrame > mFlatFrame;
  CGuiFrame* mLoadedFlatFrame;
  const TLockedToken< CStringTable >& mStrings;
  CScanDisplay* mScanDisplay; // Owned; construction/destruction awaits its shared interface.
  TUniqueId mLatestHudPoi;
  TUniqueId mLatestScanningObject;
  int mLatestScanState;
  float mScanningTime;
  CAssetId mCurrentScan;
  float x30;
  float x34;
  CVector3f x38;
  CVector3f x44;
  float x50;
  float x54;
  float mScanningTextAlpha;
  float mScanBarAlpha;
  CGuiCamera* mCamera;
  CGuiTextPane* mScanningText;
  CGuiWidget* mScanGauge;
  CAuiEnergyBarT01* mScanBar;
  CGuiWidget* mTextGroup;
  CGuiTextPane* mMessage;
  CGuiTextPane* mScrollMessage;
  CGuiWidget* mXMark;
  CGuiWidget* mAButton;
  CGuiWidget* mDash;
  CGuiWidget* mStartButton;
  CGuiTextPane* mPressStart;
  CGuiWidget* mHistoryRoot;
  CGuiWidget* mHistoryLeft;
  CGuiWidget* mHistoryRight;
  rstl::auto_ptr< uchar > mHierarchyBuffer;
  uint mHierarchyBufferLength;
  rstl::auto_ptr< CDvdRequest > mHierarchyRequest;
  rstl::vector< SScanHierarchyNode > mHierarchy;
  rstl::vector< SScanHistoryWidgets > mHistoryWidgets;
};
CHECK_SIZEOF(CHudDecoInterfaceScan, 0xd0)
#endif // _CHUDDECOINTERFACESCAN
