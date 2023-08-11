#ifndef _SLDRENVFXDENSITYCONTROLLER
#define _SLDRENVFXDENSITYCONTROLLER

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrEnvFxDensityController {
    SLdrEnvFxDensityController();
    ~SLdrEnvFxDensityController();

    SLdrEditorProperties editorProperties;
    float density;
    int fadeSpeed;
};

void LoadTypedefSLdrEnvFxDensityController(SLdrEnvFxDensityController&, CInputStream&);

#endif // _SLDRENVFXDENSITYCONTROLLER
