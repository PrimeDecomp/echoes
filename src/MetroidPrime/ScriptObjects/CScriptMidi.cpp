#include "MetroidPrime/ScriptObjects/CScriptMidi.hpp"

#include "Kyoto/Basics/CCast.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CInGameTweakManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrMidi.hpp"

CScriptMidi::CScriptMidi(TUniqueId id, const CEntityInfo& info, const rstl::string& name,
                         CAssetId song, float fadeIn, float fadeOut, int volume)
: CEntity(id, info, name, 0)
, mSong(gpSimplePool->GetObj(SObjectTag('CSNG', song)))
, mHandle()
, mFadeInTime(fadeIn)
, mFadeOutTime(fadeOut)
, mVolume(volume) {}

void CScriptMidi::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CEntity::AcceptScriptMsg(mgr, msg);

  switch (message) {
  case kSM_Play:
    if (GetActive()) {
      Play(mgr, mFadeInTime);
    }
    break;
  case kSM_Stop:
    if (GetActive()) {
      Stop(mgr, mFadeOutTime);
    }
    break;
  case kSM_Deactivate:
    StopInternal(0.f);
    break;
  default:
    break;
  }
}

CScriptMidi::~CScriptMidi() { StopInternal(0.f); }

void CScriptMidi::Play(CStateManager& mgr, float fadeTime) {
  const rstl::string key(CInGameTweakManager::GetIdentifierForMidiEvent(
      mgr.GetWorld()->GetWorldAssetId(),
      mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()).GetAreaAssetId(), rstl::string_l("")));

  short volume = mVolume;
  if (gpTweakManager->HasTweakValue(key)) {
    const CTweakValue::Audio& audio = gpTweakManager->GetTweakValue(key)->GetAudio();
    fadeTime = audio.GetFadeIn();
    mSong = gpSimplePool->GetObj(SObjectTag('CSNG', audio.GetResId()));
    volume = static_cast< short >(audio.GetVolume() * 127.f);
  }

  mHandle = CMidiManager::Play(**mSong, CCast::FtoUS(fadeTime * 1000.f), false, volume);
}

void CScriptMidi::Stop(CStateManager& mgr, float fadeTime) {
  const rstl::string key(CInGameTweakManager::GetIdentifierForMidiEvent(
      mgr.GetWorld()->GetWorldAssetId(),
      mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()).GetAreaAssetId(), rstl::string_l("")));

  if (gpTweakManager->HasTweakValue(key)) {
    const CTweakValue::Audio& audio = gpTweakManager->GetTweakValue(key)->GetAudio();
    fadeTime = audio.GetFadeOut();
  }

  StopInternal(fadeTime);
}

void CScriptMidi::StopInternal(float fadeTime) {
  if (mHandle != CSfxHandle::NullHandle()) {
    CMidiManager::Stop(mHandle, CCast::FtoUS(fadeTime * 1000.f));
  }
  mHandle.Clear();
}

CEntity* LoadMidi(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrMidi sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrMidi.inc"
  return rs_new CScriptMidi(mgr.AllocateUniqueId(),
                            LdrToEntityInfo(info, sldrThis.editorProperties),
                            sldrThis.editorProperties.name, sldrThis.songFile, sldrThis.fadeInTime,
                            sldrThis.fadeOutTime, sldrThis.volume);
}
