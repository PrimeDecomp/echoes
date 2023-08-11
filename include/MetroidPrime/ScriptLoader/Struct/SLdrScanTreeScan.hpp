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

    SLdrEditorProperties editorProperties;
    CAssetId nameStringTable;
    rstl::string nameStringName;
    SLdrScannableParameters scannableParameters;
};

void LoadTypedefSLdrScanTreeScan(SLdrScanTreeScan&, CInputStream&);

#endif // _SLDRSCANTREESCAN
