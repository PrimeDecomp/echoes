#ifndef _SLDREMPERORINGSTAGE3STRUCTA
#define _SLDREMPERORINGSTAGE3STRUCTA

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPlasmaBeamInfo.hpp"

struct SLdrEmperorIngStage3StructA {
    SLdrEmperorIngStage3StructA();
    ~SLdrEmperorIngStage3StructA();

    SLdrPlasmaBeamInfo beamInfo;
    SLdrDamageInfo damage;
    int beamSound;
};

void LoadTypedefSLdrEmperorIngStage3StructA(SLdrEmperorIngStage3StructA&, CInputStream&);

#endif // _SLDREMPERORINGSTAGE3STRUCTA
