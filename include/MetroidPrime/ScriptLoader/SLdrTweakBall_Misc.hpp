#ifndef _SLDRTWEAKBALL_MISC
#define _SLDRTWEAKBALL_MISC

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakBall_Misc {
    SLdrTweakBall_Misc();
    ~SLdrTweakBall_Misc();

    float unknown_0x13cfde23;
    float unknown_0xf3499713;
    float unknown_0x895a47fb;
    float ball_touch_radius;
    float dark_world_light_radius;
    float unknown_0xad662ae9;
    float unknown_0xb0575d4e;
};

void LoadTypedefSLdrTweakBall_Misc(SLdrTweakBall_Misc&, CInputStream&);

#endif // _SLDRTWEAKBALL_MISC
