#ifndef _CGUIHEADWIDGET
#define _CGUIHEADWIDGET

#include "GuiSys/CGuiWidget.hpp"

class CGuiHeadWidget : public CGuiWidget {
public:
  explicit CGuiHeadWidget(const CGuiWidgetParms& parms);

  static CGuiWidget* Create(CGuiFrame* frame, CInputStream& in, CSimplePool* pool, uint version);

  // CGuiWidget
  FourCC GetWidgetTypeID() const override { return 'HWIG'; }

  EWidgetUsageFlags GetWidgetUsageFlags() const override { return kWUF_None; }
};
CHECK_SIZEOF(CGuiHeadWidget, 0xbc)

#endif // _CGUIHEADWIDGET
