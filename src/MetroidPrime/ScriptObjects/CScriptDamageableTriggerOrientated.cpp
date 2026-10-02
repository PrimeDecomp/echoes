#include "MetroidPrime/ScriptObjects/CScriptDamageableTriggerOrientated.hpp"

CScriptDamageableTriggerOrientated::CScriptDamageableTriggerOrientated(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CVector3f& extents,
    const CTransform4f& xf, const CHealthInfo& health, const CDamageVulnerability& vulnerability,
    ECanOrbit canOrbit, ESeekerLockOn seekerLockOn, EInvulnerable invulnerable,
    const CVisorParameters& visor)
: CScriptDamageableTrigger(uid, name, info, xf.GetTranslation(), extents, health, vulnerability,
                           canOrbit, seekerLockOn, invulnerable, visor)
, mExtents(extents)
, mWorldOBBox(xf, extents)
, mWorldBounds(mWorldOBBox.CalculateAABox(CTransform4f::Identity())) {
  SetTransform(xf);
}

rstl::optional_object< CAABox > CScriptDamageableTriggerOrientated::GetTouchBounds() const {
  return rstl::optional_object< CAABox >(mWorldBounds);
}

void CScriptDamageableTriggerOrientated::Touch(CActor& actor, CStateManager& mgr) {
  const rstl::optional_object< CAABox > bounds = actor.GetTouchBounds();
  if (bounds && mWorldOBBox.IntersectsAABox(*bounds)) {
    CActor::Touch(actor, mgr);
  }
}

void CScriptDamageableTriggerOrientated::Think(float dt, CStateManager& mgr) {
  if (GetTransformDirtySpare()) {
    mWorldOBBox = COBBox(GetTransform(), mExtents);
    mWorldBounds = mWorldOBBox.CalculateAABox(CTransform4f::Identity());
    SetTransformDirtySpare(false);
  }
  CScriptDamageableTrigger::Think(dt, mgr);
}

const COBBox& CScriptDamageableTriggerOrientated::GetOBBox() const { return mWorldOBBox; }
