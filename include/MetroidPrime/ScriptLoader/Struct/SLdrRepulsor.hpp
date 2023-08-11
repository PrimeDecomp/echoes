#ifndef _SLDRREPULSOR
#define _SLDRREPULSOR

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrRepulsor {
    SLdrRepulsor();
    ~SLdrRepulsor();

    SLdrEditorProperties editorProperties;
    int shape;
    float radius;
    float value;
    int flagsRepulsor;
};

void LoadTypedefSLdrRepulsor(SLdrRepulsor&, CInputStream&);

#endif // _SLDRREPULSOR
