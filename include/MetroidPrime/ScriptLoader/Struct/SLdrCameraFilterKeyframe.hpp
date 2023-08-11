#ifndef _SLDRCAMERAFILTERKEYFRAME
#define _SLDRCAMERAFILTERKEYFRAME

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrCameraFilterKeyframe {
    SLdrCameraFilterKeyframe();
    ~SLdrCameraFilterKeyframe();

    SLdrEditorProperties editorProperties;
    int filterType;
    int filterShape;
    int filterStage;
    int whichFilterGroup;
    CColor color;
    float interpolateInTime;
    float interpolateOutTime;
    CAssetId texture;
};

void LoadTypedefSLdrCameraFilterKeyframe(SLdrCameraFilterKeyframe&, CInputStream&);

#endif // _SLDRCAMERAFILTERKEYFRAME
