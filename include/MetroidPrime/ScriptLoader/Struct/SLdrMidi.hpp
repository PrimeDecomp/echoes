#ifndef _SLDRMIDI
#define _SLDRMIDI

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrMidi {
    SLdrMidi();
    ~SLdrMidi();

    SLdrEditorProperties editor_properties;
    CAssetId song_file;
    float fade_in_time;
    float fade_out_time;
    int volume;
};

void LoadTypedefSLdrMidi(SLdrMidi&, CInputStream&);

#endif // _SLDRMIDI
