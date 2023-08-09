#ifndef _SLDRTWEAKPLAYERGUN_POSITION
#define _SLDRTWEAKPLAYERGUN_POSITION

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakPlayerGun_Position {
    SLdrTweakPlayerGun_Position();
    ~SLdrTweakPlayerGun_Position();

    float unknown;
    float x;
    float y;
    float z;
};

void LoadTypedefSLdrTweakPlayerGun_Position(SLdrTweakPlayerGun_Position&, CInputStream&);

#endif // _SLDRTWEAKPLAYERGUN_POSITION
