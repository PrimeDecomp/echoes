#ifndef _SLDRACTORPARAMETERS
#define _SLDRACTORPARAMETERS

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrLightParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrScannableParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrVisorParameters.hpp"

struct SLdrActorParameters {
    SLdrActorParameters();
    ~SLdrActorParameters();

    SLdrLightParameters lighting;
    SLdrScannableParameters scannable;
    CAssetId dark_model;
    CAssetId dark_skin;
    CAssetId echo_model;
    CAssetId echo_skin;
    bool use_global_render_time;
    float fade_in_time;
    float fade_out_time;
    SLdrVisorParameters visor;
    bool unknown_0xcd4c81a1;
    bool force_render_unsorted;
    bool takes_projected_shadow;
    bool unknown_0xf07981e8;
    bool unknown_0x6df33845;
    int max_volume;
    int max_echo_volume;
};

void LoadTypedefSLdrActorParameters(SLdrActorParameters&, CInputStream&);

#endif // _SLDRACTORPARAMETERS
