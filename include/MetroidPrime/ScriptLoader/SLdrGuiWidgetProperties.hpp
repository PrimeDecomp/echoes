#ifndef _SLDRGUIWIDGETPROPERTIES
#define _SLDRGUIWIDGETPROPERTIES

#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/string.hpp"

struct SLdrGuiWidgetProperties {
    SLdrGuiWidgetProperties();
    ~SLdrGuiWidgetProperties();

    rstl::string guiLabel;
    int controllerNumber;
    bool isLocked;
};

void LoadTypedefSLdrGuiWidgetProperties(SLdrGuiWidgetProperties&, CInputStream&);

#endif // _SLDRGUIWIDGETPROPERTIES
