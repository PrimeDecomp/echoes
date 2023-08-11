#ifndef _SLDRCAMERAHINT
#define _SLDRCAMERAHINT

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrCameraHintStructA.hpp"
#include "MetroidPrime/ScriptLoader/SLdrCameraHintStructB.hpp"
#include "MetroidPrime/ScriptLoader/SLdrCameraHintStructC.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct4.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct5.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct6.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct7.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct8.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct9.hpp"

struct SLdrCameraHint {
    SLdrCameraHint();
    ~SLdrCameraHint();

    SLdrEditorProperties editorProperties;
    int priority;
    float timer;
    SLdrUnknownStruct4 unknownStruct4;
    int flagsCameraHint;
    SLdrCameraHintStructB cameraHintStructA_0x456d05c6;
    SLdrCameraHintStructB cameraHintStructA_0xf5521ffa;
    SLdrCameraHintStructB cameraHintStructA_0x89658a06;
    SLdrUnknownStruct5 unknownStruct5;
    CVector3f worldOffset;
    SLdrUnknownStruct6 unknownStruct6;
    SLdrCameraHintStructC cameraHintStructB_0x664c450a;
    SLdrCameraHintStructC cameraHintStructB_0xc82395fa;
    SLdrUnknownStruct7 unknownStruct7;
    SLdrUnknownStruct8 unknownStruct8;
    float unknown_0x2ae08be1;
    float unknown_0x4361d075;
    float unknown_0xc91ef813;
    SLdrCameraHintStructA cameraHintStructA1;
    SLdrUnknownStruct9 unknownStruct9;
    SLdrCameraHintStructA cameraHintStructA_0x138729a7;
};

void LoadTypedefSLdrCameraHint(SLdrCameraHint&, CInputStream&);

#endif // _SLDRCAMERAHINT
