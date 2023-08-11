#ifndef _SLDRGENERATOR
#define _SLDRGENERATOR

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrGenerator {
    SLdrGenerator();
    ~SLdrGenerator();

    SLdrEditorProperties editorProperties;
    int randomCount;
    bool uniqueLocations;
    bool keepOrientation;
    bool useOriginatorTransform;
    CVector3f offset;
    float randomScaleMin;
    float randomScaleMax;
};

void LoadTypedefSLdrGenerator(SLdrGenerator&, CInputStream&);

#endif // _SLDRGENERATOR
