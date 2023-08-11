#ifndef _SLDRSKYRIPPLE
#define _SLDRSKYRIPPLE

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrSkyRipple {
    SLdrSkyRipple();
    ~SLdrSkyRipple();

    SLdrEditorProperties editorProperties;
};

void LoadTypedefSLdrSkyRipple(SLdrSkyRipple&, CInputStream&);

#endif // _SLDRSKYRIPPLE
