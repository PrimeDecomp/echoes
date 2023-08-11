#ifndef _SLDRCONTROLLERACTION
#define _SLDRCONTROLLERACTION

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrControllerActionStruct.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrControllerAction {
    SLdrControllerAction();
    ~SLdrControllerAction();

    SLdrEditorProperties editorProperties;
    SLdrControllerActionStruct controllerActionStruct;
    bool oneShot;
};

void LoadTypedefSLdrControllerAction(SLdrControllerAction&, CInputStream&);

#endif // _SLDRCONTROLLERACTION
