#ifndef _SLDRSPECIALFUNCTION
#define _SLDRSPECIALFUNCTION

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPlayerItem.hpp"
#include "rstl/string.hpp"

struct SLdrSpecialFunction {
    SLdrSpecialFunction();
    ~SLdrSpecialFunction();

    SLdrEditorProperties editor_properties;
    int function;
    rstl::string string_parm;
    float value_parm;
    float value_parm2;
    float value_parm3;
    float value_parm4;
    int int_parm1;
    int int_parm2;
    SLdrPlayerItem inventory_item_parm;
    int sound1;
    int sound2;
    int sound3;
};

void LoadTypedefSLdrSpecialFunction(SLdrSpecialFunction&, CInputStream&);

#endif // _SLDRSPECIALFUNCTION
