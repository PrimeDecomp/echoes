#ifndef _CEFFECT
#define _CEFFECT

#include "MetroidPrime/CActor.hpp"

class CEffect : public CActor {
public:
  CEffect(TUniqueId uid, const CEntityInfo& info, const rstl::string& name, const CTransform4f& xf);

  // CEntity
  ~CEffect() override;

  // CActor
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
};
CHECK_SIZEOF(CEffect, 0x158)

#endif // _CEFFECT
