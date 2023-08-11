#ifndef _SLDRUNKNOWNSTRUCT30
#define _SLDRUNKNOWNSTRUCT30

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrHealthInfo.hpp"

struct SLdrUnknownStruct30 {
    SLdrUnknownStruct30();
    ~SLdrUnknownStruct30();

    CAssetId stateMachine;
    SLdrHealthInfo health;
    float puddleSpeed;
    CAssetId blobEffect;
    CAssetId part_0xe8a6e174;
    CAssetId part_0x1ab2b090;
    CAssetId puddleDeath;
    int soundIngSpotIdle;
    int soundIngSpotMove;
    int sound_0xb392943a;
    int sound_0x24ecc1e9;
    int soundIngSpotDeath;
    SLdrDamageVulnerability vulnerability;
};

void LoadTypedefSLdrUnknownStruct30(SLdrUnknownStruct30&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT30
