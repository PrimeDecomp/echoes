#ifndef _SLDRINGPUDDLE
#define _SLDRINGPUDDLE

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct30.hpp"

struct SLdrIngPuddle {
    SLdrIngPuddle();
    ~SLdrIngPuddle();

    SLdrEditorProperties editorProperties;
    SLdrActorParameters actorInformation;
    SLdrUnknownStruct30 unknownStruct30;
};

void LoadTypedefSLdrIngPuddle(SLdrIngPuddle&, CInputStream&);

#endif // _SLDRINGPUDDLE
