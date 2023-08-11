#ifndef _SLDRGUNTURRETTOP
#define _SLDRGUNTURRETTOP

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrGunTurretTop {
    SLdrGunTurretTop();
    ~SLdrGunTurretTop();

    SLdrEditorProperties editorProperties;
    float powerUpTime;
    float powerDownTime;
    CAssetId part_0xbf87e353;
    CAssetId part_0xaf6e671a;
    int alwaysFf_0x67c8a8f4;
    int alwaysFf_0x68d8b844;
    CColor lightColor;
    int sound_0xe4aeeba4;
    int sound_0x5d9ed447;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
};

void LoadTypedefSLdrGunTurretTop(SLdrGunTurretTop&, CInputStream&);

#endif // _SLDRGUNTURRETTOP
