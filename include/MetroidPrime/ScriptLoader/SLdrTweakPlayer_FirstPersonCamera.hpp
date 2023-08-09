#ifndef _SLDRTWEAKPLAYER_FIRSTPERSONCAMERA
#define _SLDRTWEAKPLAYER_FIRSTPERSONCAMERA

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakPlayer_FirstPersonCamera {
    SLdrTweakPlayer_FirstPersonCamera();
    ~SLdrTweakPlayer_FirstPersonCamera();

    float unknown_0xba5eb7f5;
    float camera_elevation;
    float unknown_0xb400ebd6;
    float unknown_0xfd26b7b9;
    float unknown_0x97b14dc6;
    float unknown_0xeb59925a;
    float unknown_0xa1d73380;
    float unknown_0xc8e8344a;
    float unknown_0xd40c480e;
    CVector3f unknown_0x7960c3a0;
};

void LoadTypedefSLdrTweakPlayer_FirstPersonCamera(SLdrTweakPlayer_FirstPersonCamera&, CInputStream&);

#endif // _SLDRTWEAKPLAYER_FIRSTPERSONCAMERA
