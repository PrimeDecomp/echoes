#ifndef _SLDRAIJUMPPOINT
#define _SLDRAIJUMPPOINT

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrAIJumpPoint {
    SLdrAIJumpPoint();
    ~SLdrAIJumpPoint();

    SLdrEditorProperties editorProperties;
    float jumpApex;
    int type;
};

void LoadTypedefSLdrAIJumpPoint(SLdrAIJumpPoint&, CInputStream&);

#endif // _SLDRAIJUMPPOINT
