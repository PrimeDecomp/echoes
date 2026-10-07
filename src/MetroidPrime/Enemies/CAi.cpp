#include "MetroidPrime/Enemies/CAi.hpp"

#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CFluidPlaneManager.hpp"
#include "MetroidPrime/CSimpleShadow.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"

#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CMath.hpp"
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

  AllocateShadow();
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
  switch (msg.GetMessage()) {
  case kSM_AreaLoaded: {
    const CMaterialList& exclude = GetMaterialFilter().GetExcludeList();
    CMaterialList include(kMT_AIBlock);
    include.Union(GetMaterialFilter().GetIncludeList());
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include, exclude));
    break;
  }
  default:
    break;
  }
  CActor::AcceptScriptMsg(mgr, msg);
}

void CAi::FluidFXThink(EFluidState state, CScriptWater& water, CStateManager& mgr) {
  switch (state) {
  case kFS_EnteredFluid:
  case kFS_LeftFluid: {
    if (mgr.GetFluidPlaneManager()->GetLastSplashDeltaTime(GetUniqueId()) >= 0.2f) {
      const float energy = 0.5f * GetMass() * GetVelocityWR().MagSquared();
      if (energy > 500.f) {
        const float intensity = 0.1f + 0.4f * (CMath::Min(energy, 30000.f) - 500.f) / 29500.f;
        const CVector3f position(GetTranslation().GetX(), GetTranslation().GetY(),
                                 water.GetTriggerBoundsWR().GetMaxPoint().GetZ());
        mgr.GetFluidPlaneManager()->CreateSplash(GetUniqueId(), mgr, water, position, intensity,
                                                 true);
      }
    }
    break;
  }
  default:
    break;
  }
}

CStateMachine* CAi::GetStateMachine() {
  if (mStateMachine->IsLoaded()) {
    TToken< CStateMachine > token(*mStateMachine);
    return *token;
  }
  return nullptr;
}

CGenericFSM2* CAi::GetStateMachine2() {
  if (mStateMachine->IsLoaded()) {
    TToken< CGenericFSM2 > token(*mStateMachine);
    return *token;
  }
  return nullptr;
}

EWeaponCollisionResponseTypes CAi::GetCollisionResponseType(const CVector3f&, const CVector3f&,
                                                            const CWeaponMode&, int) const {
  return kWCR_EnemyNormal;
}
