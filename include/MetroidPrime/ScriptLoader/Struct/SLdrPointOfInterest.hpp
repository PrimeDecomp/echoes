#ifndef _SLDRPOINTOFINTEREST
#define _SLDRPOINTOFINTEREST

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrScannableParameters.hpp"

struct SLdrPointOfInterest {
    SLdrPointOfInterest();
    ~SLdrPointOfInterest();

    SLdrEditorProperties editor_properties;
    SLdrScannableParameters scan_info;
    float scan_offset;
    bool look_at_poi;
};

void LoadTypedefSLdrPointOfInterest(SLdrPointOfInterest&, CInputStream&);

#endif // _SLDRPOINTOFINTEREST
