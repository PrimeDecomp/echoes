#ifndef _SLDRPLAYERCONTROLLER
#define _SLDRPLAYERCONTROLLER

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "rstl/string.hpp"

struct SLdrPlayerController {
    SLdrPlayerController();
    ~SLdrPlayerController();

    SLdrEditorProperties editorProperties;
    int unknown_0xe71de331;
    CAssetId model;
    SLdrAnimationParameters animationInformation;
    SLdrActorParameters actorInformation;
    int proxyType;
    CVector3f playerOffset;
    int initialState;
    int playerVisor;
    float unknown_0xf09c2b4b;
    float unknown_0x760859e5;
    float unknown_0xbd548a40;
    CVector3f rotationForType3;
    rstl::string unknown_0x70bc90a6;
};

void LoadTypedefSLdrPlayerController(SLdrPlayerController&, CInputStream&);

#endif // _SLDRPLAYERCONTROLLER
