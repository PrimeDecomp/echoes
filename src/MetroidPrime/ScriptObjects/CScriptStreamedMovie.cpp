#include "MetroidPrime/ScriptObjects/CScriptStreamedMovie.hpp"

#include "Kyoto/CDvdFile.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrStreamedMovie.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSequenceTimer.hpp"
#include "MetroidPrime/TCastTo.hpp"

static CScriptStreamedMovie* sPlayingMovie = nullptr; // Guessed name

CScriptStreamedMovie::CScriptStreamedMovie(TUniqueId uid, const rstl::string& name,
                                           const CEntityInfo& info, const CTransform4f& xf,
                                           const rstl::string& movieFile, bool loop,
                                           bool videoFilterEnabled, float cacheLength,
                                           float fadeOutTime, int whenToDraw, int volume,
                                           int volumeType)
: CActor(uid, name, info, 0, xf, CModelData::None(), CMaterialList(), CActorParameters::None(),
         kInvalidUniqueId)
, mMovieFile(movieFile)
, mWhenToDraw(whenToDraw)
, mVolume(volume)
, mVolumeType(volumeType)
, mCacheLength(cacheLength)
, mFadeOutTime(fadeOutTime)
, mLoop(loop)
, mVideoFilterEnabled(videoFilterEnabled)
, mSkipping(false)
, mSkipTime(0.f)
, mMoviePlayer(nullptr) {
  if (!CDvdFile::FileExists(mMovieFile.c_str())) {
    mMovieFile.assign("", -1);
  }
}

void CScriptStreamedMovie::Think(float dt, CStateManager& mgr) {
  CActor::Think(dt, mgr);
  if (!GetActive() || mMoviePlayer.get() == nullptr) {
    return;
  }

  if (!mMoviePlayer->CanDrawVideo()) {
    if (!mMoviePlayer->ContinueLoading()) {
      SendScriptMsgs(kSS_Arrived, mgr, kInvalidUniqueId, kSM_None);
    }
  }

  if (!gpMain->IsMaxSpeed()) {
    const float previousTime = mMoviePlayer->GetPlayedSeconds();
    const bool wasFinished = mMoviePlayer->GetIsMovieFinishedPlaying();
    mMoviePlayer->Update(dt);
    const float fadeStart = mMoviePlayer->GetTotalSeconds() - mFadeOutTime;
    if (previousTime < fadeStart && mMoviePlayer->GetPlayedSeconds() >= fadeStart) {
      SendScriptMsgs(kSS_Approach, mgr, kInvalidUniqueId, kSM_None);
    }
    if (mMoviePlayer->GetIsMovieFinishedPlaying() && !wasFinished) {
      SendScriptMsgs(kSS_MaxReached, mgr, kInvalidUniqueId, kSM_None);
    }
    UpdateVolume(mgr);
    SendExternalTime(mMoviePlayer->GetPlayedSeconds(), mgr);
  } else {
    if (!mSkipping) {
      mSkipTime = mMoviePlayer->GetPlayedSeconds();
      mSkipping = true;
      StopMovie(mgr);
    }
    const float fadeStart = mMoviePlayer->GetTotalSeconds() - mFadeOutTime;
    if (mSkipTime < fadeStart) {
      mSkipTime = fadeStart;
      SendScriptMsgs(kSS_Approach, mgr, kInvalidUniqueId, kSM_None);
    } else if (mSkipTime < mMoviePlayer->GetTotalSeconds()) {
      mSkipTime = mMoviePlayer->GetTotalSeconds();
      SendScriptMsgs(kSS_MaxReached, mgr, kInvalidUniqueId, kSM_None);
    }
    SendExternalTime(mSkipTime, mgr);
  }
}

void CScriptStreamedMovie::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CActor::AcceptScriptMsg(mgr, msg);
  if (!GetActive()) {
    return;
  }
  switch (msg.GetMessage()) {
  case kSM_Load:
    LoadMovie(mgr);
    break;
  case kSM_Alert:
    if (mMoviePlayer.get() != nullptr && mMoviePlayer->CanDrawVideo()) {
      SendScriptMsgs(kSS_Arrived, mgr, kInvalidUniqueId, kSM_None);
    }
    break;
  case kSM_Play:
    PlayMovie(mgr);
    break;
  case kSM_Stop:
    StopMovie(mgr);
    break;
  case kSM_Reset:
    RewindMovie(mgr);
    break;
  case kSM_Unload:
  case kSM_Delete:
    UnloadMovie(mgr);
    break;
  default:
    break;
  }
}

void CScriptStreamedMovie::PreRender(CStateManager& mgr) {
  if (mWhenToDraw == 0) {
    mgr.RenderLastOverlay(GetUniqueId());
  } else {
    mgr.RenderLastHUD(GetUniqueId());
  }
}

void CScriptStreamedMovie::Render(const CStateManager& mgr) const {
  CMoviePlayer* player = mMoviePlayer.get();
  if (player != nullptr && player->GetPlayMode() == CMoviePlayer::kPM_Playing) {
    const int viewportLeft = CGraphics::GetViewport().mLeft;
    const int viewportTop = CGraphics::GetViewport().mTop;
    const int viewportWidth = CGraphics::GetViewport().mWidth;
    const int viewportHeight = CGraphics::GetViewport().mHeight;
    const CTransform4f viewMatrix = CGraphics::GetViewMatrix();
    const CGraphics::CProjectionState projection = CGraphics::GetProjectionState();
    const float depthNear = CGraphics::GetDepthNear();
    const float depthFar = CGraphics::GetDepthFar();

    int left, bottom, width, height;
    mgr.CalculatePlayerViewport(
        mgr.mPlayerStates[mgr.mCurrentRenderPlayerIndex]->GetPlayerSelection(), &left, &bottom,
        &width, &height);
    gpRender->SetViewport(left, bottom, width, height);
    player->DrawVideo();
    gpRender->SetViewport(viewportLeft, viewportTop, viewportWidth, viewportHeight);
    CGraphics::SetViewPointMatrix(viewMatrix);
    CGraphics::SetProjectionState(projection);
    CGraphics::SetDepthRange(depthNear, depthFar);
  }
}

void CScriptStreamedMovie::SendExternalTime(float time, CStateManager& mgr) {
  const rstl::vector< SConnection >& connections = GetConnectionList();
  for (rstl::vector< SConnection >::const_iterator it = connections.begin();
       it != connections.end(); ++it) {
    if (it->state != kSS_InternalState0) {
      continue;
    }
    CEntity* entity = mgr.ObjectById(mgr.GetIdForScript(it->objId));
    if (entity != nullptr && entity->GetActive()) {
      TCastToPtr< CScriptSequenceTimer >(entity)->ReceiveExternalTime(mgr, time);
    }
  }
}

void CScriptStreamedMovie::LoadMovie(CStateManager& mgr) {
  if (mMoviePlayer.get() == nullptr && mMovieFile.size() != 0) {
    mMoviePlayer =
        rs_new CMoviePlayer(mMovieFile.c_str(), mCacheLength, mLoop, mVideoFilterEnabled);
    mMoviePlayer->SetPlayMode(CMoviePlayer::kPM_Stopped);
  }
}

void CScriptStreamedMovie::PlayMovie(CStateManager& mgr) {
  if (mMoviePlayer.get() != nullptr && mMoviePlayer->CanDrawVideo() && sPlayingMovie == nullptr) {
    sPlayingMovie = this;
    mMoviePlayer->SetPlayMode(CMoviePlayer::kPM_Playing);
    mMoviePlayer->Update(0.f);
  }
}

void CScriptStreamedMovie::StopMovie(CStateManager& mgr) {
  if (mMoviePlayer.get() != nullptr && mMoviePlayer->CanDrawVideo()) {
    if (this == sPlayingMovie) {
      sPlayingMovie = nullptr;
      CMoviePlayer::SetSfxVolume(127);
    }
    mMoviePlayer->SetPlayMode(CMoviePlayer::kPM_Stopped);
  }
}

void CScriptStreamedMovie::RewindMovie(CStateManager& mgr) {
  if (mMoviePlayer.get() != nullptr && mMoviePlayer->CanDrawVideo()) {
    mMoviePlayer->Rewind();
  }
}

void CScriptStreamedMovie::UnloadMovie(CStateManager& mgr) {
  if (this == sPlayingMovie) {
    sPlayingMovie = nullptr;
    CMoviePlayer::SetSfxVolume(127);
  }
  mMoviePlayer = nullptr;
}

void CScriptStreamedMovie::UpdateVolume(CStateManager& mgr) {
  if (this != sPlayingMovie) {
    return;
  }
  int volume = mVolume;
  switch (mVolumeType) {
  case 1:
    volume = volume * int(gpGameState->GameOptions().GetSfxVolume()) / 127;
    break;
  case 2:
    volume = volume * int(gpGameState->GameOptions().GetMusicVolume()) / 127;
    break;
  default:
    break;
  }
  CMoviePlayer::SetSfxVolume(volume);
}

CEntity* LoadStreamedMovie(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrStreamedMovie sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrStreamedMovie.inc"

  return rs_new CScriptStreamedMovie(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      sldrThis.movieFile, sldrThis.loop, sldrThis.videoFilterEnabled, sldrThis.cacheLength,
      sldrThis.fadeOutTime, sldrThis.whenToDraw, sldrThis.volume, sldrThis.volumeType);
}

#ifndef MONOLITHIC
static void SetFuncPtrs() {
  static SStreamedMovie_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &LoadStreamedMovie;
  SetSStreamedMovie_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSStreamedMovie_FuncPtrs(nullptr); }
#endif
