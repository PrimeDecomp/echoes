#include "MetroidPrime/CInGameGuiManager.hpp"

#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiCamera.hpp"
#include "GuiSys/CGuiFrameLoader.hpp"
#include "GuiSys/CGuiHeadWidget.hpp"
#include "GuiSys/CGuiWidgetDrawParms.hpp"
#include "Kyoto/CDependencyGroup.hpp"
#include "Kyoto/CFrameDelayedKiller.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Audio/CStreamAudioManager.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Input/IController.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "MetroidPrime/CAutoMapper.hpp"
#include "MetroidPrime/CArchitectureQueue.hpp"
#include "MetroidPrime/Decode.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/CGameGlobalObjects.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/CInGameQuitScreen.hpp"
#include "MetroidPrime/CMessageScreen.hpp"
#include "MetroidPrime/CPauseScreen.hpp"
#include "MetroidPrime/CPauseScreenBlur.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CGameMode.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakAutoMapper.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CTurretHud.hpp"
#include "MetroidPrime/HUD/CSamusHud.hpp"
#include "MetroidPrime/Player/CPlayerVisor.hpp"
#include "MetroidPrime/Player/CSamusFaceReflection.hpp"
#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"

#include <limits.h>

static const char* const skInGameGuiDGRPs[] = {
    "InGameGui_DGRP", "grappleArm_DGRP", "Bomb_DGRP", "Common_DGRP", "Ice_DGRP", "Phazon_DGRP",
    "Plasma_DGRP", "Power_DGRP", "Wave_DGRP", "BallTransition_DGRP", "SamusFace_DGRP",
    "SamusBallCMDL_DGRP"};

static float skMapScreenCameraOffset = 2.f;
static uint skInGameTextureReloadPriority = 0;

struct SDumpableTextureInfo {
  CAssetId mId;
  int mScore;
  TToken< CTexture > mToken;

  SDumpableTextureInfo(int score, CAssetId id, TToken< CTexture >& token)
  : mId(id), mScore(score), mToken(token) {}
};

struct CTextureScoreGreaterThan {
  bool operator()(const SDumpableTextureInfo& a, const SDumpableTextureInfo& b) const {
    return a.mScore < b.mScore;
  }
};

CInGameGuiManager::CInGameGuiManager(const CStateManager& mgr, CGuiFrameLoader& hud,
                                     CGuiFrameLoader& memo, CGuiFrameLoader* helmet,
                                     CGuiFrameLoader* darkMask, int playerIndex)
: mPlayerIndex(playerIndex)
, mIsSinglePlayer(mgr.GetViewportLayoutIndex() == 0)
, mDeathDot(gpSimplePool->GetObj("TXTR_DeathDot"))
, mFaceplateDecoration(mgr, playerIndex)
, mPlayerVisor(rs_new CPlayerVisor(mgr, playerIndex))
, mSamusHud(rs_new CSamusHud(mgr, hud, memo, helmet, playerIndex))
, mAutoMapper(mIsSinglePlayer ? rs_new CAutoMapper(mgr, playerIndex) : nullptr)
, mSamusReflection(nullptr)
, mPauseScreenBlur(rs_new CPauseScreenBlur())
, mQuitScreen(nullptr)
, mMessageScreen(nullptr)
, mPauseScreen(nullptr)
, mTurretHud(nullptr)
, mPauseGameHudMessage(kInvalidAssetId)
, mPauseGameHudTime(0.f)
, mPauseScreenDGRPs(LockPauseScreenDependencies())
, mPrevState(kIGGS_Zero)
, mNextState(kIGGS_Zero)
, mHelmetVisMode(gpTweakGui->GetHelmetVisMode())
, mEnableTargetingManager(gpTweakGui->GetEnableTargetingManager())
, mEnableAutoMapper(gpTweakGui->GetEnableAutoMapper())
, mHudVisMode(gpTweakGui->GetHudVisMode())
, mEnablePlayerVisor(gpTweakGui->GetEnablePlayerVisor())
, mAutoMapperRotation(CQuaternion::NoRotation())
, mAutoMapperOffset(CVector3f::Zero())
, mCameraRotation(CQuaternion::NoRotation())
, mCameraOffset(CVector3f::Zero())
, mMapCameraTransform(CTransform4f::Identity())
, mVisorStaticAlpha(mgr.GetPlayer(playerIndex)->GetVisorStaticAlpha())
, mDarkOuterMask(nullptr)
, mLoaded(false)
, mPlayerAlive(true)
, mDeferTransition(false) {
  mDeathDot.Lock();
  if (gpGameState->GetGameMode().GetGameModeType() == 'SNGL') {
    mInGameGuiDGRPs.reserve(12);
    for (uint i = 0; i < 12; ++i) {
      TToken< CDependencyGroup > token = gpSimplePool->GetObj(skInGameGuiDGRPs[i]);
      token.Lock();
      mInGameGuiDGRPs.push_back(token);
    }
  }
  if (darkMask != nullptr) {
    mDarkMaskFrame = rstl::auto_ptr< CGuiFrame >(darkMask->CreateFrame());
  }
}

bool CInGameGuiManager::CheckDGRPLoadComplete() {
  for (TPauseScreenDGRPs::iterator it = mPauseScreenDGRPs.begin();
       it != mPauseScreenDGRPs.end(); ++it) {
    if (!it->IsLoaded()) {
      return false;
    }
  }
  for (rstl::vector< TToken< CDependencyGroup > >::iterator it = mInGameGuiDGRPs.begin();
       it != mInGameGuiDGRPs.end(); ++it) {
    if (!it->IsLoaded()) {
      return false;
    }
  }
  return true;
}

bool CInGameGuiManager::CheckLoadComplete(const CStateManager& mgr) {
  if (mLoaded) {
    return true;
  }
  if (!CheckDGRPLoadComplete()) {
    return false;
  }
  if (mDarkMaskFrame.get() != nullptr) {
    if (!mDarkMaskFrame->GetIsFinishedLoading()) {
      return false;
    }
    mDarkOuterMask = mDarkMaskFrame->FindWidget("model_dark_outermask");
  }
  if (mAutoMapper.null()) {
    CGuiWidget* mapWidget = mSamusHud->GetLoadedHudFrame()->FindWidget(rstl::string_l("model_automapper"));
    if (mapWidget) {
      mapWidget->SetVisibility(false, kTM_Children);
    }
  }
  if ((!mAutoMapper.null() && !mAutoMapper->CheckLoadComplete()) ||
      !mSamusHud->CheckLoadComplete(mgr) || !mDeathDot.TryCache()) {
    return false;
  }

  if (!mAutoMapper.null()) {
    CGuiWidget* root = mSamusHud->GetAutomapperRoot();
    CGuiCamera* camera = mSamusHud->GetHudCamera();
    if (root && camera) {
      CTransform4f rotation = root->GetWorldTransform();
      rotation.Orthonormalize();
      mAutoMapperRotation = CQuaternion::FromMatrix(rotation);
      mAutoMapperOffset = root->GetWorldTransform().GetTranslation();
      mCameraRotation = CQuaternion::NoRotation();
      mCameraOffset = camera->GetWorldTransform().GetTranslation() +
                      CVector3f(0.f, skMapScreenCameraOffset, gpTweakAutoMapper->GetCamVerticalOffset());
      mMapCameraTransform = CTransform4f(mCameraRotation.BuildTransform(), mCameraOffset);
    }
  }
  InitializeDumpableARAMTextures();
  mLoaded = true;
  return true;
}

bool CInGameGuiManager::GetIsGameDraw() const {
  return mPauseScreenBlur->IsGameDraw();
}

void CInGameGuiManager::PrepareScanDisplay(const CStateManager& mgr, int playerIndex) {
  if (!mSamusHud.null()) {
    mSamusHud->PrepareScanDisplay(mgr, playerIndex);
  }
}

void CInGameGuiManager::PreDraw(CStateManager& mgr, bool cameraActive) {
  if (!mSamusReflection.null() && cameraActive) {
    mSamusReflection->PreDraw(mgr);
  }
}

void CInGameGuiManager::DrawDarkVisorMask(const CStateManager& mgr) const {
  if (mDarkMaskFrame.get() != nullptr) {
    CGraphics::SetDepthRange(1.f / 512.f, 1.f / 256.f);
    mSamusHud->GetLoadedHudFrame()->GetRootWidget()->Draw(CGuiWidgetDrawParms::Default());
    mDarkMaskFrame->GetRootWidget()->Draw(CGuiWidgetDrawParms::Default());
  }
}

void CInGameGuiManager::DrawScanVisor(float time, const CStateManager& mgr, const CColor& sweepColor,
                                      const CColor& inactiveColor, const CColor& inactiveExternalColor,
                                      const CColor* palette, int paletteSize,
                                      const CVector3f& direction) const {
  if (!mPlayerVisor.null()) {
    const CVector2i size = mPlayerVisor->GetScanWindowViewportSize(mgr);
    gpRender->DrawScanVisor(time, float(size.GetX()), float(size.GetY()), sweepColor, inactiveColor,
                             inactiveExternalColor, palette, paletteSize, direction);
  }
}

void CInGameGuiManager::Draw(const CStateManager& mgr) const {
  const bool notInCine = !mgr.GetCameraManager(mPlayerIndex)->IsInCinematicCamera();
  float staticAlpha = 0.f;
  const CPlayer& player = *mgr.GetPlayer(mPlayerIndex);
  const float deathTime = player.GetDeathTime();
  if (player.GetMorphballTransitionState() == CPlayer::kMS_Unmorphed && deathTime > 0.f) {
    staticAlpha = CMath::Clamp(0.f, deathTime / 0.75f, 1.f);
  }
  const bool inTurret = player.IsInTurret();
  if (mPauseScreenBlur->IsGameDraw()) {
    if (mHelmetVisMode != 0 && notInCine) {
      CGraphics::SetDepthRange(1.f / 512.f, 1.f / 256.f);
      mSamusHud->DrawHelmet(mgr, 0.f);
    }
    if (mgr.GetPlayer(mPlayerIndex)->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed) {
      mSamusHud->GetTargetingManager().Draw(mgr, true);
    }
    CGraphics::SetDepthRange(1.f / 64.f, 1.f / 32.f);
    const CPlayer::EPlayerMorphBallState morphState =
        mgr.GetPlayer(mPlayerIndex)->GetMorphballTransitionState();
    const bool drawVisor = !inTurret && notInCine &&
                           (mPrevState == kIGGS_InGame || mNextState == kIGGS_InGame ||
                            morphState == CPlayer::kMS_Morphed);
    const bool targeting = morphState != CPlayer::kMS_Morphed;
    if (inTurret && !mTurretHud.null()) {
      mTurretHud->Draw();
      mSamusHud->DrawHudMemo();
    }
    if (drawVisor) {
      if (mgr.GetPlayer(mPlayerIndex)->GetCameraState() == CPlayer::kCS_FirstPerson) {
        mFaceplateDecoration.Draw(mgr);
      }
      mPlayerVisor->Draw(mgr, nullptr);
    }
    if (!mSamusReflection.null()) {
      mSamusReflection->Draw(mgr);
    }
    if (drawVisor) {
      CGraphics::SetDepthRange(1.f / 512.f, 1.f / 256.f);
      if (staticAlpha > 0.f) {
        CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Blend,
                                      CCameraFilterPass::kFS_RandomStatic,
                                      CColor::White().WithAlphaOf(staticAlpha), nullptr, 1.f);
      }
      mSamusHud->Draw(mgr, mVisorStaticAlpha * (1.f - staticAlpha), 1, true, targeting);
      mSamusHud->DrawHudMemo();
    }
  }

  const bool preDrawBlur =
      (InGameGuiStates::IsGameplayState(mPrevState) &&
       InGameGuiStates::IsGameplayState(mNextState)) ||
      mPrevState == kIGGS_MapScreen || mNextState == kIGGS_MapScreen;
  if (preDrawBlur) {
    mPauseScreenBlur->Draw(mgr);
  }
  if (!mAutoMapper.null() && notInCine &&
      (!inTurret || mPrevState == kIGGS_MapScreen || mNextState == kIGGS_MapScreen) &&
      (mPauseScreenBlur->IsGameDraw() || mPrevState == kIGGS_MapScreen ||
       mNextState == kIGGS_MapScreen) && mgr.GetPendingDockArea() == kInvalidAreaId) {
    const CPlayerState& playerState = *mgr.GetPlayerState(mPlayerIndex);
    float mapAlpha = 1.f;
    if (playerState.GetCurrentVisor() == CPlayerState::kPV_Scan) {
      mapAlpha = 0.f;
    } else if (playerState.GetTransitioningVisor() == CPlayerState::kPV_Scan) {
      mapAlpha = playerState.GetVisorTransitionFactor();
    }
    if (mSamusHud->GetLoadedHudFrame() != nullptr) {
      mSamusHud->GetLoadedHudFrame()->GetFrameCamera()->Draw(
          CGuiWidgetDrawParms(0.f, CVector3f::Zero()));
    }
    CGraphics::SetDepthRange(0.f, 1.f / 512.f);
    CGuiWidget* model = mSamusHud->GetAutomapperModel();
    if (model != nullptr) {
      model->SetIsVisible(true);
      model->Draw(CGuiWidgetDrawParms(1.f, CVector3f::Zero()));
    }
    CGraphics::SetDepthWriteMode(true, kE_GEqual, false);
    mAutoMapper->Draw(mgr, CTransform4f::Translate(0.f, 0.02f, 0.f) * mMapCameraTransform,
                      mVisorStaticAlpha * mapAlpha);
    CGraphics::SetDepthWriteMode(true, kE_LEqual, true);
    if (model != nullptr) {
      model->SetIsVisible(false);
    }
  }
  if (!preDrawBlur) {
    mPauseScreenBlur->Draw(mgr);
  }
  if (!mMessageScreen.null()) {
    mMessageScreen->Draw();
  }
  if (!mPauseScreen.null()) {
    mPauseScreen->Draw();
  }

  if (deathTime > 0.f) {
    const float alpha = CMath::Clamp(0.f, deathTime / 2.5f, 1.f);
    CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Blend, CCameraFilterPass::kFS_Fullscreen,
                                  CColor::White().WithAlphaOf(alpha), nullptr, 1.f);
    if (deathTime > 0.5f) {
      const float zT = 1.f - CMath::Clamp(0.f, (deathTime - 0.5f) / 0.5f, 1.f);
      const float xT = CMath::Clamp(0.f, (deathTime - 1.f) / 0.5f, 1.f);
      const float colorT = 1.f - CMath::Clamp(0.f, deathTime - 1.5f, 1.f);
      const CViewport& viewport = CGraphics::GetViewport();
      const int width = viewport.mWidth;
      const int height = viewport.mHeight;
      void* spareBuffer = CGraphics::GetDolphinSpareBuffer();
      GXSetTexCopySrc(viewport.mLeft, viewport.mTop, width, height);
      const int halfWidth = width / 2;
      const int halfHeight = height / 2;
      GXGetTexBufferSize(halfWidth, halfHeight, GX_TF_RGB565, false, 0);
      GXSetTexCopyDst(halfWidth, halfHeight, GX_TF_RGB565, true);
      GXCopyTex(spareBuffer, false);
      GXPixModeSync();
      CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Blend, CCameraFilterPass::kFS_Fullscreen,
                                    CColor::Black(), nullptr, 1.f);
      CGraphics::LoadDolphinSpareTexture(halfWidth, halfHeight, GX_TF_RGB565, spareBuffer,
                                       GX_TEXMAP0);
      const float zFactor = zT * zT * zT * zT * zT;
      const float z = 0.5f * (zFactor * (height - 12.f) + 12.f);
      const float negZ = -z;
      const float x = 0.5f * ((1.f - xT) * (width - 12.f) + 12.f);
      const float negX = -x;
      CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
      CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
      CGraphics::StreamBegin(kP_TriangleStrip);
      CGraphics::StreamColor(CColor::White().WithAlphaOf(colorT));
      CGraphics::StreamTexcoord(0.f, 0.f);
      CGraphics::StreamVertex(CVector3f(negX, 0.f, z));
      CGraphics::StreamTexcoord(0.f, 1.f);
      CGraphics::StreamVertex(CVector3f(negX, 0.f, negZ));
      CGraphics::StreamTexcoord(1.f, 0.f);
      CGraphics::StreamVertex(CVector3f(x, 0.f, z));
      CGraphics::StreamTexcoord(1.f, 1.f);
      CGraphics::StreamVertex(CVector3f(x, 0.f, negZ));
      CGraphics::StreamEnd();
      gpRender->SetBlendMode_ColorMultiply();
      mDeathDot.GetObject()->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
      CGraphics::StreamBegin(kP_TriangleStrip);
      CGraphics::StreamColor(CColor::White().WithAlphaOf(colorT));
      CGraphics::StreamTexcoord(0.f, 0.f);
      CGraphics::StreamVertex(CVector3f(negX, 0.f, z));
      CGraphics::StreamTexcoord(0.f, 1.f);
      CGraphics::StreamVertex(CVector3f(negX, 0.f, negZ));
      CGraphics::StreamTexcoord(1.f, 0.f);
      CGraphics::StreamVertex(CVector3f(x, 0.f, z));
      CGraphics::StreamTexcoord(1.f, 1.f);
      CGraphics::StreamVertex(CVector3f(x, 0.f, negZ));
      CGraphics::StreamEnd();
    }
  }
  if (!mQuitScreen.null()) {
    CGraphics::SetDepthRange(1.f / 512.f, 0.f);
    mQuitScreen->Draw();
  }
}

void CInGameGuiManager::Update(const CStateManager& mgr, float dt, CRandom16& random,
                               CArchitectureQueue& queue, bool cameraActive) {
  const bool pendingDock = mgr.GetPendingDockArea() != kInvalidAreaId;
  EnsureStates(mgr);
  if (!mQuitScreen.null()) {
    const EQuitAction action = mQuitScreen->Update(dt, const_cast< CStateManager& >(mgr));
    if (action == kQA_Yes) {
      queue.Push(MakeMsg::CreateQuitGameplay(kAMT_Game));
    } else if (action == kQA_No) {
      mQuitScreen = nullptr;
    }
    return;
  }

  mSamusHud->GetTargetingManager().Touch();
  if (cameraActive) {
    const float visorStaticAlpha = mgr.GetPlayer(mPlayerIndex)->GetVisorStaticAlpha();
    if (visorStaticAlpha != mVisorStaticAlpha) {
      if (CCameraManager::CastGameCameratoFirstPersonCamera(
              mgr.GetCameraManager(mPlayerIndex)->GetCurrentCamera(mgr, true))) {
        if (CMath::AbsF(visorStaticAlpha - mVisorStaticAlpha) < 0.5f) {
          if (mVisorStaticAlpha == 0.f) {
            CSfxManager::SfxStart(0x274, 127, 64, CSfxManager::kAllAreas, false, false,
                                  CSfxManager::kMedPriority);
          } else if (mVisorStaticAlpha == 1.f) {
            CSfxManager::SfxStart(0x273, 127, 64, CSfxManager::kAllAreas, false, false,
                                  CSfxManager::kMedPriority);
          }
        }
      }
    }
    mVisorStaticAlpha = visorStaticAlpha;
  }

  const bool inTurret = mgr.GetPlayer(mPlayerIndex)->GetTurretState() != CPlayer::kTS_None;
  if (inTurret && mTurretHud.null()) {
    mTurretHud = rs_new CTurretHud(mgr, mPlayerIndex);
  } else if (!inTurret && !mTurretHud.null()) {
    mTurretHud = nullptr;
  }
  if (inTurret && !mTurretHud.null()) {
    mTurretHud->Update(dt, mgr);
    mSamusHud->UpdateHudMemo(dt, mgr);
  }
  if (cameraActive) {
    mFaceplateDecoration.Update(mgr);
  }

  if (mIsSinglePlayer) {
    if (!CCameraManager::CastGameCameratoFirstPersonCamera(
            mgr.GetCameraManager(mPlayerIndex)->GetCurrentCamera(mgr, true))) {
      mSamusReflection = nullptr;
    } else if (mSamusReflection.null()) {
      mSamusReflection = rs_new CSamusFaceReflection(mgr, mPlayerIndex);
    }
  }
  if (!mSamusReflection.null() && cameraActive && !pendingDock) {
    mSamusReflection->Update(dt, mgr, random);
  }
  if (cameraActive) {
    mPlayerVisor->Update(dt, mgr);
  }
  if (cameraActive || mSamusHud->GetHudCamera() == nullptr) {
    if (mPlayerAlive) {
      mSamusHud->Update(dt, mgr, 1, true, true);
    }
    if (mgr.IsMultiplayer()) {
      mSamusHud->UpdateHudFrame(dt, mgr);
    }
  }
  if (!pendingDock) {
    UpdateAutoMapper(mgr, dt);
  }

  CPlayer& player = *const_cast< CPlayer* >(mgr.GetPlayer(mPlayerIndex));
  mPlayerAlive = player.GetPlayerState()->IsPlayerAlive();
  if (mgr.GetCameraManager(mPlayerIndex)->IsInCinematicCamera()) {
    player.SetViewportScaleX(1.f);
    player.SetViewportScaleY(1.f);
  } else {
    const float scaleX = rstl::min_val(mSamusHud->GetDesiredViewportScaleX(),
                                     mPlayerVisor->GetDesiredViewportScaleX(mgr));
    const float scaleY = rstl::min_val(mSamusHud->GetDesiredViewportScaleY(),
                                     mPlayerVisor->GetDesiredViewportScaleY(mgr));
    player.SetViewportScaleX(scaleX);
    player.SetViewportScaleY(scaleY);
  }
  mPauseScreenBlur->Update(dt, mgr, mDumpedTextures.empty());

  if (mPrevState == kIGGS_PauseSaveGame && mNextState == kIGGS_PauseSaveGame) {
    if (!mgr.HasSaveGameScreen()) {
      BeginStateTransition(kIGGS_InGame, mgr);
    }
  } else if (!mMessageScreen.null()) {
    if (!mMessageScreen->Update(dt, mPauseScreenBlur->GetBlurAmt())) {
      BeginStateTransition(kIGGS_InGame, mgr);
    }
  }
  if (!mPauseScreen.null()) {
    mPauseScreen->Update(dt, mgr, queue);
    if (mPauseScreen->IsDone() && mNextState != kIGGS_InGame) {
      BeginStateTransition(kIGGS_InGame, mgr);
    }
  }
  if (mPrevState != mNextState) {
    if (InGameGuiStates::IsGameplayState(mNextState)) {
      TryReloadAreaTextures();
    }
    if (IsTransitionReady()) {
      TryCompleteStateTransition();
    }
  }
}

void CInGameGuiManager::ProcessControllerInput(const CStateManager& mgr,
                                               const CFinalInput& input,
                                               CArchitectureQueue& queue) {
  if (!mQuitScreen.null()) {
    mQuitScreen->ProcessUserInput(input);
  } else if (IsInPausedState()) {
    if (mPrevState >= kIGGS_MapScreen && mPrevState <= kIGGS_QuitGame &&
        mNextState >= kIGGS_MapScreen && mNextState <= kIGGS_QuitGame) {
      if (mPrevState == kIGGS_MapScreen) {
        if (mAutoMapper->IsInMapperState(CAutoMapper::kAMS_MapScreen) ||
            mAutoMapper->IsInMapperState(CAutoMapper::kAMS_MapScreenUniverse)) {
          mAutoMapper->ProcessControllerInput(input, const_cast< CStateManager& >(mgr));
          if (mAutoMapper->CanLeaveMapScreen(mgr)) {
            BeginStateTransition(kIGGS_InGame, mgr);
          }
        }
      } else if (mPrevState == kIGGS_PauseSaveGame) {
      } else if (mPrevState == kIGGS_PauseHUDMessage) {
        mMessageScreen->ProcessControllerInput(input);
      } else if (!mPauseScreen.null()) {
        mPauseScreen->ProcessControllerInput(input);
      }
    }
  } else {
    mSamusHud->ProcessControllerInput(input);
  }
}

void CInGameGuiManager::UpdateAutoMapper(const CStateManager& mgr, float dt) {
  if (mAutoMapper.null() || gpMain->IsMaxSpeed()) {
    return;
  }
  mAutoMapper->Update(dt, const_cast< CStateManager& >(mgr));
  CGuiWidget* root = mSamusHud->GetAutomapperRoot();
  if (root != nullptr) {
    const CTransform4f xf = root->GetParent()->GetWorldTransform() * root->GetIdleXform();
    mAutoMapperRotation = CQuaternion::FromMatrix(xf);
    mAutoMapperOffset = xf.GetTranslation();
  }

  CGuiCamera* camera = mSamusHud->GetHudCamera();
  if (camera == nullptr) {
    return;
  }
  const CTransform4f& cameraXf = camera->GetWorldTransform();
  mCameraRotation = CQuaternion::FromMatrix(cameraXf);
  mCameraOffset = cameraXf.GetTranslation() + skMapScreenCameraOffset * cameraXf.GetForward() +
                  gpTweakAutoMapper->GetCamVerticalOffset() * cameraXf.GetUp();

  const float frameLength =
      CMath::SlowTangentR(0.5f * CMath::Deg2Rad(camera->GetParms().mPerspective.mFov)) / 0.7f;
  const float scaleX = frameLength * gpTweakAutoMapper->GetMapScreenClipWindowScaleX();
  const float scaleZ = frameLength * gpTweakAutoMapper->GetMapScreenClipWindowScaleY();
  if (mAutoMapper->IsFullyOutOfMiniMapState()) {
    if (mSamusHud->GetAutomapperModel() != nullptr) {
      mSamusHud->GetAutomapperModel()->SetO2WTransform(
          CTransform4f(mCameraRotation.BuildTransform(), mCameraOffset) *
          CTransform4f::Scale(scaleX, 1.f, scaleZ));
    }
    mMapCameraTransform = CTransform4f(mCameraRotation.BuildTransform(), mCameraOffset) *
                          CTransform4f::Scale(frameLength, 1.f, frameLength);
  } else if (mAutoMapper->IsFullyInMiniMapState()) {
    if (mSamusHud->GetAutomapperModel() != nullptr) {
      mSamusHud->GetAutomapperModel()->SetO2WTransform(
          CTransform4f(mAutoMapperRotation.BuildTransform(), mAutoMapperOffset));
      mMapCameraTransform = mSamusHud->GetAutomapperModel()->GetWorldTransform();
    }
  } else {
    const float t = mAutoMapper->GetNextState() != CAutoMapper::kAMS_MiniMap
                        ? mAutoMapper->GetInterp()
                        : 1.f - mAutoMapper->GetInterp();
    const CQuaternion rotation = CQuaternion::Slerp(mAutoMapperRotation, mCameraRotation, t);
    const CVector3f offset = CVector3f::Lerp(mAutoMapperOffset, mCameraOffset, t);
    const float st = t * (frameLength - 1.f) + 1.f;
    mMapCameraTransform =
        CTransform4f(rotation.BuildTransform(), offset) * CTransform4f::Scale(st, 1.f, st);
    if (mSamusHud->GetAutomapperModel() != nullptr) {
      mSamusHud->GetAutomapperModel()->SetO2WTransform(
          CTransform4f(rotation.BuildTransform(), offset) *
          CTransform4f::Scale(t * (scaleX - 1.f) + 1.f, 1.f, t * (scaleZ - 1.f) + 1.f));
    }
  }
}

CInGameGuiManager::TPauseScreenDGRPs CInGameGuiManager::LockPauseScreenDependencies() {
  static const char* const names[] = {"PauseScreenDontDump_DGRP", "PauseScreenDontDump_NoARAM_DGRP",
                                      "PauseScreenTokens_DGRP"};
  TPauseScreenDGRPs result;
  for (int i = 0; i < 3; ++i) {
    TToken< CDependencyGroup > token = gpSimplePool->GetObj(names[i]);
    token.Lock();
    result.push_back(token);
  }
  return result;
}

bool CInGameGuiManager::IsTransitionReady() const {
  if (!mPauseScreenBlur->IsNotTransitioning()) {
    return false;
  }
  if (const CAutoMapper* autoMapper = mAutoMapper.get()) {
    return autoMapper->GetCurrentState() == autoMapper->GetNextState();
  }
  return true;
}

void CInGameGuiManager::TryCompleteStateTransition() {
  if (mNextState != kIGGS_PauseGame && mNextState != kIGGS_PauseLogBook) {
    mPauseScreen = nullptr;
  }
  if (InGameGuiStates::IsGameplayState(mNextState)) {
    mMessageScreen = nullptr;
    if (!TryReloadAreaTextures()) {
      return;
    }
    CModel::EnableTextureTimeout();
  }
  mPrevState = mNextState;
}

void CInGameGuiManager::BeginStateTransition(EInGameGuiState state, const CStateManager& mgr) {
  if (mNextState == state) {
    return;
  }
  mPrevState = mNextState;
  mNextState = state;

  if (state == kIGGS_InGame) {
    CSfxManager::SetChannel(CSfxManager::kSC_Game);
  } else if (state == kIGGS_PauseHUDMessage) {
    mMessageScreen = rs_new CMessageScreen(mPauseGameHudMessage, mPauseGameHudTime);
  } else if (state != kIGGS_PauseSaveGame && InGameGuiStates::IsGameplayState(mPrevState)) {
    mDeferTransition = true;
  }

  mPauseScreenBlur->OnNewInGameGuiState(state, mgr, *this);
  if (!mDeferTransition) {
    DoStateTransition(mgr);
  }
  if (state == kIGGS_InGame && !mAutoMapper.null()) {
    mAutoMapper->UnmuteAllLoopedSounds();
  }
}

void CInGameGuiManager::DoStateTransition(const CStateManager& mgr) {
  if (!mAutoMapper.null()) {
    mAutoMapper->OnNewInGameGuiState(mNextState, const_cast< CStateManager& >(mgr));
  }
  if ((mNextState == kIGGS_PauseGame || mNextState == kIGGS_PauseLogBook) &&
      mPauseScreen.null()) {
    mPauseScreen = rs_new CPauseScreen();
  }
  const bool paused = InGameGuiStates::IsPausedState(mNextState);
  for (rstl::vector< CToken >::iterator it = mPauseResources.begin();
       it != mPauseResources.end(); ++it) {
    if (paused) {
      it->Lock();
    } else {
      it->Unlock();
    }
  }
}

void CInGameGuiManager::InitializeDumpableARAMTextures() {
  if (!mIsSinglePlayer) {
    return;
  }

  int count = 0;
  for (AUTO(it, mInGameGuiDGRPs.begin()); it != mInGameGuiDGRPs.end(); ++it) {
    count += (*it)->GetCountForResType('TXTR');
  }
  mInGameTextureIds.reserve(count);

  for (AUTO(it, mInGameGuiDGRPs.begin()); it != mInGameGuiDGRPs.end(); ++it) {
    const rstl::vector< SObjectTag >& tags = (*it)->GetObjectTagVector();
    for (AUTO(tag, tags.begin()); tag != tags.end(); ++tag) {
      if (tag->GetType() == 'TXTR' &&
          rstl::find(mInGameTextureIds.begin(), mInGameTextureIds.end(), tag->GetId()) ==
              mInGameTextureIds.end()) {
        mInGameTextureIds.push_back(tag->GetId());
      }
    }
  }
  mInGameGuiDGRPs = rstl::vector< TToken< CDependencyGroup > >();

  const rstl::vector< SObjectTag >& tags = mPauseScreenDGRPs[2]->GetObjectTagVector();
  mPauseResources.reserve(tags.size());
  for (AUTO(it, tags.begin()); it != tags.end(); ++it) {
    mPauseResources.push_back(gpSimplePool->GetObj(*it));
  }
}

void CInGameGuiManager::PauseGame(const CStateManager& mgr, EInGameGuiState state) {
  if (state == kIGGS_QuitGame) {
    mQuitScreen = rs_new CInGameQuitScreen(mgr.GetViewportLayoutIndex());
    return;
  }
  gpController->SetMotorState(kIOP_Player1, kMS_Stop);
  CSfxManager::SetChannel(CSfxManager::kSC_PauseScreen);
  if (state != kIGGS_PauseHUDMessage) {
    CStreamAudioManager::StopSfx();
  }
  BeginStateTransition(state, mgr);
}

void CInGameGuiManager::ShowPauseGameHudMessage(const CStateManager& mgr, CAssetId message,
                                                float time) {
  mPauseGameHudMessage = message;
  mPauseGameHudTime = time;
  PauseGame(mgr, kIGGS_PauseHUDMessage);
}

bool CInGameGuiManager::IsInPausedState() const {
  if (!mQuitScreen.null()) {
    return true;
  }
  bool gameplay = false;
  if (InGameGuiStates::IsGameplayState(mPrevState) &&
      InGameGuiStates::IsGameplayState(mNextState)) {
    gameplay = true;
  }
  return !gameplay;
}

void CInGameGuiManager::EnsureStates(const CStateManager& mgr) {
  if (mDeferTransition && !mPauseScreenBlur->IsGameDraw()) {
    DestroyAreaTextures(mgr);
    mDeferTransition = false;
    DoStateTransition(mgr);
  }
}

bool CInGameGuiManager::IsTextureInPauseScreen(CAssetId id) const {
  TPauseScreenDGRPs::const_iterator groupIt = mPauseScreenDGRPs.begin();
  for (int i = 0; i < 3; ++i, ++groupIt) {
    TToken< CDependencyGroup > group = *groupIt;
    const rstl::vector< SObjectTag >& tags = group->GetObjectTagVector();
    for (rstl::vector< SObjectTag >::const_iterator it = tags.begin(); it != tags.end(); ++it) {
      if (it->id == id) {
        return true;
      }
    }
  }
  return false;
}

void CInGameGuiManager::DestroyAreaTextures(const CStateManager& mgr) {
  rstl::vector< SDumpableTextureInfo > candidates;
  rstl::vector< SDumpableTextureInfo > fallbackCandidates;
  candidates.reserve(64);
  fallbackCandidates.reserve(64);
  const CWorld& world = *mgr.GetWorld();
  const CGameArea& currentArea = *world.GetArea(world.GetCurrentAreaId());
  for (int i = -1; i < currentArea.GetNumAttachedAreas(); ++i) {
    if (candidates.size() == candidates.capacity()) {
      break;
    }
    const TAreaId areaId = i == -1 ? currentArea.GetId() : currentArea.GetAttachedAreaId(i);
    if (!world.GetArea(areaId)->IsLoaded()) {
      continue;
    }
    const CGameArea& area = *world.GetArea(areaId);
    for (int j = 0; j < area.GetTokenCount(); ++j) {
      const rstl::pair< CAssetId, uint >& asset = area.GetAssetID(j);
      if (IsTextureInPauseScreen(asset.first)) {
        continue;
      }
      if (candidates.size() == candidates.capacity() &&
          fallbackCandidates.size() == fallbackCandidates.capacity()) {
        break;
      }
      if (asset.second == 'TXTR') {
        TToken< CTexture > token = gpSimplePool->GetObj(SObjectTag(asset.second, asset.first));
        if (token.IsLoaded()) {
          CTexture& texture = **token;
          if (texture.GetTexelFormat() != kTF_C4) {
            if (texture.GetNumberOfMipMaps() < 2 && texture.GetBitmapDataStatus() == 0) {
              if (candidates.size() != candidates.capacity()) {
                candidates.push_back_unsafe(
                    SDumpableTextureInfo(texture.GetMemoryAllocated(), asset.first, token));
              }
            } else if (fallbackCandidates.size() != fallbackCandidates.capacity()) {
              fallbackCandidates.push_back_unsafe(
                  SDumpableTextureInfo(texture.GetMemoryAllocated(), asset.first, token));
            }
          }
        }
      }
    }
  }

  int memoryFreed = 0;
  for (rstl::vector< CAssetId >::iterator it = mInGameTextureIds.begin();
       it != mInGameTextureIds.end(); ++it) {
    CAssetId id = *it;
    if (!IsTextureInPauseScreen(id)) {
      TToken< CTexture > token = gpSimplePool->GetObj(SObjectTag('TXTR', id));
      if (token.IsLoaded()) {
        if (memoryFreed >= 0x180000) {
          break;
        }
        CTexture& texture = **token;
        if (texture.GetTexelFormat() != kTF_C4 && texture.GetBitmapDataStatus() == 0) {
          const int size = texture.GetMemoryAllocated();
          mDumpedTextures.push_back(TDumpedTexture(skInGameTextureReloadPriority, token));
          texture.UnloadBitmapData(id);
          memoryFreed += size;
        }
      }
    }
  }

  rstl::sort(candidates.begin(), candidates.end(), CTextureScoreGreaterThan());
  CFrameDelayedKiller::StallAndFlushAllAllocations();
  CTexture::sCurrentFrameCount = INT_MAX;
  CResLoader& loader = gpResourceFactory->GetResLoader();
  rstl::vector< SDumpableTextureInfo >* groups[] = {&candidates, &fallbackCandidates};
  for (uint i = 0; i < 2; ++i) {
    rstl::vector< SDumpableTextureInfo >& group = *groups[i];
    for (rstl::vector< SDumpableTextureInfo >::iterator it = group.begin();
         it != group.end() && memoryFreed < 0x180000; ++it) {
      TToken< CTexture >& token = it->mToken;
      CTexture& texture = **token;
      bool transferred = false;
      if (!texture.GetNoSwap()) {
        texture.LoadToARAM();
        if (texture.IsARAMTransferInProgress()) {
          while (texture.IsARAMTransferInProgress()) {
            CARAMToken::UpdateAllDMAs();
          }
          transferred = true;
        }
      }
      if (!transferred) {
        texture.UnloadBitmapData(it->mId);
        mDumpedTextures.push_back(
            TDumpedTexture(loader.GetResourceOffset(SObjectTag('TXTR', it->mId)), token));
      }
      memoryFreed += texture.GetMemoryAllocated();
    }
  }

  CTexture::sCurrentFrameCount = 0;
  mDumpedTextures.sort(
      rstl::pair_sorter_finder< TDumpedTexture, rstl::less< uint > >(rstl::less< uint >()));
  CModel::DisableTextureTimeout();
}

bool CInGameGuiManager::TryReloadAreaTextures() {
  bool complete = true;
  rstl::list< TDumpedTexture >::iterator it = mDumpedTextures.begin();
  while (it != mDumpedTextures.end()) {
    if (it->second->TryReloadBitmapData(*gpResourceFactory)) {
      it = mDumpedTextures.erase(it);
    } else {
      complete = false;
      ++it;
    }
  }
  return complete;
}

void CInGameGuiManager::StopSounds(const CStateManager& mgr) {
  mQuitScreen = nullptr;
  if (!mSamusHud.null()) {
    mSamusHud->StopSounds(mgr);
  }
}
