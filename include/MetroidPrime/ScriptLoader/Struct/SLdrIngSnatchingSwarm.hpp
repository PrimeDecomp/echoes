#ifndef _SLDRINGSNATCHINGSWARM
#define _SLDRINGSNATCHINGSWARM

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrIngSnatchingSwarm {
    SLdrIngSnatchingSwarm();
    ~SLdrIngSnatchingSwarm();

    SLdrEditorProperties editorProperties;
    CAssetId stateMachine;
    CAssetId swarmParticleSystem;
    float unknown_0x7cae2ed5;
    CAssetId part_0x35a88fa1;
    float unknown_0xf65e7ec5;
    float lifetime;
    float maxLinearSpeed;
    float maxLinearAcceleration;
    float maxTurnSpeed;
    bool unknown_0xdffdf5a2;
    bool ignorePlayer;
    float unknown_0xe6b57a25;
    float exitPortalDistance;
    float unknown_0x2de5a19a;
    float unknown_0x4e79f717;
    float unknown_0xe8e0b5a6;
    float beginSnatchingRange;
    CAssetId part_0x2d2afc26;
    SLdrDamageInfo impactDamage;
    int soundImpact;
    int soundIdle;
    int soundMove;
    float health;
    SLdrDamageVulnerability swarmVulnerability;
};

void LoadTypedefSLdrIngSnatchingSwarm(SLdrIngSnatchingSwarm&, CInputStream&);

#endif // _SLDRINGSNATCHINGSWARM
