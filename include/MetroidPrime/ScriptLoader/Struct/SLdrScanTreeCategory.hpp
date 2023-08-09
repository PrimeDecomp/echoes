#ifndef _SLDRSCANTREECATEGORY
#define _SLDRSCANTREECATEGORY

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "rstl/string.hpp"

struct SLdrScanTreeCategory {
    SLdrScanTreeCategory();
    ~SLdrScanTreeCategory();

    SLdrEditorProperties editor_properties;
    CAssetId name_string_table;
    rstl::string name_string_name;
};

void LoadTypedefSLdrScanTreeCategory(SLdrScanTreeCategory&, CInputStream&);

#endif // _SLDRSCANTREECATEGORY
