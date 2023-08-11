#ifndef _SLDRECHOPARAMETERS
#define _SLDRECHOPARAMETERS

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrEchoParameters {
    SLdrEchoParameters();
    ~SLdrEchoParameters();

    bool isEchoEmitter;
    bool onlyEmitDamage;
    int numSoundWaves;
    float spaceBetweenWaves;
    float waveLineSize;
    float forcedMinimumVis;
};

void LoadTypedefSLdrEchoParameters(SLdrEchoParameters&, CInputStream&);

#endif // _SLDRECHOPARAMETERS
