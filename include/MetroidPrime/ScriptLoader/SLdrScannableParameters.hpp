#ifndef _SLDRSCANNABLEPARAMETERS
#define _SLDRSCANNABLEPARAMETERS

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrScannableParameters {
    SLdrScannableParameters();
    ~SLdrScannableParameters();

    CAssetId scannable_info0;
};

void LoadTypedefSLdrScannableParameters(SLdrScannableParameters&, CInputStream&);

#endif // _SLDRSCANNABLEPARAMETERS
