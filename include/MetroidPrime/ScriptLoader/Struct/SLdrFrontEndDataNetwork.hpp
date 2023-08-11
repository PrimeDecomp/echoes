#ifndef _SLDRFRONTENDDATANETWORK
#define _SLDRFRONTENDDATANETWORK

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrFrontEndDataNetwork {
    SLdrFrontEndDataNetwork();
    ~SLdrFrontEndDataNetwork();

    SLdrEditorProperties editorProperties;
    bool isRoot;
    bool unknown_0x77f59f4a;
    bool unknown_0x29c0cb7f;
    bool canBeSelected;
    bool isProxy;
    bool isLocked;
    bool unknown_0x8b8fa0fe;
    bool unknown_0xd0f2d612;
    float connectionRadius;
    CAssetId hotDotTexture;
    CAssetId txtr_0x547fffc3;
    CAssetId txtr_0xcdaaba00;
    CColor selectedColor;
    CColor unselectedMinColor;
    CColor unselectedMaxColor;
    CColor disabledColor;
    int rotationSound;
    spline transitionShrinkSpline;
    float transitionShrinkTime;
    spline transitionMoveSpline;
    float transitionMoveTime;
    spline transitionExpandSpline;
    float transitionExpandTime;
    spline transitionMoveInSpline;
    float transitionMoveInTime;
    int rotationSoundVolume;
};

void LoadTypedefSLdrFrontEndDataNetwork(SLdrFrontEndDataNetwork&, CInputStream&);

#endif // _SLDRFRONTENDDATANETWORK
