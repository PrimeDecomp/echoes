#ifndef _SLDRRANDOMRELAY
#define _SLDRRANDOMRELAY

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrRandomRelay {
    SLdrRandomRelay();
    ~SLdrRandomRelay();

    SLdrEditorProperties editor_properties;
    int count;
    int random_adjust;
    bool percent_count;
    bool is_random_chance;
};

void LoadTypedefSLdrRandomRelay(SLdrRandomRelay&, CInputStream&);

#endif // _SLDRRANDOMRELAY
