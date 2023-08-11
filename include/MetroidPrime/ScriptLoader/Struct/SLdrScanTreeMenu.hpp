#ifndef _SLDRSCANTREEMENU
#define _SLDRSCANTREEMENU

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "rstl/string.hpp"

struct SLdrScanTreeMenu {
    SLdrScanTreeMenu();
    ~SLdrScanTreeMenu();

    SLdrEditorProperties editorProperties;
    CAssetId nameStringTable;
    rstl::string nameStringName;
    int unknown_0x0261a4e0;
    CAssetId menuOptionsStringTable;
    rstl::string option1StringName;
    int unknown_0x50bce632;
    rstl::string option2StringName;
    int unknown_0x420949dc;
    rstl::string option3StringName;
    int unknown_0xfab52eb9;
    rstl::string option4StringName;
    int unknown_0x67621600;
};

void LoadTypedefSLdrScanTreeMenu(SLdrScanTreeMenu&, CInputStream&);

#endif // _SLDRSCANTREEMENU
