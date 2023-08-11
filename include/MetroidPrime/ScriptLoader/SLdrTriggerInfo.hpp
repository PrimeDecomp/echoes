#ifndef _SLDRTRIGGERINFO
#define _SLDRTRIGGERINFO

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"

struct SLdrTriggerInfo {
    SLdrTriggerInfo();
    ~SLdrTriggerInfo();

    SLdrDamageInfo damage;
    CVector3f forceField;
    int flagsTrigger;
};

void LoadTypedefSLdrTriggerInfo(SLdrTriggerInfo&, CInputStream&);

#endif // _SLDRTRIGGERINFO
