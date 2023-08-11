#ifndef _SLDRSPIDERBALLATTRACTIONSURFACE
#define _SLDRSPIDERBALLATTRACTIONSURFACE

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrSpiderBallAttractionSurface {
    SLdrSpiderBallAttractionSurface();
    ~SLdrSpiderBallAttractionSurface();

    SLdrEditorProperties editorProperties;
};

void LoadTypedefSLdrSpiderBallAttractionSurface(SLdrSpiderBallAttractionSurface&, CInputStream&);

#endif // _SLDRSPIDERBALLATTRACTIONSURFACE
