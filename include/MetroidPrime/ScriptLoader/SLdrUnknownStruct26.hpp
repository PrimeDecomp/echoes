#ifndef _SLDRUNKNOWNSTRUCT26
#define _SLDRUNKNOWNSTRUCT26

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPlasmaBeamInfo.hpp"

struct SLdrUnknownStruct26 {
    SLdrUnknownStruct26();
    ~SLdrUnknownStruct26();

    CAssetId effect;
    int portalOpenSound;
    SLdrDamageInfo projectileDamage;
    SLdrPlasmaBeamInfo beamInfo;
};

void LoadTypedefSLdrUnknownStruct26(SLdrUnknownStruct26&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT26
