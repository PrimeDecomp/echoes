#ifndef _SLDRBALLTRIGGER
#define _SLDRBALLTRIGGER

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTriggerInfo.hpp"

struct SLdrBallTrigger {
    SLdrBallTrigger();
    ~SLdrBallTrigger();

    SLdrEditorProperties editorProperties;
    SLdrTriggerInfo trigger;
    float attractionForce;
    float attractionAngle;
    float attractionDistance;
    CVector3f attractionDirection;
    bool noBallMovement;
    float boundsSizeMultiplier;
};

void LoadTypedefSLdrBallTrigger(SLdrBallTrigger&, CInputStream&);

#endif // _SLDRBALLTRIGGER
