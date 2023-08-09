#ifndef _SLDRSCANINFOSECONDARYMODEL
#define _SLDRSCANINFOSECONDARYMODEL

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "rstl/string.hpp"

struct SLdrScanInfoSecondaryModel {
    SLdrScanInfoSecondaryModel();
    ~SLdrScanInfoSecondaryModel();

    CAssetId secondary_static_model;
    SLdrAnimationParameters secondary_animated_model;
    rstl::string secondary_model_locator;
};

void LoadTypedefSLdrScanInfoSecondaryModel(SLdrScanInfoSecondaryModel&, CInputStream&);

#endif // _SLDRSCANINFOSECONDARYMODEL
