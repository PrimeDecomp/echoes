#ifndef _SLDRADVANCEDCOUNTER
#define _SLDRADVANCEDCOUNTER

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrAdvancedCounter {
    SLdrAdvancedCounter();
    ~SLdrAdvancedCounter();

    SLdrEditorProperties editorProperties;
    int initialCount;
    int maxCount;
    bool autoReset;
    int counterCondition1;
    int counterCondition2;
    int counterCondition3;
    int counterCondition4;
    int counterCondition5;
    int counterCondition6;
    int counterCondition7;
    int counterCondition8;
    int counterCondition9;
    int counterCondition10;
};

void LoadTypedefSLdrAdvancedCounter(SLdrAdvancedCounter&, CInputStream&);

#endif // _SLDRADVANCEDCOUNTER
