#include "MetroidPrime/Player/CWorldTransManager.hpp"

#include "Kyoto/Text/CGuiTextSupport.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Audio/CStreamAudioManager.hpp"
#include "Kyoto/CDvdFile.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/CFrameDelayedKiller.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/Cameras/CCameraFilterPass.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CPortalTransition.hpp"
#include "MetroidPrime/Factories/CCharacterFactory.hpp"
#include "MetroidPrime/Factories/CCharacterFactoryBuilder.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerRes.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "MetroidPrime/Tweaks/CTweakGame.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "rstl/math.hpp"
#include "rstl/StringExtras.hpp"
#include "rstl/list.hpp"
#include "dolphin/os.h"

// Guessed names.
static CColor sDarkPointLightColor(uchar(80), uchar(49), uchar(130), uchar(255));
static CColor sDarkMovingLightColor(uchar(156), uchar(123), uchar(200), uchar(255));

struct CWorldTransManager::SModelDatas {
  CAnimRes mSamusRes;
  CModelData mSamusModelData;
  // Guessed name: a second Samus model using animation 1; pass ownership is provisional.
  CModelData mSecondPassSamusModelData;
  CModelData mBeamModelData;
  CModelData mGrappleModelData;
  CModelData mPlatformModelData;
  CModelData mBgModelData;
  rstl::optional_object< CToken > mBeamModel;
  rstl::optional_object< CToken > mGrappleModel;
  rstl::optional_object< CToken > mSuitModel;
  rstl::optional_object< CToken > mSuitSkin;
  CTransform4f mGunXf;
  CTransform4f mGrappleXf;
  rstl::vector< CLight > mLights;
  CVector2f mShakeResult;
  CVector2f mShakeDelta;
  float mRandTimeout;
  float mBlurResult;
  float mBlurDelta;
  float mDissolveStartTime;
  float mDissolveEndTime;
  float mTransCompleteTime;
  bool mDissolveStarted;

  explicit SModelDatas(const CAnimRes& samusRes);
};
NESTED_CHECK_SIZEOF(CWorldTransManager, SModelDatas, 0x2b0)

CWorldTransManager::CWorldTransManager()
: mCurTime(0.f)
, mRandom(99)
, mSfx(0x258b)
, mVolume(127)
, mPanning(64)
, mTransType(kTT_Disabled)
, mTextStartTime(0.f)
, mAudioStream(rstl::string_l(""))
, mTextElapsedTime(0.f)
, mIntroTextFadeTimer(0.f)
, mPortalFade(0.f)
, mCameraTransform(CTransform4f::Identity())
, mTransitionFinished(true)
, mStopSoon(false)
, mGoingUp(false)
, mFadeWhite(false)
, mTextDirty(false)
, mLongShaft(false) {}

CWorldTransManager::~CWorldTransManager() {}

CWorldTransManager::SModelDatas::SModelDatas(const CAnimRes& samusRes)
: mSamusRes(samusRes)
, mSamusModelData(CModelData::CModelDataNull())
, mSecondPassSamusModelData(CModelData::CModelDataNull())
, mBeamModelData(CModelData::CModelDataNull())
, mGrappleModelData(CModelData::CModelDataNull())
, mPlatformModelData(CModelData::CModelDataNull())
, mBgModelData(CModelData::CModelDataNull())
, mGunXf(CTransform4f::Identity())
, mGrappleXf(CTransform4f::Identity())
, mShakeResult(0.f, 0.f)
, mShakeDelta(0.f, 0.f)
, mRandTimeout(0.f)
, mBlurResult(0.f)
, mBlurDelta(0.f)
, mDissolveStartTime(99999.f)
, mDissolveEndTime(99999.f)
, mTransCompleteTime(99999.f)
, mDissolveStarted(false) {
  mLights.reserve(8);
}

void CWorldTransManager::DisableTransition() {
  mTransType = kTT_Disabled;
  mModelData = nullptr;
  mTextData = nullptr;
  mSubtitleData = nullptr;
  mDarkWorldInfo.clear();
  mPortalTransition = nullptr;
  mGoingUp = false;
}

void CWorldTransManager::TouchModels() {
  if (!mPortalTransition.null()) {
    mPortalTransition->TouchModels();
  }
  if (mModelData.null()) {
    return;
  }
  SModelDatas& data = *mModelData;
  if (data.mBeamModel && data.mBeamModel->IsLoaded()) {
    data.mBeamModelData = CModelData(CStaticRes(data.mBeamModel->GetTag().GetId(),
                                                 data.mSamusRes.GetScale()));
    data.mBeamModel.clear();
  }
  if (data.mGrappleModel && data.mGrappleModel->IsLoaded()) {
    data.mGrappleModelData = CModelData(CStaticRes(data.mGrappleModel->GetTag().GetId(),
                                                    data.mSamusRes.GetScale()));
    data.mGrappleModel.clear();
  }
  if (data.mSuitModel && data.mSuitSkin && data.mSuitModel->IsLoaded() &&
      data.mSuitSkin->IsLoaded()) {
    data.mSamusModelData = CModelData(data.mSamusRes);
    data.mSamusModelData.AnimationData()->SetAnimation(
        CAnimPlaybackParms(data.mSamusRes.GetDefaultAnim(), -1, 1.f, true), false);
    data.mSuitModel.clear();
    data.mSuitSkin.clear();
  }
  if (!data.mSamusModelData.IsNull()) {
    data.mSamusModelData.Touch(CModelData::kWM_Normal, 0);
  }
  if (!data.mSecondPassSamusModelData.IsNull()) {
    data.mSecondPassSamusModelData.Touch(CModelData::kWM_Normal, 0);
  }
  if (!data.mPlatformModelData.IsNull()) {
    data.mPlatformModelData.Touch(CModelData::kWM_Normal, 0);
  }
  if (!data.mBgModelData.IsNull()) {
    data.mBgModelData.Touch(CModelData::kWM_Normal, 0);
  }
  if (!data.mBeamModelData.IsNull()) {
    data.mBeamModelData.Touch(CModelData::kWM_Normal, 0);
  }
  if (!data.mGrappleModelData.IsNull()) {
    data.mGrappleModelData.Touch(CModelData::kWM_Normal, 0);
  }
}

void CWorldTransManager::EnableTransition(const CAnimRes& samusRes, bool renderGrapple,
                                          CAssetId platformRes, const CVector3f& platformScale,
                                          CAssetId bgRes, const CVector3f& bgScale, bool goingUp,
                                          const CGameCameraSpline* firstPassCamera,
                                          const CGameCameraSpline* secondPassCamera,
                                          const CTransform4f& cameraTransform,
                                          rstl::optional_object< CToken > soundGroup,
                                          const CDarkWorldInfo* darkWorldInfo) {
  mStopSoon = false;
  mTransType = kTT_Enabled;
  mGoingUp = goingUp;
  mModelData = rs_new SModelDatas(samusRes);
  mTextData = nullptr;
  mSubtitleData = nullptr;
  if (firstPassCamera != nullptr) {
    mFirstPassCamera = *firstPassCamera;
  }
  if (secondPassCamera != nullptr) {
    mSecondPassCamera = *secondPassCamera;
  }
  mCameraTransform = cameraTransform;
  mRandom.SetSeed(99);

  SModelDatas& data = *mModelData;
  data.mSamusModelData = CModelData(samusRes);
  data.mSamusModelData.AnimationData()->SetAnimation(
      CAnimPlaybackParms(samusRes.GetDefaultAnim(), -1, 1.f, true), false);
  data.mSecondPassSamusModelData = CModelData(samusRes);
  data.mSecondPassSamusModelData.AnimationData()->SetAnimation(
      CAnimPlaybackParms(1, -1, 1.f, true), false);

  const CAssetId beamRes = gpTweakPlayerRes->GetCinematicBeamResId(
      gpGameState->GetPlayerState()->GetCurrentBeam());
  data.mBeamModel = gpSimplePool->GetObj(SObjectTag('CMDL', beamRes));
  data.mBeamModel->Lock();
  if (renderGrapple) {
    data.mGrappleModel = gpSimplePool->GetObj(
        SObjectTag('CMDL', gpTweakPlayerRes->GetCinematicGrappleResId()));
    data.mGrappleModel->Lock();
  }
  mCharacterFactory = TLockedToken< CCharacterFactory >(
      gpCharacterFactoryBuilder->GetFactory(samusRes));
  const CCharacterInfo& character =
      (*mCharacterFactory)->GetCharInfo(samusRes.GetCharacterNodeId());
  data.mSuitModel = gpSimplePool->GetObj(SObjectTag('CMDL', character.GetModelId()));
  data.mSuitModel->Lock();
  data.mSuitSkin = gpSimplePool->GetObj(SObjectTag('CSKR', character.GetSkinRulesId()));
  data.mSuitSkin->Lock();

  if (platformRes != kInvalidAssetId) {
    data.mPlatformModelData = CModelData(CStaticRes(platformRes, platformScale));
    data.mPlatformModelData.Touch(CModelData::kWM_Normal, 0);
  }
  if (bgRes != kInvalidAssetId) {
    data.mBgModelData = CModelData(CStaticRes(bgRes, bgScale));
    data.mBgModelData.Touch(CModelData::kWM_Normal, 0);
    const CAABox bounds = data.mBgModelData.GetBounds();
    mBgHeight = (bounds.GetMaxPoint().GetZ() - bounds.GetMinPoint().GetZ()) * bgScale.GetZ();
    mLightHeight = mBgHeight;
    if (mLightHeight > 10.f) {
      mLongShaft = true;
      mLightHeight = 10.f;
    }
  } else {
    mBgHeight = 0.f;
    mLightHeight = 0.f;
  }
  mSoundGroup = soundGroup;
  if (mSoundGroup) {
    mSoundGroup->Lock();
  }
  if (darkWorldInfo != nullptr) {
    mDarkWorldInfo = *darkWorldInfo;
  }
  StartTransition();
  TouchModels();
}

void CWorldTransManager::StartTransition() {
  mCurTime = 0.f;
  mBgOffset = 0.f;
  mLightOffset = 0.f;
  mTransitionFinished = false;
  mTextDirty = true;
}

void CWorldTransManager::EndTransition() {
  mCharacterFactory.clear();
  DisableTransition();
}

void CWorldTransManager::Update(float dt) {
  mCurTime += dt;
  switch (mTransType) {
  case kTT_Disabled:
    UpdateDisabled(dt);
    break;
  case kTT_Enabled:
    UpdateEnabled(dt);
    break;
  case kTT_Text:
    UpdateText(dt);
    break;
  case kTT_Portal:
    UpdatePortalTransition(dt);
    break;
  }
}

void CWorldTransManager::UpdateDisabled(float dt) {
  if (mCurTime > 2.f) {
    mTransitionFinished = true;
  }
}

void CWorldTransManager::UpdatePortalTransition(float dt) {
  if (mPortalTransition.null()) {
    return;
  }
  if (mPortalTransition->IsReady()) {
    const float direction = mPortalTransition->IsFinished() ? -1.f : 1.f;
    mPortalFade = CMath::Clamp(0.f, mPortalFade + 0.5f * dt * direction, 1.f);
  }
  mPortalTransition->Update(dt);
  if (mPortalTransition->IsFinished() && mPortalFade <= 0.f) {
    mTransitionFinished = true;
  }
}

void CWorldTransManager::UpdateEnabled(float dt) {
  if (!mModelData.null() && !mModelData->mSamusModelData.IsNull()) {
    SModelDatas& data = *mModelData;
    if (mStopSoon && !data.mDissolveStarted && mCurTime >= 4.f) {
      data.mDissolveStarted = true;
      data.mDissolveStartTime = mCurTime;
      data.mDissolveEndTime = mCurTime;
      if (!mSecondPassCamera) {
        data.mTransCompleteTime = 1.f + mCurTime;
      } else {
        data.mTransCompleteTime = mCurTime + mSecondPassCamera->GetDuration();
        data.mSamusModelData.AnimationData()->SetAnimation(
            CAnimPlaybackParms(1, -1, 1.f, true), false);
      }
    }
    if (data.mDissolveStarted && mCurTime > data.mTransCompleteTime) {
      mTransitionFinished = true;
    }
    data.mSamusModelData.AdvanceAnimationIgnoreParticles(dt, mRandom, true);
    data.mGunXf = data.mSamusModelData.GetScaledLocatorTransform(rstl::string_l("GUN_LCTR"));
    data.mGrappleXf =
        data.mSamusModelData.GetScaledLocatorTransform(rstl::string_l("GRAPPLE_LCTR"));
    data.mRandTimeout -= dt;
    if (data.mRandTimeout <= 0.f) {
      data.mRandTimeout = mRandom.Range(0.016666668f, 0.1f);
      const CVector2f shake(mRandom.Range(-0.025f, 0.025f),
                            mRandom.Range(-0.075f, 0.075f));
      data.mShakeDelta = (shake - data.mShakeResult) / data.mRandTimeout;
      const float blur = mRandom.Range(-2.f, 4.f);
      data.mBlurDelta = (blur - data.mBlurResult) / data.mRandTimeout;
    }
    data.mShakeResult += data.mShakeDelta * dt;
    data.mBlurResult += dt * data.mBlurDelta;
  }

  float bgDelta = 37.5f * dt;
  float lightDelta = 18.75f * dt;
  if (mGoingUp) {
    bgDelta = -bgDelta;
    lightDelta = -lightDelta;
  }
  mBgOffset += bgDelta;
  if (mBgOffset > mBgHeight) {
    mBgOffset -= mBgHeight;
  }
  if (mBgOffset < 0.f) {
    mBgOffset += mBgHeight;
  }
  mLightOffset += lightDelta;
  if (mLightOffset > mLightHeight) {
    mLightOffset -= mLightHeight;
  }
  if (mLightOffset < 0.f) {
    mLightOffset += mLightHeight;
  }
  UpdateLights(dt);
}

void CWorldTransManager::Draw() const {
  switch (mTransType) {
  case kTT_Disabled:
    DrawDisabled();
    break;
  case kTT_Enabled:
    DrawEnabled();
    break;
  case kTT_Text:
    DrawText();
    break;
  case kTT_Portal:
    DrawPortalTransition();
    break;
  }
}

void CWorldTransManager::UpdateLights(float dt) {
  if (mModelData.null()) {
    return;
  }

  rstl::vector< CLight >& lights = mModelData->mLights;
  lights.clear();
  CColor movingColor = CColor::White();
  CColor shaftColor = CColor::White();
  if (mLongShaft) {
    shaftColor = CColor(uchar(215), uchar(220), uchar(193), uchar(225));
  }
  if (mDarkWorldInfo) {
    movingColor = sDarkMovingLightColor;
    shaftColor = sDarkPointLightColor;
  }

  const CVector3f lightPos(0.f, 1.2f, 0.f);
  CLight light = CLight::BuildPoint(lightPos, movingColor);
  light.SetAttenuation(0.f, 0.f, 0.1f);
  CLight movingLight = light;
  movingLight.SetColor(shaftColor);
  movingLight.SetPosition(lightPos + CVector3f(0.f, 0.f, 2.f * mLightOffset - mLightHeight));

  float intensity = 1.f;
  if (!mGoingUp && mLightHeight - mLightOffset < 2.f) {
    intensity = (mLightHeight - mLightOffset) * 0.5f;
  } else if (mGoingUp && mLightOffset < 2.f) {
    intensity = mLightOffset * 0.5f;
  }
  if (intensity < 1.f) {
    CLight wrappedLight = light;
    wrappedLight.SetPosition(lightPos + CVector3f(0.f, 0.f,
                                                 mGoingUp ? mLightHeight : -mLightHeight));
    wrappedLight.SetColor(CColor::Lerp(CColor::Black(), movingColor, 1.f - intensity));
    lights.push_back(wrappedLight);
    movingLight.SetColor(CColor::Lerp(CColor::Black(), shaftColor, intensity));
  }
  lights.push_back(movingLight);
  movingLight.SetPosition(CVector3f(movingLight.GetPosition().GetX(), -1.2f,
                                    movingLight.GetPosition().GetZ()));
  lights.push_back(movingLight);
}

float CWorldTransManager::GetCameraFov(int pass) const {
  if (pass == 0 && mFirstPassCamera) {
    return mFirstPassCamera->GetFovByTime(mCurTime);
  }
  if (pass == 1 && mSecondPassCamera) {
    return mSecondPassCamera->GetFovByTime(mCurTime - mModelData->mDissolveStartTime);
  }
  return gpTweakGame->GetFieldOfView();
}

CTransform4f CWorldTransManager::GetCameraTransform(int pass) const {
  CGameCameraSpline* spline = nullptr;
  float time = 0.f;
  if (pass == 0) {
    if (!mFirstPassCamera) {
      const float rotationT = CMath::Clamp(0.f, mCurTime / 25.f, 100.f);
      const float translationT = CMath::Clamp(0.f, mCurTime / 10.f, 1.f);
      const CRelAngle angle = CRelAngle::FromDegrees(360.f * rotationT + 180.f - 90.f);
      return CTransform4f::RotateZ(angle) *
             CTransform4f::Translate(mModelData->mShakeResult.GetX(),
                                     -3.5f * (1.f - translationT) - 3.5f,
                                     2.f + mModelData->mShakeResult.GetY());
    }
    spline = &*mFirstPassCamera;
    time = mCurTime;
  } else if (pass == 1) {
    if (!mSecondPassCamera) {
      const float t = CMath::Clamp(0.f, (4.f + mCurTime - mModelData->mDissolveStartTime) / 5.f,
                                   1.f);
      const CRelAngle angle = CRelAngle::FromDegrees(48.f * t + 180.f - 24.f);
      const CVector3f& scale = mModelData->mSamusRes.GetScale();
      return CTransform4f::RotateZ(angle) *
             CTransform4f::Translate(-0.1f * scale.GetX(), -0.5f * scale.GetY(),
                                     1.5f * scale.GetZ());
    }
    spline = &*mSecondPassCamera;
    time = mCurTime - mModelData->mDissolveStartTime;
  }

  const CVector3f position = mCameraTransform * spline->GetPositionByTime(time);
  const CVector3f lookAt = mCameraTransform * spline->GetLookAtByTime(time);
  return CTransform4f::LookAt(position, lookAt, CVector3f::Up());
}

void CWorldTransManager::DrawAllModels() const {
  SModelDatas& data = *mModelData.get();
  CActorLights lights(0, CVector3f::Zero(), 4, 4);
  lights.BuildFakeLightList(data.mLights, CColor(0.f, 0.f, 0.f, 1.f));
  if (!data.mBgModelData.IsNull()) {
    data.mBgModelData.Render(CModelData::kWM_Normal,
                             CTransform4f::Translate(0.f, 0.f, -(2.f * mBgHeight - mBgOffset)),
                             &lights, CModelFlags::Normal());
    data.mBgModelData.Render(CModelData::kWM_Normal,
                             CTransform4f::Translate(0.f, 0.f, mBgOffset - mBgHeight),
                             &lights, CModelFlags::Normal());
    data.mBgModelData.Render(CModelData::kWM_Normal,
                             CTransform4f::Translate(0.f, 0.f, mBgOffset),
                             &lights, CModelFlags::Normal());
  }
  if (!data.mPlatformModelData.IsNull()) {
    data.mPlatformModelData.Render(CModelData::kWM_Normal, CTransform4f::Identity(),
                                   &lights, CModelFlags::Normal());
  }
  if (!data.mSamusModelData.IsNull()) {
    const CTransform4f& samusXf = CTransform4f::Identity();
    data.mSamusModelData.AnimationData()->PreRender();
    data.mSamusModelData.Render(CModelData::kWM_Normal, samusXf, &lights,
                                CModelFlags::Normal());
    if (!data.mBeamModelData.IsNull()) {
      data.mBeamModelData.Render(CModelData::kWM_Normal, samusXf * data.mGunXf,
                                 &lights, CModelFlags::Normal());
    }
    if (!data.mGrappleModelData.IsNull()) {
      data.mGrappleModelData.Render(CModelData::kWM_Normal, samusXf * data.mGrappleXf,
                                    &lights, CModelFlags::Normal());
    }
  }
  if (mDarkWorldInfo) {
    CVector3f scale = CVector3f::One();
    if (!data.mPlatformModelData.IsNull()) {
      const CAABox bounds = data.mPlatformModelData.GetBounds();
      const float halfWidth = 0.5f * (bounds.GetMaxPoint().GetX() - bounds.GetMinPoint().GetX());
      scale = CVector3f(halfWidth, halfWidth, halfWidth);
    }
    const CDarkWorldInfo& dark = *mDarkWorldInfo;
    gpRender->DrawDarkWorldVolume(CVector3f::Zero(), scale, 255, 128, true, 1.f,
                                  dark.mScroll1, dark.mScroll2, dark.mTexScale1,
                                  dark.mTexScale2, **dark.mEnvironment, **dark.mCloud1,
                                  **dark.mCloud2, dark.mColor, dark.mAdditiveColor, false, false);
  }
}

void CWorldTransManager::DrawFirstPass() const {
  const float fov = GetCameraFov(0);
  gpRender->SetPerspective(0.7f * fov, 1.42f, CCameraManager::GetDefaultFirstPersonNearClipDistance(),
                           CCameraManager::GetDefaultFirstPersonFarClipDistance());
  CGraphics::SetViewPointMatrix(GetCameraTransform(0));
  DrawAllModels();
}

void CWorldTransManager::DrawSecondPass() const {
  const float fov = GetCameraFov(1);
  gpRender->SetPerspective(0.7f * fov, 1.42f, CCameraManager::GetDefaultFirstPersonNearClipDistance(),
                           CCameraManager::GetDefaultFirstPersonFarClipDistance());
  CGraphics::SetViewPointMatrix(GetCameraTransform(1));
  DrawAllModels();
}

void CWorldTransManager::DrawEnabled() const {
  if (mModelData.null()) {
    return;
  }
  gpRender->SetRequestRGBA6(true);
  if (mCurTime <= mModelData->mDissolveStartTime) {
    DrawFirstPass();
  } else {
    DrawSecondPass();
  }
  CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Multiply, CCameraFilterPass::kFS_CinemaBars,
                                CColor::Black(), nullptr, 1.f);

  float alpha = 0.f;
  if (mCurTime < 0.25f) {
    alpha = 1.f - mCurTime / 0.25f;
  } else if (mCurTime > mModelData->mTransCompleteTime) {
    alpha = 1.f;
  } else if (mCurTime > mModelData->mTransCompleteTime - 0.25f) {
    alpha = 1.f - (mModelData->mTransCompleteTime - mCurTime) / 0.25f;
  }
  if (alpha > 0.f) {
    CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Blend,
                                  CCameraFilterPass::kFS_Fullscreen,
                                  CColor(0.f, 0.f, 0.f, alpha), nullptr, 1.f);
  }
  CGraphics::SetIsBeginSceneClearFb(true);
}

void CWorldTransManager::DrawDisabled() const {
  CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Blend,
                                CCameraFilterPass::kFS_Fullscreen,
                                CColor(uchar(0), uchar(0), uchar(0), uchar(3)), nullptr, 1.f);
}

void CWorldTransManager::DrawPortalTransition() const {
  if (mPortalTransition.null()) {
    return;
  }
  mPortalTransition->Draw();
  CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Add, CCameraFilterPass::kFS_Fullscreen,
                                CColor::Lerp(CColor::White(), CColor::Black(), mPortalFade),
                                nullptr, 1.f);
  CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Multiply,
                                CCameraFilterPass::kFS_CinemaBars, CColor::Black(), nullptr, 1.f);
  CGraphics::SetIsBeginSceneClearFb(true);
}

void CWorldTransManager::SfxStart() {
  if (!mSfxHandle && mSfx != CSfxManager::kInternalInvalidSfxId) {
    mSfxHandle = CSfxManager::SfxStart(mSfx, mVolume, mPanning, CSfxManager::kAllAreas, false, true,
                                       CSfxManager::kMedPriority);
  }
}

void CWorldTransManager::SfxStop() {
  if (mSfxHandle) {
    CSfxManager::SfxStop(mSfxHandle);
    mSfxHandle.Clear();
  }
}

void CWorldTransManager::SetSfx(ushort sfx, uchar volume, uchar panning) {
  mSfx = sfx;
  mVolume = volume;
  mPanning = panning;
}

void CWorldTransManager::EnableTransition(CAssetId fontId, CAssetId stringId, int stringIdx,
                                          bool fadeWhite, float charFadeTime, float charFadeRate,
                                          float textStartTime, float textEndDelay,
                                          float subtitleFadeInDelay, float subtitleFadeTime,
                                          const rstl::string& audioStream, uchar volume,
                                          bool displaySubtitles, bool introText) {
  mIntroText = introText;
  mIntroTextSeen = false;
  mAudioStream = audioStream;
  mStrIdx = stringIdx;
  mDisplaySubtitles = displaySubtitles;
  mTextStartTime = textStartTime;
  mTextEndDelay = textEndDelay;
  mSubtitleFadeInDelay = subtitleFadeInDelay;
  mSubtitleFadeTime = rstl::max_val(0.0001f, subtitleFadeTime);
  mVolume = volume;
  mStopSoon = false;
  mTransType = kTT_Text;
  mModelData = nullptr;
  mFadeWhite = fadeWhite;
  const CGuiTextProperties properties(true, kJustification_Center,
                                      kVerticalJustification_Center);
  mTextData = rs_new CGuiTextSupport(fontId, 640, 448, properties, CColor::White(),
                                     CColor::Black(), CColor::White(), gpSimplePool);
  mTextData->SetTypeWriteEffectOptions(true, charFadeTime, charFadeRate);
  mTextData->SetText(rstl::wstring_l(L""));
  if (mIntroText) {
    mTextData->SetExtentX(640 - 64);
  }
  if (mDisplaySubtitles) {
    mSubtitleData = rs_new CGuiTextSupport(fontId, 640, 120, properties,
                                           CColor::White(), CColor::Black(),
                                           CColor::White(), gpSimplePool);
    mSubtitleData->SetText(rstl::wstring_l(L""));
  }
  mStrTable = TToken< CStringTable >(gpSimplePool->GetObj(SObjectTag('STRG', stringId)));
  mStrTable->Lock();
  StartTransition();
}

void CWorldTransManager::EnableTransition(rstl::single_ptr< CPortalTransition >& transition,
                                          uchar volume) {
  if (!transition.null()) {
    mTransType = kTT_Portal;
    mPortalTransition = transition;
  }
  mStopSoon = false;
  mTextData = nullptr;
  mSubtitleData = nullptr;
  mVolume = volume;
  mPortalFade = 0.f;
  StartTransition();
  TouchModels();
}

void CWorldTransManager::UpdateText(float dt) {
  if (mTextDirty) {
    if (mStrTable && mStrTable->IsLoaded()) {
      const CStringTable& strings = ***mStrTable;
      if (mStrIdx < strings.GetStringCount()) {
        mTextData->SetText(rstl::wstring(strings.GetString(mStrIdx)));
        if (mDisplaySubtitles && mStrIdx + 1 < strings.GetStringCount()) {
          mSubtitleData->SetText(rstl::wstring(strings.GetString(mStrIdx + 1)));
        }
      }
      if (mAudioStream == rstl::string_l("UseStringTable") &&
          mStrIdx + 1 < strings.GetStringCount()) {
        mAudioStream =
            CStringExtras::ConvertToANSI(rstl::wstring(strings.GetString(mStrIdx + 1)));
      }
      if (mIntroText && strings.GetString(0)[0] != L'\0') {
        mAudioStream = CStringExtras::ConvertToANSI(rstl::wstring(strings.GetString(0)));
      }
      mSfxInterval = 0.f;
      if (mIntroText) {
        mIntroAudioStopped = false;
        CStreamAudioManager::PlaySoftwareAudio(CStreamAudioManager::kSC_OneShot,
                                                rstl::string_l(""), 0.25f, 3.f, 87, true);
      }
      mTextDirty = false;
    } else if (mCurTime >= mTextStartTime) {
      mTextStartTime += dt;
    }
  }

  if (mCurTime >= mTextStartTime) {
    if (mAudioStream.size() != 0 && CDvdFile::FileExists(mAudioStream.c_str())) {
      CStreamAudioManager::PlaySoftwareAudio(CStreamAudioManager::kSC_Default, mAudioStream,
                                              0.f, 0.f, mVolume, false);
      mAudioStream = rstl::string_l("");
    }
    if (mTextData->GetCurTime() < mTextData->GetTotalAnimationTime() + 0.1f) {
      mTextData->Update(dt);
    }
    mTextElapsedTime += dt;
    if (mDisplaySubtitles && !mSubtitleData.null()) {
      const float elapsed = rstl::max_val(0.f, mCurTime - mTextStartTime - mSubtitleFadeInDelay);
      const float fraction = rstl::min_val(1.f, elapsed / mSubtitleFadeTime);
      mSubtitleData->SetFontColor(CColor::White().WithAlphaOf(0.75f * fraction * fraction));
      mSubtitleData->Update(dt);
    }
    const float printed = mTextData->GetNumCharactersPrinted();
    const float charsPerSfx = gpTweakGui->GetWorldTransManagerCharsPerSfx();
    if (printed >= mSfxInterval + charsPerSfx) {
      mSfxInterval += charsPerSfx;
      CSfxManager::SfxStart(0x618, 127, 64);
    }
  }

  if (mIntroText && mStrTable && mStrTable->IsLoaded()) {
    const CStringTable& strings = ***mStrTable;
    if (!mIntroAudioStopped && mCurTime >= mTextStartTime + 27.25f) {
      mIntroAudioStopped = true;
      CStreamAudioManager::StopSoftwareAudio(CStreamAudioManager::kSC_OneShot,
                                              rstl::string_l(""));
    }
    if (mIntroTextFadeTimer > 0.f) {
      mIntroTextFadeTimer = rstl::max_val(0.f, mIntroTextFadeTimer - 2.f * dt);
    }
    const bool finalPage = mStrIdx == strings.GetStringCount() - 1;
    const float pageDelay = finalPage ? 4.f : (mStrIdx & 1 ? 0.5f : 2.f);
    if (mIntroTextFadeTimer <= 0.f &&
        mTextElapsedTime > 1.f + mTextData->GetTotalAnimationTime() + pageDelay) {
      if (mStrIdx + 1 < strings.GetStringCount()) {
        ++mStrIdx;
        const rstl::wstring text(strings.GetString(mStrIdx));
        if (mStrIdx & 1) {
          mTextData->SetText(text);
          mSfxInterval = 0.f;
          mTextElapsedTime = 0.f;
        } else {
          mTextData->AddText(text);
        }
        if (mDisplaySubtitles && mStrIdx + 1 < strings.GetStringCount()) {
          mSubtitleData->SetText(rstl::wstring(strings.GetString(mStrIdx + 1)));
        }
      } else {
        mIntroTextFadeTimer = 1.f;
      }
    }
  }

  if (mStopSoon) {
    const float completion = 1.f + mTextData->GetTotalAnimationTime() + mTextEndDelay;
    if (mTextElapsedTime > completion && mCurTime - mStopTime > 1.f) {
      mTransitionFinished = true;
      if (mIntroText) {
        gpGameState->SystemOptions().FindEnvironmentVariable("SeenIntroText")->Set(1);
      }
      if (mIntroText && mIntroTextSeen && !mIntroAudioStopped) {
        mIntroAudioStopped = true;
        CStreamAudioManager::FadeOutSoftwareAudio(CStreamAudioManager::kSC_OneShot, 1.f);
        CStreamAudioManager::StopSoftwareAudio(CStreamAudioManager::kSC_OneShot,
                                                rstl::string_l(""));
      }
    } else {
      mStopTime = mCurTime;
    }
  }
}

void CWorldTransManager::DrawText() const {
  gpRender->SetViewportOrtho(false, -4096.f, 4096.f);
  gpRender->SetModelMatrix(CTransform4f::Translate(mIntroText ? 32.f : 0.f, 0.f, 448.f));
  CGraphics::SetCullMode(kCM_None);
  gpRender->SetDepthReadWrite(false, false);
  gpRender->SetBlendMode_AdditiveAlpha();
  mTextData->Render();
  if (mDisplaySubtitles && !mSubtitleData.null()) {
    gpRender->SetModelMatrix(CTransform4f::Translate(0.f, 0.f, 120.f));
    mSubtitleData->Render();
  }

  float alpha = 0.f;
  if (mCurTime < 1.f) {
    alpha = 1.f - mCurTime;
  } else if (mStopSoon) {
    alpha = rstl::min_val(1.f, mCurTime - mStopTime);
  }
  if (alpha > 0.f) {
    const CColor color = (mFadeWhite ? CColor::White() : CColor::Black()).WithAlphaOf(alpha);
    CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Blend,
                                  CCameraFilterPass::kFS_Fullscreen, color, nullptr, 1.f);
  }
  CGraphics::SetIsBeginSceneClearFb(true);
}

void CWorldTransManager::StartTextFadeOut() {
  if (!mStopSoon) {
    mStopTime = mCurTime;
  }
  mStopSoon = true;
}

void CWorldTransManager::CheckIntroTextSeen() {
  const CEnvironmentVariable* seen =
      gpGameState->SystemOptions().FindEnvironmentVariable("SeenIntroText");
  if (seen != nullptr && seen->GetValue() != 0) {
    mIntroTextSeen = true;
  }
}

bool CWorldTransManager::WaitForModelsAndTextures() {
  rstl::vector< SObjectTag > tags = gpSimplePool->GetReferencedTags();
  CTexture::sCurrentFrameCount = 0x7fffffff;
  rstl::list< CARAMToken > modelData;
  CFrameDelayedKiller::StallAndFlushAllAllocations();
  for (int pass = 0; pass < 2; ++pass) {
    for (rstl::vector< SObjectTag >::iterator it = tags.begin(); it != tags.end(); ++it) {
      if (!gpSimplePool->GetObj(*it).IsLoaded()) {
        continue;
      }
      if (it->GetType() == 'TXTR') {
        TToken< CTexture > texture = gpSimplePool->GetObj(*it);
        if (pass == 0) {
          texture->MakeSwappable();
          texture->LoadToARAM();
          while (texture->IsARAMTransferInProgress()) {
            CARAMToken::UpdateAllDMAs();
          }
        } else {
          texture->LoadToMRAM();
        }
      } else if (it->GetType() == 'CMDL') {
        TToken< CModel > modelToken = gpSimplePool->GetObj(*it);
        CModel* model = *modelToken;
        if (pass == 0) {
          rstl::auto_ptr< uchar > data = model->GetData();
          CARAMToken token(data.release(), OSRoundUp32B(model->GetDataSize()), 1);
          token.LoadToARAM();
          token.ForceSyncARAM();
          modelData.push_back(token);
        } else {
          void* data = modelData.front().ForceSyncMRAM();
          modelData.pop_front();
          model->RemapData(static_cast< uchar* >(data));
        }
      }
    }
  }
  CTexture::sCurrentFrameCount = 0;
  return true;
}
