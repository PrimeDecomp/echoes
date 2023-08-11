#ifndef _SLDRUNKNOWNSTRUCT17
#define _SLDRUNKNOWNSTRUCT17

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct16.hpp"

struct SLdrUnknownStruct17 {
    SLdrUnknownStruct17();
    ~SLdrUnknownStruct17();

    float unknown_0xb5af7831;
    float unknown_0xac65eb7a;
    float unknown_0x4f3855a0;
    float unknown_0x08c0b02c;
    float unknown_0x695f68c7;
    float unknown_0xd061ff99;
    float pauseDurationMin;
    float pauseDurationMax;
    float chanceToDoubleDash;
    SLdrUnknownStruct16 unknownStruct16;
    bool unknown_0x3ff87a8c;
    bool unknown_0x49b9936d;
    bool unknown_0xc96b8223;
    bool unknown_0x53fdcb5b;
    bool unknown_0x0d7ef013;
    bool unknown_0xaa85c885;
    float pause;
    float taunt;
    bool lookAround;
    float meleeAttack;
    float meleeDash;
    float scatterShot;
    float unknown_0x94f48974;
    float diveAttack;
    bool unknown_0xb2c1e4fa;
    bool unknown_0xf5cf3c0f;
    float normalMissile;
    float missileJump;
    float superMissile;
    float unknown_0xe63286eb;
    float unknown_0x4aae6186;
    float sweepBeam;
    float boostBall;
    bool unknown_0x2d7551e6;
    float phazonAttack;
    float phazonEnrage;
    bool unknown_0x911a2476;
};

void LoadTypedefSLdrUnknownStruct17(SLdrUnknownStruct17&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT17
