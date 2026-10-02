#ifndef _CSCRIPTDAMAGEABLETRIGGERORIENTATED
#define _CSCRIPTDAMAGEABLETRIGGERORIENTATED

#include "Collision/COBBox.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDamageableTrigger.hpp"

// Guessed name; the DTRO loader constructs the oriented-box damageable trigger.
class CScriptDamageableTriggerOrientated : public CScriptDamageableTrigger {
public:
  CScriptDamageableTriggerOrientated(TUniqueId uid, const rstl::string& name,
                                     const CEntityInfo& info, const CVector3f& extents,
                                     const CTransform4f& xf, const CHealthInfo& health,
                                     const CDamageVulnerability& vulnerability, ECanOrbit canOrbit,
                                     ESeekerLockOn seekerLockOn, EInvulnerable invulnerable,
                                     const CVisorParameters& visor);

  // CEntity
  ~CScriptDamageableTriggerOrientated() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;

  // CActor
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;

  const COBBox& GetOBBox() const;

private:
  CVector3f mExtents;
  COBBox mWorldOBBox;
  CAABox mWorldBounds;
};
CHECK_SIZEOF(CScriptDamageableTriggerOrientated, 0x228)

#endif // _CSCRIPTDAMAGEABLETRIGGERORIENTATED
