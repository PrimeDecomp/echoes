#include "GuiSys/CGuiCompoundWidget.hpp"

CGuiCompoundWidget::CGuiCompoundWidget(const CGuiWidgetParms& parms) : CGuiWidget(parms) {}

void CGuiCompoundWidget::OnActivate() {
  CGuiWidget* widget = static_cast< CGuiWidget* >(ChildObject());
  while (widget != nullptr) {
    widget->SetIsActive(GetIsActive());
    widget = static_cast< CGuiWidget* >(widget->NextSibling());
  }

  CGuiWidget::OnActivate();
}

void CGuiCompoundWidget::OnVisible() {
  CGuiWidget* widget = static_cast< CGuiWidget* >(ChildObject());
  while (widget != nullptr) {
    widget->SetIsVisible(GetIsVisible());
    widget = static_cast< CGuiWidget* >(widget->NextSibling());
  }

  CGuiWidget::OnVisible();
}
