#ifndef _SLDRTWEAKPLAYER_SCANVISOR
#define _SLDRTWEAKPLAYER_SCANVISOR

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakPlayer_ScanVisor {
    SLdrTweakPlayer_ScanVisor();
    ~SLdrTweakPlayer_ScanVisor();

    float scanDistance;
    bool scanRetention;
    bool scanFreezesGame;
    bool scanLineOfSight;
    float scanMaxTargetDistance;
    float scanMaxLockDistance;
    float scanCameraSpeed;
};

void LoadTypedefSLdrTweakPlayer_ScanVisor(SLdrTweakPlayer_ScanVisor&, CInputStream&);

#endif // _SLDRTWEAKPLAYER_SCANVISOR
