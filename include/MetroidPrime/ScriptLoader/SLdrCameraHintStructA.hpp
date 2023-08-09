#ifndef _SLDRCAMERAHINTSTRUCTA
#define _SLDRCAMERAHINTSTRUCTA

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrCameraHintStructA {
    SLdrCameraHintStructA();
    ~SLdrCameraHintStructA();

    int type;
};

void LoadTypedefSLdrCameraHintStructA(SLdrCameraHintStructA&, CInputStream&);

#endif // _SLDRCAMERAHINTSTRUCTA
