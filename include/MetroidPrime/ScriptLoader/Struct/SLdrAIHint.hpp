#ifndef _SLDRAIHINT
#define _SLDRAIHINT

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrAIHint {
    SLdrAIHint();
    ~SLdrAIHint();

    SLdrEditorProperties editorProperties;
    int hintType;
    float radius;
    float valueParm;
    float valueParm2;
    float valueParm3;
};

void LoadTypedefSLdrAIHint(SLdrAIHint&, CInputStream&);

#endif // _SLDRAIHINT
