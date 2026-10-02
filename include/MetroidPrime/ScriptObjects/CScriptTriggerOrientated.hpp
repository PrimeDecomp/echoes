#ifndef _CSCRIPTTRIGGERORIENTATED
#define _CSCRIPTTRIGGERORIENTATED

#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"

#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CVector3f.hpp"

// Scaffold; evidence in Echoes research/CScriptSafeZone-CScriptTriggerOrientated-G2ME01.md.
class CScriptTriggerOrientated : public CScriptTrigger {
public:
  CScriptTriggerOrientated(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                           const CVector3f& scale, const CTransform4f& xf,
                           const CDamageInfo& damage, const CVector3f& forceField, uint flags,
                           bool deactivateOnEntered, bool deactivateOnExited, uint unknown);

  // CEntity
  ~CScriptTriggerOrientated() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;

  // CActor
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;

  // CScriptTrigger
  bool BoundsOverlap(const CAABox& bounds) const override;

private:
  CVector3f mScale;        // 0x1c8, half extents
  CVector3f mInverseScale; // 0x1d4
  CAABox mWorldBounds;     // 0x1e0, bounds of the oriented box transformed into world space
  float x1f8_;
  uint x1fc_;
};
CHECK_SIZEOF(CScriptTriggerOrientated, 0x200)

#endif // _CSCRIPTTRIGGERORIENTATED
