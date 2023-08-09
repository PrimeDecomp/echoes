#ifndef _SLDRMETAREE
#define _SLDRMETAREE

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAudioPlaybackParms.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrMetaree {
    SLdrMetaree();
    ~SLdrMetaree();

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    SLdrDamageInfo radius_damage;
    float drop_height;
    CVector3f unknown;
    float attack_speed;
    float drop_delay;
    float halt_delay;
    float launch_speed;
    SLdrAudioPlaybackParms turn_sound;
};

void LoadTypedefSLdrMetaree(SLdrMetaree&, CInputStream&);

#endif // _SLDRMETAREE
