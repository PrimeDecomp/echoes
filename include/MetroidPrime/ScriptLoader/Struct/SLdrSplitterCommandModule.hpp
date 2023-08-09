#ifndef _SLDRSPLITTERCOMMANDMODULE
#define _SLDRSPLITTERCOMMANDMODULE

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct43.hpp"

struct SLdrSplitterCommandModule {
    SLdrSplitterCommandModule();
    ~SLdrSplitterCommandModule();

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    SLdrUnknownStruct43 unknown_struct43;
};

void LoadTypedefSLdrSplitterCommandModule(SLdrSplitterCommandModule&, CInputStream&);

#endif // _SLDRSPLITTERCOMMANDMODULE
