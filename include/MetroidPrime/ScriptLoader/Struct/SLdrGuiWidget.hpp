#ifndef _SLDRGUIWIDGET
#define _SLDRGUIWIDGET

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrGuiWidgetProperties.hpp"

struct SLdrGuiWidget {
    SLdrGuiWidget();
    ~SLdrGuiWidget();

    SLdrEditorProperties editor_properties;
    SLdrGuiWidgetProperties gui_widget_properties;
    int controller_number;
};

void LoadTypedefSLdrGuiWidget(SLdrGuiWidget&, CInputStream&);

#endif // _SLDRGUIWIDGET
