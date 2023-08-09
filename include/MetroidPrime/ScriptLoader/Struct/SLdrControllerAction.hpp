#ifndef _SLDRCONTROLLERACTION
#define _SLDRCONTROLLERACTION

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrControllerActionStruct.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrControllerAction {
    SLdrControllerAction();
    ~SLdrControllerAction();

    SLdrEditorProperties editor_properties;
    SLdrControllerActionStruct controller_action_struct;
    bool one_shot;
};

void LoadTypedefSLdrControllerAction(SLdrControllerAction&, CInputStream&);

#endif // _SLDRCONTROLLERACTION
