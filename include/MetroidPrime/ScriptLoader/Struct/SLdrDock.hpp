#ifndef _SLDRDOCK
#define _SLDRDOCK

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrDock {
    SLdrDock();
    ~SLdrDock();

    SLdrEditorProperties editor_properties;
    int dock_number;
    int area_number;
    bool is_virtual;
    bool load_connected_immediate;
    bool show_soft_transition;
};

void LoadTypedefSLdrDock(SLdrDock&, CInputStream&);

#endif // _SLDRDOCK
