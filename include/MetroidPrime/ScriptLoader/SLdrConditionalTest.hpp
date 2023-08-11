#ifndef _SLDRCONDITIONALTEST
#define _SLDRCONDITIONALTEST

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPlayerItem.hpp"

struct SLdrConditionalTest {
    SLdrConditionalTest();
    ~SLdrConditionalTest();

    int boolean;
    SLdrPlayerItem playerItem;
    int amountOrCapacity;
    int condition;
    int value;
};

void LoadTypedefSLdrConditionalTest(SLdrConditionalTest&, CInputStream&);

#endif // _SLDRCONDITIONALTEST
