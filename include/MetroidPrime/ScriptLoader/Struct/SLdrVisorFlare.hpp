#ifndef _SLDRVISORFLARE
#define _SLDRVISORFLARE

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrFlareDef.hpp"

struct SLdrVisorFlare {
    SLdrVisorFlare();
    ~SLdrVisorFlare();

    SLdrEditorProperties editorProperties;
    int blendMode;
    bool constantScale;
    float fadeTime;
    float fadeFactor;
    float rotateFactor;
    int combatVisorMode;
    bool unknown;
    bool noOcclusionTest;
    SLdrFlareDef flare1;
    SLdrFlareDef flare2;
    SLdrFlareDef flare3;
    SLdrFlareDef flare4;
    SLdrFlareDef flare5;
};

void LoadTypedefSLdrVisorFlare(SLdrVisorFlare&, CInputStream&);

#endif // _SLDRVISORFLARE
