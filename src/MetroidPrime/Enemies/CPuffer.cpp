#include "MetroidPrime/Enemies/CPuffer.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPuffer.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CDamageEffect.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "REL/REL_Setup.h"

static EMaterialTypes SolidMaterial = kMT_Solid;

static const char* skGasJetLocators[] = {
    "GasJet01", "GasJet02", "GasJet03", "GasJet04", "GasJet05", "GasJet06", "GasJet07",
    "GasJet08", "GasJet09", "GasJet10", "GasJet11", "GasJet12", "GasJet13", "GasJet14",
};

static const char* skGasLocators[] = {
    "Gas_01_LCTR", "Gas_02_LCTR", "Gas_03_LCTR", "Gas_04_LCTR", "Gas_05_LCTR",
    "Gas_06_LCTR", "Gas_07_LCTR", "Gas_08_LCTR", "Gas_09_LCTR", "Gas_10_LCTR",
    "Gas_11_LCTR", "Gas_12_LCTR", "Gas_13_LCTR", "Gas_14_LCTR",
};

static EMaterialTypes skIncludeMaterial = kMT_Player;
static EMaterialTypes skExcludeMaterial = kMT_NoStaticCollision;

CPuffer::CPuffer(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                 const CTransform4f& xf, const CModelData& modelData,
                 const CActorParameters& actorParameters, const CPatternedInfo& patternedInfo,
                 float hoverSpeed, CAssetId cloudEffect, const CDamageInfo& cloudDamage,
                 CAssetId cloudSteam, float cloudSteamAlpha, bool cloudInCombatOrScan,
                 bool cloudInDark, bool cloudInEcho, const CDamageInfo& explosionDamage,
                 ushort sfxId)
: CPatterned(static_cast< EPatternedAI >(45), uid, name, kFT_Zero, info, xf, modelData,
             patternedInfo, kMT_Flyer, kCT_One, static_cast< EBodyType >(5), actorParameters)
, mFace(xf.GetColumn(kDY))
, mCloudEffect(gpSimplePool->GetObj(SObjectTag('PART', cloudEffect)))
, mCloudDamage(cloudDamage)
, mCloudInCombatOrScan(cloudInCombatOrScan)
, mCloudInEcho(cloudInEcho)
, mCloudInDark(cloudInDark)
, mSfxId(sfxId)
, mExplosionDamage(explosionDamage)
, mCloudSteamAlpha(cloudSteamAlpha)
, mCloudSteam(cloudSteam)
, mMove(CVector3f::Zero())
, mLastDestObj(kInvalidUniqueId)
, mEnabledParticles(0) {
  SetDrawShadow(false);
  KnockBackController().SetPhysicsKnockBackType(CKnockBackMgr::EPhysicsKnockBackType(1));
  mCloudEffect.Lock();
  BodyController()->SetRestrictedFlyerMoveSpeed(hoverSpeed);
}

CPuffer::~CPuffer() {}

void CPuffer::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CPatterned::AcceptScriptMsg(mgr, msg);

  switch (message) {
  case kSM_Create:
    BodyController()->Activate(mgr, pas::kAS_Invalid);
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(CMaterialList(skIncludeMaterial),
                                                          CMaterialList(skExcludeMaterial)));
    break;
  case kSM_Action:
    if (GetActive()) {
      SetPendingDeath(true);
    }
    break;
  default:
    break;
  }
}

void CPuffer::Touch(CActor& actor, CStateManager& mgr) {
  CPatterned::Touch(actor, mgr);

  if (GetAlive()) {
    if (CPlayer* player = TCastToPtr< CPlayer >(actor)) {
      if (player->GetUniqueId() == actor.GetUniqueId()) {
        SetPendingDeath(true);
      }
    }
  }
}

rstl::optional_object< CAABox > CPuffer::GetTouchBounds() const {
  rstl::optional_object< CAABox > touchBounds = CPatterned::GetTouchBounds();
  if (touchBounds) {
    CAABox box = *touchBounds;
    box.AccumulateBounds(box.GetMinPoint() - CVector3f(0.5f, 0.5f, 0.5f));
    box.AccumulateBounds(box.GetMaxPoint() + CVector3f(0.5f, 0.5f, 0.5f));
    return box;
  }
  return touchBounds;
}

void CPuffer::Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) {
  CPatterned::Death(mgr, direction, state);

  mgr.ApplyDamageToWorld(
      GetUniqueId(), *this, GetTranslation(), mExplosionDamage,
      CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList()));

  TUniqueId uid = mgr.AllocateUniqueId();
  CAABox aabb =
      CAABox(CVector3f(-1.f, -1.f, -1.f), CVector3f(1.f, 1.f, 1.f))
          .GetTransformedAABox(GetTransform() * CTransform4f::Scale(mCloudDamage.GetRadius()));
  mgr.AddObject(rs_new CDamageEffect(mCloudEffect, uid, GetCurrentAreaId(), true, GetUniqueId(),
                                     GetTransform(), mCloudDamage, aabb, 1.f,
                                     CVector3f(1.f, 1.f, 1.f), true, mCloudSteamAlpha,
                                     mCloudSteam, 1.f, 1.f, mCloudInCombatOrScan, mCloudInDark,
                                     mCloudInEcho));
}

void CPuffer::Think(float dt, CStateManager& mgr) {
  CPatterned::Think(dt, mgr);

  UpdateJets(mgr);
  CVector3f moveVector = BodyController()->GetCommandMgr().GetMoveVector();

  if (mLastDestObj != GetDestObj()) {
    mLastDestObj = GetDestObj();
    const ushort sfx = mSfxId;
    CSfxManager::AddEmitter(sfx, GetTranslation(), GetCurrentAreaId().Value(), true, false);
  }

  BodyController()->CommandMgr().ClearLocomotionCmds();
  if (moveVector.CanBeNormalized()) {
    mMove = CVector3f::Lerp(mMove, moveVector, dt / 0.5f).AsNormalized();
    BodyController()->CommandMgr().DeliverCmd(CBCLocomotionCmd(mMove, mFace, 1.f));
  }
}

void CPuffer::UpdateJets(CStateManager& mgr) {
  const CVector3f moveVector = BodyController()->GetCommandMgr().GetMoveVector();

  if (mGasLocators.empty()) {
    for (int i = 0; i < ARRAY_SIZE(skGasLocators); ++i) {
      mGasLocators.push_back(
          ModelData()->AnimationData()->GetLocatorSegId(rstl::string_l(skGasLocators[i])));
    }
  }

  if (moveVector.CanBeNormalized()) {
    const CVector3f moveNorm = -moveVector.AsNormalized();
    CAnimData* animData = ModelData()->AnimationData();
    bool enable;
    for (int i = 0; i < ARRAY_SIZE(skGasJetLocators); ++i) {
      CVector3f offset = GetTransform().Rotate(
          animData->GetLocatorTransform(mGasLocators[i], nullptr).GetColumn(kDY));
      const float ang = CMath::FastCosR(1.0471976f);
      enable = CVector3f::Dot(moveNorm, offset) > ang;
      const bool isEnabled = IsParticleEnabled(i);
      if (isEnabled != enable) {
        ModelData()->AnimationData()->SetEffectState(rstl::string_l(skGasJetLocators[i]), enable,
                                                     mgr);
      }
      SetParticleEnabled(i, enable);
    }
  } else {
    for (int i = 0; i < ARRAY_SIZE(skGasJetLocators); ++i) {
      if (IsParticleEnabled(i)) {
        ModelData()->AnimationData()->SetEffectState(rstl::string_l(skGasJetLocators[i]), false,
                                                     mgr);
      }
    }
    mEnabledParticles = 0;
  }
}

CEntity* LoadPuffer(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrPuffer sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrPuffer.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  return rs_new CPuffer(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      *modelData, LdrToActorParameters(sldrThis.actorInformation),
      LdrToPatternedInfo(sldrThis.patterned, nullptr), sldrThis.hoverSpeed, sldrThis.cloudEffect,
      LdrToDamageInfo(sldrThis.cloudDamage), sldrThis.cloudSteam, sldrThis.cloudSteamAlpha,
      sldrThis.cloudInCombatOrScan, sldrThis.cloudInDark, sldrThis.cloudInEcho,
      LdrToDamageInfo(sldrThis.explosionDamage), sldrThis.sound_Turn);
}

static void SetFuncPtrs() {
  static SPuffer_FuncPtrs funcPtrs;
  funcPtrs.mLoadPuffer = &LoadPuffer;
  SetSPuffer_FuncPtrs(&funcPtrs);
}

void RELMain() { SetFuncPtrs(); }

void RELExit() { SetSPuffer_FuncPtrs(nullptr); }
