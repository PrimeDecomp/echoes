#ifndef _SLDRDAMAGEACTOR
#define _SLDRDAMAGEACTOR

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrDamageActor {
    SLdrDamageActor();
    ~SLdrDamageActor();

    SLdrEditorProperties editorProperties;
    SLdrDamageInfo damage;
};

void LoadTypedefSLdrDamageActor(SLdrDamageActor&, CInputStream&);

#endif // _SLDRDAMAGEACTOR
