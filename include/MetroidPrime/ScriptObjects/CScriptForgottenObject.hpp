#ifndef _CSCRIPTFORGOTTENOBJECT
#define _CSCRIPTFORGOTTENOBJECT

#include "MetroidPrime/CEntity.hpp"

class CScriptForgottenObject : public CEntity {
public:
  CScriptForgottenObject(TUniqueId uid, const CEntityInfo& info, const rstl::string& name);

  // CEntity
  CEntity* TypesMatch(int typeId) const override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg&) override;
  ~CScriptForgottenObject() override;
  TUniqueId DisableTargetRendering(CStateManager& mgr, EScriptObjectState state) const;
  // Guessed names, based on the native color/destination-alpha controls.
  virtual void RenderDepthOnly(CStateManager& mgr);
  virtual void RenderAlphaMask(CStateManager& mgr);

private:
  void RenderInternal(CStateManager& mgr, TUniqueId uid, bool writeAlphaMask) const;

  // Guessed names. Targets of the Zero and MaxReached connections respectively.
  TUniqueId mDepthOnlyActorId;
  TUniqueId mAlphaMaskActorId;
};
CHECK_SIZEOF(CScriptForgottenObject, 0x28)

#endif // _CSCRIPTFORGOTTENOBJECT
