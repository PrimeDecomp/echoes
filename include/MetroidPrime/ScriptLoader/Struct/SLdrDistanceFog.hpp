#ifndef _SLDRDISTANCEFOG
#define _SLDRDISTANCEFOG

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrVector2f.hpp"

struct SLdrDistanceFog {
    SLdrDistanceFog();
    ~SLdrDistanceFog();

    SLdrEditorProperties editorProperties;
    int mode;
    CColor color;
    SLdrVector2f nearFarPlane;
    float colorRate;
    SLdrVector2f distanceRate;
    bool forceSettings;
};

void LoadTypedefSLdrDistanceFog(SLdrDistanceFog&, CInputStream&);

#endif // _SLDRDISTANCEFOG
