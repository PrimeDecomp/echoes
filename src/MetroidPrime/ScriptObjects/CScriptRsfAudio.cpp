#include "MetroidPrime/ScriptObjects/CScriptRsfAudio.hpp"

#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/CDvdFile.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrRsfAudio.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "rstl/math.hpp"

CScriptRsfAudio::CScriptRsfAudio(TUniqueId uid, const CEntityInfo& info, const rstl::string& name,
                                 const rstl::string& fileName, int loopStart, int loopEnd,
                                 float fadeInTime, float fadeOutTime, int volume)
: CEntity(uid, info, name, 0)
, mFileName(fileName)
, mFadeInTime(fadeInTime)
, mFadeOutTime(fadeOutTime)
, mVolume(volume)
, mLoopStart(loopStart)
, mLoopEnd(loopEnd)
, mMusicVolume(int(gpGameState->GameOptions().GetMusicVolume()))
, mTargetMusicVolume(mMusicVolume)
, mMusicVolumeRate(0.f)
, mPlayer(nullptr)
, mCurrentVolume(0.f)
, mFadeTimeLeft(0.f)
, mLoadRequested(false)
, mLoaded(false)
, mFadingIn(false)
, mFadingOut(false)
, mPlaying(false) {
  if (!CDvdFile::FileExists(mFileName.c_str())) {
    mFileName.assign("", -1);
  }
  if (fadeInTime <= 0.f) {
    mCurrentVolume = mVolume / 127.f * mMusicVolume;
  }
}

void CScriptRsfAudio::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  CEntity::Think(dt, mgr);

  if (mLoadRequested && mPlayer->IsReady()) {
    mLoadRequested = false;
    mLoaded = true;
    SendScriptMsgs(kSS_Arrived, mgr, kInvalidUniqueId, kSM_None);
  }

  const float musicVolume = int(gpGameState->GameOptions().GetMusicVolume());
  if (mTargetMusicVolume != musicVolume) {
    mTargetMusicVolume = musicVolume;
    mMusicVolumeRate = (mTargetMusicVolume - mMusicVolume) / 0.5f;
  }
  if (mMusicVolumeRate != 0.f) {
    mMusicVolume += mMusicVolumeRate * dt;
    bool reached = false;
    if (mMusicVolumeRate > 0.f) {
      if (mMusicVolume >= mTargetMusicVolume) {
        reached = true;
      }
    } else if (mMusicVolume <= mTargetMusicVolume) {
      reached = true;
    }
    if (reached) {
      mMusicVolume = mTargetMusicVolume;
      mMusicVolumeRate = 0.f;
    }
  }

  if (!mPlaying) {
    return;
  }
  if (mFadingIn || mFadingOut) {
    mFadeTimeLeft = rstl::max_val(0.f, mFadeTimeLeft - dt);
    const float duration = mFadingIn ? mFadeInTime : mFadeOutTime;
    const float elapsed = mFadingIn ? mFadeInTime - mFadeTimeLeft : mFadeTimeLeft;
    float fade = 0.f;
    if (duration > 0.f) {
      fade = CMath::Clamp(0.f, elapsed / duration, 1.f);
    } else if (mFadingIn) {
      fade = 1.f;
    }
    SetCurrentVolume(mMusicVolume * (mVolume / 127.f * fade));
    if (mFadeTimeLeft == 0.f) {
      mFadingIn = false;
      mFadingOut = false;
    }
  } else {
    SetCurrentVolume(mVolume / 127.f * mMusicVolume);
  }
}

void CScriptRsfAudio::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CEntity::AcceptScriptMsg(mgr, msg);
  switch (message) {
  case kSM_Play:
    if (GetActive()) {
      Play(mgr);
    }
    break;
  case kSM_Stop:
    if (GetActive()) {
      Stop(mgr);
    }
    break;
  case kSM_Deactivate:
    if (GetActive()) {
      Stop(mgr);
    }
    break;
  case kSM_Increment:
    StartFade(true);
    break;
  case kSM_Decrement:
    StartFade(false);
    break;
  case kSM_Load:
    Load(mgr);
    break;
  case kSM_Unload:
    Unload(mgr);
    break;
  default:
    break;
  }
}

void CScriptRsfAudio::SetCurrentVolume(float volume) {
  if (volume != mCurrentVolume) {
    mCurrentVolume = volume;
    mPlayer->SetVolume(CCast::ToUint8(mCurrentVolume));
  }
}

void CScriptRsfAudio::StartFade(bool fadeIn) {
  if (mVolume > 0) {
    mFadeTimeLeft = 127.f * (mCurrentVolume / mVolume) / mMusicVolume;
    if (fadeIn) {
      mFadeTimeLeft = 1.f - mFadeTimeLeft;
    }
    mFadeTimeLeft *= fadeIn ? mFadeInTime : mFadeOutTime;
    mFadingIn = fadeIn;
    mFadingOut = !fadeIn;
  }
}

void CScriptRsfAudio::Load(CStateManager& mgr) {
  if (mPlayer.get() == nullptr && static_cast< int >(mFileName.size()) != 0) {
    mPlayer = rs_new CStaticAudioPlayer(mFileName, mLoopStart, mLoopEnd);
    mLoadRequested = true;
  }
}

void CScriptRsfAudio::Unload(CStateManager& mgr) {
  Stop(mgr);
  mPlayer = nullptr;
  mLoadRequested = false;
  mLoaded = false;
}

void CScriptRsfAudio::Play(CStateManager& mgr) {
  if (mLoaded && !mPlaying && mPlayer.get() != nullptr) {
    mPlayer->SetVolume(CCast::ToUint8(mCurrentVolume));
    mPlayer->StartMixOut();
    mPlaying = true;
  }
}

void CScriptRsfAudio::Stop(CStateManager& mgr) {
  if (mLoaded && mPlaying && mPlayer.get() != nullptr) {
    mPlayer->StopMixOut();
    mPlaying = false;
  }
}

CEntity* LoadRsfAudio(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrRsfAudio sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrRsfAudio.inc"

  return rs_new CScriptRsfAudio(
      mgr.AllocateUniqueId(), LdrToEntityInfo(info, sldrThis.editorProperties),
      sldrThis.editorProperties.name, sldrThis.unknown_0xfe97e5b3, sldrThis.loopStart,
      sldrThis.loopEnd, sldrThis.fadeInTime, sldrThis.fadeOutTime, sldrThis.volume);
}

#ifndef MONOLITHIC
static void SetFuncPtrs() {
  static SScriptRsfAudio_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &LoadRsfAudio;
  SetSScriptRsfAudio_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSScriptRsfAudio_FuncPtrs(nullptr); }
#endif
