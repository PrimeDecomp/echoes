#include "MetroidPrime/ScriptObjects/CScriptDamageableTriggerOrientated.hpp"

#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageableTriggerOrientated.hpp"

CEntity* LoadDamageableTriggerOriented(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrDamageableTriggerOrientated sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrDamageableTriggerOrientated.inc"

  if (close_enough(sldrThis.editorProperties.transform.rotation, CVector3f::Zero())) {
    return rs_new CScriptDamageableTrigger(
        mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
        LdrToEntityInfo(info, sldrThis.editorProperties),
        sldrThis.editorProperties.transform.position, sldrThis.editorProperties.transform.scale,
        LdrToHealthInfo(sldrThis.health), LdrToDamageVulnerability(sldrThis.vulnerability),
        CScriptDamageableTrigger::ECanOrbit(sldrThis.orbitable),
        CScriptDamageableTrigger::ESeekerLockOn(sldrThis.enableSeekerLockOn),
        CScriptDamageableTrigger::EInvulnerable(sldrThis.invulnerable),
        LdrToVisorParameters(sldrThis.visor));
  }

  return rs_new CScriptDamageableTriggerOrientated(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties),
      0.5f * sldrThis.editorProperties.transform.scale, LdrToTransform4f(sldrThis.editorProperties),
      LdrToHealthInfo(sldrThis.health), LdrToDamageVulnerability(sldrThis.vulnerability),
      CScriptDamageableTrigger::ECanOrbit(sldrThis.orbitable),
      CScriptDamageableTrigger::ESeekerLockOn(sldrThis.enableSeekerLockOn),
      CScriptDamageableTrigger::EInvulnerable(sldrThis.invulnerable),
      LdrToVisorParameters(sldrThis.visor));
}

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
