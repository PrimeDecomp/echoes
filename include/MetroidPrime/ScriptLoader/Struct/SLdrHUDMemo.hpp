#ifndef _SLDRHUDMEMO
#define _SLDRHUDMEMO

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrHUDMemo {
    SLdrHUDMemo()
    : editorProperties()
    , displayTime(3.0f)
    , clearWindow(true)
    , player1(true)
    , player2(true)
    , player3(true)
    , player4(true)
    , typeOut(true)
    , useOriginator(false)
    , displayType(0)
    , string(0xffffffff)
    {}
    ~SLdrHUDMemo() {}

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
