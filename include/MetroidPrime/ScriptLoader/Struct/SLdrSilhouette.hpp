#ifndef _SLDRSILHOUETTE
#define _SLDRSILHOUETTE

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrSilhouette {
    SLdrSilhouette();
    ~SLdrSilhouette();

    SLdrEditorProperties editorProperties;
    float unknown;
    CColor silhouetteColor;
    float fadeInTime;
    float fadeOutTime;
};

void LoadTypedefSLdrSilhouette(SLdrSilhouette&, CInputStream&);

#endif // _SLDRSILHOUETTE
