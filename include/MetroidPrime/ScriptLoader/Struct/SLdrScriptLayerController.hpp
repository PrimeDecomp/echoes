#ifndef _SLDRSCRIPTLAYERCONTROLLER
#define _SLDRSCRIPTLAYERCONTROLLER

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrLayerSwitch.hpp"

struct SLdrScriptLayerController {
    SLdrScriptLayerController();
    ~SLdrScriptLayerController();

    SLdrEditorProperties editorProperties;
    SLdrLayerSwitch layer;
    bool isDynamic;
};

void LoadTypedefSLdrScriptLayerController(SLdrScriptLayerController&, CInputStream&);

#endif // _SLDRSCRIPTLAYERCONTROLLER
