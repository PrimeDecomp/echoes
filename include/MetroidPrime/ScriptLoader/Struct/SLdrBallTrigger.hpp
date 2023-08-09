#ifndef _SLDRBALLTRIGGER
#define _SLDRBALLTRIGGER

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTriggerInfo.hpp"

struct SLdrBallTrigger {
    SLdrBallTrigger();
    ~SLdrBallTrigger();

    SLdrEditorProperties editor_properties;
    SLdrTriggerInfo trigger;
    float attraction_force;
    float attraction_angle;
    float attraction_distance;
    CVector3f attraction_direction;
    bool no_ball_movement;
    float bounds_size_multiplier;
};

void LoadTypedefSLdrBallTrigger(SLdrBallTrigger&, CInputStream&);

#endif // _SLDRBALLTRIGGER
