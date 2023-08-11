#ifndef _SLDRFORGOTTENOBJECT
#define _SLDRFORGOTTENOBJECT

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrForgottenObject {
    SLdrForgottenObject();
    ~SLdrForgottenObject();

    SLdrEditorProperties editorProperties;
};

void LoadTypedefSLdrForgottenObject(SLdrForgottenObject&, CInputStream&);

#endif // _SLDRFORGOTTENOBJECT
