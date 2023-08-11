#ifndef _SLDRTWEAKPLAYER_COLLISION
#define _SLDRTWEAKPLAYER_COLLISION

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakPlayer_Collision {
    SLdrTweakPlayer_Collision();
    ~SLdrTweakPlayer_Collision();

    float playerHeight;
    float playerRadius;
    float stepUpHeight;
    float stepDownHeight;
    float ballRadius;
};

void LoadTypedefSLdrTweakPlayer_Collision(SLdrTweakPlayer_Collision&, CInputStream&);

#endif // _SLDRTWEAKPLAYER_COLLISION
