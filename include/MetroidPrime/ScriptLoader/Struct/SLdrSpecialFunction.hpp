#ifndef _SLDRSPECIALFUNCTION
#define _SLDRSPECIALFUNCTION

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPlayerItem.hpp"
#include "rstl/string.hpp"

struct SLdrSpecialFunction {
    SLdrSpecialFunction();
    ~SLdrSpecialFunction();

    SLdrEditorProperties editorProperties;
    int function;
    rstl::string stringParm;
    float valueParm;
    float valueParm2;
    float valueParm3;
    float valueParm4;
    int intParm1;
    int intParm2;
    SLdrPlayerItem inventoryItemParm;
    int sound1;
    int sound2;
    int sound3;
};

void LoadTypedefSLdrSpecialFunction(SLdrSpecialFunction&, CInputStream&);

#endif // _SLDRSPECIALFUNCTION
