#ifndef _SLDRFISHCLOUDMODIFIER
#define _SLDRFISHCLOUDMODIFIER

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrFishCloudModifier {
    SLdrFishCloudModifier();
    ~SLdrFishCloudModifier();

    SLdrEditorProperties editor_properties;
    bool active;
    bool unknown;
    bool rotate;
    float influence_distance;
    float influence_priority;
};

void LoadTypedefSLdrFishCloudModifier(SLdrFishCloudModifier&, CInputStream&);

#endif // _SLDRFISHCLOUDMODIFIER
