#ifndef _SLDRREZBIT
#define _SLDRREZBIT

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"
#include "MetroidPrime/ScriptLoader/SLdrRezbitData.hpp"

struct SLdrRezbit {
    SLdrRezbit();
    ~SLdrRezbit();

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    SLdrRezbitData rezbitData;
};

void LoadTypedefSLdrRezbit(SLdrRezbit&, CInputStream&);

#endif // _SLDRREZBIT
