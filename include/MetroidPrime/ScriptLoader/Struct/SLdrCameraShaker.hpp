#ifndef _SLDRCAMERASHAKER
#define _SLDRCAMERASHAKER

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrCameraShakerData.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrCameraShaker {
    SLdrCameraShaker();
    ~SLdrCameraShaker();

    SLdrEditorProperties editor_properties;
    SLdrCameraShakerData shaker_data;
};

void LoadTypedefSLdrCameraShaker(SLdrCameraShaker&, CInputStream&);

#endif // _SLDRCAMERASHAKER
