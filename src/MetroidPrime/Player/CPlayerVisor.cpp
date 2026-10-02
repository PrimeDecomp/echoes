#include "MetroidPrime/Player/CPlayerVisor.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakGuiColors.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "MetaRender/CCubeRenderer.hpp"

#include <dolphin/gx/GXFrameBuffer.h>
#include <dolphin/gx/GXManage.h>
#include <dolphin/gx/GXTexture.h>

static const int skPixelsPerTileDimension16Bit = 4;

static inline int round_up_n(int value, int n) { return (value + n - 1) & ~(n - 1); }

static inline int round_up_to_tile(float value) {
  return round_up_n(static_cast< int >(value), skPixelsPerTileDimension16Bit);
}

CPlayerVisor::CPlayerVisor(const CStateManager& mgr, int playerIndex)
: mPlayerIndex(playerIndex)
, mCurVisor(CPlayerState::kPV_Combat)
, mNextVisor(CPlayerState::kPV_Combat)
, mVisorSfxVol(127)
, mVisorTransitioning(false)
, x29_25_(false)
, mScanTimer(0.f)
, mScanDimInterp(1.f)
, mPrevState(kSWS_NotInScanVisor)
, mNextState(kSWS_NotInScanVisor)
, mWindowInterpDuration(0.f)
, mWindowInterpTimer(0.f)
, mPrevWindowDims(CVector2f::Zero())
, mInterpWindowDims(mPrevWindowDims)
, mNextWindowDims(mPrevWindowDims)
, mScanMagInterp(1.f)
, mVpScaleX(1.f)
, mVpScaleY(1.f)
, mScanFrameFixedCorner(gpSimplePool->GetObj("CMDL_ScanFrame2FixedCorner"))
, mScanFrameCenterLeft(gpSimplePool->GetObj("CMDL_ScanFrame2CenterLeft"))
, mScanFrameCenterTop(gpSimplePool->GetObj("CMDL_ScanFrame2CenterTop"))
, mScanFrameBottomLeftCorner(gpSimplePool->GetObj("CMDL_ScanFrame2BottomLeftCorner"))
, mScanFrameStretchCorner(gpSimplePool->GetObj("CMDL_ScanFrame2StretchCorner"))
, mScanFrameLowerRight(gpSimplePool->GetObj("CMDL_ScanFrame2LowerRight"))
, mScanFrameWindow(gpSimplePool->GetObj("CMDL_ScanFrame2Window"))
, mAssetLockCountdown(0)
, mScanTargets(64, SScanObjectIndicatorInfo(kInvalidUniqueId, 0.f, 0.f))
, mScanFrameColorInterp(0.f)
, mScanFrameColorImpulseInterp(0.f) {
  mScanWindowSizes.push_back(CVector2f::Zero());
  const CTweakGui& tweakGui = *gpTweakGui;
  const float idleHeight = tweakGui.GetScanWindowIdleHeight();
  const float idleWidth = tweakGui.GetScanWindowIdleWidth();
  mScanWindowSizes.push_back(CVector2f(idleWidth, idleHeight));
  const CTweakGui& tweakGui2 = *gpTweakGui;
  const float activeHeight = tweakGui2.GetScanWindowActiveHeight();
  const float activeWidth = tweakGui2.GetScanWindowActiveWidth();
  mScanWindowSizes.push_back(CVector2f(activeWidth, activeHeight));
}

CPlayerVisor::~CPlayerVisor() {
  CSfxManager::SfxStop(mVisorLoopSfx);
  CSfxManager::SfxStop(mScanningLoopSfx);
}

float CPlayerVisor::GetDesiredViewportScaleX(const CStateManager& mgr) const {
  return mgr.GetPlayerState(mPlayerIndex)->GetActiveVisor(mgr) == CPlayerState::kPV_Combat
             ? 1.f
             : mVpScaleX;
}

float CPlayerVisor::GetDesiredViewportScaleY(const CStateManager& mgr) const {
  return mgr.GetPlayerState(mPlayerIndex)->GetActiveVisor(mgr) == CPlayerState::kPV_Combat
             ? 1.f
             : mVpScaleY;
}

void CPlayerVisor::BeginTransitionOut(const CStateManager& mgr) {
  if (mVisorLoopSfx) {
    CSfxManager::SfxStop(mVisorLoopSfx);
    mVisorLoopSfx.Clear();
  }
  switch (mCurVisor) {
  case CPlayerState::kPV_Combat:
    break;
  case CPlayerState::kPV_Echo:
    CSfxManager::SfxStart(mgr.ReturnFirstIfSingleElseSecond(0x7e, 0x2660), mVisorSfxVol,
                          mgr.GetPlayer(mPlayerIndex)->GetSoundPan(CPlayer::kMSP_4),
                          CSfxManager::kAllAreas, false, false, CSfxManager::kMedPriority);
    break;
  case CPlayerState::kPV_Scan:
    if (mScanningLoopSfx) {
      CSfxManager::SfxStop(mScanningLoopSfx);
      mScanningLoopSfx.Clear();
    }
    CSfxManager::SfxStart(mgr.ReturnFirstIfSingleElseSecond(0x7e, 0x2660), mVisorSfxVol,
                          mgr.GetPlayer(mPlayerIndex)->GetSoundPan(CPlayer::kMSP_4),
                          CSfxManager::kAllAreas, false, false, CSfxManager::kMedPriority);
    break;
  case CPlayerState::kPV_Dark:
    CSfxManager::SfxStart(mgr.ReturnFirstIfSingleElseSecond(0x7e, 0x2660), mVisorSfxVol,
                          mgr.GetPlayer(mPlayerIndex)->GetSoundPan(CPlayer::kMSP_4),
                          CSfxManager::kAllAreas, false, false, CSfxManager::kMedPriority);
    break;
  default:
    break;
  }
}

void CPlayerVisor::FinishTransitionOut(const CStateManager& mgr) {
  switch (mCurVisor) {
  case CPlayerState::kPV_Combat:
    break;
  case CPlayerState::kPV_Scan:
    mScanDim.DisableFilter(0.f);
    mNextState = kSWS_NotInScanVisor;
    mPrevState = kSWS_NotInScanVisor;
    break;
  case CPlayerState::kPV_Echo:
  case CPlayerState::kPV_Dark:
    mBlur.DisableBlur(0.f);
    break;
  default:
    break;
  }
}

void CPlayerVisor::BeginTransitionIn(const CStateManager& mgr) {
  ushort loopSingle = 0xFFFF;
  ushort loopMulti = 0xFFFF;
  switch (mCurVisor) {
  case CPlayerState::kPV_Combat:
    break;
  case CPlayerState::kPV_Scan:
    CSfxManager::SfxStart(mgr.ReturnFirstIfSingleElseSecond(0x7f, 0x2661), mVisorSfxVol,
                          mgr.GetPlayer(mPlayerIndex)->GetSoundPan(CPlayer::kMSP_4),
                          CSfxManager::kAllAreas, false, false, CSfxManager::kMedPriority);
    mScanDim.SetFilter(CCameraFilterPass::kFT_Multiply, CCameraFilterPass::kFS_Fullscreen, 0.f,
                       CColor::White(), kInvalidAssetId);
    break;
  case CPlayerState::kPV_Echo:
    CSfxManager::SfxStart(mgr.ReturnFirstIfSingleElseSecond(0x7f, 0x2661), mVisorSfxVol,
                          mgr.GetPlayer(mPlayerIndex)->GetSoundPan(CPlayer::kMSP_4),
                          CSfxManager::kAllAreas, false, false, CSfxManager::kMedPriority);
    break;
  case CPlayerState::kPV_Dark:
    CSfxManager::SfxStart(mgr.ReturnFirstIfSingleElseSecond(0x7f, 0x2661), mVisorSfxVol,
                          mgr.GetPlayer(mPlayerIndex)->GetSoundPan(CPlayer::kMSP_4),
                          CSfxManager::kAllAreas, false, false, CSfxManager::kMedPriority);
    loopSingle = 0x160;
    loopMulti = 0x2663;
    break;
  default:
    break;
  }
  if (mCurVisor != CPlayerState::kPV_Combat && !mVisorLoopSfx &&
      static_cast< ushort >(mgr.ReturnFirstIfSingleElseSecond(loopSingle, loopMulti)) != 0xFFFF) {
    mVisorLoopSfx = CSfxManager::SfxStart(
        mgr.ReturnFirstIfSingleElseSecond(loopSingle, loopMulti), mVisorSfxVol,
        mgr.GetPlayer(mPlayerIndex)->GetSoundPan(CPlayer::kMSP_4), CSfxManager::kAllAreas, false,
        true, CSfxManager::kMedPriority + 0x10);
  }
}

void CPlayerVisor::FinishTransitionIn(const CStateManager& mgr) {
  ushort loopSingle = 0xFFFF;
  ushort loopMulti = 0xFFFF;
  switch (mCurVisor) {
  case CPlayerState::kPV_Combat:
    mBlur.DisableBlur(0.f);
    break;
  case CPlayerState::kPV_Echo:
    break;
  case CPlayerState::kPV_Scan: {
    const CTweakGuiColors& colors = *gpTweakGuiColors;
    CColor dimColor = CColor::Lerp(colors.GetScanVisorScreenDimColor(),
                                   colors.GetScanVisorHUDLightMultiply(), mScanDimInterp);
    mScanDim.SetFilter(CCameraFilterPass::kFT_Multiply, CCameraFilterPass::kFS_Fullscreen, 0.f,
                       dimColor, kInvalidAssetId);
    loopSingle = 0x36;
    loopMulti = 0x2664;
    break;
  }
  case CPlayerState::kPV_Dark:
  default:
    break;
  }
  if (mCurVisor != CPlayerState::kPV_Combat && !mVisorLoopSfx &&
      static_cast< ushort >(mgr.ReturnFirstIfSingleElseSecond(loopSingle, loopMulti)) != 0xFFFF) {
    mVisorLoopSfx = CSfxManager::SfxStart(
        mgr.ReturnFirstIfSingleElseSecond(loopSingle, loopMulti), mVisorSfxVol,
        mgr.GetPlayer(mPlayerIndex)->GetSoundPan(CPlayer::kMSP_4), CSfxManager::kAllAreas, false,
        true, CSfxManager::kMedPriority + 0x10);
  }
}

void CPlayerVisor::UpdateCurrentVisor(float transFactor) {
  switch (mCurVisor) {
  case CPlayerState::kPV_Combat:
    break;
  case CPlayerState::kPV_Echo:
    break;
  case CPlayerState::kPV_Scan: {
    const CTweakGuiColors& colors = *gpTweakGuiColors;
    CColor dimColor =
        CColor::Lerp(colors.GetScanVisorHUDLightMultiply(), CColor::White(), 1.f - transFactor);
    mScanDim.SetFilter(CCameraFilterPass::kFT_Multiply, CCameraFilterPass::kFS_Fullscreen, 0.f,
                       dimColor, kInvalidAssetId);
    break;
  }
  case CPlayerState::kPV_Dark:
  default:
    break;
  }
}

void CPlayerVisor::Update(float dt, const CStateManager& mgr) {
  mBlur.Update(dt);
  const CPlayerState& playerState = *mgr.GetPlayerState(mPlayerIndex);
  CPlayerState::EPlayerVisor activeVisor = playerState.GetActiveVisor(mgr);
  CPlayerState::EPlayerVisor curVisor = playerState.GetCurrentVisor();
  CPlayerState::EPlayerVisor transVisor = playerState.GetTransitioningVisor();
  CPlayer::EPlayerScanState scanState = mgr.GetPlayer(mPlayerIndex)->GetPlayerScanState();
  bool visorTransitioning = playerState.GetIsVisorTransitioning();
  UpdateScanWindow(dt, mgr);
  if (transVisor != mNextVisor)
    mNextVisor = transVisor;
  LockUnlockAssets();
  if (scanState == CPlayer::kSS_ScanComplete)
    mScanDimInterp = rstl::max_val(0.f, mScanDimInterp - 2.f * dt);
  else
    mScanDimInterp = rstl::min_val(1.f, mScanDimInterp + 2.f * dt);
  if (visorTransitioning) {
    if (!mVisorTransitioning)
      BeginTransitionOut(mgr);
    if (curVisor != mCurVisor) {
      FinishTransitionOut(mgr);
      mCurVisor = curVisor;
      BeginTransitionIn(mgr);
    }
    UpdateCurrentVisor(playerState.GetVisorTransitionFactor());
  } else {
    if (mVisorTransitioning) {
      FinishTransitionIn(mgr);
    } else if (curVisor == CPlayerState::kPV_Scan) {
      const CTweakGuiColors& colors = *gpTweakGuiColors;
      CColor dimColor = CColor::Lerp(colors.GetScanVisorScreenDimColor(),
                                     colors.GetScanVisorHUDLightMultiply(), mScanDimInterp);
      mScanDim.SetFilter(CCameraFilterPass::kFT_Multiply, CCameraFilterPass::kFS_Fullscreen, 0.f,
                         dimColor, kInvalidAssetId);
    }
  }
  mVisorTransitioning = visorTransitioning;
  if (activeVisor != mCurVisor) {
    if (mVisorSfxVol != 0) {
      mVisorSfxVol = 0;
      CSfxManager::SfxVolume(mVisorLoopSfx, mVisorSfxVol);
      CSfxManager::SfxVolume(mScanningLoopSfx, mVisorSfxVol);
    }
  } else {
    if (mVisorSfxVol != 127) {
      mVisorSfxVol = 127;
      CSfxManager::SfxVolume(mVisorLoopSfx, mVisorSfxVol);
      CSfxManager::SfxVolume(mScanningLoopSfx, mVisorSfxVol);
    }
  }
  float scanMag = gpTweakGui->GetScanWindowMagnification();
  if (mScanMagInterp < scanMag)
    mScanMagInterp = rstl::min_val(scanMag, mScanMagInterp + 2.f * dt);
  else
    mScanMagInterp = rstl::max_val(scanMag, mScanMagInterp - 2.f * dt);
}

void CPlayerVisor::Draw(const CStateManager& mgr, const CTargetingManager* tgtMgr) const {
  CGraphics::SetAmbientColor(CColor::White());
  CGraphics::DisableAllLights();
  switch (mgr.GetPlayerState(mPlayerIndex)->GetActiveVisor(mgr)) {
  case CPlayerState::kPV_Combat:
    break;
  case CPlayerState::kPV_Echo:
    DrawEchoEffect(mgr);
    break;
  case CPlayerState::kPV_Dark:
    DrawDarkEffect(mgr);
    break;
  case CPlayerState::kPV_Scan:
    DrawScanEffect(mgr, tgtMgr);
    break;
  default:
    break;
  }
}

void CPlayerVisor::DrawDarkEffect(const CStateManager& mgr) const {}

void CPlayerVisor::DrawEchoEffect(const CStateManager& mgr) const {}

void CPlayerVisor::DrawScanEffect(const CStateManager& mgr,
                                  const CTargetingManager* const tgtMgr) const {
  const CViewport& viewport = CGraphics::GetViewport();
  const int vpLeft = viewport.mLeft;
  const int vpTop = viewport.mTop;
  const int vpWidth = viewport.mWidth;
  const int vpHeight = viewport.mHeight;
  const float transFactor = mgr.GetPlayerState(mPlayerIndex)->GetVisorTransitionFactor();
  const float scanSidesStart = gpTweakGui->GetScanSidesStartTime();
  const float scanSidesDuration = gpTweakGui->GetScanSidesDuration();
  float t;
  if (mNextState == kSWS_Scan)
    t = 1.f - (mWindowInterpTimer < scanSidesDuration
                   ? 0.f
                   : (mWindowInterpTimer - scanSidesDuration) / scanSidesStart);
  else
    t = mWindowInterpTimer > scanSidesStart ? 1.f : mWindowInterpTimer / scanSidesStart;
  const float divisor =
      transFactor * ((1.f - t) * mScanMagInterp + t * gpTweakGui->GetScanWindowScanningAspect()) +
      (1.f - transFactor);
  static const float skViewportLayoutScale[3] = {1.f, 0.8f, 0.6f};
  const float layoutScale = skViewportLayoutScale[mgr.GetViewportLayoutIndex()];
  const float windowX = mInterpWindowDims.GetX();
  const float windowY = mInterpWindowDims.GetY();
  const float vpW = 139.35576f * windowX;
  const float vpH = 125.36f * windowY;
  const int width = CMath::Clamp(skPixelsPerTileDimension16Bit,
                                 round_up_to_tile(layoutScale * (vpW / divisor)), vpWidth);
  const int height = CMath::Clamp(skPixelsPerTileDimension16Bit,
                                  round_up_to_tile(layoutScale * (vpH / divisor)), vpHeight);
  GXSetTexCopySrc(vpLeft + (vpWidth - width) / 2, vpTop + (vpHeight - height) / 2, width, height);
  void* const buffer = CGraphics::GetDolphinSpareBuffer();
  GXSetTexCopyDst(width, height, GX_TF_RGB565, GX_FALSE);
  GXCopyTex(buffer, GX_FALSE);
  GXPixModeSync();
  mScanDim.Draw();
  gpRender->SetViewportOrtho(true, -1.f, 1.f);
  const CTransform4f windowScale = CTransform4f::Scale(windowX, 1.f, windowY);
  const CTransform4f seventeenScale =
      CTransform4f::Scale(14.f * layoutScale, 1.f, 14.f * layoutScale);
  const CTransform4f mm = seventeenScale * windowScale;
  const CTransform4f verticalFlip = CTransform4f::Scale(1.f, 1.f, -1.f);
  const CTransform4f horizontalFlip = CTransform4f::Scale(-1.f, 1.f, 1.f);
  gpRender->SetModelMatrix(mm);
  if (!CMath::IsEpsilon(mScanDimInterp, 0.f, 0.00001f)) {
    CGraphics::LoadDolphinSpareTexture(width, height, GX_TF_RGB565, buffer, GX_TEXMAP0);
    GXInvalidateTexAll();
    const CColor paneColor =
        CColor::Lerp(gpTweakGuiColors->GetScanWindowTintColor(), CColor::White(), mScanDimInterp)
            .WithAlphaOf(transFactor);
    if (const CModel* pane = mScanFrameWindow.GetObject())
      pane->Draw(CModelFlags::AlphaBlended(paneColor).DontLoadTextures());
  }
  if (!CMath::IsEpsilon(mScanDimInterp, 1.f, 0.00001f)) {
    const float paneAlpha = 1.f - mScanDimInterp;
    const CColor paneColor = CColor::White().WithAlphaOf(paneAlpha);
    if (const CModel* pane = mScanFrameWindow.GetObject())
      pane->Draw(CModelFlags::AlphaBlended(paneColor).DepthCompareUpdate(false, false));
  }
  CGraphics::SetCullMode(kCM_None);
  CColor frameColor =
      CColor::Lerp(gpTweakGuiColors->GetScanWindowFrameBaseColor(),
                   gpTweakGuiColors->GetScanWindowFrameActiveColor(), mScanFrameColorInterp);
  frameColor = frameColor.WithAlphaOf(transFactor);
  const CColor impulseColor =
      CColor::Modulate(gpTweakGuiColors->GetScanWindowFrameFlashAddColor(),
                       CColor(mScanFrameColorImpulseInterp, mScanFrameColorImpulseInterp,
                              mScanFrameColorImpulseInterp, mScanFrameColorImpulseInterp));
  frameColor = CColor::Add(frameColor, impulseColor);
  const CModelFlags flags = CModelFlags::AlphaBlended(frameColor).DepthCompareUpdate(false, false);
  const float xScale = windowScale.Get00();
  const CVector3f up = windowScale.GetColumn(kDZ);
  const float zScale = up.GetZ();
  const CVector3f topPosition(0.f, 0.f, 4.447f * zScale);
  if (const CModel* model = mScanFrameCenterTop.GetObject()) {
    const CTransform4f modelXf =
        seventeenScale * CTransform4f::Translate(topPosition) *
        CTransform4f::Scale(2.f * (4.977f * xScale + 1.f - 2.432f) / 6.464f, 1.f, 1.f);
    gpRender->SetModelMatrix(modelXf);
    model->Draw(flags);
  }
  const CVector3f sidePosition(-4.977f * xScale, 0.f, 0.f);
  if (const CModel* model = mScanFrameCenterLeft.GetObject()) {
    const CTransform4f modelXf =
        seventeenScale * CTransform4f::Translate(sidePosition) *
        CTransform4f::Scale(1.f, 1.f, 2.f * (1.f + 4.447f * zScale - 2.233f) / 6.396743f);
    gpRender->SetModelMatrix(modelXf);
    model->Draw(flags);
    gpRender->SetModelMatrix(horizontalFlip * modelXf);
    model->Draw(flags);
  }
  const CVector3f cornerOffset(-4.977f, 0.f, 4.447f);
  const CVector3f cornerPosition = windowScale * cornerOffset;
  if (const CModel* model = mScanFrameFixedCorner.GetObject()) {
    const CTransform4f modelXf = seventeenScale * CTransform4f::Translate(cornerPosition);
    gpRender->SetModelMatrix(modelXf);
    model->Draw(flags);
    gpRender->SetModelMatrix(horizontalFlip * modelXf);
    model->Draw(flags);
  }
  const CVector3f bottomCornerOffset(-4.977f, 0.f, -4.447f);
  const CVector3f bottomCornerPosition = windowScale * bottomCornerOffset;
  if (const CModel* model = mScanFrameBottomLeftCorner.GetObject()) {
    const CTransform4f modelXf = seventeenScale * CTransform4f::Translate(bottomCornerPosition);
    gpRender->SetModelMatrix(modelXf);
    model->Draw(flags);
    gpRender->SetModelMatrix(horizontalFlip * modelXf);
    model->Draw(flags);
  }
  const float stretchWidth = (2.f * (4.977f * xScale - 0.4f) - 8.697885f) * 0.5f;
  const CVector3f stretchPosition(-4.3489423f, 0.f, 4.447f * -zScale);
  if (const CModel* model = mScanFrameStretchCorner.GetObject()) {
    const CTransform4f modelXf = seventeenScale * CTransform4f::Translate(stretchPosition) *
                                 CTransform4f::Scale(stretchWidth / 0.151104f, 1.f, 1.f);
    gpRender->SetModelMatrix(modelXf);
    model->Draw(flags);
    gpRender->SetModelMatrix(horizontalFlip * modelXf);
    model->Draw(flags);
  }
  const CVector3f lowerPosition(0.f, 0.f, 4.447f * -zScale);
  if (const CModel* model = mScanFrameLowerRight.GetObject()) {
    const CTransform4f modelXf = seventeenScale * CTransform4f::Translate(lowerPosition);
    gpRender->SetModelMatrix(modelXf);
    model->Draw(flags);
  }
  CGraphics::SetCullMode(kCM_Front);
}

void CPlayerVisor::LockUnlockAssets() {
  if (mCurVisor == CPlayerState::kPV_Scan)
    mAssetLockCountdown = 2;
  else
    --mAssetLockCountdown;
  if (mAssetLockCountdown > 0) {
    mScanFrameFixedCorner.Lock();
    mScanFrameCenterLeft.Lock();
    mScanFrameCenterTop.Lock();
    mScanFrameWindow.Lock();
    mScanFrameBottomLeftCorner.Lock();
    mScanFrameStretchCorner.Lock();
    mScanFrameLowerRight.Lock();
    mScanFrameFixedCorner.IsLoaded();
    mScanFrameCenterLeft.IsLoaded();
    mScanFrameCenterTop.IsLoaded();
    mScanFrameWindow.IsLoaded();
    mScanFrameBottomLeftCorner.IsLoaded();
    mScanFrameStretchCorner.IsLoaded();
    mScanFrameLowerRight.IsLoaded();
  } else {
    mScanFrameFixedCorner.Unlock();
    mScanFrameCenterLeft.Unlock();
    mScanFrameCenterTop.Unlock();
    mScanFrameWindow.Unlock();
    mScanFrameBottomLeftCorner.Unlock();
    mScanFrameStretchCorner.Unlock();
    mScanFrameLowerRight.Unlock();
  }
}

CPlayerVisor::EScanWindowState
CPlayerVisor::GetDesiredScanWindowState(const CStateManager& mgr) const {
  const CPlayer* player = mgr.GetPlayer(mPlayerIndex);
  CPlayerState::EPlayerVisor visor = player->GetPlayerState()->GetCurrentVisor();
  CPlayer::EPlayerScanState scanState = player->GetPlayerScanState();
  if (visor == CPlayerState::kPV_Scan) {
    if (scanState == CPlayer::kSS_Scanning || scanState == CPlayer::kSS_ScanComplete)
      return kSWS_Scan;
    return kSWS_Idle;
  }
  return kSWS_NotInScanVisor;
}

void CPlayerVisor::UpdateScanWindow(float dt, const CStateManager& mgr) {
  const CPlayer* player = mgr.GetPlayer(mPlayerIndex);
  CPlayer::EPlayerScanState scanState = player->GetPlayerScanState();
  float scanTimer = player->GetScanningTime();
  if (scanState == CPlayer::kSS_Scanning) {
    if (!mScanningLoopSfx) {
      if (mgr.IsMultiplayer()) {
        mScanningLoopSfx =
            CSfxManager::SfxStart(0x2613, mVisorSfxVol, player->GetSoundPan(CPlayer::kMSP_4),
                                  CSfxManager::kAllAreas, false, true, CSfxManager::kMedPriority);
      } else {
        mScanningLoopSfx =
            CSfxManager::SfxStart(0x3ae, mVisorSfxVol, player->GetSoundPan(CPlayer::kMSP_4),
                                  CSfxManager::kAllAreas, false, true, CSfxManager::kMedPriority);
      }
    }
  } else {
    CSfxManager::SfxStop(CSfxManager::kSC_Game, mScanningLoopSfx);
    mScanningLoopSfx.Clear();
  }
  const bool scanTimerChanged = mScanTimer != scanTimer;
  mScanTimer = scanTimer;
  EScanWindowState desiredState = GetDesiredScanWindowState(mgr);
  switch (mNextState) {
  case kSWS_NotInScanVisor:
    if (desiredState != kSWS_NotInScanVisor) {
      if (mPrevState == kSWS_NotInScanVisor)
        mInterpWindowDims = mScanWindowSizes[desiredState];
      mNextWindowDims = mScanWindowSizes[desiredState];
      mPrevWindowDims = mInterpWindowDims;
      mPrevState = mNextState;
      mNextState = desiredState;
      mWindowInterpDuration =
          desiredState == kSWS_Scan ? gpTweakGui->GetScanSidesEndTime() - mWindowInterpTimer : 0.f;
      mWindowInterpTimer = mWindowInterpDuration;
    }
    break;
  case kSWS_Idle:
    if (desiredState != kSWS_Idle) {
      mNextWindowDims =
          desiredState == kSWS_NotInScanVisor ? mInterpWindowDims : mScanWindowSizes[desiredState];
      mPrevWindowDims = mInterpWindowDims;
      mPrevState = mNextState;
      mNextState = desiredState;
      mWindowInterpDuration =
          desiredState == kSWS_Scan ? gpTweakGui->GetScanSidesEndTime() - mWindowInterpTimer : 0.f;
      mWindowInterpTimer = mWindowInterpDuration;
      if (desiredState == kSWS_Scan)
        CSfxManager::SfxStart(0x3b0, mVisorSfxVol, player->GetSoundPan(CPlayer::kMSP_4),
                              CSfxManager::kAllAreas, false, false, CSfxManager::kMedPriority);
    }
    break;
  case kSWS_Scan:
    if (desiredState != kSWS_Scan) {
      mNextWindowDims =
          desiredState == kSWS_NotInScanVisor ? mInterpWindowDims : mScanWindowSizes[desiredState];
      mPrevWindowDims = mInterpWindowDims;
      mPrevState = mNextState;
      mNextState = desiredState;
      mWindowInterpDuration =
          desiredState == kSWS_Idle ? gpTweakGui->GetScanSidesEndTime() - mWindowInterpTimer : 0.f;
      mWindowInterpTimer = mWindowInterpDuration;
      if (mgr.GetPlayerState(mPlayerIndex)->GetVisorTransitionFactor() == 1.f &&
          !mgr.IsMultiplayer())
        CSfxManager::SfxStart(0x3af, mVisorSfxVol, player->GetSoundPan(CPlayer::kMSP_4),
                              CSfxManager::kAllAreas, false, false, CSfxManager::kMedPriority);
    }
    if (scanTimerChanged && mgr.IsMultiplayer() &&
        player->GetPlayerScanState() == CPlayer::kSS_ScanComplete)
      CSfxManager::SfxStart(0x2614, mVisorSfxVol, player->GetSoundPan(CPlayer::kMSP_4),
                            CSfxManager::kAllAreas, false, false, CSfxManager::kMedPriority);
    break;
  default:
    break;
  }
  if (mPrevState != mNextState) {
    mWindowInterpTimer = rstl::max_val(0.f, mWindowInterpTimer - dt);
    if (mWindowInterpTimer == 0.f)
      mPrevState = mNextState;
    float t = 0.f;
    if (mWindowInterpDuration > 0.f) {
      float scanSidesStart = gpTweakGui->GetScanSidesStartTime();
      float scanSidesDuration = gpTweakGui->GetScanSidesDuration();
      if (mNextState == kSWS_Scan)
        t = mWindowInterpTimer < scanSidesDuration
                ? 0.f
                : (mWindowInterpTimer - scanSidesDuration) / scanSidesStart;
      else
        t = mWindowInterpTimer > scanSidesStart ? 1.f : mWindowInterpTimer / scanSidesStart;
    }
    mInterpWindowDims = CVector2f::Lerp(mNextWindowDims, mPrevWindowDims, t);
  }
}

CVector2i CPlayerVisor::GetScanWindowViewportSize(const CStateManager& mgr) const {
  const CViewport& viewport = CGraphics::GetViewport();
  const int vpWidth = viewport.mWidth;
  const int vpHeight = viewport.mHeight;
  const float transFactor = mgr.GetPlayerState(mPlayerIndex)->GetVisorTransitionFactor();
  const float scanSidesStart = gpTweakGui->GetScanSidesStartTime();
  const float scanSidesDuration = gpTweakGui->GetScanSidesDuration();
  float t;
  if (mNextState == kSWS_Scan)
    t = 1.f - (mWindowInterpTimer < scanSidesDuration
                   ? 0.f
                   : (mWindowInterpTimer - scanSidesDuration) / scanSidesStart);
  else
    t = mWindowInterpTimer > scanSidesStart ? 1.f : mWindowInterpTimer / scanSidesStart;
  const float divisor =
      transFactor * ((1.f - t) * mScanMagInterp + t * gpTweakGui->GetScanWindowScanningAspect()) +
      (1.f - transFactor);
  static const float skViewportLayoutScale[3] = {1.f, 0.8f, 0.6f};
  const float layoutScale = skViewportLayoutScale[mgr.GetViewportLayoutIndex()];
  const float windowX = mInterpWindowDims.GetX();
  const float windowY = mInterpWindowDims.GetY();
  const float vpW = layoutScale * (139.35576f * windowX);
  const float vpH = layoutScale * (125.36f * windowY);
  const int width =
      CMath::Clamp(skPixelsPerTileDimension16Bit, round_up_to_tile(vpW / divisor), vpWidth);
  const int height =
      CMath::Clamp(skPixelsPerTileDimension16Bit, round_up_to_tile(vpH / divisor), vpHeight);
  return CVector2i(width, height);
}
