#ifndef _SLDRGRAPPLEPOINT
#define _SLDRGRAPPLEPOINT

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrGrappleParameters.hpp"

struct SLdrGrapplePoint {
    SLdrGrapplePoint();
    ~SLdrGrapplePoint();

    SLdrEditorProperties editorProperties;
    SLdrGrappleParameters grappleInfo;
};

void LoadTypedefSLdrGrapplePoint(SLdrGrapplePoint&, CInputStream&);

#endif // _SLDRGRAPPLEPOINT
