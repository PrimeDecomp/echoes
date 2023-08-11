#ifndef _SLDRCAMERABLURKEYFRAME
#define _SLDRCAMERABLURKEYFRAME

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrCameraBlurKeyframe {
    SLdrCameraBlurKeyframe();
    ~SLdrCameraBlurKeyframe();

    SLdrEditorProperties editorProperties;
    int blurType;
    float blurRadius;
    int whichFilterGroup;
    float interpolateInTime;
    float interpolateOutTime;
};

void LoadTypedefSLdrCameraBlurKeyframe(SLdrCameraBlurKeyframe&, CInputStream&);

#endif // _SLDRCAMERABLURKEYFRAME
