#ifndef _CSCRIPTRIPPLE
#define _CSCRIPTRIPPLE

#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/CEntity.hpp"

// Guessed name, correlated with Prime's ripple script object.
class CScriptRipple : public CEntity {
public:
  CScriptRipple(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                const CVector3f& center, float energy);

  // CEntity
  ~CScriptRipple() override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

private:
  float mEnergy;
  CVector3f mCenter;
};
CHECK_SIZEOF(CScriptRipple, 0x34)

#endif // _CSCRIPTRIPPLE
