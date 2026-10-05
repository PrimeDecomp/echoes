#ifndef _CSCANHISTORY
#define _CSCANHISTORY

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/string.hpp"

class CGuiWidget;
class CGuiTextPane;
class CAuiBitmapMeter;

// Guessed name; shared by the scan HUD and display.
struct SScanHierarchyNode {
  explicit SScanHierarchyNode(CInputStream& in);

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
                      CAuiBitmapMeter* percent, CGuiWidget* flash, CGuiWidget* doubleWidget);

  CGuiWidget* mRoot;
  CGuiTextPane* mHistory;
  CGuiTextPane* mNumber;
  CAuiBitmapMeter* mPercent;
  CGuiWidget* mFlash;
  CGuiWidget* mDouble;
};
CHECK_SIZEOF(SScanHistoryWidgets, 0x18)

#endif // _CSCANHISTORY
