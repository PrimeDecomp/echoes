#ifndef _SLDRFORGOTTENOBJECT
#define _SLDRFORGOTTENOBJECT

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrForgottenObject {
    SLdrForgottenObject();
    ~SLdrForgottenObject();

    SLdrEditorProperties editor_properties;
};

void LoadTypedefSLdrForgottenObject(SLdrForgottenObject&, CInputStream&);

#endif // _SLDRFORGOTTENOBJECT
