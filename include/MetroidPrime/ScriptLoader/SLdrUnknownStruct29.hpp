#ifndef _SLDRUNKNOWNSTRUCT29
#define _SLDRUNKNOWNSTRUCT29

#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrIngBoostBallGuardianStruct.hpp"

struct SLdrUnknownStruct29 {
    SLdrUnknownStruct29();
    ~SLdrUnknownStruct29();

    CVector3f boostBallScale;
    float boostBallMass;
    float unknown_0xbea96fb7;
    SLdrSpline boostBallSpeed;
    SLdrDamageInfo damageInfo_0x0e1a78bd;
    SLdrDamageInfo damageInfo_0x19c3d263;
    float unknown_0xd8047cba;
    float unknown_0x7b21e31a;
    CAssetId boostBallModel;
    CAssetId part_0x15534429;
    CAssetId boostBallShieldEffect;
    CAssetId spsc;
    int soundBounce;
    int soundIntoBall;
    int soundOutofBall;
    int sound;
    int soundBoost;
    int soundRolling;
    SLdrDamageVulnerability boostBallVulnerability;
    int unknown_0xee69b993;
    int unknown_0x4b2de673;
    float searchConeAngle;
    float unknown_0xb0e85d53;
    SLdrDamageInfo damageInfo_0x5616d5f1;
    SLdrDamageInfo damageInfo_0xed685533;
    CAssetId part_0xd771ec43;
    CAssetId part_0x2009a977;
    CAssetId part_0x62ab33a2;
    SLdrIngBoostBallGuardianStruct ingBoostBallGuardianStruct_0xbab98497;
    SLdrIngBoostBallGuardianStruct ingBoostBallGuardianStruct_0xfe18a18f;
    SLdrIngBoostBallGuardianStruct ingBoostBallGuardianStruct_0xc2784287;
};

void LoadTypedefSLdrUnknownStruct29(SLdrUnknownStruct29&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT29
