#ifndef _SLDREMPERORINGSTAGE3STRUCTB
#define _SLDREMPERORINGSTAGE3STRUCTB

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct2.hpp"

struct SLdrEmperorIngStage3StructB {
    SLdrEmperorIngStage3StructB();
    ~SLdrEmperorIngStage3StructB();

    float min_health_percentage;
    float unknown_0x95e7a2c2;
    float unknown_0x76ba1c18;
    SLdrUnknownStruct2 unknown_struct2_0x3826ec75;
    SLdrUnknownStruct2 unknown_struct2_0x93bf1106;
    SLdrUnknownStruct2 unknown_struct2_0xc4b88b80;
    SLdrUnknownStruct2 unknown_struct2_0x32c6dc77;
    SLdrUnknownStruct2 unknown_struct2_0xc6e7b293;
    SLdrUnknownStruct2 unknown_struct2_0x20746b56;
    SLdrUnknownStruct2 unknown_struct2_0x2ab44adb;
    SLdrUnknownStruct2 unknown_struct2_0xe2e78a78;
};

void LoadTypedefSLdrEmperorIngStage3StructB(SLdrEmperorIngStage3StructB&, CInputStream&);

#endif // _SLDREMPERORINGSTAGE3STRUCTB
