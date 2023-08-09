#ifndef _SLDRTWEAKGUI_SCANNABLEOBJECTDOWNLOADTIMES
#define _SLDRTWEAKGUI_SCANNABLEOBJECTDOWNLOADTIMES

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakGui_ScannableObjectDownloadTimes {
    SLdrTweakGui_ScannableObjectDownloadTimes();
    ~SLdrTweakGui_ScannableObjectDownloadTimes();

    float fast;
    float slow;
};

void LoadTypedefSLdrTweakGui_ScannableObjectDownloadTimes(SLdrTweakGui_ScannableObjectDownloadTimes&, CInputStream&);

#endif // _SLDRTWEAKGUI_SCANNABLEOBJECTDOWNLOADTIMES
