#ifndef _SLDRPOINTOFINTEREST
#define _SLDRPOINTOFINTEREST

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrScannableParameters.hpp"

struct SLdrPointOfInterest {
    SLdrPointOfInterest();
    ~SLdrPointOfInterest();

    SLdrEditorProperties editorProperties;
    SLdrScannableParameters scanInfo;
    float scanOffset;
    bool lookAtPoi;
};

void LoadTypedefSLdrPointOfInterest(SLdrPointOfInterest&, CInputStream&);

#endif // _SLDRPOINTOFINTEREST
