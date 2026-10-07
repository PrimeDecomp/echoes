#include "MetroidPrime/ScriptObjects/CScriptSoundModifier.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSoundModifier.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSound.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Math/CMath.hpp"

#include "float.h"

CScriptSoundModifier::CScriptSoundModifier(TUniqueId uid, const rstl::string& name,
                                           const CEntityInfo& info, float duration,
                                           const bool autoReset, const bool autoStart,
                                           const CMayaSpline& volume, const CMayaSpline& pan,
                                           const CMayaSpline& surroundPan, const CMayaSpline& pitch)
: CEntity(uid, info, name, 0)
, mVolume(volume)
, mPan(pan)
, mSurroundPan(surroundPan)
, mPitch(pitch)
, mElapsedTime(0.f)
, mDuration(duration)
, mAutoReset(autoReset)
, mAutoStart(autoStart)
, mRunning(autoStart) {}

CScriptSoundModifier::~CScriptSoundModifier() {}

void CScriptSoundModifier::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  if (GetActive()) {
    switch (message) {
    case kSM_AreaLoaded:
      mSounds = FindConnectedObjects(mgr, kSS_Connect, kSM_Attach);
      for (rstl::vector< TUniqueId >::const_iterator it = mSounds.begin(); it != mSounds.end();
           ++it) {
        if (CScriptSound* sound = TCastToPtr< CScriptSound >(mgr.ObjectById(*it))) {
          sound->SetSoundModifierAttached(true);
        }
      }
      break;
    case kSM_Start:
      mRunning = true;
      break;
    case kSM_Stop:
      mRunning = false;
      break;
    case kSM_Reset:
      mElapsedTime = 0.f;
      if (mAutoStart) {
        mRunning = true;
      }
      break;
    case kSM_StopAndReset:
      mElapsedTime = 0.f;
      mRunning = false;
      break;
    case kSM_ResetAndStart:
      mElapsedTime = 0.f;
      mRunning = true;
      break;
    case kSM_SetToMax:
      if (mRunning) {
        mElapsedTime = mDuration - FLT_EPSILON;
      }
      break;
    case kSM_SetToZero:
      if (mRunning) {
        mElapsedTime = 0.f;
      }
      break;
    default:
      break;
    }
  }

  CEntity::AcceptScriptMsg(mgr, msg);
}

uchar CScriptSoundModifier::GetCurrentVolume() {
  const float volume = mVolume.EvaluateAt(mElapsedTime);
  return CCast::ToUint8(CMath::Clamp(0.f, 127.f * volume, 127.f));
}

ushort CScriptSoundModifier::GetCurrentPitch() {
  const float pitch = mPitch.EvaluateAt(mElapsedTime);
  return CCast::FtoUS(CMath::Clamp(0.f, 8192.f * (1.f + pitch), 16383.f));
}

uchar CScriptSoundModifier::GetCurrentPan() {
  const float pan = mPan.EvaluateAt(mElapsedTime);
  return CCast::ToUint8(CMath::Clamp(0.f, 64.f * (1.f + pan), 127.f));
}

uchar CScriptSoundModifier::GetCurrentSurroundPan() {
  const float pan = mSurroundPan.EvaluateAt(mElapsedTime);
  return CCast::ToUint8(CMath::Clamp(0.f, 64.f * (1.f - pan), 127.f));
}

void CScriptSoundModifier::Think(float dt, CStateManager& mgr) {
  if (GetActive() && mRunning && mElapsedTime < mDuration) {
    mElapsedTime += dt;
    if (mElapsedTime >= mDuration) {
      mRunning = false;
      SendScriptMsgs(kSS_MaxReached, mgr);
      if (mAutoReset) {
        mElapsedTime = 0.f;
        if (mAutoStart) {
          mRunning = true;
        }
      }
    }

    if (mRunning) {
      for (rstl::vector< TUniqueId >::const_iterator it = mSounds.begin(); it != mSounds.end();
           ++it) {
        if (CScriptSound* sound = TCastToPtr< CScriptSound >(mgr.ObjectById(*it))) {
          const CSfxHandle handle = sound->GetSfxHandle();
          if (handle) {
            if (sound->IsNonEmitter()) {
              CSfxManager::SfxPan(handle, GetCurrentPan());
              CSfxManager::SfxSpan(handle, GetCurrentSurroundPan());
            }
            CSfxManager::PitchBend(handle, GetCurrentPitch());
            sound->SetMaxVolume(GetCurrentVolume());
          }
        }
      }
    }
  }
}

CEntity* LoadSoundModifier(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrSoundModifier sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrSoundModifier.inc"
  return rs_new CScriptSoundModifier(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), sldrThis.time, sldrThis.autoReset,
      sldrThis.autoStart, sldrThis.volume, sldrThis.pan, sldrThis.surroundPan, sldrThis.pitch);
}
