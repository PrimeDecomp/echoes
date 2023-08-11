#ifndef _SLDRGUIMENU
#define _SLDRGUIMENU

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrGuiWidgetProperties.hpp"

struct SLdrGuiMenu {
    SLdrGuiMenu();
    ~SLdrGuiMenu();

    SLdrEditorProperties editorProperties;
    SLdrGuiWidgetProperties guiWidgetProperties;
    int controlDirection;
    bool wrapSelection;
    int selectionChangedSound;
};

void LoadTypedefSLdrGuiMenu(SLdrGuiMenu&, CInputStream&);

#endif // _SLDRGUIMENU
