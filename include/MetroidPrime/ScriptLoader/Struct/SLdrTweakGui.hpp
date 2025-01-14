#ifndef _SLDRTWEAKGUI
#define _SLDRTWEAKGUI

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGui_Completion.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGui_Credits.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGui_DarkVisor.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGui_EchoVisor.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGui_LogBook.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGui_Misc.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGui_MovieVolumes.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGui_ScanVisor.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGui_ScannableObjectDownloadTimes.hpp"
#include "rstl/string.hpp"

struct SLdrTweakGui {
    SLdrTweakGui();
    ~SLdrTweakGui();

    rstl::string instanceName;
    SLdrTweakGui_Misc misc;
    SLdrTweakGui_ScannableObjectDownloadTimes scannableObjectDownloadTimes;
    SLdrTweakGui_DarkVisor unknown;
    SLdrTweakGui_EchoVisor echoVisor;
    SLdrTweakGui_ScanVisor scanVisor;
    SLdrTweakGui_LogBook logBook;
    SLdrTweakGui_Credits credits;
    SLdrTweakGui_Completion completion;
    SLdrTweakGui_MovieVolumes movieVolumes;
};

void LoadTypedefSLdrTweakGui(SLdrTweakGui&, CInputStream&);

#endif // _SLDRTWEAKGUI
