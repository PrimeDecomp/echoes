#ifndef _SLDRGUISCREEN
#define _SLDRGUISCREEN

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrGuiScreen {
    SLdrGuiScreen();
    ~SLdrGuiScreen();

    SLdrEditorProperties editorProperties;
    int whichScreen;
    CAssetId stringTable;
};

void LoadTypedefSLdrGuiScreen(SLdrGuiScreen&, CInputStream&);

#endif // _SLDRGUISCREEN
