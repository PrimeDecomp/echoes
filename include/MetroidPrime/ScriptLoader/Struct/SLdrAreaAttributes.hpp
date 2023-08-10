#ifndef _SLDRAREAATTRIBUTES
#define _SLDRAREAATTRIBUTES

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrAreaAttributes {
    SLdrAreaAttributes();
    ~SLdrAreaAttributes();

    SLdrEditorProperties editor_properties;
    bool need_sky;
    bool dark_world;
    int environment_effects;
    int environment_group_sound;
    float density;
    float normal_lighting;
    CAssetId override_sky;
    int phazon_damage;
};

void LoadTypedefSLdrAreaAttributes(SLdrAreaAttributes&, CInputStream&);

#endif // _SLDRAREAATTRIBUTES
