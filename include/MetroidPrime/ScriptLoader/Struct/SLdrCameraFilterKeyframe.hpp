#ifndef _SLDRCAMERAFILTERKEYFRAME
#define _SLDRCAMERAFILTERKEYFRAME

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrCameraFilterKeyframe {
    SLdrCameraFilterKeyframe();
    ~SLdrCameraFilterKeyframe();

    SLdrEditorProperties editor_properties;
    int filter_type;
    Choice filter_shape;
    int filter_stage;
    int which_filter_group;
    CColor color;
    float interpolate_in_time;
    float interpolate_out_time;
    CAssetId texture;
};

void LoadTypedefSLdrCameraFilterKeyframe(SLdrCameraFilterKeyframe&, CInputStream&);

#endif // _SLDRCAMERAFILTERKEYFRAME
