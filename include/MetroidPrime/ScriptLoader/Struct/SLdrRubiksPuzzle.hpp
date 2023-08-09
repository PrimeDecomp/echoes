#ifndef _SLDRRUBIKSPUZZLE
#define _SLDRRUBIKSPUZZLE

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct35.hpp"

struct SLdrRubiksPuzzle {
    SLdrRubiksPuzzle();
    ~SLdrRubiksPuzzle();

    SLdrEditorProperties editor_properties;
    SLdrUnknownStruct35 unknown_struct35;
};

void LoadTypedefSLdrRubiksPuzzle(SLdrRubiksPuzzle&, CInputStream&);

#endif // _SLDRRUBIKSPUZZLE
