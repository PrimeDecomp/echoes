#include "MetroidPrime/ScriptObjects/CScriptTriggerOrientated.hpp"

CScriptTriggerOrientated::CScriptTriggerOrientated(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CVector3f& extents,
    const CTransform4f& xf, const CDamageInfo& damage, const CVector3f& forceField, uint flags,
    bool deactivateOnEntered, bool deactivateOnExited)
: CScriptTrigger(uid, name, info, xf.GetTranslation(), CAABox(CVector3f::Zero(), CVector3f::Zero()),
                 damage, forceField, flags, deactivateOnEntered, deactivateOnExited)
, mExtents(extents)
, mWorldOBBox(xf, extents)
, mWorldBounds(mWorldOBBox.CalculateAABox(CTransform4f::Identity())) {
  SetTransform(xf);
}

rstl::optional_object< CAABox > CScriptTriggerOrientated::GetTouchBounds() const {
  return rstl::optional_object< CAABox >(mWorldBounds);
}

void CScriptTriggerOrientated::Touch(CActor& actor, CStateManager& mgr) {
  const rstl::optional_object< CAABox > bounds = actor.GetTouchBounds();
  if (bounds && mWorldOBBox.IntersectsAABox(*bounds)) {
    CScriptTrigger::Touch(actor, mgr);
  }
}

bool CScriptTriggerOrientated::BoundsOverlap(const CAABox& bounds) const {
  return mWorldOBBox.IntersectsAABox(bounds);
}

void CScriptTriggerOrientated::Think(float dt, CStateManager& mgr) {
  if (GetTransformDirtySpare()) {
    mWorldOBBox = COBBox(GetTransform(), mExtents);
    mWorldBounds = mWorldOBBox.CalculateAABox(CTransform4f::Identity());
    SetTransform(GetTransform());
    SetTransformDirtySpare(false);
  }
  CScriptTrigger::Think(dt, mgr);
}

const COBBox& CScriptTriggerOrientated::GetOBBox() const { return mWorldOBBox; }
