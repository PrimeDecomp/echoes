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

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    SLdrDarkCommandoData dark_commando_properties;
};

void LoadTypedefSLdrDarkCommando(SLdrDarkCommando&, CInputStream&);

#endif // _SLDRDARKCOMMANDO
