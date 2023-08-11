#ifndef _SLDRSPLITTERMAINCHASSIS
#define _SLDRSPLITTERMAINCHASSIS

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSplitterMainChassisData.hpp"

struct SLdrSplitterMainChassis {
    SLdrSplitterMainChassis();
    ~SLdrSplitterMainChassis();

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    SLdrSplitterMainChassisData splitterMainChassisData;
};

void LoadTypedefSLdrSplitterMainChassis(SLdrSplitterMainChassis&, CInputStream&);

#endif // _SLDRSPLITTERMAINCHASSIS
