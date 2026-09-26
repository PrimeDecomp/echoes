#ifndef _CSCANHISTORY
#define _CSCANHISTORY

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/string.hpp"

class CGuiWidget;
class CGuiTextPane;
class CAuiMeter;

// Guessed name; shared by the scan HUD and display.
struct SScanHierarchyNode {
  explicit SScanHierarchyNode(CInputStream& in)
  : mStringTable(in.ReadInt32())
  , mName(in)
  , mScan(in.ReadInt32())
  , mParent(in.ReadInt32())
  , mTotalScans(0)
  , mCompletedScans(0) {}

  CAssetId mStringTable;
  rstl::string mName;
  CAssetId mScan;
  int mParent;
  int mTotalScans;
  int mCompletedScans;
};
CHECK_SIZEOF(SScanHierarchyNode, 0x24)

// Guessed name
struct SScanHistoryWidgets {
  SScanHistoryWidgets(CGuiWidget* root, CGuiTextPane* history, CGuiTextPane* number,
                      CAuiMeter* percent, CGuiWidget* flash, CGuiWidget* doubleWidget)
  : mRoot(root)
  , mHistory(history)
  , mNumber(number)
  , mPercent(percent)
  , mFlash(flash)
  , mDouble(doubleWidget) {}

  CGuiWidget* mRoot;
  CGuiTextPane* mHistory;
  CGuiTextPane* mNumber;
  CAuiMeter* mPercent;
  CGuiWidget* mFlash;
  CGuiWidget* mDouble;
};
CHECK_SIZEOF(SScanHistoryWidgets, 0x18)

#endif // _CSCANHISTORY
