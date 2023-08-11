#ifndef _SLDRSCANTREECATEGORY
#define _SLDRSCANTREECATEGORY

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "rstl/string.hpp"

struct SLdrScanTreeCategory {
    SLdrScanTreeCategory();
    ~SLdrScanTreeCategory();

    SLdrEditorProperties editorProperties;
    CAssetId nameStringTable;
    rstl::string nameStringName;
};

void LoadTypedefSLdrScanTreeCategory(SLdrScanTreeCategory&, CInputStream&);

#endif // _SLDRSCANTREECATEGORY
