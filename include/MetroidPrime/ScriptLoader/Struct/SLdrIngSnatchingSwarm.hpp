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

    SLdrEditorProperties editor_properties;
    CAssetId state_machine;
    CAssetId swarm_particle_system;
    float unknown_0x7cae2ed5;
    CAssetId part_0x35a88fa1;
    float unknown_0xf65e7ec5;
    float lifetime;
    float max_linear_speed;
    float max_linear_acceleration;
    float max_turn_speed;
    bool unknown_0xdffdf5a2;
    bool ignore_player;
    float unknown_0xe6b57a25;
    float exit_portal_distance;
    float unknown_0x2de5a19a;
    float unknown_0x4e79f717;
    float unknown_0xe8e0b5a6;
    float begin_snatching_range;
    CAssetId part_0x2d2afc26;
    SLdrDamageInfo impact_damage;
    int sound_impact;
    int sound_idle;
    int sound_move;
    float health;
    SLdrDamageVulnerability swarm_vulnerability;
};

void LoadTypedefSLdrIngSnatchingSwarm(SLdrIngSnatchingSwarm&, CInputStream&);

#endif // _SLDRINGSNATCHINGSWARM
