#ifndef _SLDRFOGVOLUME
#define _SLDRFOGVOLUME

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrFogVolume {
    SLdrFogVolume();
    ~SLdrFogVolume();

    SLdrEditorProperties editorProperties;
    float fogBobHeight;
    float fogBobFreq;
    CColor fogColor;
};

void LoadTypedefSLdrFogVolume(SLdrFogVolume&, CInputStream&);

#endif // _SLDRFOGVOLUME
