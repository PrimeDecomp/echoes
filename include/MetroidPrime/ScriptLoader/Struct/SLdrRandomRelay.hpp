#ifndef _SLDRRANDOMRELAY
#define _SLDRRANDOMRELAY

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrRandomRelay {
    SLdrRandomRelay();
    ~SLdrRandomRelay();

    SLdrEditorProperties editorProperties;
    int count;
    int randomAdjust;
    bool percentCount;
    bool isRandomChance;
};

void LoadTypedefSLdrRandomRelay(SLdrRandomRelay&, CInputStream&);

#endif // _SLDRRANDOMRELAY
