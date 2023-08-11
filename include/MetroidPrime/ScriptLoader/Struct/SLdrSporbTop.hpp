#ifndef _SLDRSPORBTOP
#define _SLDRSPORBTOP

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrSporbTop {
    SLdrSporbTop();
    ~SLdrSporbTop();

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
};

void LoadTypedefSLdrSporbTop(SLdrSporbTop&, CInputStream&);

#endif // _SLDRSPORBTOP
