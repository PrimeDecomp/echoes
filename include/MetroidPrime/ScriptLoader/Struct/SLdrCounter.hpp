#ifndef _SLDRCOUNTER
#define _SLDRCOUNTER

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrCounter {
    SLdrCounter();
    ~SLdrCounter();

    SLdrEditorProperties editor_properties;
    int initial_count;
    int max_count;
    bool auto_reset;
    bool wrap;
};

void LoadTypedefSLdrCounter(SLdrCounter&, CInputStream&);

#endif // _SLDRCOUNTER
