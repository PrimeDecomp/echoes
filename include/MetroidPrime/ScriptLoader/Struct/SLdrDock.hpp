#ifndef _SLDRDOCK
#define _SLDRDOCK

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrDock {
    SLdrDock();
    ~SLdrDock();

    SLdrEditorProperties editorProperties;
    int dockNumber;
    int areaNumber;
    bool isVirtual;
    bool loadConnectedImmediate;
    bool showSoftTransition;
};

void LoadTypedefSLdrDock(SLdrDock&, CInputStream&);

#endif // _SLDRDOCK
