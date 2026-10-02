#ifndef _CSCRIPTDAMAGEABLETRIGGER
#define _CSCRIPTDAMAGEABLETRIGGER

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CHealthInfo.hpp"

// Original Wii export name, correlated with the GameCube DTRG loader and runtime.
class CScriptDamageableTrigger : public CActor {
public:
  // Prime-correlated names; the target tests the enabled value against one.
  enum ECanOrbit { kCO_NoOrbit, kCO_Orbit };
  // Guessed names; the loader property name is not verified, but it adds material 63.
  enum ESeekerLockOn { kSLO_Disabled, kSLO_Enabled };
  // Guessed names; constructor and message consumers establish the two states.
  enum EInvulnerable { kIV_Vulnerable, kIV_Invulnerable };

  CScriptDamageableTrigger(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                           const CVector3f& position, const CVector3f& extent,
                           const CHealthInfo& health, const CDamageVulnerability& vulnerability,
                           ECanOrbit canOrbit, ESeekerLockOn seekerLockOn,
                           EInvulnerable invulnerable, const CVisorParameters& visor);

  // CEntity
  ~CScriptDamageableTrigger() override {}
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  CHealthInfo* HealthInfo() override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f& point,
                                                         const CVector3f& normal,
                                                         const CWeaponMode& weapon,
                                                         int attributes) const override;

private:
  CAABox mBounds;
  CHealthInfo mHealth;
  CDamageVulnerability mVulnerability;
  TUniqueId mDeathOriginator; // Guessed name; forwarded with the Dead state.
  bool mNotOccluded : 1;
  bool mInvulnerable : 1;
  bool mCanOrbit : 1;
  bool mPendingDeath : 1; // Guessed name; set by XDamage, consumed by Think.
};
CHECK_SIZEOF(CScriptDamageableTrigger, 0x1c8)

#endif // _CSCRIPTDAMAGEABLETRIGGER
