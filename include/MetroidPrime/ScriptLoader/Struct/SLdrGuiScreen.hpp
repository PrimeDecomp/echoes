#ifndef _SLDRGUISCREEN
#define _SLDRGUISCREEN

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrGuiScreen {
    SLdrGuiScreen();
    ~SLdrGuiScreen();

    SLdrEditorProperties editor_properties;
    int which_screen;
    CAssetId string_table;
};

void LoadTypedefSLdrGuiScreen(SLdrGuiScreen&, CInputStream&);

#endif // _SLDRGUISCREEN
