#ifndef _SLDRSCANTREEMENU
#define _SLDRSCANTREEMENU

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "rstl/string.hpp"

struct SLdrScanTreeMenu {
    SLdrScanTreeMenu();
    ~SLdrScanTreeMenu();

    SLdrEditorProperties editor_properties;
    CAssetId name_string_table;
    rstl::string name_string_name;
    int unknown_0x0261a4e0;
    CAssetId menu_options_string_table;
    rstl::string option_1_string_name;
    int unknown_0x50bce632;
    rstl::string option_2_string_name;
    int unknown_0x420949dc;
    rstl::string option_3_string_name;
    int unknown_0xfab52eb9;
    rstl::string option_4_string_name;
    int unknown_0x67621600;
};

void LoadTypedefSLdrScanTreeMenu(SLdrScanTreeMenu&, CInputStream&);

#endif // _SLDRSCANTREEMENU
