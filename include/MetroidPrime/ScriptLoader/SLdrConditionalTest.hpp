#ifndef _SLDRCONDITIONALTEST
#define _SLDRCONDITIONALTEST

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPlayerItem.hpp"

struct SLdrConditionalTest {
    SLdrConditionalTest();
    ~SLdrConditionalTest();

    Choice boolean;
    SLdrPlayerItem player_item;
    Choice amount_or_capacity;
    Choice condition;
    int value;
};

void LoadTypedefSLdrConditionalTest(SLdrConditionalTest&, CInputStream&);

#endif // _SLDRCONDITIONALTEST
