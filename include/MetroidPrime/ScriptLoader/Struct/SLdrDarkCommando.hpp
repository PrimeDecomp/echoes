#ifndef _SLDRDARKCOMMANDO
#define _SLDRDARKCOMMANDO

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDarkCommandoData.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrDarkCommando {
    SLdrDarkCommando();
    ~SLdrDarkCommando();

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    SLdrDarkCommandoData darkCommandoProperties;
};

void LoadTypedefSLdrDarkCommando(SLdrDarkCommando&, CInputStream&);

#endif // _SLDRDARKCOMMANDO
