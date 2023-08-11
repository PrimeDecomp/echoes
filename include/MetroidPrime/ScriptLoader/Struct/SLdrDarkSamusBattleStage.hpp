#ifndef _SLDRDARKSAMUSBATTLESTAGE
#define _SLDRDARKSAMUSBATTLESTAGE

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct17.hpp"

struct SLdrDarkSamusBattleStage {
    SLdrDarkSamusBattleStage();
    ~SLdrDarkSamusBattleStage();

    SLdrEditorProperties editorProperties;
    SLdrUnknownStruct17 unknownStruct17;
};

void LoadTypedefSLdrDarkSamusBattleStage(SLdrDarkSamusBattleStage&, CInputStream&);

#endif // _SLDRDARKSAMUSBATTLESTAGE
