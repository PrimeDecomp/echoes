#ifndef _SLDRSCANNABLEOBJECTINFO
#define _SLDRSCANNABLEOBJECTINFO

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrScanInfoSecondaryModel.hpp"
#include "MetroidPrime/ScriptLoader/SLdrScanSpeed.hpp"

struct SLdrScannableObjectInfo {
    SLdrScannableObjectInfo();
    ~SLdrScannableObjectInfo();

    CAssetId string;
    SLdrScanSpeed scanSpeed;
    bool critical;
    bool unknown_0x1733b1ec;
    CAssetId unknown_0x53336141;
    float modelInitialPitch;
    float modelInitialYaw;
    float modelScale;
    CAssetId staticModel;
    SLdrAnimationParameters animatedModel;
    SLdrAnimationParameters unknown_0x58f9fe99;
    SLdrScanInfoSecondaryModel secondaryModel0;
    SLdrScanInfoSecondaryModel secondaryModel1;
    SLdrScanInfoSecondaryModel secondaryModel2;
    SLdrScanInfoSecondaryModel secondaryModel3;
    SLdrScanInfoSecondaryModel secondaryModel4;
    SLdrScanInfoSecondaryModel secondaryModel5;
    SLdrScanInfoSecondaryModel secondaryModel6;
    SLdrScanInfoSecondaryModel secondaryModel7;
    SLdrScanInfoSecondaryModel secondaryModel8;
};

void LoadTypedefSLdrScannableObjectInfo(SLdrScannableObjectInfo&, CInputStream&);

#endif // _SLDRSCANNABLEOBJECTINFO
