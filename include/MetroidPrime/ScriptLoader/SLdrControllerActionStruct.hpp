#ifndef _SLDRCONTROLLERACTIONSTRUCT
#define _SLDRCONTROLLERACTIONSTRUCT

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrControllerActionStruct {
    SLdrControllerActionStruct();
    ~SLdrControllerActionStruct();

    int command;
};

void LoadTypedefSLdrControllerActionStruct(SLdrControllerActionStruct&, CInputStream&);

#endif // _SLDRCONTROLLERACTIONSTRUCT
