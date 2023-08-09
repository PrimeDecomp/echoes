#ifndef _SLDRINVENTORYSLOT
#define _SLDRINVENTORYSLOT

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrInventorySlot {
    SLdrInventorySlot();
    ~SLdrInventorySlot();

    int value;
};

void LoadTypedefSLdrInventorySlot(SLdrInventorySlot&, CInputStream&);

#endif // _SLDRINVENTORYSLOT
