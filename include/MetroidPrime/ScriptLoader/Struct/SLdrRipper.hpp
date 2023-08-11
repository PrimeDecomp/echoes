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

    SLdrEditorProperties editorProperties;
    int flavor;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    SLdrGrappleParameters grappleInfo;
};

void LoadTypedefSLdrRipper(SLdrRipper&, CInputStream&);

#endif // _SLDRRIPPER
