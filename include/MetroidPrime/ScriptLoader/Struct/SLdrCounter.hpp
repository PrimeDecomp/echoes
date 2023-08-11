#ifndef _SLDRCOUNTER
#define _SLDRCOUNTER

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrCounter {
    SLdrCounter();
    ~SLdrCounter();

    SLdrEditorProperties editorProperties;
    int initialCount;
    int maxCount;
    bool autoReset;
    bool wrap;
};

void LoadTypedefSLdrCounter(SLdrCounter&, CInputStream&);

#endif // _SLDRCOUNTER
