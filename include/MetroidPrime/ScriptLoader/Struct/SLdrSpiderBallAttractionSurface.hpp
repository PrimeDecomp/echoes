#ifndef _SLDRSPIDERBALLATTRACTIONSURFACE
#define _SLDRSPIDERBALLATTRACTIONSURFACE

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrSpiderBallAttractionSurface {
    SLdrSpiderBallAttractionSurface();
    ~SLdrSpiderBallAttractionSurface();

    SLdrEditorProperties editor_properties;
};

void LoadTypedefSLdrSpiderBallAttractionSurface(SLdrSpiderBallAttractionSurface&, CInputStream&);

#endif // _SLDRSPIDERBALLATTRACTIONSURFACE
