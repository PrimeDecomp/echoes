#ifndef _SLDRUNKNOWNSTRUCT39
#define _SLDRUNKNOWNSTRUCT39

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"

struct SLdrUnknownStruct39 {
    SLdrUnknownStruct39();
    ~SLdrUnknownStruct39();

    int startState;
    SLdrDamageInfo explosionDamage;
    float minHeight;
    float maxHeight;
    float minDownHeight;
    float maxDownHeight;
    float separationDistance;
    float minLifeTime;
    float maxLifeTime;
    float normalKnockback;
    float heavyKnockback;
    float knockbackDecline;
    bool isDarkShredder;
    float desiredDistance;
};

void LoadTypedefSLdrUnknownStruct39(SLdrUnknownStruct39&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT39
