#ifndef _SLDRSCANINFOSECONDARYMODEL
#define _SLDRSCANINFOSECONDARYMODEL

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "rstl/string.hpp"

struct SLdrScanInfoSecondaryModel {
    SLdrScanInfoSecondaryModel();
    ~SLdrScanInfoSecondaryModel();

    CAssetId secondaryStaticModel;
    SLdrAnimationParameters secondaryAnimatedModel;
    rstl::string secondaryModelLocator;
};

void LoadTypedefSLdrScanInfoSecondaryModel(SLdrScanInfoSecondaryModel&, CInputStream&);

#endif // _SLDRSCANINFOSECONDARYMODEL
