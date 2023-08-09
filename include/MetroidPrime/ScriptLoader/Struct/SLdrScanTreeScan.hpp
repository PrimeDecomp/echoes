#ifndef _SLDRSCANTREESCAN
#define _SLDRSCANTREESCAN

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrScannableParameters.hpp"
#include "rstl/string.hpp"

struct SLdrScanTreeScan {
    SLdrScanTreeScan();
    ~SLdrScanTreeScan();

    SLdrEditorProperties editor_properties;
    CAssetId name_string_table;
    rstl::string name_string_name;
    SLdrScannableParameters scannable_parameters;
};

void LoadTypedefSLdrScanTreeScan(SLdrScanTreeScan&, CInputStream&);

#endif // _SLDRSCANTREESCAN
