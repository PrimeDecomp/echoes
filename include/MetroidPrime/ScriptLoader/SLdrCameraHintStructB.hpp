#ifndef _SLDRCAMERAHINTSTRUCTB
#define _SLDRCAMERAHINTSTRUCTB

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrCameraHintStructB {
    SLdrCameraHintStructB();
    ~SLdrCameraHintStructB();

    bool override;
    float distance;
};

void LoadTypedefSLdrCameraHintStructB(SLdrCameraHintStructB&, CInputStream&);

#endif // _SLDRCAMERAHINTSTRUCTB
