#ifndef _SLDRFISHCLOUDMODIFIER
#define _SLDRFISHCLOUDMODIFIER

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrFishCloudModifier {
    SLdrFishCloudModifier();
    ~SLdrFishCloudModifier();

    SLdrEditorProperties editorProperties;
    bool active;
    bool unknown;
    bool rotate;
    float influenceDistance;
    float influencePriority;
};

void LoadTypedefSLdrFishCloudModifier(SLdrFishCloudModifier&, CInputStream&);

#endif // _SLDRFISHCLOUDMODIFIER
