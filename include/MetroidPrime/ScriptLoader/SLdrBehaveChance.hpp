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
    float lurk_time;
    float charge_attack;
    int num_bolts;
};

void LoadTypedefSLdrBehaveChance(SLdrBehaveChance&, CInputStream&);

#endif // _SLDRBEHAVECHANCE
