#include "MetroidPrime/Enemies/CAi.hpp"

#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CSimpleShadow.hpp"

#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/TToken.hpp"

CAi::CAi(TUniqueId uid, const rstl::string& name, const CEntityInfo& info, uint castFlags,
         const CTransform4f& xf, const CModelData& modelData, const CAABox& bounds, float mass,
         const CHealthInfo& health, const CDamageVulnerability& vulnerability,
         const CMaterialList& materials, CAssetId stateMachine, CAssetId stateMachine2,
         const CActorParameters& params, float stepUp, float stepDown)
: CPhysicsActor(uid, name, info, castFlags | 8, xf, modelData,
                CMaterialList(kMT_AIBlock, kMT_CameraPassthrough).Union(materials), bounds,
                SMoverData(mass), params, StepData(stepUp, stepDown, 0))
, mHealthInfo(health)
, mDamageVulnerability(vulnerability) {
  if (stateMachine != kInvalidAssetId) {
    mStateMachine = gpSimplePool->GetObj(SObjectTag('AFSM', stateMachine));
  } else {
    mStateMachine = gpSimplePool->GetObj(SObjectTag('FSM2', stateMachine2));
  }
  mStateMachine->Lock();

  fn_8004ab14();
  if (HasShadow()) {
    SetDrawShadow(true);
    Shadow()->SetAlwaysCalculateRadius(false);
  }
  if (HasActorLights()) {
    ActorLights()->SetCastShadows(true);
  }
}

CAi::~CAi() {}

CHealthInfo* CAi::HealthInfo() { return &mHealthInfo; }

const CDamageVulnerability* CAi::GetDamageVulnerability() const { return &mDamageVulnerability; }

CDamageVulnerability* CAi::DamageVulnerability() { return &mDamageVulnerability; }

void CAi::TakeDamage(const CVector3f&, float) {}

void CAi::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  if (msg.GetMessage() == kSM_XALD) {
    CMaterialList include = GetMaterialFilter().GetIncludeList();
    include.Add(kMT_AIBlock);
    SetMaterialFilter(
        CMaterialFilter::MakeIncludeExclude(include, GetMaterialFilter().GetExcludeList()));
  }
  CActor::AcceptScriptMsg(mgr, msg);
}

void CAi::FluidFXThink(EFluidState, CScriptWater&, CStateManager&) {
  // TODO: Restore entry/exit splashes when the water and fluid-manager interfaces are available.
}

CStateMachine* CAi::GetStateMachine() {
  if (!mStateMachine->IsLoaded()) {
    return nullptr;
  }
  TToken< CStateMachine > token(*mStateMachine);
  return *token;
}

CStateMachine2* CAi::GetStateMachine2() {
  if (!mStateMachine->IsLoaded()) {
    return nullptr;
  }
  TToken< CStateMachine2 > token(*mStateMachine);
  return *token;
}

EWeaponCollisionResponseTypes CAi::GetCollisionResponseType(const CVector3f&, const CVector3f&,
                                                            const CWeaponMode&, int) const {
  return kWCR_EnemyNormal;
}
