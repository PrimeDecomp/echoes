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

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    SLdrSplitterMainChassisData splitter_main_chassis_data;
};

void LoadTypedefSLdrSplitterMainChassis(SLdrSplitterMainChassis&, CInputStream&);

#endif // _SLDRSPLITTERMAINCHASSIS
