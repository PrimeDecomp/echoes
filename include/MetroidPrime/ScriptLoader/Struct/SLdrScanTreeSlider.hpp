#ifndef _SLDRSCANTREESLIDER
#define _SLDRSCANTREESLIDER

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "rstl/string.hpp"

struct SLdrScanTreeSlider {
    SLdrScanTreeSlider();
    ~SLdrScanTreeSlider();

    SLdrEditorProperties editorProperties;
    CAssetId nameStringTable;
    rstl::string nameStringName;
    int unknown;
};

void LoadTypedefSLdrScanTreeSlider(SLdrScanTreeSlider&, CInputStream&);

#endif // _SLDRSCANTREESLIDER
