#include "MetroidPrime/ScriptObjects/CScriptPlayerActor.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Kyoto/Animation/CCharacterInfo.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "MetroidPrime/CActorModelParticles.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CAnimationParameters.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CEnvFxManager.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CProjectedShadow.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Factories/CCharacterFactory.hpp"
#include "MetroidPrime/Factories/CCharacterFactoryBuilder.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerGun.hpp"
#include "MetroidPrime/SEchoParameters.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPlayerActor.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerRes.hpp"
#include "REL/REL_Setup.h"

CScriptPlayerActor::CScriptPlayerActor(TUniqueId uid, const rstl::string& name, CEntityInfo& info,
                                       const CTransform4f& xf, const CAnimRes& animRes,
                                       const CModelData& modelData, int characterCount,
                                       const CAABox& bounds, bool setBoundingBox,
                                       const CMaterialList& materials, float mass, float zMomentum,
                                       const CHealthInfo& health,
                                       const CDamageVulnerability& vulnerability,
                                       const CActorParameters& parameters, bool loop, uint flags,
                                       CPlayerState::EBeamId beam, bool usePlayerBeamModel)
: CScriptActor(uid, name, info, xf, modelData, bounds, materials, mass, zMomentum, health,
               vulnerability, parameters, SEchoParameters::None(), loop, 0, false, false, false,
               0.f, kInvalidAssetId, CDamageInfo(), kInvalidAssetId)
, mSuitRes(animRes)
, mBeam(beam)
, mSuit(CPlayerState::kPS_Invalid)
, mPreviousSetBeamId(CPlayerState::kBI_Invalid)
, mSetBeamId(CPlayerState::kBI_Invalid)
, mLoadedCharIdx(-1)
, mBeamModelData(nullptr)
, mGrappleModelData(nullptr)
, mSuitModelData(nullptr)
, mBeamModel(nullptr)
, mGrappleModel(nullptr)
, mSuitModel(nullptr)
, mSuitSkin(nullptr)
, mBackupModelData(rstl::optional_object_null())
, mProjectedShadow(nullptr)
, mDeallocateBackupCountdown(0)
, mCharacterCount(characterCount)
, mFlags(flags)
, mSetBoundingBox(setBoundingBox)
, mDeferOnlineModelData(false)
, mDeferOfflineModelData(false)
, mBeamModelLoading(false)
, mGrappleModelLoading(false)
, mSuitModelLoading(false)
, mLoading(true)
, mEnableLoading(true)
, mDeferOnlineLoad(false)
, mAreaTrackingLoad(false)
, mUsePlayerBeamModel(usePlayerBeamModel)
, mWaitForIncrement((flags & 0x80) != 0)
, mSentArrived(false)
, mDrawGrapple((flags & 0x200) == 0)
, mNextPlayerActor(kInvalidUniqueId) {
  CMaterialList exclude = GetMaterialFilter().GetExcludeList();
  CMaterialList include = GetMaterialFilter().GetIncludeList();
  exclude.Add(kMT_Player);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include, exclude));
  SetActorLights(parameters.GetLighting().MakeActorLights());
  SetDrawEnabled(true);
  mIsPlayerActor = true;
}

void CScriptPlayerActor::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_AreaLoaded:
    mDeferOnlineLoad = true;
    if (mFlags & 8) {
      CGameArea* area = mgr.World()->Area(GetCurrentAreaId());
      ++area->GetPostConstructed()->mPlayerActorsLoading;
      mAreaTrackingLoad = true;
    }
    if (GetActive()) {
      SetupEnvFx(mgr, true);
      SetIntoStateManager(mgr, true);
    }
    break;
  case kSM_Activate:
    if (!GetActive()) {
      if (mFlags & 1) {
        LoadSuit(GetNextSuitCharIdx(mgr));
      }
      SetIntoStateManager(mgr, true);
      SetupEnvFx(mgr, true);
      mEnableLoading = true;
    }
    break;
  case kSM_Left:
    mDrawGrapple = true;
    break;
  case kSM_Increment:
    if (mWaitForIncrement) {
      mWaitForIncrement = false;
      mSentArrived = false;
      return;
    }
    if (mFlags & 1) {
      mDeferOnlineModelData = false;
      mDeferOfflineModelData = true;
      mgr.GetPlayer(0)->AsyncLoadSuit(mgr);
      mSentArrived = false;
      return;
    }
    break;
  case kSM_Deactivate:
    if (GetActive()) {
      if (!(mFlags & 0x10)) {
        SetIntoStateManager(mgr, false);
      }
      SetupEnvFx(mgr, false);
    }
    if (!(mFlags & 4)) {
      break;
    }
  case kSM_Reset:
    if (GetActive() || msg.GetMessage() == kSM_Reset) {
      mPreviousSetBeamId = CPlayerState::kBI_Invalid;
      mSetBeamId = CPlayerState::kBI_Invalid;
      mLoadedCharIdx = -1;
      mBeamModelData = nullptr;
      mGrappleModelData = nullptr;
      mSuitModelData = nullptr;
      mBeamModel = nullptr;
      mSuitModel = nullptr;
      mSuitSkin = nullptr;
      mBackupModelData = rstl::optional_object_null();
      mDeallocateBackupCountdown = 0;
      mFlags &= ~1;
      mDeferOnlineModelData = false;
      mDeferOfflineModelData = false;
      mBeamModelLoading = false;
      mGrappleModelLoading = false;
      mSuitModelLoading = false;
      mEnableLoading = false;
      SetModelData(CModelData::None(), mgr);
      SetActive(false);
    }
    break;
  case kSM_Delete:
    SetIntoStateManager(mgr, false);
    break;
  default:
    break;
  }
  CScriptActor::AcceptScriptMsg(mgr, msg);
}

void CScriptPlayerActor::Think(float dt, CStateManager& mgr) {
  CPlayer* player = mgr.GetPlayer(0);
  CPlayerState* playerState = player->GetPlayerState();
  if (mDeferOnlineLoad) {
    if (mWaitForIncrement) {
      return;
    }
    mDeferOnlineModelData = true;
    mDeferOnlineLoad = false;
    mSuit = playerState->GetCurrentSuitRaw();
    LoadSuit(GetSuitCharIdx(mgr, mSuit));
  }
  if (mEnableLoading) {
    if (!(mFlags & 1) && !(mFlags & 0x400)) {
      int charIdx = GetSuitCharIdx(mgr, playerState->GetCurrentSuitRaw());
      if (charIdx != mLoadedCharIdx) {
        SetModelData(CModelData::None(), mgr);
        LoadSuit(charIdx);
        mDeferOnlineModelData = true;
      }
    }
    UpdateGrappleModel(mgr);
    if (mUsePlayerBeamModel) {
      if (mBeamModelData.get() == nullptr) {
        UsePlayerBeamModel(*player);
      }
    } else if (mBeam != CPlayerState::kBI_Invalid) {
      LoadBeam(mBeam);
    } else if (HasGunModelData() && player->GetCameraManager()->IsInCinematicCamera()) {
      CancelBeamLoad();
    } else {
      LoadBeam(player->GetPlayerGun()->GetPrimaryWeaponId());
    }
    if (mBeamModelLoading) {
      PumpBeamModel(mgr);
    }
    if (mGrappleModelLoading) {
      PumpGrappleModel(mgr);
    }
    if (mSuitModelLoading) {
      PumpSuitModel(mgr);
    }
    if (!mLoading) {
      if (mSuitModelLoading || mBeamModelLoading || mGrappleModelLoading || !HasModelData() ||
          !GetModelData()->IsLoaded(0)) {
        mLoading = true;
      }
    }
    if (mLoading && !mSuitModelLoading && !mBeamModelLoading && !mGrappleModelLoading &&
        HasModelData() && GetModelData()->IsLoaded(0)) {
      if (mAreaTrackingLoad) {
        --mgr.World()->Area(GetCurrentAreaId())->GetPostConstructed()->mPlayerActorsLoading;
        mAreaTrackingLoad = false;
      }
      mLoading = false;
      if (!mSentArrived) {
        SendScriptMsgs(kSS_Arrived, mgr, kSM_None);
        mSentArrived = true;
      }
    }
  }
  CScriptActor::Think(dt, mgr);
}

CTransform4f CScriptPlayerActor::GetGunTransform() const {
  return GetTransform() * GetModelData()->GetScaledLocatorTransform(rstl::string_l(kGunLocator));
}

CTransform4f CScriptPlayerActor::GetGrappleTransform() const {
  return GetTransform() *
         GetModelData()->GetScaledLocatorTransform(rstl::string_l(kGrappleLocator));
}

void CScriptPlayerActor::RenderGrapple(const CStateManager& mgr) const {
  if (HasGrappleModelData() && HasModelData() && mDrawGrapple) {
    mGrappleModelData->Render(mgr, GetGrappleTransform(), GetActorLights(), GetModelFlags());
  }
}

void CScriptPlayerActor::RenderGun(const CStateManager& mgr) const {
  if (HasGunModelData() && HasModelData() && !(mFlags & 0x40)) {
    mBeamModelData->Render(mgr, GetGunTransform(), GetActorLights(), GetModelFlags());
  }
}

void CScriptPlayerActor::Render(const CStateManager& mgr) const {
  const bool gunFirst = (mFlags & 0x100) != 0;
  if (gunFirst) {
    RenderGun(mgr);
  }
  RenderGrapple(mgr);
  CScriptActor::Render(mgr);
  if (!gunFirst) {
    RenderGun(mgr);
  }
}

void CScriptPlayerActor::TouchModels(CStateManager& mgr) {
  TouchModels_Internal(mgr);
  TUniqueId id = mNextPlayerActor;
  while (id != kInvalidUniqueId) {
    const CScriptActor* actor = TCastToConstPtr< CScriptActor >(mgr.GetObjectById(id));
    if (actor && actor->IsPlayerActor()) {
      const CScriptPlayerActor* playerActor = static_cast< const CScriptPlayerActor* >(actor);
      playerActor->TouchModels_Internal(mgr);
      id = playerActor->mNextPlayerActor;
    } else {
      id = kInvalidUniqueId;
    }
  }
}

void CScriptPlayerActor::TouchModels_Internal(const CStateManager& mgr) const {
  if (mWaitForIncrement) {
    return;
  }
  if (HasModelData()) {
    GetModelData()->Touch(mgr, 0);
  }
  if (HasSuitModelData()) {
    mSuitModelData->Touch(mgr, 0);
  }
  if (!mBeamModelLoading && HasGunModelData()) {
    mBeamModelData->Touch(mgr, 0);
  }
  if (!mGrappleModelLoading && HasGrappleModelData()) {
    mGrappleModelData->Touch(mgr, 0);
  }
}

void CScriptPlayerActor::AddToRenderer(const CStateManager& mgr) const {
  if (!mWaitForIncrement) {
    TouchModels_Internal(mgr);
    if (GetActive()) {
      CScriptActor::AddToRenderer(mgr);
    }
  }
}

void CScriptPlayerActor::BuildBeamModelData() {
  const CStaticRes res(gpTweakPlayerRes->GetCinematicBeamResId(mSetBeamId), mSuitRes.GetScale());
  mBeamModelData = rs_new CModelData(res);
}

void CScriptPlayerActor::BuildGrappleModelData() {
  const CStaticRes res(gpTweakPlayerRes->GetCinematicGrappleResId(), mSuitRes.GetScale());
  mGrappleModelData = rs_new CModelData(res);
}

void CScriptPlayerActor::UsePlayerBeamModel(CPlayer& player) {
  mPreviousSetBeamId = mSetBeamId;
  mSetBeamId = CPlayerState::kBI_Invalid;
  mBeamModelData = rs_new CModelData(*player.BallTransitionBeamModel());
  mBeamModelData->SetScale(mSuitRes.GetScale());
  mBeamModelLoading = false;
}

void CScriptPlayerActor::SetupOnlineModelData(CStateManager& mgr) {
  if (mLoadedCharIdx == mSuitRes.GetCharacterNodeId() && HasModelData() &&
      GetModelData()->HasAnimation()) {
    return;
  }
  mSuitRes = CAnimRes(mSuitRes.GetId(), mLoadedCharIdx, mSuitRes.GetScale(),
                      mSuitRes.GetDefaultAnim(), mSuitRes.CanLoop());
  CModelData modelData(mSuitRes);
  SetModelData(modelData, mgr);
  const CAnimPlaybackParms parms(mSuitRes.GetDefaultAnim(), -1, 1.f, true);
  AnimationData()->SetAnimation(parms, false);
  if (mSetBoundingBox) {
    SetBoundingBox(GetModelData()->GetBounds(GetTransform().GetRotation()));
  }
}

void CScriptPlayerActor::SetupOfflineModelData() {
  mSuitRes = CAnimRes(mSuitRes.GetId(), mLoadedCharIdx, mSuitRes.GetScale(),
                      mSuitRes.GetDefaultAnim(), mSuitRes.CanLoop());
  mSuitModelData = rs_new CModelData(mSuitRes);
  if (!gpMain->IsMaxSpeed()) {
    mBackupModelData = GetAnimationData()->GetModelData();
    mDeallocateBackupCountdown = 2;
  }
  AnimationData()->SetSkinnedModel(mSuitModelData->GetAnimationData()->GetModelData());
}

void CScriptPlayerActor::LoadSuit(int charIdx) {
  if (charIdx == mLoadedCharIdx) {
    return;
  }
  TToken< CCharacterFactory > factory = gpCharacterFactoryBuilder->GetFactory(mSuitRes);
  const CCharacterInfo& charInfo = factory->GetCharInfo(charIdx);
  mSuitModel = rs_new TCachedToken< CModel >(
      gpSimplePool->GetObj(SObjectTag('CMDL', charInfo.GetModelId())));
  mSuitModel->Lock();
  mSuitSkin = rs_new TToken< CSkinRules >(
      gpSimplePool->GetObj(SObjectTag('CSKR', charInfo.GetSkinRulesId())));
  mSuitSkin->Lock();
  mSuitModelLoading = true;
  mLoadedCharIdx = charIdx;
}

void CScriptPlayerActor::LoadBeam(CPlayerState::EBeamId beam) {
  if (beam == mSetBeamId) {
    return;
  }
  CAssetId id = gpTweakPlayerRes->GetCinematicBeamResId(beam);
  mBeamModel = rs_new TToken< CModel >(gpSimplePool->GetObj(SObjectTag('CMDL', id)));
  mBeamModel->Lock();
  mBeamModelLoading = true;
  mPreviousSetBeamId = mSetBeamId;
  mSetBeamId = beam;
}

void CScriptPlayerActor::CancelBeamLoad() {
  if (mBeamModelLoading) {
    mBeamModel = nullptr;
    mBeamModelLoading = false;
    mSetBeamId = mPreviousSetBeamId;
  }
}

void CScriptPlayerActor::CancelGrappleLoad() {
  if (mGrappleModelLoading) {
    mGrappleModel = nullptr;
    mGrappleModelLoading = false;
  }
}

void CScriptPlayerActor::UpdateGrappleModel(CStateManager& mgr) {
  if (mgr.GetPlayerState(0)->ShouldDrawGrapple()) {
    if (!mGrappleModelLoading && mGrappleModel.get() == nullptr &&
        mGrappleModelData.get() == nullptr) {
      mGrappleModel = rs_new TToken< CModel >(
          gpSimplePool->GetObj(SObjectTag('CMDL', gpTweakPlayerRes->GetCinematicGrappleResId())));
      mGrappleModel->Lock();
      mGrappleModelLoading = true;
    }
  } else {
    mGrappleModel = nullptr;
    mGrappleModelData = nullptr;
    mGrappleModelLoading = false;
  }
}

void CScriptPlayerActor::PumpSuitModel(CStateManager& mgr) {
  if (mSuitModel.get() != nullptr && mSuitModel->TryCache() && mSuitSkin->IsLoaded()) {
    if (mSuitModel->IsLoaded()) {
      mSuitModel->GetObject()->Touch(0);
      mgr.World()->PauseAndUnpauseAreaLoading();
      bool didSetup = false;
      if (mDeferOfflineModelData) {
        didSetup = true;
        mDeferOfflineModelData = false;
        SetupOfflineModelData();
      } else if (mDeferOnlineModelData) {
        didSetup = true;
        mDeferOnlineModelData = false;
        SetupOnlineModelData(mgr);
      }
      if (didSetup) {
        mSuitModelLoading = false;
        mSuitModel = nullptr;
        mSuitSkin = nullptr;
      }
    }
  }
}

void CScriptPlayerActor::PumpBeamModel(CStateManager& mgr) {
  if (mBeamModel.get() != nullptr && mBeamModel->IsLoaded()) {
    BuildBeamModelData();
    mBeamModelData->Touch(mgr, 0);
    mgr.World()->PauseAndUnpauseAreaLoading();
    mBeamModel = nullptr;
    mBeamModelLoading = false;
  }
}

void CScriptPlayerActor::PumpGrappleModel(CStateManager& mgr) {
  if (mGrappleModel.get() != nullptr && mGrappleModel->IsLoaded()) {
    BuildGrappleModelData();
    mGrappleModelData->Touch(mgr, 0);
    mgr.World()->PauseAndUnpauseAreaLoading();
    mGrappleModel = nullptr;
    mGrappleModelLoading = false;
  }
}

int CScriptPlayerActor::GetNextSuitCharIdx(const CStateManager& mgr) const {
  CPlayerState::EPlayerSuit nextSuit = CPlayerState::kPS_Light;
  if (mFlags & 2) {
    if (mSuit == CPlayerState::kPS_Light) {
      nextSuit = CPlayerState::kPS_Dark;
    } else {
      nextSuit = CPlayerState::kPS_Varia;
    }
  } else {
    switch (mSuit) {
    case CPlayerState::kPS_Varia:
      nextSuit = CPlayerState::kPS_Dark;
      break;
    case CPlayerState::kPS_Dark:
      nextSuit = CPlayerState::kPS_Light;
      break;
    default:
      break;
    }
  }
  return GetSuitCharIdx(mgr, nextSuit);
}

int CScriptPlayerActor::GetSuitCharIdx(const CStateManager& mgr,
                                       CPlayerState::EPlayerSuit suit) const {
  int charIdx = CPlayerState::GetRenderSuit(mgr, *mgr.GetPlayerState(0), suit);
  if (charIdx >= mCharacterCount) {
    charIdx = suit;
    if (charIdx >= mCharacterCount) {
      charIdx = 0;
    }
  }
  return charIdx;
}

void CScriptPlayerActor::PreRender(CStateManager& mgr) {
  if (mBackupModelData) {
    if (mDeallocateBackupCountdown == 0) {
      mBackupModelData = rstl::optional_object_null();
    } else {
      --mDeallocateBackupCountdown;
    }
  }
  CScriptActor::PreRender(mgr);
  if (mProjectedShadow.get() != nullptr && HasModelData() && HasGunModelData()) {
    const CModelData* models[] = {GetModelData(), mBeamModelData.get(), mGrappleModelData.get()};
    const CTransform4f gunTransform = GetGunTransform();
    const CTransform4f grappleTransform = GetGrappleTransform();
    const CTransform4f* transforms[] = {&GetTransform(), &gunTransform, &grappleTransform};
    mProjectedShadow->RenderShadowBuffer(mgr, HasGrappleModelData() ? 3 : 2, models, transforms, 0,
                                         CVector3f::Zero(), 1.f, 20.f);
  }
}

void CScriptPlayerActor::SetActive(const bool active) {
  CActor::SetActive(active);
  SetDrawEnabled(true);
}

void CScriptPlayerActor::SetupEnvFx(CStateManager& mgr, bool set) {
  if (set) {
    const CEnvFxManager* envFx = mgr.GetEnvFxManager();
    if (mgr.GetWorld()->GetNeededEnvFx() == kEFX_Rain && HasModelData() &&
        envFx->GetRainMagnitude() != 0.f) {
      mgr.ActorModelParticles()->StartRainSplashes(*this, mgr, 250, 10, 1.2f);
    }
    if (!(mFlags & 0x20)) {
      mProjectedShadow = rs_new CProjectedShadow(128, 128, 1, 0);
      mProjectedShadow->SetOpacity(0.5f);
    }
  } else {
    mgr.ActorModelParticles()->StopRainSplashes(*this);
    mProjectedShadow = nullptr;
  }
}

void CScriptPlayerActor::SetIntoStateManager(CStateManager& mgr, bool set) {
  const TUniqueId selfId = GetUniqueId();
  if (!set && mgr.GetPlayerActorHead() == selfId) {
    mgr.SetPlayerActorHead(mNextPlayerActor);
    mNextPlayerActor = kInvalidUniqueId;
  } else {
    TUniqueId id = mgr.GetPlayerActorHead();
    CScriptPlayerActor* previous = nullptr;
    while (id != kInvalidUniqueId) {
      if (id == selfId) {
        if (!set && previous) {
          previous->mNextPlayerActor = mNextPlayerActor;
          mNextPlayerActor = kInvalidUniqueId;
        }
        return;
      }
      CScriptActor* actor = TCastToPtr< CScriptActor >(mgr.ObjectById(id));
      if (actor && actor->IsPlayerActor()) {
        previous = static_cast< CScriptPlayerActor* >(actor);
        id = previous->mNextPlayerActor;
      } else {
        id = kInvalidUniqueId;
        mNextPlayerActor = kInvalidUniqueId;
      }
    }
    if (set) {
      mNextPlayerActor = mgr.GetPlayerActorHead();
      mgr.SetPlayerActorHead(selfId);
    }
  }
}

CEntity* REL_LoadPlayerActor(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrPlayerActor sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrPlayerActor.inc"

  const CAnimationParameters animParams = LdrToAnimationParameters(sldrThis.animationInformation);
  if (gpResourceFactory->GetResourceTypeById(animParams.GetACSFile()) == 0) {
    return nullptr;
  }

  const CAnimRes animRes(animParams.GetACSFile(), 3, sldrThis.editorProperties.transform.scale,
                         animParams.GetInitialAnimation(), sldrThis.isLoop);
  TToken< CCharacterFactory > factory = gpCharacterFactoryBuilder->GetFactory(animRes);
  const int characterCount = factory->GetCharacterCount();

  CMaterialList materials;
  if (sldrThis.immovable) {
    materials.Add(kMT_Immovable);
  }
  if (sldrThis.isSolid) {
    materials.Add(kMT_Solid);
  }

  const CAABox bounds =
      LoadCAABox(mgr, info.GetAreaId(), sldrThis.collisionBox, sldrThis.collisionOffset);

  CPlayerState::EBeamId beam = CPlayerState::kBI_Invalid;
  switch (sldrThis.renderGunOverride) {
  case 1:
    beam = CPlayerState::kBI_Power;
    break;
  case 2:
    beam = CPlayerState::kBI_Dark;
    break;
  case 3:
    beam = CPlayerState::kBI_Light;
    break;
  case 4:
    beam = CPlayerState::kBI_Annihilator;
    break;
  default:
    break;
  }
  const bool usePlayerBeamModel = sldrThis.renderGunOverride - 1 > CPlayerState::kBI_Annihilator;

  return rs_new CScriptPlayerActor(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      animRes, CModelData::None(), characterCount, bounds, true, materials, sldrThis.mass,
      sldrThis.gravity, LdrToHealthInfo(sldrThis.health),
      LdrToDamageVulnerability(sldrThis.vulnerability),
      LdrToActorParameters(sldrThis.actorInformation), sldrThis.isLoop, sldrThis.flagsPlayerActor,
      beam, usePlayerBeamModel);
}

static void SetFuncPtrs() {
  static SPlayerActor_FuncPtrs funcPtrs;
  funcPtrs.mLoadPlayerActor = &REL_LoadPlayerActor;
  funcPtrs.mTouchModels =
      static_cast< void (CEntity::*)(CStateManager&) >(&CScriptPlayerActor::TouchModels);
  SetSPlayerActor_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSPlayerActor_FuncPtrs(nullptr); }
