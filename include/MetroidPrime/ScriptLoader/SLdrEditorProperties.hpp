#ifndef _SLDREDITORPROPERTIES
#define _SLDREDITORPROPERTIES

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTransform.hpp"
#include "rstl/string.hpp"

struct SLdrEditorProperties {
    SLdrEditorProperties();
    ~SLdrEditorProperties();

    rstl::string name;
    SLdrTransform transform;
    bool active;
    int unknown;
};

const CEntityInfo& EntityInfoWithEditorProperties(const CEntityInfo&, const SLdrEditorProperties&);
void LoadTypedefSLdrEditorProperties(SLdrEditorProperties&, CInputStream&);

#endif // _SLDREDITORPROPERTIES
