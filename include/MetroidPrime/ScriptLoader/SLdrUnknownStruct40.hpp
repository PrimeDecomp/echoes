#ifndef _SLDRUNKNOWNSTRUCT40
#define _SLDRUNKNOWNSTRUCT40

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"

struct SLdrUnknownStruct40 {
    SLdrUnknownStruct40();
    ~SLdrUnknownStruct40();

    float unknown_0xbed8a4ba;
    float unknown_0xc2b98161;
    float unknown_0x5fb66017;
    float unknown_0xbab42316;
    CAssetId part_0x8f06342a;
    int sound_0xd8b11129;
    int sound_0xe99e5316;
    SLdrDamageInfo damageInfo;
    CAssetId part_0x686489fd;
};

void LoadTypedefSLdrUnknownStruct40(SLdrUnknownStruct40&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT40
