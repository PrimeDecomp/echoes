#ifndef _SLDRDISTANCEFOG
#define _SLDRDISTANCEFOG

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrVector2f.hpp"

struct SLdrDistanceFog {
    SLdrDistanceFog();
    ~SLdrDistanceFog();

    SLdrEditorProperties editor_properties;
    int mode;
    CColor color;
    SLdrVector2f near_far_plane;
    float color_rate;
    SLdrVector2f distance_rate;
    bool force_settings;
};

void LoadTypedefSLdrDistanceFog(SLdrDistanceFog&, CInputStream&);

#endif // _SLDRDISTANCEFOG
