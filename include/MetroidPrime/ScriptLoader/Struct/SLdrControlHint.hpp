#ifndef _SLDRCONTROLHINT
#define _SLDRCONTROLHINT

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrControlHintStruct.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrControlHint {
    SLdrControlHint();
    ~SLdrControlHint();

    SLdrEditorProperties editorProperties;
    int priority;
    float timer;
    int cancelMethod;
    int cancelPressCount;
    float cancelPressTime;
    float cancelTimer;
    int unknown;
    SLdrControlHintStruct command1;
    SLdrControlHintStruct command2;
    SLdrControlHintStruct command3;
    SLdrControlHintStruct command4;
    SLdrControlHintStruct command5;
    SLdrControlHintStruct command6;
    SLdrControlHintStruct command7;
    SLdrControlHintStruct command8;
};

void LoadTypedefSLdrControlHint(SLdrControlHint&, CInputStream&);

#endif // _SLDRCONTROLHINT
