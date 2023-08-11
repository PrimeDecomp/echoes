#ifndef _SLDRUNKNOWNSTRUCT32
#define _SLDRUNKNOWNSTRUCT32

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrIngSpaceJumpGuardianStruct.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPlasmaBeamInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrShockWaveInfo.hpp"

struct SLdrUnknownStruct32 {
    SLdrUnknownStruct32();
    ~SLdrUnknownStruct32();

    CAssetId ingSpotBlobEffect;
    int sound;
    SLdrIngSpaceJumpGuardianStruct ingSpaceJumpGuardianStruct_0x5e1d1931;
    SLdrIngSpaceJumpGuardianStruct ingSpaceJumpGuardianStruct_0x6b08e2e5;
    SLdrIngSpaceJumpGuardianStruct ingSpaceJumpGuardianStruct_0xf223aa76;
    SLdrIngSpaceJumpGuardianStruct ingSpaceJumpGuardianStruct_0xd0db5f7a;
    CColor lightColor;
    float lightAttenuation;
    CAssetId miniPortalEffect;
    int soundMiniPortal;
    SLdrDamageInfo miniPortalProjectileDamage;
    SLdrPlasmaBeamInfo miniPortalBeamInfo;
    SLdrShockWaveInfo shockWaveInfo;
};

void LoadTypedefSLdrUnknownStruct32(SLdrUnknownStruct32&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT32
