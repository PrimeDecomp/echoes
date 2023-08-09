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
    SLdrScanSpeed scan_speed;
    bool critical;
    bool unknown_0x1733b1ec;
    CAssetId unknown_0x53336141;
    float model_initial_pitch;
    float model_initial_yaw;
    float model_scale;
    CAssetId static_model;
    SLdrAnimationParameters animated_model;
    SLdrAnimationParameters unknown_0x58f9fe99;
    SLdrScanInfoSecondaryModel secondary_model0;
    SLdrScanInfoSecondaryModel secondary_model1;
    SLdrScanInfoSecondaryModel secondary_model2;
    SLdrScanInfoSecondaryModel secondary_model3;
    SLdrScanInfoSecondaryModel secondary_model4;
    SLdrScanInfoSecondaryModel secondary_model5;
    SLdrScanInfoSecondaryModel secondary_model6;
    SLdrScanInfoSecondaryModel secondary_model7;
    SLdrScanInfoSecondaryModel secondary_model8;
};

void LoadTypedefSLdrScannableObjectInfo(SLdrScannableObjectInfo&, CInputStream&);

#endif // _SLDRSCANNABLEOBJECTINFO
