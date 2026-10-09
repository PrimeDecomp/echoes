#include "MetroidPrime/Enemies/CKrocuss.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CDecalManager.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrKrocuss.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "REL/REL_Setup.h"
#include "Weapons/CDecal.hpp"

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"WingsOpen", static_cast< CPatterned::StateMachine::TriggerFunc >(&CKrocuss::WingsOpen)},
    {"WingsOpening", static_cast< CPatterned::StateMachine::TriggerFunc >(&CKrocuss::WingsOpening)},
    {"WingsClosed", static_cast< CPatterned::StateMachine::TriggerFunc >(&CKrocuss::WingsClosed)},
    {"WingsClosing", static_cast< CPatterned::StateMachine::TriggerFunc >(&CKrocuss::WingsClosing)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Patrol", static_cast< CPatterned::StateMachine::StateFunc >(&CKrocuss::Patrol)},
    {"Sniff", static_cast< CPatterned::StateMachine::StateFunc >(&CKrocuss::Sniff)},
    {"Stop", static_cast< CPatterned::StateMachine::StateFunc >(&CKrocuss::Stop)},
};

static CVector3f skWingLightOffset(0.f, 0.f, 1.f); // Guessed name

CKrocuss::CKrocuss(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                   const CTransform4f& xf, const CModelData& modelData,
                   const CPatternedInfo& patternedInfo, const CActorParameters& actorParams,
                   const CColor& wingLightColor, const CDamageVulnerability& closedVulnerability,
                   CAssetId decalDescription, ushort openSound, ushort closeSound,
                   float animSpeedScalar, float timeShellClosed, float timeToOpenShell,
                   float timeShellOpen, float timeToCloseShell, float timeToCloseShellDamaged,
                   float maxAudibleDistance)
: CPatterned(kPAI_Krocuss, uid, name, kFT_Zero, info, xf, modelData, patternedInfo, kMT_Ground,
             kCT_Zero, kBT_BiPedal, actorParams)
, mWingState(kWS_Closed)
, mTimeShellClosed(timeShellClosed)
, mTimeToOpenShell(timeToOpenShell)
, mTimeShellOpen(timeShellOpen)
, mTimeToCloseShell(timeToCloseShell)
, mTimeToCloseShellDamaged(timeToCloseShellDamaged)
, mWingStateTime(0.f)
, mWingAnimation(0)
, mAnimSpeedScalar(animSpeedScalar)
, mUnusedId(kInvalidUniqueId)
, mMaxAudibleDistance(maxAudibleDistance)
, mDecal(decalDescription == kInvalidAssetId
             ? nullptr
             : rs_new TCachedToken< CDecalDescription >(
                   gpSimplePool->GetObj(SObjectTag('DPSC', decalDescription)), true))
, mUnusedFloat(0.f)
, mOpenVulnerability(patternedInfo.GetDamageVulnerability())
, mClosedVulnerability(closedVulnerability)
, mWingLightColor(wingLightColor)
, mWingLightOffset(skWingLightOffset * GetModelData()->GetScale())
, mWingLightId(kInvalidUniqueId)
, mOpenSound(openSound)
, mCloseSound(closeSound)
, mClosedByDamage(false) {
  SetDrawShadow(false);
  mSpeed = mAnimSpeedScalar;
  const CPASDatabase& database = GetAnimationData()->GetPASDatabase();
  const CPASAnimParmData parms(pas::kAS_Unknown26);
  const rstl::pair< float, int > anim = database.FindBestAnimation(parms, -1);
  if (anim.first > FLT_EPSILON) {
    mWingAnimation = anim.second;
  }
  KnockBackController().EnableKnockBackPhysics(false);
}

CKrocuss::~CKrocuss() {}

void CKrocuss::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    break;
  }
  CPatterned::Patrol(mgr, msg, dt);
}

void CKrocuss::Stop(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    break;
  }
}

void CKrocuss::Sniff(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->SetLocomotionType(pas::kLT_Combat);
    break;
  case kStateMsg_Deactivate:
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    break;
  default:
    break;
  }
}

bool CKrocuss::WingsOpen(CStateManager& mgr, const CTriggerData& data) const {
  return mWingState == kWS_Open;
}

bool CKrocuss::WingsOpening(CStateManager& mgr, const CTriggerData& data) const {
  return mWingState == kWS_Opening;
}

bool CKrocuss::WingsClosed(CStateManager& mgr, const CTriggerData& data) const {
  return mWingState == kWS_Closed;
}

bool CKrocuss::WingsClosing(CStateManager& mgr, const CTriggerData& data) const {
  return mWingState == kWS_Closing;
}

void CKrocuss::SetupStateMachine(CStateManager& mgr) {
  StateMachine* stateMachine = mStateMachine.get();
  CPatterned::SetupStateMachine(mgr);
  stateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
  stateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
}

void CKrocuss::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Create:
    if (!BodyController()->GetIsActive()) {
      BodyController()->SetLocomotionType(pas::kLT_Relaxed);
      BodyController()->Activate(mgr, pas::kAS_Invalid);
    }
    break;
  case kSM_Damage:
  case kSM_ResistedDamage:
  case kSM_ReflectedDamage:
    if (mWingState == kWS_Opening) {
      mWingState = kWS_Closing;
      mWingStateTime = mTimeToCloseShell * (1.f - mWingStateTime / mTimeToOpenShell);
      mClosedByDamage = true;
    }
    break;
  case kSM_Delete:
    mgr.DeleteObjectRequest(mWingLightId);
    break;
  case kSM_Increment:
  case kSM_Decrement:
  case kSM_AreaLoaded:
  case kSM_AIUpdateDisabled:
    break;
  default:
    break;
  }
  CPatterned::AcceptScriptMsg(mgr, msg);
}

void CKrocuss::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    if (CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(mWingLightId))) {
      const CTransform4f lightXf = GetTransform() * CTransform4f::Translate(mWingLightOffset);
      light->SetTransform(lightXf);
    }
    CPatterned::Think(dt, mgr);
    mWingStateTime += dt;
    float weight = 0.f;
    switch (mWingState) {
    case kWS_Closed:
      if (mWingStateTime >= mTimeShellClosed) {
        mWingStateTime = 0.f;
        mWingState = kWS_Opening;
        ProcessSoundEvent(mOpenSound, 1.f, 0, 0.1f, mMaxAudibleDistance, CSegId(0), 0, 0, 0.f, 20,
                          127, GetDistanceToCamera(mgr), GetTranslation(),
                          mgr.GetNextAreaId().Value(), mgr, true);
      }
      break;
    case kWS_Opening: {
      weight = CMath::Min(1.f, mWingStateTime / mTimeToOpenShell);
      if (CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(mWingLightId))) {
        light->SetActive(true);
        light->SetLight(
            CLight::BuildPoint(GetTransform() * mWingLightOffset,
                               CColor::Lerp(CColor(0.f, 0.f, 0.f, 0.f), mWingLightColor, weight)));
      }
      if (mWingStateTime >= mTimeToOpenShell) {
        mWingStateTime = 0.f;
        mWingState = kWS_Open;
      }
      break;
    }
    case kWS_Open:
      weight = 1.f;
      if (mWingStateTime >= mTimeShellOpen) {
        mWingStateTime = 0.f;
        mWingState = kWS_Closing;
        ProcessSoundEvent(mCloseSound, 1.f, 0, 0.1f, mMaxAudibleDistance, CSegId(0), 0, 0, 0.f, 20,
                          127, GetDistanceToCamera(mgr), GetTranslation(),
                          mgr.GetNextAreaId().Value(), mgr, true);
      }
      break;
    case kWS_Closing: {
      weight =
          1.f - mWingStateTime / (mClosedByDamage ? mTimeToCloseShellDamaged : mTimeToCloseShell);
      weight = weight < 0.f ? 0.f : weight;
      if (CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(mWingLightId))) {
        light->SetActive(false);
        light->SetLight(
            CLight::BuildPoint(GetTransform() * mWingLightOffset,
                               CColor::Lerp(CColor(0.f, 0.f, 0.f, 0.f), mWingLightColor, weight)));
      }
      if (mWingStateTime >= mTimeToCloseShell) {
        mWingStateTime = 0.f;
        mWingState = kWS_Closed;
        mClosedByDamage = false;
        if (CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(mWingLightId))) {
          light->SetActive(false);
        }
      }
      break;
    }
    }
    AnimationData()->AddAdditiveAnimation(mWingAnimation, weight, false, false);
  }
}

void CKrocuss::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                               float dt) {
  switch (type) {
  case kUE_EggLay: {
    const CTransform4f decalXf =
        GetTransform() * CTransform4f::RotateX(CRelAngle::FromDegrees(90.f));
    CDecalManager::AddDecal(*mDecal, decalXf, CUnitVector3f(GetTransform().GetUp()), mgr);
    break;
  }
  default:
    break;
  }
}

void CKrocuss::Render(const CStateManager& mgr) const { CPatterned::Render(mgr); }

void CKrocuss::PreRender(CStateManager& mgr) { CPatterned::PreRender(mgr); }

void CKrocuss::AddToRenderer(const CStateManager& mgr) const { CPatterned::AddToRenderer(mgr); }

const CDamageVulnerability* CKrocuss::GetDamageVulnerability() const { return &mOpenVulnerability; }

const CDamageVulnerability* CKrocuss::GetDamageVulnerability(const CVector3f& position,
                                                             const CVector3f& direction,
                                                             const CDamageInfo& damage) const {
  const CVector3f toTarget = position - GetTranslation();
  if (toTarget.CanBeNormalized()) {
    const CVector3f forward = GetTransform().GetForward();
    const CVector3f dir = toTarget.AsNormalized();
    const float facing = CVector3f::Dot(dir, forward);
    if (mWingState == kWS_Open && facing < -0.5f) {
      return &mOpenVulnerability;
    }
  }
  return &mClosedVulnerability;
}

CEntity* LoadKrocuss(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrKrocuss sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrKrocuss.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  return rs_new CKrocuss(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      *modelData, LdrToPatternedInfo(sldrThis.patterned, nullptr),
      LdrToActorParameters(sldrThis.actorInformation), sldrThis.wingLightColor,
      LdrToDamageVulnerability(sldrThis.shellClosedVulnerability), sldrThis.dPSC,
      sldrThis.shellOpenSound, sldrThis.shellCloseSound, sldrThis.animSpeedScalar,
      sldrThis.timeShellClosed, sldrThis.timeToOpenShell, sldrThis.timeShellOpen,
      sldrThis.timeToCloseShell, sldrThis.unknown_0xbbebed9e, sldrThis.maxAudibleDistance);
}

#ifndef MONOLITHIC
static void SetFuncPtrs() {
  static SKrocuss_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &LoadKrocuss;
  SetSKrocuss_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSKrocuss_FuncPtrs(nullptr); }
#endif
