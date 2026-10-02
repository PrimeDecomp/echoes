#ifndef _CSCRIPTTARGETINGPOINT
#define _CSCRIPTTARGETINGPOINT

#include "MetroidPrime/CActor.hpp"

// Wii SEL class name, correlated with the Echoes targeting-point loader and GetLocked.
class CScriptTargetingPoint : public CActor {
public:
  CScriptTargetingPoint(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                        const CTransform4f& xf);

  // CEntity
  ~CScriptTargetingPoint() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;

  bool GetLocked() const;

private:
  // No consumers establishing the purpose of these two fields yet.
  bool mUnknownFlag : 1;
  TUniqueId mUnknownId;
  float mTime;
};
CHECK_SIZEOF(CScriptTargetingPoint, 0x160)

#endif // _CSCRIPTTARGETINGPOINT
