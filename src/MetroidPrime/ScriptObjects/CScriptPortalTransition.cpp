#include "MetroidPrime/ScriptObjects/CScriptPortalTransition.hpp"

#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CPortalTransition.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCameraFilterPass.hpp"
#include "MetroidPrime/Factories/CCharacterFactory.hpp"
#include "MetroidPrime/Factories/CCharacterFactoryBuilder.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPortalTransition.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCamera.hpp"
#include "MetroidPrime/ScriptObjects/CScriptEffect.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakGame.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerRes.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "rstl/math.hpp"

CScriptPortalTransition::CScriptPortalTransition(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info, int direction,
    const CAnimRes& samusRes, CAssetId soundGroupCommon, CAssetId soundGroupDirectional,
    ushort startPortal, ushort inPortal1, ushort inPortal2, uchar volume, uchar pan)
: CEntity(uid, info, name, 0)
, mDirection(direction)
, mSamusRes(samusRes)
, mSoundGroupCommon(soundGroupCommon)
, mSoundGroupDirectional(soundGroupDirectional)
, mStartPortal(startPortal)
, mInPortal1(inPortal1)
, mInPortal2(inPortal2)
, mVolume(volume)
, mPan(pan) {}

CToken CScriptEffect::GetDescription() const { return *mDescription; }

rstl::single_ptr< CPortalTransition >
CScriptPortalTransition::CreateTransition(CStateManager& mgr) const {
  const CActor* actor = TCastToConstPtr< CActor >(
      mgr.GetObjectById(FindConnectedObject(mgr, kSS_InternalState00, kSM_None)));
  const CScriptEffect* firstEffect = TCastToConstPtr< CScriptEffect >(
      mgr.GetObjectById(FindConnectedObject(mgr, kSS_InternalState03, kSM_None)));
  const CScriptEffect* secondEffect = TCastToConstPtr< CScriptEffect >(
      mgr.GetObjectById(FindConnectedObject(mgr, kSS_InternalState04, kSM_None)));
  const CScriptCamera* firstCamera = TCastToConstPtr< CScriptCamera >(
      mgr.GetObjectById(FindConnectedObject(mgr, kSS_InternalState05, kSM_None)));
  const CScriptCamera* secondCamera = TCastToConstPtr< CScriptCamera >(
      mgr.GetObjectById(FindConnectedObject(mgr, kSS_InternalState06, kSM_None)));

  rstl::optional_object< CToken > firstDescription;
  if (firstEffect != nullptr) {
    firstDescription = firstEffect->GetDescription();
  }
  rstl::optional_object< CToken > secondDescription;
  if (secondEffect != nullptr) {
    secondDescription = secondEffect->GetDescription();
  }
  rstl::optional_object< CToken > commonSound(
      gpSimplePool->GetObj(SObjectTag('AGSC', mSoundGroupCommon)));
  rstl::optional_object< CToken > directionalSound(
      gpSimplePool->GetObj(SObjectTag('AGSC', mSoundGroupDirectional)));

  int suitCharIdx = gpGameState->GetPlayerState()->ShouldDrawGravityBoost(mgr);
  {
    TLockedToken< CCharacterFactory > factory(gpCharacterFactoryBuilder->GetFactory(mSamusRes));
    const int characterCount = factory->GetCharacterCount();
    if (suitCharIdx >= characterCount) {
      suitCharIdx = gpGameState->GetPlayerState()->GetCurrentSuitRaw();
      if (suitCharIdx >= characterCount) {
        suitCharIdx = 0;
      }
    }
  }

  return rstl::single_ptr< CPortalTransition >(rs_new CPortalTransition(
      mSamusRes, suitCharIdx, gpGameState->GetPlayerState()->ShouldDrawGrapple(),
      actor != nullptr ? actor->GetTransform() : CTransform4f::Identity(), firstDescription,
      firstEffect != nullptr ? firstEffect->GetTransform() : CTransform4f::Identity(),
      firstEffect != nullptr ? firstEffect->GetGlobalScale() : CVector3f::One(), secondDescription,
      secondEffect != nullptr ? secondEffect->GetTransform() : CTransform4f::Identity(),
      secondEffect != nullptr ? secondEffect->GetGlobalScale() : CVector3f::One(),
      firstCamera != nullptr ? &firstCamera->GetSpline() : nullptr,
      secondCamera != nullptr ? &secondCamera->GetSpline() : nullptr, CTransform4f::Identity(),
      commonSound, directionalSound, mStartPortal, mInPortal1, mInPortal2, mVolume, mPan,
      mDirection));
}

CEntity* LoadPortalTransition(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  // TODO: Native sound-ID defaults are -1; the templates currently generate zero.
  SLdrPortalTransition sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrPortalTransition.inc"

  const CAnimRes samusRes(sldrThis.animationInformation.ancs,
                          sldrThis.animationInformation.character_index, sldrThis.playerScale,
                          sldrThis.animationInformation.initial_anim, true);
  return rs_new CScriptPortalTransition(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), sldrThis.direction, samusRes,
      sldrThis.portalSoundGroupCommon, sldrThis.portalSoundGroupDirectional, sldrThis.startPortal,
      sldrThis.inPortal1, sldrThis.inPortal2, sldrThis.volume, sldrThis.pan);
}

CPortalTransition::CPortalTransition(
    const CAnimRes& samusRes, int suitCharIdx, bool renderGrapple,
    const CTransform4f& samusTransform, rstl::optional_object< CToken > firstEffectDescription,
    const CTransform4f& firstEffectTransform, const CVector3f& firstEffectScale,
    rstl::optional_object< CToken > secondEffectDescription,
    const CTransform4f& secondEffectTransform, const CVector3f& secondEffectScale,
    const CGameCameraSpline* firstPassCamera, const CGameCameraSpline* secondPassCamera,
    const CTransform4f& cameraTransform, rstl::optional_object< CToken > soundGroupCommon,
    rstl::optional_object< CToken > soundGroupDirectional, ushort startPortal, ushort inPortal1,
    ushort inPortal2, uchar volume, uchar pan, int direction)
: mCurTime(0.f)
, mDirection(direction)
, mRandom(99)
, mSamusRes(samusRes)
, mSuitCharIdx(suitCharIdx)
, mSamusModelData(samusRes)
, mModelFlags(CModelFlags::Normal())
, mSamusTransform(samusTransform)
, mBeamModelData(CModelData::CModelDataNull())
, mGrappleModelData(CModelData::CModelDataNull())
, mGunTransform(CTransform4f::Identity())
, mGrappleTransform(CTransform4f::Identity())
, mFirstEffectDescription(firstEffectDescription)
, mFirstEffectTransform(firstEffectTransform)
, mSecondEffectDescription(secondEffectDescription)
, mSecondEffectTransform(secondEffectTransform)
, mCameraPass(kCP_None)
, mCameraTransform(cameraTransform)
, mSoundGroupCommon(soundGroupCommon)
, mSoundGroupDirectional(soundGroupDirectional)
, mStartPortal(startPortal)
, mInPortal1(inPortal1)
, mInPortal2(inPortal2)
, mVolume(volume)
, mPan(pan) {
  mSamusModelData.AnimationData()->SetAnimation(
      CAnimPlaybackParms(samusRes.GetDefaultAnim(), -1, 1.f, true), false);
  if (mSoundGroupCommon) {
    mSoundGroupCommon->Lock();
  }
  if (mSoundGroupDirectional) {
    mSoundGroupDirectional->Lock();
  }

  mBeamModel = gpSimplePool->GetObj(
      SObjectTag('CMDL', gpTweakPlayerRes->GetCinematicBeamResId(CPlayerState::kBI_Power)));
  mBeamModel->Lock();
  if (renderGrapple) {
    mGrappleModel =
        gpSimplePool->GetObj(SObjectTag('CMDL', gpTweakPlayerRes->GetCinematicGrappleResId()));
    mGrappleModel->Lock();
  }
  mCharacterFactory =
      TLockedToken< CCharacterFactory >(gpCharacterFactoryBuilder->GetFactory(mSamusRes));
  const CCharacterInfo& character = (*mCharacterFactory)->GetCharInfo(mSuitCharIdx);
  mSuitModel = gpSimplePool->GetObj(SObjectTag('CMDL', character.GetModelId()));
  mSuitModel->Lock();
  mSuitSkin = gpSimplePool->GetObj(SObjectTag('CSKR', character.GetSkinRulesId()));
  mSuitSkin->Lock();

  if (mFirstEffectDescription) {
    mFirstEffect = CElementGen::ConstructChildParticleSystem(
        *mFirstEffectDescription, mFirstEffectDescription->GetTag().GetType(), 0,
        CElementGen::kOSF_One, false, true, firstEffectTransform.GetTranslation(),
        firstEffectTransform, CVector3f::Zero(), CTransform4f::Identity(), firstEffectScale,
        CColor::White(), CVector3f::One());
  }
  if (mSecondEffectDescription) {
    mSecondEffect = CElementGen::ConstructChildParticleSystem(
        *mSecondEffectDescription, mSecondEffectDescription->GetTag().GetType(), 0,
        CElementGen::kOSF_One, false, true, CVector3f::Zero(), CTransform4f::Identity(),
        secondEffectTransform.GetTranslation(), secondEffectTransform, secondEffectScale,
        CColor::White(), CVector3f::One());
  }
  if (firstPassCamera != nullptr) {
    mCameraPass = kCP_First;
    mFirstPassCamera = *firstPassCamera;
  }
  if (secondPassCamera != nullptr) {
    if (mCameraPass == kCP_None) {
      mCameraPass = kCP_Second;
    }
    mSecondPassCamera = *secondPassCamera;
  }
}

CPortalTransition::~CPortalTransition() {
  if (mStartPortalHandle) {
    CSfxManager::SfxStop(mStartPortalHandle);
    mStartPortalHandle.Clear();
  }
  if (mInPortal1Handle) {
    CSfxManager::SfxStop(mInPortal1Handle);
    mInPortal1Handle.Clear();
  }
  if (mInPortal2Handle) {
    CSfxManager::SfxStop(mInPortal2Handle);
    mInPortal2Handle.Clear();
  }
}

bool CPortalTransition::IsFinished() const {
  return mCurTime >= 9.f - (mDirection == 2 ? 2.f : 0.f);
}

bool CPortalTransition::TouchModels() {
  bool ready = true;
  if (mBeamModel) {
    if (mBeamModel->IsLoaded()) {
      mBeamModelData = CModelData(CStaticRes(mBeamModel->GetTag().GetId(), mSamusRes.GetScale()));
      mBeamModel.clear();
    } else {
      ready = false;
    }
  }
  if (mGrappleModel) {
    if (mGrappleModel->IsLoaded()) {
      mGrappleModelData =
          CModelData(CStaticRes(mGrappleModel->GetTag().GetId(), mSamusRes.GetScale()));
      mGrappleModel.clear();
    } else {
      ready = false;
    }
  }
  if (mSuitModel && mSuitSkin) {
    if (mSuitModel->IsLoaded() && mSuitSkin->IsLoaded()) {
      mSamusModelData = CModelData(CAnimRes(mSamusRes.GetId(), mSuitCharIdx, mSamusRes.GetScale(),
                                            mSamusRes.GetDefaultAnim(), mSamusRes.CanLoop()));
      mSamusModelData.AnimationData()->SetAnimation(
          CAnimPlaybackParms(mSamusRes.GetDefaultAnim(), -1, 1.f, true), false);
      mSuitModel.clear();
      mSuitSkin.clear();
    } else {
      ready = false;
    }
  }

  if (!mSamusModelData.IsNull()) {
    mSamusModelData.Touch(CModelData::kWM_Normal, 0);
  }
  if (!mBeamModelData.IsNull()) {
    mBeamModelData.Touch(CModelData::kWM_Normal, 0);
  }
  if (!mGrappleModelData.IsNull()) {
    mGrappleModelData.Touch(CModelData::kWM_Normal, 0);
  }
  return ready;
}

void CPortalTransition::UpdateLights() {
  static const CColor warm(1.f, 1.f, 0.9f, 1.f);
  static const CColor cool(0.05f, 0.f, 1.f, 1.f);
  mLights.clear();
  mLights.reserve(2);
  mLights.push_back(
      CLight::BuildDirectional(CVector3f::Forward(), mDirection == 2 ? CColor::White() : warm));
  mLights.push_back(
      CLight::BuildDirectional(CVector3f::Back(), mDirection == 2 ? CColor::White() : cool));
}

void CPortalTransition::Update(float dt) {
  TouchModels();
  if (mDirection == 2 && !IsReady()) {
    return;
  }

  if (mCurTime == 0.f) {
    if (!mStartPortalHandle) {
      mStartPortalHandle = CSfxManager::SfxStart(mStartPortal, mVolume, mPan);
    }
    if (!mInPortal1Handle) {
      mInPortal1Handle =
          CSfxManager::SfxStart(mInPortal1, mVolume, mPan, CSfxManager::kAllAreas, false, true);
    }
    if (!mInPortal2Handle) {
      mInPortal2Handle =
          CSfxManager::SfxStart(mInPortal2, mVolume, mPan, CSfxManager::kAllAreas, false, true);
    }
  }
  mCurTime += dt;
  mSamusModelData.AdvanceAnimationIgnoreParticles(dt, mRandom, true);
  mGunTransform = mSamusModelData.GetScaledLocatorTransform(rstl::string_l("GUN_LCTR"));
  mGrappleTransform = mSamusModelData.GetScaledLocatorTransform(rstl::string_l("GRAPPLE_LCTR"));
  if (!mFirstEffect.null() && (mDirection != 1 || mCurTime >= 3.5f)) {
    mFirstEffect->Update(dt);
  }
  if (!mSecondEffect.null()) {
    mSecondEffect->Update(dt);
  }

  if (mCurTime <= 4.f || mDirection == 2) {
    mModelFlags = CModelFlags::Normal();
  } else {
    mModelFlags =
        CModelFlags(CModelFlags::kT_Two, CColor::Lerp(CColor::Black(), CColor::White(),
                                                      rstl::min_val(0.5f, mCurTime - 4.f) / 0.5f));
  }
  UpdateLights();
  if (mCameraPass == kCP_First && mFirstPassCamera && mSecondPassCamera &&
      mCurTime >= mFirstPassCamera->GetDuration()) {
    mCameraPass = kCP_Second;
  }
}

float CPortalTransition::GetCameraFov(ECameraPass pass) const {
  CGameCameraSpline* camera = nullptr;
  float time = mCurTime;
  if (pass == kCP_First && mFirstPassCamera) {
    camera = &*mFirstPassCamera;
  } else if (pass == kCP_Second && mSecondPassCamera) {
    camera = &*mSecondPassCamera;
    time -= mFirstPassCamera ? mFirstPassCamera->GetDuration() : 0.f;
  }
  return camera != nullptr ? camera->GetFovByTime(time) : gpTweakGame->GetFieldOfView();
}

CTransform4f CPortalTransition::GetCameraTransform(ECameraPass pass) const {
  CGameCameraSpline* camera = nullptr;
  float time = mCurTime;
  if (pass == kCP_First && mFirstPassCamera) {
    camera = &*mFirstPassCamera;
  } else if (pass == kCP_Second && mSecondPassCamera) {
    camera = &*mSecondPassCamera;
    time -= mFirstPassCamera ? mFirstPassCamera->GetDuration() : 0.f;
  }
  if (camera == nullptr) {
    return CTransform4f::Identity();
  }

  const CVector3f position = mCameraTransform * camera->GetPositionByTime(time);
  const CVector3f lookAt = mCameraTransform * camera->GetLookAtByTime(time);
  return CTransform4f::LookAt(position, lookAt, CVector3f::Up());
}

void CPortalTransition::Draw() const {
  gpRender->SetPerspective(GetCameraFov(mCameraPass) * 0.7f, 1.42f,
                           CCameraManager::GetDefaultFirstPersonNearClipDistance(),
                           CCameraManager::GetDefaultFirstPersonFarClipDistance());
  CGraphics::SetViewPointMatrix(GetCameraTransform(mCameraPass));
  CActorLights lights(0, CVector3f::Zero(), 4, 4, 0.f, false, false, false, false);
  lights.BuildFakeLightList(mLights, CColor(0.f, 0.f, 0.f, 1.f));

  if (!mSamusModelData.IsNull() && (mCurTime <= 4.75f || mDirection == 2)) {
    mSamusModelData.AnimationData()->PreRender();
    mSamusModelData.Render(CModelData::kWM_Normal, mSamusTransform, &lights, mModelFlags);
    if (!mBeamModelData.IsNull()) {
      mBeamModelData.Render(CModelData::kWM_Normal, mSamusTransform * mGunTransform, &lights,
                            mModelFlags);
    }
    if (!mGrappleModelData.IsNull()) {
      mGrappleModelData.Render(CModelData::kWM_Normal, mSamusTransform * mGrappleTransform, &lights,
                               mModelFlags);
    }
  }
  if (!mSecondEffect.null()) {
    mSecondEffect->Render();
  }
  if (!mFirstEffect.null()) {
    mFirstEffect->Render();
  }
  CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Multiply, CCameraFilterPass::kFS_CinemaBars,
                                CColor::Black(), nullptr, 1.f);
}

bool CPortalTransition::IsReady() const {
  return (!mSuitModel || mSuitModel->IsLoaded()) && (!mSuitSkin || mSuitSkin->IsLoaded()) &&
         (!mBeamModel || mBeamModel->IsLoaded()) && (!mGrappleModel || mGrappleModel->IsLoaded()) &&
         mSamusModelData.IsLoaded(0) && mBeamModelData.IsLoaded(0) && mGrappleModelData.IsLoaded(0);
}
