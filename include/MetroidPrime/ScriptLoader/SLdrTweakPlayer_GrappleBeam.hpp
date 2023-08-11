#ifndef _SLDRTWEAKPLAYER_GRAPPLEBEAM
#define _SLDRTWEAKPLAYER_GRAPPLEBEAM

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakPlayer_GrappleBeam {
    SLdrTweakPlayer_GrappleBeam();
    ~SLdrTweakPlayer_GrappleBeam();

    float travelSpeed;
    float xWaveAmplitude;
    float zWaveAmplitude;
    float anglePhaseDelta;
};

void LoadTypedefSLdrTweakPlayer_GrappleBeam(SLdrTweakPlayer_GrappleBeam&, CInputStream&);

#endif // _SLDRTWEAKPLAYER_GRAPPLEBEAM
