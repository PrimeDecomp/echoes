#ifndef _SLDRPATHMESHCTRL
#define _SLDRPATHMESHCTRL

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrPathMeshCtrl {
    SLdrPathMeshCtrl();
    ~SLdrPathMeshCtrl();

    SLdrEditorProperties editor_properties;
    int type;
    int initial_count;
};

void LoadTypedefSLdrPathMeshCtrl(SLdrPathMeshCtrl&, CInputStream&);

#endif // _SLDRPATHMESHCTRL
