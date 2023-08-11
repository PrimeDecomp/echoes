#ifndef _SLDRRUBIKSPUZZLE
#define _SLDRRUBIKSPUZZLE

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct35.hpp"

struct SLdrRubiksPuzzle {
    SLdrRubiksPuzzle();
    ~SLdrRubiksPuzzle();

    SLdrEditorProperties editorProperties;
    SLdrUnknownStruct35 unknownStruct35;
};

void LoadTypedefSLdrRubiksPuzzle(SLdrRubiksPuzzle&, CInputStream&);

#endif // _SLDRRUBIKSPUZZLE
