#include "MetroidPrime/ScriptObjects/CScriptTriggerOrientated.hpp"

#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTriggerOrientated.hpp"

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
  return mWorldBounds;
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

CEntity* LoadTriggerOrientated(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrTriggerOrientated sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrTriggerOrientated.inc"

  const CVector3f forceField =
      mgr.GetWorld()->GetAreaAlways(info.GetAreaId()).GetTM().Rotate(sldrThis.trigger.forceField);
  return rs_new CScriptTriggerOrientated(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties),
      0.5f * sldrThis.editorProperties.transform.scale, LdrToTransform4f(sldrThis.editorProperties),
      LdrToDamageInfo(sldrThis.trigger.damage), forceField, sldrThis.trigger.flagsTrigger,
      sldrThis.deactivateOnEnter, sldrThis.deactivateOnExit);
}
