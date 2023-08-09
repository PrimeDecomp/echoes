#ifndef _SLDRRIPPER
#define _SLDRRIPPER

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrGrappleParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrRipper {
    SLdrRipper();
    ~SLdrRipper();

    SLdrEditorProperties editor_properties;
    int flavor;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    SLdrGrappleParameters grapple_info;
};

void LoadTypedefSLdrRipper(SLdrRipper&, CInputStream&);

#endif // _SLDRRIPPER
