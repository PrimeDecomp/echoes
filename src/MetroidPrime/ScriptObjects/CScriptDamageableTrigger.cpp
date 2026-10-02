#include "MetroidPrime/ScriptObjects/CScriptDamageableTrigger.hpp"

#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"

static CMaterialList skDamageableTriggerMaterials(kMT_Trigger, kMT_Immovable,
                                                  kMT_NonSolidDamageable);

static CMaterialList
make_damageable_trigger_materials(CScriptDamageableTrigger::ECanOrbit canOrbit,
                                  CScriptDamageableTrigger::ESeekerLockOn seekerLockOn) {
  CMaterialList materials = skDamageableTriggerMaterials;
  if (canOrbit == CScriptDamageableTrigger::kCO_Orbit) {
    materials.Add(kMT_Orbit);
  }
  if (seekerLockOn == CScriptDamageableTrigger::kSLO_Enabled) {
    materials.Add(kMT_SixtyThree);
  }
  return materials;
}

CScriptDamageableTrigger::CScriptDamageableTrigger(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CVector3f& position,
    const CVector3f& extent, const CHealthInfo& health, const CDamageVulnerability& vulnerability,
    ECanOrbit canOrbit, ESeekerLockOn seekerLockOn, EInvulnerable invulnerable,
    const CVisorParameters& visor)
: CActor(uid, name, info, 0, CTransform4f::Translate(position), CModelData::CModelDataNull(),
         make_damageable_trigger_materials(canOrbit, seekerLockOn),
         CActorParameters::None().MakeDamageableTriggerActorParms(visor), kInvalidUniqueId)
, mBounds(-(0.5f * extent), 0.5f * extent)
, mHealth(health)
, mVulnerability(vulnerability)
, mDeathOriginator(kInvalidUniqueId)
, mNotOccluded(false)
, mInvulnerable(invulnerable == kIV_Invulnerable)
, mCanOrbit(canOrbit == kCO_Orbit)
, mPendingDeath(false) {}

rstl::optional_object< CAABox > CScriptDamageableTrigger::GetTouchBounds() const {
  if (GetActive() && mNotOccluded) {
    const CVector3f& position = GetTranslation();
    return CAABox(mBounds.GetMinPoint() + position, mBounds.GetMaxPoint() + position);
  }
  return rstl::optional_object_null();
}

void CScriptDamageableTrigger::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Activate:
    if (!GetActive()) {
      mHealth.SetHP(mHealth.GetInitialHP());
      if (mCanOrbit) {
        AddMaterial(kMT_Orbit, mgr);
      }
    }
    break;
  case kSM_XDamage:
    if (mHealth.GetHP() <= 0.f) {
      mDeathOriginator = msg.GetUnk();
      if (mgr.IsMultiplayer()) {
        mDeathOriginator = mHealth.GetDamageId1();
        if (mDeathOriginator == kInvalidUniqueId) {
          mDeathOriginator = mHealth.GetDamageId2();
        }
      }
      mPendingDeath = true;
    }
    break;
  case kSM_Increment:
    mInvulnerable = true;
    break;
  case kSM_Decrement:
    mInvulnerable = false;
    break;
  default:
    break;
  }
  CActor::AcceptScriptMsg(mgr, msg);
}

void CScriptDamageableTrigger::Think(float, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  const CGameArea& area = mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId());
  const bool wasNotOccluded = mNotOccluded;
  mNotOccluded = area.GetOcclusionState() == CGameArea::kOS_Visible;
  if (mNotOccluded != wasNotOccluded) {
    SetTransformDirty();
  }

  if (mPendingDeath && !mInvulnerable && mHealth.GetHP() <= 0.f) {
    SendScriptMsgs(kSS_Dead, mgr, mDeathOriginator, kSM_None);
    RemoveMaterial(kMT_Orbit, mgr);
    SetActive(false);
    mPendingDeath = false;
  }
}

CHealthInfo* CScriptDamageableTrigger::HealthInfo() { return &mHealth; }

const CDamageVulnerability* CScriptDamageableTrigger::GetDamageVulnerability() const {
  return &mVulnerability;
}

EWeaponCollisionResponseTypes
CScriptDamageableTrigger::GetCollisionResponseType(const CVector3f&, const CVector3f&,
                                                   const CWeaponMode& weapon, int) const {
  return mVulnerability.WeaponHits(weapon, 0) ? kWCR_OtherProjectile : kWCR_Unknown15;
}
