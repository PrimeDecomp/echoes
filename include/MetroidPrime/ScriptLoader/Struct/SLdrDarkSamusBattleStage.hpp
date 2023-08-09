#ifndef _SLDRDARKSAMUSBATTLESTAGE
#define _SLDRDARKSAMUSBATTLESTAGE

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct17.hpp"

struct SLdrDarkSamusBattleStage {
    SLdrDarkSamusBattleStage();
    ~SLdrDarkSamusBattleStage();

    SLdrEditorProperties editor_properties;
    SLdrUnknownStruct17 unknown_struct17;
};

void LoadTypedefSLdrDarkSamusBattleStage(SLdrDarkSamusBattleStage&, CInputStream&);

#endif // _SLDRDARKSAMUSBATTLESTAGE
