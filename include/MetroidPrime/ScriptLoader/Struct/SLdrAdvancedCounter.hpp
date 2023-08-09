#ifndef _SLDRADVANCEDCOUNTER
#define _SLDRADVANCEDCOUNTER

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrAdvancedCounter {
    SLdrAdvancedCounter();
    ~SLdrAdvancedCounter();

    SLdrEditorProperties editor_properties;
    int initial_count;
    int max_count;
    bool auto_reset;
    int counter_condition1;
    int counter_condition2;
    int counter_condition3;
    int counter_condition4;
    int counter_condition5;
    int counter_condition6;
    int counter_condition7;
    int counter_condition8;
    int counter_condition9;
    int counter_condition10;
};

void LoadTypedefSLdrAdvancedCounter(SLdrAdvancedCounter&, CInputStream&);

#endif // _SLDRADVANCEDCOUNTER
