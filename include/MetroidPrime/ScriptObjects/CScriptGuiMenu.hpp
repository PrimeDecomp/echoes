#ifndef _CSCRIPTGUIMENU
#define _CSCRIPTGUIMENU

#include "MetroidPrime/ScriptObjects/CScriptGuiWidget.hpp"

#include "GuiSys/CRepeatState.hpp"
#include "MetroidPrime/TGameTypes.hpp"

class CScriptGuiMenu : public CScriptGuiWidget {
public:
  CScriptGuiMenu(TUniqueId uid, const rstl::string& name, const CEntityInfo& info, bool wrap,
                 bool vertical, int controller, const rstl::string& label, bool locked,
                 ushort selectionChangedSfx);
  ~CScriptGuiMenu() override;

  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // Guessed names.
  bool ProcessMenuInput(const CFinalInput& input, CStateManager& mgr);
  bool IsItemLocked(int idx, CStateManager& mgr) const;
  void UpdateItemStates(CStateManager& mgr, EScriptObjectMessage msg);
  void SelectLast(CStateManager& mgr);
  void SelectFirst(CStateManager& mgr);
  void SelectNext(CStateManager& mgr);
  void SelectPrevious(CStateManager& mgr);
  void SetSelection(int idx, CStateManager& mgr);
  void BuildItemList(CStateManager& mgr);

  int GetSelection() const { return mSelection; }
  const rstl::vector< TUniqueId >& GetItems() const { return mItems; }
  TUniqueId GetItem(int idx) const { return mItems[idx]; }

private:
  // Guessed names.
  rstl::vector< TUniqueId > mItems;
  int mSelection;
  bool mWrapSelection;
  bool mVertical;
  CRepeatState mPrevRepeat;
  CRepeatState mNextRepeat;
  ushort mSelectionChangedSfx;
};
CHECK_SIZEOF(CScriptGuiMenu, 0x90)

#endif // _CSCRIPTGUIMENU
