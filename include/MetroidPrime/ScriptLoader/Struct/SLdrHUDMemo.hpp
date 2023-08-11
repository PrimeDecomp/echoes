#ifndef _SLDRHUDMEMO
#define _SLDRHUDMEMO

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrHUDMemo {
    SLdrHUDMemo();
    ~SLdrHUDMemo();

    SLdrEditorProperties editorProperties;
    float displayTime;
    bool clearWindow;
    bool player1;
    bool player2;
    bool player3;
    bool player4;
    bool typeOut;
    bool useOriginator;
    int displayType;
    CAssetId string;
};

void LoadTypedefSLdrHUDMemo(SLdrHUDMemo&, CInputStream&);

#endif // _SLDRHUDMEMO
