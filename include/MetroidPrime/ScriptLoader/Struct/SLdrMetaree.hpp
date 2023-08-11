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

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    SLdrDamageInfo radiusDamage;
    float dropHeight;
    CVector3f unknown;
    float attackSpeed;
    float dropDelay;
    float haltDelay;
    float launchSpeed;
    SLdrAudioPlaybackParms turnSound;
};

void LoadTypedefSLdrMetaree(SLdrMetaree&, CInputStream&);

#endif // _SLDRMETAREE
