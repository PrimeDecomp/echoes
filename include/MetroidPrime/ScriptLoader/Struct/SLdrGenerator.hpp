#ifndef _SLDRGENERATOR
#define _SLDRGENERATOR

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrGenerator {
    SLdrGenerator();
    ~SLdrGenerator();

    SLdrEditorProperties editor_properties;
    int random_count;
    bool unique_locations;
    bool keep_orientation;
    bool use_originator_transform;
    CVector3f offset;
    float random_scale_min;
    float random_scale_max;
};

void LoadTypedefSLdrGenerator(SLdrGenerator&, CInputStream&);

#endif // _SLDRGENERATOR
