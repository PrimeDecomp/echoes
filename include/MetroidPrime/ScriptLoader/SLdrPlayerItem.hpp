#ifndef _SLDRPLAYERITEM
#define _SLDRPLAYERITEM

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrPlayerItem {
    SLdrPlayerItem();
    ~SLdrPlayerItem();

    int value;
};

void LoadTypedefSLdrPlayerItem(SLdrPlayerItem&, CInputStream&);

#endif // _SLDRPLAYERITEM
