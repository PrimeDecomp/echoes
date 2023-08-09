#ifndef _SLDRGUIPLAYERJOINMANAGER
#define _SLDRGUIPLAYERJOINMANAGER

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrGuiPlayerJoinManager {
    SLdrGuiPlayerJoinManager();
    ~SLdrGuiPlayerJoinManager();

    SLdrEditorProperties editor_properties;
};

void LoadTypedefSLdrGuiPlayerJoinManager(SLdrGuiPlayerJoinManager&, CInputStream&);

#endif // _SLDRGUIPLAYERJOINMANAGER
