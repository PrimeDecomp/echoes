#ifndef _SLDRPLAYERSTATECHANGE
#define _SLDRPLAYERSTATECHANGE

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrPlayerStateChange {
    SLdrPlayerStateChange();
    ~SLdrPlayerStateChange();

    SLdrEditorProperties editorProperties;
    int itemToChange;
    int capacityIncrease;
    int amount;
    int command;
    int commandAction;
};

void LoadTypedefSLdrPlayerStateChange(SLdrPlayerStateChange&, CInputStream&);

#endif // _SLDRPLAYERSTATECHANGE
