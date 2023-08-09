#ifndef _SLDRGUIWIDGETPROPERTIES
#define _SLDRGUIWIDGETPROPERTIES

#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/string.hpp"

struct SLdrGuiWidgetProperties {
    SLdrGuiWidgetProperties();
    ~SLdrGuiWidgetProperties();

    rstl::string gui_label;
    int controller_number;
    bool is_locked;
};

void LoadTypedefSLdrGuiWidgetProperties(SLdrGuiWidgetProperties&, CInputStream&);

#endif // _SLDRGUIWIDGETPROPERTIES
