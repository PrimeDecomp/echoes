#ifndef _CSCRIPTGUIWIDGET
#define _CSCRIPTGUIWIDGET

#include "MetroidPrime/CEntity.hpp"

#include "Kyoto/TFunctor.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CFinalInput;

class CScriptGuiWidget : public CEntity {
public:
  // Guessed names. Event 0/1 follow A/B presses, 2/3 follow Open/Close messages.
  enum EWidgetEvent {
    kWE_Accept,
    kWE_Back,
    kWE_Open,
    kWE_Close,
  };
  typedef TFunctor3< CStateManager&, CScriptGuiWidget*, int > TEventCallback;

  CScriptGuiWidget(TUniqueId uid, const rstl::string& name, const CEntityInfo& info, int controller,
                   const rstl::string& label, bool locked);
  ~CScriptGuiWidget() override;

  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // Guessed names.
  void ClearControllers();
  void AddController(int controller);
  void SetLocked(bool locked, CStateManager& mgr);
  bool ProcessInput(const CFinalInput& input, CStateManager& mgr);

  int GetControllerNumber() const { return mControllerNumber; }
  bool IsLocked() const { return mLocked; }
  void SetEventCallback(const TEventCallback& callback) { mCallback = callback; }

protected:
  // Guessed names.
  rstl::vector< int > mControllers;
  int mActiveController;
  int mControllerNumber;
  rstl::string mLabel;
  bool mControllerPresent;
  TEventCallback mCallback;
  bool mLocked;
};
CHECK_SIZEOF(CScriptGuiWidget, 0x6c)

#endif // _CSCRIPTGUIWIDGET
