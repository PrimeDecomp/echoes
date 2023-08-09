#ifndef _SLDRFOGVOLUME
#define _SLDRFOGVOLUME

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrFogVolume {
    SLdrFogVolume();
    ~SLdrFogVolume();

    SLdrEditorProperties editor_properties;
    float fog_bob_height;
    float fog_bob_freq;
    CColor fog_color;
};

void LoadTypedefSLdrFogVolume(SLdrFogVolume&, CInputStream&);

#endif // _SLDRFOGVOLUME
