#ifndef _SLDRTWEAKPLAYERCONTROLS
#define _SLDRTWEAKPLAYERCONTROLS

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerControls_UnknownStruct1.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerControls_UnknownStruct2.hpp"
#include "rstl/string.hpp"

struct SLdrTweakPlayerControls {
    SLdrTweakPlayerControls();
    ~SLdrTweakPlayerControls();

    rstl::string instanceName;
    SLdrTweakPlayerControls_UnknownStruct1 unknown_0x3c34dfed;
    SLdrTweakPlayerControls_UnknownStruct2 unknown_0x168a79f1;
};

void LoadTypedefSLdrTweakPlayerControls(SLdrTweakPlayerControls&, CInputStream&);

#endif // _SLDRTWEAKPLAYERCONTROLS
