#include "MetroidPrime/Enemies/CAi.hpp"

#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CSimpleShadow.hpp"

#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CFluidPlaneManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "rstl/math.hpp"

CAi::CAi(TUniqueId uid, const rstl::string& name, const CEntityInfo& info, uint castFlags,
         const CTransform4f& xf, const CModelData& modelData, const CAABox& bounds, float mass,
         const CHealthInfo& health, const CDamageVulnerability& vulnerability,
         const CMaterialList& materials, CAssetId stateMachine, CAssetId stateMachine2,
         const CActorParameters& params, float stepUp, float stepDown)
: CPhysicsActor(uid, name, info, castFlags | 8, xf, modelData,
                materials.Union(CMaterialList(kMT_AIBlock, kMT_CameraPassthrough)), bounds,
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
  case kSM_AreaLoaded:
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
        GetMaterialFilter().GetIncludeList().Union(CMaterialList(kMT_AIBlock)),
        GetMaterialFilter().GetExcludeList()));
    break;
  }
  CActor::AcceptScriptMsg(mgr, msg);
}

void CAi::FluidFXThink(EFluidState state, CScriptWater& water, CStateManager& mgr) {
  switch (state) {
  case kFS_EnteredFluid:
  case kFS_LeftFluid:
    if (mgr.GetFluidPlaneManager()->GetLastSplashDeltaTime(GetUniqueId()) >= 0.2f) {
      const float energy = 0.5f * GetMass() * GetVelocityWR().MagSquared();
      if (energy > 500.f) {
        const float clampedEnergy = rstl::min_val(30000.f, energy);
        const CVector3f pos(GetTranslation().GetX(), GetTranslation().GetY(),
                            water.GetTriggerBoundsWR().GetMaxPoint().GetZ());
        mgr.GetFluidPlaneManager()->CreateSplash(GetUniqueId(), mgr, water, pos,
                                                 0.1f + 0.4f * (clampedEnergy - 500.f) / 29500.f,
                                                 true);
      }
    }
    break;
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
