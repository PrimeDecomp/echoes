#ifndef _CSCRIPTFORGOTTENOBJECT
#define _CSCRIPTFORGOTTENOBJECT

#include "MetroidPrime/CEntity.hpp"

class CScriptForgottenObject : public CEntity {
public:
  CScriptForgottenObject(TUniqueId uid, const CEntityInfo& info, const rstl::string& name);
  CEntity* TypesMatch(int typeId) const override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg&) override;
  ~CScriptForgottenObject() override;

  virtual void Render1(CStateManager& mgr);
  virtual void Render2(CStateManager& mgr);

private:
  void RenderInternal(CStateManager& mgr, TUniqueId uid, bool b);

  TUniqueId x24_;
  TUniqueId x28_;
};

#endif // _CSCRIPTFORGOTTENOBJECT
