#ifndef _SLDRCONTROLHINTSTRUCT
#define _SLDRCONTROLHINTSTRUCT

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrControllerActionStruct.hpp"

struct SLdrControlHintStruct {
    SLdrControlHintStruct();
    ~SLdrControlHintStruct();

    SLdrControllerActionStruct command;
    int state;
};

void LoadTypedefSLdrControlHintStruct(SLdrControlHintStruct&, CInputStream&);

#endif // _SLDRCONTROLHINTSTRUCT
