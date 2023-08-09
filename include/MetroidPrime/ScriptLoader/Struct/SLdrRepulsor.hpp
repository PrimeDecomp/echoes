#ifndef _SLDRREPULSOR
#define _SLDRREPULSOR

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrRepulsor {
    SLdrRepulsor();
    ~SLdrRepulsor();

    SLdrEditorProperties editor_properties;
    int shape;
    float radius;
    float value;
    int flags_repulsor;
};

void LoadTypedefSLdrRepulsor(SLdrRepulsor&, CInputStream&);

#endif // _SLDRREPULSOR
