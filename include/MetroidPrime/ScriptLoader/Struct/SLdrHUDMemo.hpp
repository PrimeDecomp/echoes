#ifndef _SLDRHUDMEMO
#define _SLDRHUDMEMO

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrHUDMemo {
    SLdrHUDMemo();
    ~SLdrHUDMemo();

    SLdrEditorProperties editor_properties;
    float display_time;
    bool clear_window;
    bool player1;
    bool player2;
    bool player3;
    bool player4;
    bool type_out;
    bool use_originator;
    int display_type;
    CAssetId string;
};

void LoadTypedefSLdrHUDMemo(SLdrHUDMemo&, CInputStream&);

#endif // _SLDRHUDMEMO
