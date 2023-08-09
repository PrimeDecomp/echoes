#ifndef _SLDRINGPUDDLE
#define _SLDRINGPUDDLE

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct30.hpp"

struct SLdrIngPuddle {
    SLdrIngPuddle();
    ~SLdrIngPuddle();

    SLdrEditorProperties editor_properties;
    SLdrActorParameters actor_information;
    SLdrUnknownStruct30 unknown_struct30;
};

void LoadTypedefSLdrIngPuddle(SLdrIngPuddle&, CInputStream&);

#endif // _SLDRINGPUDDLE
