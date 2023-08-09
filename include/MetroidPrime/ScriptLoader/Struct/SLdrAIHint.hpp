#ifndef _SLDRAIHINT
#define _SLDRAIHINT

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrAIHint {
    SLdrAIHint();
    ~SLdrAIHint();

    SLdrEditorProperties editor_properties;
    int hint_type;
    float radius;
    float value_parm;
    float value_parm2;
    float value_parm3;
};

void LoadTypedefSLdrAIHint(SLdrAIHint&, CInputStream&);

#endif // _SLDRAIHINT
