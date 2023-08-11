#ifndef _SLDRBEHAVECHANCE
#define _SLDRBEHAVECHANCE

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrBehaveChance {
    SLdrBehaveChance();
    ~SLdrBehaveChance();

    float lurk;
    float unknown;
    float attack;
    float move;
    float lurkTime;
    float chargeAttack;
    int numBolts;
};

void LoadTypedefSLdrBehaveChance(SLdrBehaveChance&, CInputStream&);

#endif // _SLDRBEHAVECHANCE
