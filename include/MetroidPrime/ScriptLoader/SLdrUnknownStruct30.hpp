#ifndef _SLDRUNKNOWNSTRUCT30
#define _SLDRUNKNOWNSTRUCT30

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrHealthInfo.hpp"

struct SLdrUnknownStruct30 {
    SLdrUnknownStruct30();
    ~SLdrUnknownStruct30();

    CAssetId state_machine;
    SLdrHealthInfo health;
    float puddle_speed;
    CAssetId blob_effect;
    CAssetId part_0xe8a6e174;
    CAssetId part_0x1ab2b090;
    CAssetId puddle_death;
    int sound_ing_spot_idle;
    int sound_ing_spot_move;
    int sound_0xb392943a;
    int sound_0x24ecc1e9;
    int sound_ing_spot_death;
    SLdrDamageVulnerability vulnerability;
};

void LoadTypedefSLdrUnknownStruct30(SLdrUnknownStruct30&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT30
