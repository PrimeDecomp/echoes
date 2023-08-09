#ifndef _SLDRCAMERAHINTSTRUCTC
#define _SLDRCAMERAHINTSTRUCTC

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrCameraHintStructC {
    SLdrCameraHintStructC();
    ~SLdrCameraHintStructC();

    bool override;
    float angle;
};

void LoadTypedefSLdrCameraHintStructC(SLdrCameraHintStructC&, CInputStream&);

#endif // _SLDRCAMERAHINTSTRUCTC
