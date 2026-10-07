#ifndef _CSCRIPTTRIGGERORIENTATED
#define _CSCRIPTTRIGGERORIENTATED

#include "Collision/COBBox.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"

// Wii export name; the oriented-box trigger, distinct from CScriptTriggerEllipsoid.
class CScriptTriggerOrientated : public CScriptTrigger {
public:
  CScriptTriggerOrientated(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                           const CVector3f& extents, const CTransform4f& xf,
                           const CDamageInfo& damage, const CVector3f& forceField, uint flags,
                           bool deactivateOnEntered, bool deactivateOnExited);

  // CEntity
  ~CScriptTriggerOrientated() override {}
  void Think(float dt, CStateManager& mgr) override;
  CEntity* TypesMatch(int typeId) const override;

  // CActor
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;

  // CScriptTrigger
  bool BoundsOverlap(const CAABox& bounds) const override;

  const COBBox& GetOBBox() const;

private:
  CVector3f mExtents;
  COBBox mWorldOBBox;
  CAABox mWorldBounds;
};
CHECK_SIZEOF(CScriptTriggerOrientated, 0x228)

#endif // _CSCRIPTTRIGGERORIENTATED
