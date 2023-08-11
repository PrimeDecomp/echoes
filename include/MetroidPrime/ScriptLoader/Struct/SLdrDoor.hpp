#ifndef _SLDRDOOR
#define _SLDRDOOR

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrHealthInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrScannableParameters.hpp"

struct SLdrDoor {
    SLdrDoor();
    ~SLdrDoor();

    SLdrEditorProperties editorProperties;
    CVector3f collisionBox;
    CVector3f collisionOffset;
    SLdrHealthInfo health;
    SLdrDamageVulnerability vulnerability;
    SLdrAnimationParameters animationInformation;
    CAssetId shellModel;
    CAssetId blueShellModel;
    CColor shellColor;
    CAssetId burnTexture;
    SLdrActorParameters actorInformation;
    CVector3f orbitOffset;
    bool isOpen;
    bool isLocked;
    float openAnimationTime;
    float closeAnimationTime;
    float closeDelay;
    float shieldFadeOutTime;
    float shieldFadeInTime;
    bool morphBallTunnel;
    bool horizontal;
    SLdrScannableParameters altScannable;
};

void LoadTypedefSLdrDoor(SLdrDoor&, CInputStream&);

#endif // _SLDRDOOR
