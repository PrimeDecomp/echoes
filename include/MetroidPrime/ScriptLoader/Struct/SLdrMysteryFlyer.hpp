#ifndef _SLDRMYSTERYFLYER
#define _SLDRMYSTERYFLYER

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrMysteryFlyerData.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrMysteryFlyer {
    SLdrMysteryFlyer();
    ~SLdrMysteryFlyer();

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    SLdrMysteryFlyerData mysteryFlyerProperties;
};

void LoadTypedefSLdrMysteryFlyer(SLdrMysteryFlyer&, CInputStream&);

#endif // _SLDRMYSTERYFLYER
