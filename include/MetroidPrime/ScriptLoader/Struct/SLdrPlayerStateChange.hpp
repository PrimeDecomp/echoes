#ifndef _SLDRPLAYERSTATECHANGE
#define _SLDRPLAYERSTATECHANGE

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrPlayerStateChange {
    SLdrPlayerStateChange();
    ~SLdrPlayerStateChange();

    SLdrEditorProperties editor_properties;
    int item_to_change;
    int capacity_increase;
    int amount;
    int command;
    int command_action;
};

void LoadTypedefSLdrPlayerStateChange(SLdrPlayerStateChange&, CInputStream&);

#endif // _SLDRPLAYERSTATECHANGE
