#include "MetroidPrime/CAutoMapper.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/IWorld.hpp"

// Work in progress: the map-loading, input and drawing bodies are not yet reconstructed.

static inline float Lerp(float a, float b, float t) { return a * (1.f - t) + b * t; }

CAutoMapper::SAutoMapperRenderState::SAutoMapperRenderState(
    const CVector2i& viewportSize, const CQuaternion& camOrientation, float camDist, float camAngle,
    const CVector3f& areaPoint, float drawDepth1, float drawDepth2, float alphaSurfaceVisited,
    float alphaOutlineVisited, float alphaSurfaceUnvisited, float alphaOutlineUnvisited)
: mViewportSize(viewportSize)
, mCamOrientation(camOrientation)
, mCamDist(camDist)
, mCamAngle(camAngle)
, mAreaPoint(areaPoint)
, mDrawDepth1(drawDepth1)
, mDrawDepth2(drawDepth2)
, mAlphaSurfaceVisited(alphaSurfaceVisited)
, mAlphaOutlineVisited(alphaOutlineVisited)
, mAlphaSurfaceUnvisited(alphaSurfaceUnvisited)
, mAlphaOutlineUnvisited(alphaOutlineUnvisited)
, mViewportEase(kE_None)
, mCamEase(kE_None)
, mPointEase(kE_None)
, mDepth1Ease(kE_None)
, mDepth2Ease(kE_None)
, mAlphaEase(kE_None) {}

void CAutoMapper::SAutoMapperRenderState::InterpolateWithClamp(const SAutoMapperRenderState& a,
                                                               SAutoMapperRenderState& out,
                                                               const SAutoMapperRenderState& b,
                                                               float t) {
  const float ct = CMath::Clamp(0.f, t, 1.f);
  const float easeIn = CMath::Clamp(0.f, ct * ct * ct, 1.f);
  const float omt = 1.f - ct;
  const float easeOut = CMath::Clamp(0.f, 1.f - omt * omt * omt, 1.f);
  float easeInOut;
  if (ct < 0.5f) {
    easeInOut = CMath::Clamp(0.f, 1.f - (0.5f * CMath::SqrtF(2.f * omt - 1.f) + 0.5f), 1.f);
  } else {
    easeInOut = CMath::Clamp(0.f, 0.5f * CMath::SqrtF(2.f * ct - 1.f) + 0.5f, 1.f);
  }
  const float eases[5] = {0.f, ct, easeOut, easeIn, easeInOut};

  if (b.mViewportEase != kE_None) {
    out.mViewportSize = CVector2i::Lerp(a.mViewportSize, b.mViewportSize, eases[b.mViewportEase]);
  }
  if (b.mCamEase != kE_None) {
    const float ease = eases[b.mCamEase];
    out.mCamOrientation = CQuaternion::Slerp(a.mCamOrientation, b.mCamOrientation, ease);
    out.mCamDist = Lerp(a.mCamDist, b.mCamDist, ease);
    out.mCamAngle = Lerp(a.mCamAngle, b.mCamAngle, ease);
  }
  if (b.mPointEase != kE_None) {
    out.mAreaPoint = CVector3f::Lerp(a.mAreaPoint, b.mAreaPoint, eases[b.mPointEase]);
  }
  if (b.mDepth1Ease != kE_None) {
    out.mDrawDepth1 = Lerp(a.mDrawDepth1, b.mDrawDepth1, eases[b.mDepth1Ease]);
  }
  if (b.mDepth2Ease != kE_None) {
    out.mDrawDepth2 = Lerp(a.mDrawDepth2, b.mDrawDepth2, eases[b.mDepth2Ease]);
  }
  if (b.mAlphaEase != kE_None) {
    const float ease = eases[b.mAlphaEase];
    out.mAlphaSurfaceVisited = Lerp(a.mAlphaSurfaceVisited, b.mAlphaSurfaceVisited, ease);
    out.mAlphaOutlineVisited = Lerp(a.mAlphaOutlineVisited, b.mAlphaOutlineVisited, ease);
    out.mAlphaSurfaceUnvisited = Lerp(a.mAlphaSurfaceUnvisited, b.mAlphaSurfaceUnvisited, ease);
    out.mAlphaOutlineUnvisited = Lerp(a.mAlphaOutlineUnvisited, b.mAlphaOutlineUnvisited, ease);
  }
}

void CAutoMapper::SAutoMapperRenderState::ResetInterpolation() {
  mViewportEase = kE_None;
  mCamEase = kE_None;
  mPointEase = kE_None;
  mDepth1Ease = kE_None;
  mDepth2Ease = kE_None;
  mAlphaEase = kE_None;
}

CAutoMapper::SAutoMapperHintLocation::SAutoMapperHintLocation(uint showBeacon, float beaconAlpha,
                                                              CAssetId worldId, int areaId)
: mShowBeacon(showBeacon), mBeaconAlpha(beaconAlpha), mWorldId(worldId), mAreaId(areaId) {}

CAutoMapper::~CAutoMapper() { CSfxManager::KillAll(CSfxManager::kSC_PauseScreen); }

void CAutoMapper::SetupTeleportNavigation() {
  mHintSteps.clear();
  mHintSteps.push_back(SAutoMapperHintStep(SAutoMapperHintStep::kHST_SwitchToUniverse, 0));
  mHintSteps.push_back(SAutoMapperHintStep(SAutoMapperHintStep::kHST_ZoomOut, 0));
}

bool CAutoMapper::NotHintNavigating(const CStateManager& mgr) const {
  if (mHintSteps.size() > 0 &&
      mHintSteps.front().mType != SAutoMapperHintStep::kHST_LeaveMapScreen) {
    return false;
  }
  return true;
}

void CAutoMapper::UnmuteAllLoopedSounds() {
  CSfxManager::SfxVolume(mPanningSfx, 127);
  CSfxManager::SfxVolume(mRotatingSfx, 127);
  CSfxManager::SfxVolume(mZoomingSfx, 127);
}

void CAutoMapper::ResetInterpolationTimer(float duration) {
  mInterpDur = duration;
  mInterpTime = 0.f;
}

CAutoMapper::SAutoMapperRenderState::SAutoMapperRenderState(const SAutoMapperRenderState& other)
: mViewportSize(other.mViewportSize)
, mCamOrientation(other.mCamOrientation)
, mCamDist(other.mCamDist)
, mCamAngle(other.mCamAngle)
, mAreaPoint(other.mAreaPoint)
, mDrawDepth1(other.mDrawDepth1)
, mDrawDepth2(other.mDrawDepth2)
, mAlphaSurfaceVisited(other.mAlphaSurfaceVisited)
, mAlphaOutlineVisited(other.mAlphaOutlineVisited)
, mAlphaSurfaceUnvisited(other.mAlphaSurfaceUnvisited)
, mAlphaOutlineUnvisited(other.mAlphaOutlineUnvisited)
, mViewportEase(other.mViewportEase)
, mCamEase(other.mCamEase)
, mPointEase(other.mPointEase)
, mDepth1Ease(other.mDepth1Ease)
, mDepth2Ease(other.mDepth2Ease)
, mAlphaEase(other.mAlphaEase) {}

void CAutoMapper::SetShouldPanningSoundBePlaying(bool shouldBePlaying) {
  if (shouldBePlaying) {
    if (!mPanningSfx) {
      mPanningSfx = CSfxManager::SfxStart(0x12c, 127, 64, CSfxManager::kAllAreas, false, true);
    }
  } else {
    CSfxManager::SfxStop(mPanningSfx);
    mPanningSfx.Clear();
  }
}

void CAutoMapper::SetShouldZoomingSoundBePlaying(bool shouldBePlaying) {
  if (shouldBePlaying) {
    if (!mZoomingSfx) {
      mZoomingSfx = CSfxManager::SfxStart(0x78, 127, 64, CSfxManager::kAllAreas, false, true);
    }
  } else {
    CSfxManager::SfxStop(mZoomingSfx);
    mZoomingSfx.Clear();
  }
}

void CAutoMapper::SetShouldRotatingSoundBePlaying(bool shouldBePlaying) {
  if (shouldBePlaying) {
    if (!mRotatingSfx) {
      mRotatingSfx = CSfxManager::SfxStart(0x77, 127, 64, CSfxManager::kAllAreas, false, true);
    }
  } else {
    CSfxManager::SfxStop(mRotatingSfx);
    mRotatingSfx.Clear();
  }
}

void CAutoMapper::LeaveMapScreenState() {
  SetShouldPanningSoundBePlaying(false);
  SetShouldZoomingSoundBePlaying(false);
  SetShouldRotatingSoundBePlaying(false);
}

float CAutoMapper::GetMapAreaMiniMapDrawDepth() { return 2.f; }

bool CAutoMapper::IsInMapperState(EAutoMapperState state) const {
  return state == mState && state == mNextState;
}

bool CAutoMapper::IsInMapperStateTransition() const { return mState != mNextState; }

bool CAutoMapper::IsRenderStateInterpolating() const { return mInterpTime < mInterpDur; }

void CAutoMapper::SetCurAreaId(int areaId) {
  if (mCurAreaId.Value() != areaId &&
      (close_enough(mDarkWorldBlend, 0.f) || close_enough(mDarkWorldBlend, 1.f)) &&
      mState != kAMS_MiniMap) {
    if (mTransitionState == kTS_Idle) {
      CSfxManager::SfxStart(0x2200, 127, 64);
    } else {
      CSfxManager::SfxStart(0x21ff, 127, 64);
    }
  }
  mCurAreaId = areaId;
}
