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

    SLdrEditorProperties editor_properties;
    int priority;
    float timer;
    SLdrUnknownStruct4 unknown_struct4;
    int flags_camera_hint;
    SLdrCameraHintStructB camera_hint_struct_a_0x456d05c6;
    SLdrCameraHintStructB camera_hint_struct_a_0xf5521ffa;
    SLdrCameraHintStructB camera_hint_struct_a_0x89658a06;
    SLdrUnknownStruct5 unknown_struct5;
    CVector3f world_offset;
    SLdrUnknownStruct6 unknown_struct6;
    SLdrCameraHintStructC camera_hint_struct_b_0x664c450a;
    SLdrCameraHintStructC camera_hint_struct_b_0xc82395fa;
    SLdrUnknownStruct7 unknown_struct7;
    SLdrUnknownStruct8 unknown_struct8;
    float unknown_0x2ae08be1;
    float unknown_0x4361d075;
    float unknown_0xc91ef813;
    SLdrCameraHintStructA camera_hint_struct_a1;
    SLdrUnknownStruct9 unknown_struct9;
    SLdrCameraHintStructA camera_hint_struct_a_0x138729a7;
};

void LoadTypedefSLdrCameraHint(SLdrCameraHint&, CInputStream&);

#endif // _SLDRCAMERAHINT
