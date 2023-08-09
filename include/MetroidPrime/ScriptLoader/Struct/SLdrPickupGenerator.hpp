#ifndef _SLDRPICKUPGENERATOR
#define _SLDRPICKUPGENERATOR

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrPickupGenerator {
    SLdrPickupGenerator();
    ~SLdrPickupGenerator();

    SLdrEditorProperties editor_properties;
    CVector3f offset;
    bool offset_is_local_space;
    CAssetId rules;
};

void LoadTypedefSLdrPickupGenerator(SLdrPickupGenerator&, CInputStream&);

#endif // _SLDRPICKUPGENERATOR
