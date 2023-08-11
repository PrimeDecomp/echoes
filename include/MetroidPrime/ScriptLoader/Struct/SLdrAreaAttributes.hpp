#ifndef _SLDRAREAATTRIBUTES
#define _SLDRAREAATTRIBUTES

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrAreaAttributes {
    SLdrAreaAttributes();
    ~SLdrAreaAttributes();

    SLdrEditorProperties editorProperties;
    bool needSky;
    bool darkWorld;
    int environmentEffects;
    int environmentGroupSound;
    float density;
    float normalLighting;
    CAssetId overrideSky;
    int phazonDamage;
};

void LoadTypedefSLdrAreaAttributes(SLdrAreaAttributes&, CInputStream&);

#endif // _SLDRAREAATTRIBUTES
