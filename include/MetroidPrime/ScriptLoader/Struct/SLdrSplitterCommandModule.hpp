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

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    SLdrUnknownStruct43 unknownStruct43;
};

void LoadTypedefSLdrSplitterCommandModule(SLdrSplitterCommandModule&, CInputStream&);

#endif // _SLDRSPLITTERCOMMANDMODULE
