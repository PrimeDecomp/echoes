#ifndef _CGUICOMPOUNDWIDGET
#define _CGUICOMPOUNDWIDGET

#include "GuiSys/CGuiWidget.hpp"

class CGuiCompoundWidget : public CGuiWidget {
public:
  explicit CGuiCompoundWidget(const CGuiWidgetParms& parms);

  // CGuiWidget
  FourCC GetWidgetTypeID() const override { return -1; }
  EWidgetUsageFlags GetWidgetUsageFlags() const override { return kWUF_None; }
  void OnVisible() override;
  void OnActivate() override;
};
CHECK_SIZEOF(CGuiCompoundWidget, 0xbc)

#endif // _CGUICOMPOUNDWIDGET
