#ifndef _SLDRMEMORYRELAY
#define _SLDRMEMORYRELAY

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrMemoryRelay {
    SLdrMemoryRelay();
    ~SLdrMemoryRelay();

    SLdrEditorProperties editor_properties;
    bool one_shot;
    bool delayed_action;
};

void LoadTypedefSLdrMemoryRelay(SLdrMemoryRelay&, CInputStream&);

#endif // _SLDRMEMORYRELAY
