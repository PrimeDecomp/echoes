#include "MetroidPrime/Player/CWorldTransManager.hpp"

#include "Kyoto/Text/CGuiTextSupport.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CPortalTransition.hpp"
#include "rstl/math.hpp"

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
  // TODO: Resolve pending beam/grapple/suit tokens and touch the loaded models.
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

  // TODO: Set up both Samus animations, model resources, bounds and shaft lighting.
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
  // TODO: Update portal readiness, fade, audio and completion.
}

void CWorldTransManager::UpdateEnabled(float dt) {
  // TODO: Advance models, repeating shaft geometry, camera shake and dissolve timing.
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
  // TODO: Build moving shaft lights, including the long-shaft color variant.
}

float CWorldTransManager::GetCameraFov(int pass) const {
  // TODO: Evaluate the selected camera spline. This is a scaffold fallback.
  return 0.f;
}

CTransform4f CWorldTransManager::GetCameraTransform(int pass) const {
  // TODO: Evaluate position/orientation splines and compose the transition camera.
  return CTransform4f::Identity();
}

void CWorldTransManager::DrawAllModels() const {
  // TODO: Render shaft, platform, Samus attachments and optional dark-world volume.
}

void CWorldTransManager::DrawFirstPass() const {
  // TODO: Set the first-pass camera and draw the transition models.
}

void CWorldTransManager::DrawSecondPass() const {
  // TODO: Set the second-pass camera and draw the transition models.
}

void CWorldTransManager::DrawEnabled() const {
  // TODO: Compose both passes, blur, dissolve and transition fade.
}

void CWorldTransManager::DrawDisabled() const {
  // TODO: Draw the disabled-transition fade.
}

void CWorldTransManager::DrawPortalTransition() const {
  // TODO: Render the portal transition and its fade overlay.
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
  // TODO: Create text/subtitle support, configure typewriter effects and lock the string table.
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
  // TODO: Load/update text, subtitles and streamed audio; handle intro skipping and fades.
}

void CWorldTransManager::DrawText() const {
  // TODO: Draw transition text and subtitles with their independent fade timings.
}

void CWorldTransManager::StartTextFadeOut() {
  if (!mStopSoon) {
    mStopTime = mCurTime;
  }
  mStopSoon = true;
}

void CWorldTransManager::CheckIntroTextSeen() {
  // TODO: Read the SeenIntroText environment variable into mIntroTextSeen.
}

bool CWorldTransManager::WaitForModelsAndTextures() {
  // TODO: Process pending model/texture transfers. Not ready until implemented.
  return false;
}
