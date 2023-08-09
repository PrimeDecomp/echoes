#ifndef _SLDRBLOGGSTRUCT
#define _SLDRBLOGGSTRUCT

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrBloggStruct {
    SLdrBloggStruct();
    ~SLdrBloggStruct();

    int unknown_0x3e505ddb;
    int unknown_0x118f1e46;
    float unknown_0x6e603df2;
    float unknown_0x1e74f1ec;
    float unknown_0xecba9fb2;
};

void LoadTypedefSLdrBloggStruct(SLdrBloggStruct&, CInputStream&);

#endif // _SLDRBLOGGSTRUCT
