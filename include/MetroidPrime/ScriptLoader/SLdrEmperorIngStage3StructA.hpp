#ifndef _SLDREMPERORINGSTAGE3STRUCTA
#define _SLDREMPERORINGSTAGE3STRUCTA

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPlasmaBeamInfo.hpp"

struct SLdrEmperorIngStage3StructA {
    SLdrEmperorIngStage3StructA();
    ~SLdrEmperorIngStage3StructA();

    SLdrPlasmaBeamInfo beam_info;
    SLdrDamageInfo damage;
    int beam_sound;
};

void LoadTypedefSLdrEmperorIngStage3StructA(SLdrEmperorIngStage3StructA&, CInputStream&);

#endif // _SLDREMPERORINGSTAGE3STRUCTA
