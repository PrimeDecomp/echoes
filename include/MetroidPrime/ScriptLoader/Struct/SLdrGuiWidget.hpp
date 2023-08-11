#ifndef _SLDRGUIWIDGET
#define _SLDRGUIWIDGET

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrGuiWidgetProperties.hpp"

struct SLdrGuiWidget {
    SLdrGuiWidget();
    ~SLdrGuiWidget();

    SLdrEditorProperties editorProperties;
    SLdrGuiWidgetProperties guiWidgetProperties;
    int controllerNumber;
};

void LoadTypedefSLdrGuiWidget(SLdrGuiWidget&, CInputStream&);

#endif // _SLDRGUIWIDGET
