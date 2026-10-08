#ifndef _CSCRIPTSKYRIPPLE
#define _CSCRIPTSKYRIPPLE

#include "MetroidPrime/CActor.hpp"

class CScriptSkyRipple : public CActor {
public:
  CScriptSkyRipple(TUniqueId uid, const CEntityInfo& info, const rstl::string& name);

  // CEntity
  ~CScriptSkyRipple() override {}
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void Render(const CStateManager& mgr) const override;

private:
  void RenderSkyActor(const CStateManager& mgr, TUniqueId id, const CVector3f& pos, int pass) const;
  TUniqueId FindSkyActor(CStateManager& mgr, EScriptObjectState state);

  TUniqueId mFirstSkyActor;  // Guessed name
  TUniqueId mSecondSkyActor; // Guessed name
};
CHECK_SIZEOF(CScriptSkyRipple, 0x160)

#endif // _CSCRIPTSKYRIPPLE
