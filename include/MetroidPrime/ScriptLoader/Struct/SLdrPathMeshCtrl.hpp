#ifndef _SLDRPATHMESHCTRL
#define _SLDRPATHMESHCTRL

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrPathMeshCtrl {
    SLdrPathMeshCtrl();
    ~SLdrPathMeshCtrl();

    SLdrEditorProperties editorProperties;
    int type;
    int initialCount;
};

void LoadTypedefSLdrPathMeshCtrl(SLdrPathMeshCtrl&, CInputStream&);

#endif // _SLDRPATHMESHCTRL
