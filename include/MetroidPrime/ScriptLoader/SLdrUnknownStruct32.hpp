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

    CAssetId ing_spot_blob_effect;
    int sound;
    SLdrIngSpaceJumpGuardianStruct ing_space_jump_guardian_struct_0x5e1d1931;
    SLdrIngSpaceJumpGuardianStruct ing_space_jump_guardian_struct_0x6b08e2e5;
    SLdrIngSpaceJumpGuardianStruct ing_space_jump_guardian_struct_0xf223aa76;
    SLdrIngSpaceJumpGuardianStruct ing_space_jump_guardian_struct_0xd0db5f7a;
    CColor light_color;
    float light_attenuation;
    CAssetId mini_portal_effect;
    int sound_mini_portal;
    SLdrDamageInfo mini_portal_projectile_damage;
    SLdrPlasmaBeamInfo mini_portal_beam_info;
    SLdrShockWaveInfo shock_wave_info;
};

void LoadTypedefSLdrUnknownStruct32(SLdrUnknownStruct32&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT32
