#ifndef _SLDRENVFXDENSITYCONTROLLER
#define _SLDRENVFXDENSITYCONTROLLER

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrEnvFxDensityController {
    SLdrEnvFxDensityController();
    ~SLdrEnvFxDensityController();

    SLdrEditorProperties editor_properties;
    float density;
    int fade_speed;
};

void LoadTypedefSLdrEnvFxDensityController(SLdrEnvFxDensityController&, CInputStream&);

#endif // _SLDRENVFXDENSITYCONTROLLER
