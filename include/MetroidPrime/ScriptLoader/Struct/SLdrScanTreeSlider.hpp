#ifndef _SLDRSCANTREESLIDER
#define _SLDRSCANTREESLIDER

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "rstl/string.hpp"

struct SLdrScanTreeSlider {
    SLdrScanTreeSlider();
    ~SLdrScanTreeSlider();

    SLdrEditorProperties editor_properties;
    CAssetId name_string_table;
    rstl::string name_string_name;
    int unknown;
};

void LoadTypedefSLdrScanTreeSlider(SLdrScanTreeSlider&, CInputStream&);

#endif // _SLDRSCANTREESLIDER
