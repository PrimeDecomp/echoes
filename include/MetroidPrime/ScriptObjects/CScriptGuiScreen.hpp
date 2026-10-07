#ifndef _CSCRIPTGUISCREEN
#define _CSCRIPTGUISCREEN

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CModelData.hpp"

#include "Kyoto/Math/CTransform4f.hpp"

// Native base of the ScriptGui REL screen. The DOL owns its vtable, TypesMatch and destructor.
class CScriptGuiScreen : public CActor {
public:
  // Emitted out of line in the ScriptGui REL.
  CScriptGuiScreen(TUniqueId uid, const rstl::string& name, const CEntityInfo& info);
  ~CScriptGuiScreen() override {}

  CEntity* TypesMatch(int typeId) const override;
};
CHECK_SIZEOF(CScriptGuiScreen, 0x158)

#endif // _CSCRIPTGUISCREEN
