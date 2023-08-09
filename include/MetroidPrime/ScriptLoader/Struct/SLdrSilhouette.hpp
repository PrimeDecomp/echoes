#ifndef _SLDRSILHOUETTE
#define _SLDRSILHOUETTE

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrSilhouette {
    SLdrSilhouette();
    ~SLdrSilhouette();

    SLdrEditorProperties editor_properties;
    float unknown;
    CColor silhouette_color;
    float fade_in_time;
    float fade_out_time;
};

void LoadTypedefSLdrSilhouette(SLdrSilhouette&, CInputStream&);

#endif // _SLDRSILHOUETTE
