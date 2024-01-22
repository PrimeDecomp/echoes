#ifndef _SLDRCANNONBALL
#define _SLDRCANNONBALL

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrCannonBall {
    SLdrCannonBall() {}
    ~SLdrCannonBall() {}

    SLdrEditorProperties editorProperties;
    CAssetId effect;
};

void LoadTypedefSLdrCannonBall(SLdrCannonBall&, CInputStream&);

#endif // _SLDRCANNONBALL
