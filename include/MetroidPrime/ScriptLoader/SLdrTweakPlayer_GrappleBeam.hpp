#ifndef _SLDRTWEAKPLAYER_GRAPPLEBEAM
#define _SLDRTWEAKPLAYER_GRAPPLEBEAM

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakPlayer_GrappleBeam {
    SLdrTweakPlayer_GrappleBeam();
    ~SLdrTweakPlayer_GrappleBeam();

    float travel_speed;
    float x_wave_amplitude;
    float z_wave_amplitude;
    float angle_phase_delta;
};

void LoadTypedefSLdrTweakPlayer_GrappleBeam(SLdrTweakPlayer_GrappleBeam&, CInputStream&);

#endif // _SLDRTWEAKPLAYER_GRAPPLEBEAM
