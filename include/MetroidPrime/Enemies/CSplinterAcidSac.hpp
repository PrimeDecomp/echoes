#ifndef _CSPLINTERACIDSAC
#define _CSPLINTERACIDSAC

#include "types.h"

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CRELFileToken.hpp"

// Guessed class: the acid sac a splinter carries; it pops when shot and then fades away.
class CSplinterAcidSac : public CActor {
public:
  CSplinterAcidSac(TUniqueId uid, TAreaId area, const rstl::string& name,
                   const CModelData& modelData, int initialAnimation, int popAnimation,
                   const CDamageVulnerability& vulnerability);

  // CEntity
  ~CSplinterAcidSac() override {}
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void Render(const CStateManager& mgr) const override;
  CHealthInfo* HealthInfo() override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;

  // CSplinterAcidSac
  bool IsPopped() const;
  void Pop();

private:
  int mPopAnimation;                         // Guessed name
  int mInitialAnimation;                     // Guessed name
  bool mPopped;                              // Guessed name
  float mPopTimer;                           // Guessed name
  CDamageVulnerability mDamageVulnerability; // Guessed name
  CHealthInfo mHealthInfo;                   // Guessed name
  CRELFileToken mRelToken;                   // Guessed name; keeps Splinter.rel loaded
};
CHECK_SIZEOF(CSplinterAcidSac, 0x1c0)

#endif // _CSPLINTERACIDSAC
