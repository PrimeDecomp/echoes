#ifndef _SLDRCAMERAPITCH
#define _SLDRCAMERAPITCH

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSplineType.hpp"

struct SLdrCameraPitch {
    SLdrCameraPitch();
    ~SLdrCameraPitch();

    SLdrEditorProperties editor_properties;
    spline forwards_pitch;
    spline backwards_pitch;
    SLdrSplineType spline_type;
    bool unknown;
};

void LoadTypedefSLdrCameraPitch(SLdrCameraPitch&, CInputStream&);

#endif // _SLDRCAMERAPITCH
