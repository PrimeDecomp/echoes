#ifndef _SLDRTWEAKPLAYER_COLLISION
#define _SLDRTWEAKPLAYER_COLLISION

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakPlayer_Collision {
    SLdrTweakPlayer_Collision();
    ~SLdrTweakPlayer_Collision();

    float player_height;
    float player_radius;
    float step_up_height;
    float step_down_height;
    float ball_radius;
};

void LoadTypedefSLdrTweakPlayer_Collision(SLdrTweakPlayer_Collision&, CInputStream&);

#endif // _SLDRTWEAKPLAYER_COLLISION
