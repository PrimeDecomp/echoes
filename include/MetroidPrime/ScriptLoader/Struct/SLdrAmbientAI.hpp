#ifndef _SLDRAMBIENTAI
#define _SLDRAMBIENTAI

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrHealthInfo.hpp"

struct SLdrAmbientAI {
    SLdrAmbientAI();
    ~SLdrAmbientAI();

    SLdrEditorProperties editor_properties;
    CVector3f collision_box;
    CVector3f collision_offset;
    float mass;
    SLdrHealthInfo health;
    SLdrDamageVulnerability vulnerability;
    SLdrAnimationParameters animation_information;
    SLdrActorParameters actor_information;
    float detect_radius;
    float explode_radius;
    int unknown_0xbfe017de;
    int unknown_0xed5f16ac;
};

void LoadTypedefSLdrAmbientAI(SLdrAmbientAI&, CInputStream&);

#endif // _SLDRAMBIENTAI
