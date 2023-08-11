#ifndef _SLDRMEMORYRELAY
#define _SLDRMEMORYRELAY

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrMemoryRelay {
    SLdrMemoryRelay();
    ~SLdrMemoryRelay();

    SLdrEditorProperties editorProperties;
    bool oneShot;
    bool delayedAction;
};

void LoadTypedefSLdrMemoryRelay(SLdrMemoryRelay&, CInputStream&);

#endif // _SLDRMEMORYRELAY
