#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "REL/REL_Setup.h"

CEntity* LoadGuiWidget(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadGuiScreen(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadGuiSlider(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadGuiMenu(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadGuiPlayerJoinManager(CStateManager& mgr, CInputStream& input, CEntityInfo& info);

#ifndef MONOLITHIC
SGuiWidget_FuncPtrs REL_loader_GuiWidget;

void SetRelLoaderFunctionToLoader() {
  REL_loader_GuiWidget.mLoadGuiWidget = LoadGuiWidget;
  REL_loader_GuiWidget.mLoadGuiScreen = LoadGuiScreen;
  REL_loader_GuiWidget.mLoadGuiSlider = LoadGuiSlider;
  REL_loader_GuiWidget.mLoadGuiMenu = LoadGuiMenu;
  REL_loader_GuiWidget.mLoadGuiPlayerJoinManager = LoadGuiPlayerJoinManager;
  SetSGuiWidget_FuncPtrs(&REL_loader_GuiWidget);
}

void RELMain() { SetRelLoaderFunctionToLoader(); }

void RELExit() { SetSGuiWidget_FuncPtrs(nullptr); }
#endif
