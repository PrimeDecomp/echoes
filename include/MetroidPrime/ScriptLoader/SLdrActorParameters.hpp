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
    CAssetId darkModel;
    CAssetId darkSkin;
    CAssetId echoModel;
    CAssetId echoSkin;
    bool useGlobalRenderTime;
    float fadeInTime;
    float fadeOutTime;
    SLdrVisorParameters visor;
    bool unknown_0xcd4c81a1;
    bool forceRenderUnsorted;
    bool takesProjectedShadow;
    bool unknown_0xf07981e8;
    bool unknown_0x6df33845;
    int maxVolume;
    int maxEchoVolume;
};

void LoadTypedefSLdrActorParameters(SLdrActorParameters&, CInputStream&);

#endif // _SLDRACTORPARAMETERS
