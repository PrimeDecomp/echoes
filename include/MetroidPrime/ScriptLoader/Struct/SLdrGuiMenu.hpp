#ifndef _SLDRGUIMENU
#define _SLDRGUIMENU

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrGuiWidgetProperties.hpp"

struct SLdrGuiMenu {
    SLdrGuiMenu();
    ~SLdrGuiMenu();

    SLdrEditorProperties editor_properties;
    SLdrGuiWidgetProperties gui_widget_properties;
    int control_direction;
    bool wrap_selection;
    int selection_changed_sound;
};

void LoadTypedefSLdrGuiMenu(SLdrGuiMenu&, CInputStream&);

#endif // _SLDRGUIMENU
