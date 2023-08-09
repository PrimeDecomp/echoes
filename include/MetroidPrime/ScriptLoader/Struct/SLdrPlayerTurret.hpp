#ifndef _SLDRPLAYERTURRET
#define _SLDRPLAYERTURRET

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrPlayerTurret {
    SLdrPlayerTurret();
    ~SLdrPlayerTurret();

    SLdrEditorProperties editor_properties;
    int flags_player_turret;
    float unknown_0x17cd8b2a;
    float unknown_0x1473dad2;
    float unknown_0x3650ce75;
    float unknown_0x78520e6e;
    float damage_angle;
    float horiz_speed;
    float vert_speed;
    float fire_rate;
    SLdrDamageInfo weapon_damage;
    CAssetId weapon_effect;
    CAssetId wpsc;
    int unknown_0xe7234f72;
    int unknown_0x3e2f7afb;
    int unknown_0x7cabd1f1;
    int unknown_0x7ef976eb;
    int unknown_0x035459fd;
};

void LoadTypedefSLdrPlayerTurret(SLdrPlayerTurret&, CInputStream&);

#endif // _SLDRPLAYERTURRET
