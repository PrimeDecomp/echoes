#ifndef _SLDRDESTRUCTIBLEBARRIER
#define _SLDRDESTRUCTIBLEBARRIER

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrHealthInfo.hpp"

struct SLdrDestructibleBarrier {
    SLdrDestructibleBarrier();
    ~SLdrDestructibleBarrier();

    SLdrEditorProperties editor_properties;
    int unknown_0xcd4f7e71;
    int unknown_0xa7f551f7;
    int unknown_0x609c6240;
    CVector3f chunk_size;
    CAssetId left_model;
    CAssetId center_model;
    CAssetId right_model;
    CAssetId unknown_0x396660b4;
    CAssetId unknown_0x48e25884;
    CAssetId base_model;
    CAssetId unknown_0x1eb90d06;
    int unknown_0x9d852dfe;
    CAssetId unknown_0x982d7fa8;
    int unknown_0x2e11003d;
    CAssetId unknown_0x5371ac0d;
    int unknown_0x409d1b7c;
    CAssetId unknown_0x4e749cb5;
    int unknown_0x92485dfa;
    int unknown_0x6e4a9d27;
    int unknown_0xbc2381a6;
    int unknown_0x6575a3d5;
    int unknown_0xc91b0946;
    int unknown_0x4b2d5a37;
    float unknown_0x605847b9;
    float unknown_0xcd9c67fe;
    float unknown_0x0af428b4;
    bool unknown_0x4d3109e3;
    SLdrHealthInfo health;
    SLdrDamageVulnerability vulnerability;
    SLdrActorParameters actor_information;
};

void LoadTypedefSLdrDestructibleBarrier(SLdrDestructibleBarrier&, CInputStream&);

#endif // _SLDRDESTRUCTIBLEBARRIER
