#include "MetroidPrime/ScriptObjects/CScriptStreamedMusic.hpp"

#include "Kyoto/Audio/CStreamAudioManager.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/CDvdFile.hpp"
#include "MetroidPrime/CInGameTweakManager.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "rstl/StringExtras.hpp"

#include <string.h>

extern "C" void fn_8015DCF0(CScriptStreamedMusic*);

CScriptStreamedMusic::CScriptStreamedMusic(TUniqueId id, const CEntityInfo& info,
                                           const rstl::string& name, const rstl::string& fileName,
                                           bool noStopOnDeactivate, float fadeIn, float fadeOut,
                                           uint volume, bool loop, bool music)
: CEntity(id, info, name, 0)
, x24_fileName(fileName)
, x34_noStopOnDeactivate(noStopOnDeactivate)
, x34_fileIsDsp(IsDSPFile(fileName))
, x34_loop(loop)
, x34_music(music)
, x34_preloadPending(false)
, x38_fadeIn(fadeIn)
, x3c_fadeOut(fadeOut)
, x40_volume(volume)
, x44_preload() {
  fn_8015DCF0(this);
}

extern "C" void fn_8015DCF0(CScriptStreamedMusic*) {}

CScriptStreamedMusic::~CScriptStreamedMusic() {}

CEntity* CScriptStreamedMusic::TypesMatch(int typeId) const {
  return typeId == kET_ScriptStreamedMusic ? const_cast< CScriptStreamedMusic* >(this)
                                           : CEntity::TypesMatch(typeId);
}

bool CScriptStreamedMusic::IsDSPFile(const rstl::string& fileName) {
  return CStringExtras::CompareCaseInsensitive(fileName, rstl::string_l("sw")) == 0 ||
         CStringExtras::IndexOfSubstring(fileName, rstl::string_l(".dsp")) != -1;
}

int CScriptStreamedMusic::IsOneShot(bool loop) { return loop ? 0 : 1; }

void CScriptStreamedMusic::Think(float dt, CStateManager& mgr) {
  if (x34_preloadPending && x44_preload->IsReady()) {
    SendScriptMsgs(static_cast< EScriptObjectState >(0x41525256), mgr, kInvalidUniqueId, kSM_None);
    x34_preloadPending = false;
  }
}

void CScriptStreamedMusic::StartStream() {
  if (!x44_preload || x44_preload->IsReady()) {
    CStreamAudioManager::Start(IsOneShot(x34_loop), x24_fileName, static_cast< uchar >(x40_volume),
                               x34_music, x38_fadeIn, x3c_fadeOut);
  }
}

void CScriptStreamedMusic::StopStream() {
  CStreamAudioManager::Stop(IsOneShot(x34_loop), x24_fileName);
}

template <>
template <>
rstl::basic_string< char >::basic_string(rstl::basic_string< char >::const_iterator first,
                                         rstl::basic_string< char >::const_iterator last,
                                         const rstl::rmemory_allocator&) {
  const int len = last - first;
  internal_allocate(len + 1);
  int i = 0;
  for (const_iterator it = first; it != last; ++it, ++i) {
    const_cast< char& >(x0_ptr[i]) = *it;
  }
  const_cast< char& >(x0_ptr[i]) = char_traits< char >::eos();
  x8_size = len;
}

template <>
rstl::string rstl::basic_string< char >::substr(int pos, int count) const {
  const pair< const_iterator, const_iterator > range = range_iterator(pos, count);
  return basic_string(range.first, range.second);
}

void CScriptStreamedMusic::PreloadMemoryAudio() {
  if (x44_preload) {
    x34_preloadPending = true;
    return;
  }
  const char* fileName = x24_fileName.data();
  if (strncmp(fileName, "mem:", strlen("mem:")) == 0) {
    const rstl::string path = x24_fileName.substr(4);
    if (CDvdFile::FileExists(path.data())) {
      x44_preload = CFilePreload(path);
      x34_preloadPending = true;
    }
  }
}

void CScriptStreamedMusic::TweakOverride(CStateManager& mgr) {
  const rstl::string key = CInGameTweakManager::GetIdentifierForMusicEvent(
      mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()).GetAreaAssetId(), rstl::string_l(""));
  if (gpTweakManager->HasTweakValue(key)) {
    const CTweakValue::Audio& audio = gpTweakManager->GetTweakValue(key)->GetAudio();
    const rstl::string fileName(audio.GetFileName());
    const float fadeIn = audio.GetFadeIn();
    const char volume = CCast::ToInt8(audio.GetVolume() * 127.f);
    const float fadeOut = audio.GetFadeOut();

    x24_fileName = fileName;
    x34_fileIsDsp = IsDSPFile(x24_fileName);
    x38_fadeIn = fadeIn;
    x40_volume = volume;
    x3c_fadeOut = fadeOut;
    fn_8015DCF0(this);
    SetStereoPair();
  }
}

void CScriptStreamedMusic::SetStereoPair() {
  if (x34_fileIsDsp && x24_fileName.find('|', 0) == -1 && x24_fileName.size() >= 5) {
    if (CStringExtras::CompareCaseInsensitive(
            rstl::string_l(x24_fileName.data() + x24_fileName.size() - 5),
            rstl::string_l("L.dsp")) == 0) {
      rstl::string right = rstl::string(x24_fileName.begin(), x24_fileName.end() - 5) + "R.dsp";
      if (CDvdFile::FileExists(right.data())) {
        x24_fileName = x24_fileName + '|' + right;
      }
    }
  }
}

void CScriptStreamedMusic::StopNonDsp() {
  CStreamAudioManager::sub_8036590c(x3c_fadeOut);
}

void CScriptStreamedMusic::PlayNonDsp() {
  const char volume = x40_volume;
  if (x34_noStopOnDeactivate) {
    CStreamAudioManager::SetDefaultAudio(x24_fileName, x3c_fadeOut, x38_fadeIn, volume);
  } else {
    CStreamAudioManager::SetCurrentAudio(x24_fileName, x3c_fadeOut, x38_fadeIn, volume);
  }
}

void CScriptStreamedMusic::Stop() {
  if (x34_fileIsDsp) {
    StopStream();
  } else {
    StopNonDsp();
  }
}

void CScriptStreamedMusic::PlayAudio() {
  if (x34_fileIsDsp) {
    StartStream();
  } else {
    PlayNonDsp();
  }
}

void CScriptStreamedMusic::Play(CStateManager& mgr) {
  if (!GetEditorFlag2() && gpMain->fn_80008A1C()) {
    return;
  }
  TweakOverride(mgr);
  if (x34_fileIsDsp) {
    StartStream();
  } else {
    PlayNonDsp();
  }
}

void CScriptStreamedMusic::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
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
      Stop();
    }
    break;
  case kSM_Deactivate:
    if (x34_fileIsDsp || !x34_noStopOnDeactivate) {
      Stop();
    }
    break;
  case kSM_Increment:
    if (x34_fileIsDsp) {
      CStreamAudioManager::FadeBackIn(IsOneShot(x34_loop), x38_fadeIn);
    } else {
      CStreamAudioManager::sub_803653f8(x38_fadeIn);
    }
    break;
  case kSM_Decrement:
    if (x34_fileIsDsp) {
      CStreamAudioManager::TemporaryFadeOut(IsOneShot(x34_loop), x3c_fadeOut);
    } else {
      CStreamAudioManager::sub_80365424(x3c_fadeOut);
    }
    break;
  default:
    if (message == static_cast< EScriptObjectMessage >(0x4c4f4144) && GetActive() &&
        x34_fileIsDsp) {
      PreloadMemoryAudio();
    }
    break;
  }
}

struct SLdrStreamedAudio {
  SLdrStreamedAudio()
  : editorProperties()
  , fileName()
  , noStopOnDeactivate(false)
  , fadeIn(0.25f)
  , fadeOut(0.25f)
  , volume(127)
  , loopMode(0)
  , music(true) {
    editorProperties.unknown = 3;
  }

  SLdrEditorProperties editorProperties;
  rstl::string fileName;
  bool noStopOnDeactivate;
  float fadeIn;
  float fadeOut;
  uint volume;
  uint loopMode;
  bool music;
};

CEntity* LoadStreamedAudio(CStateManager& mgr, CInputStream& input, const CEntityInfo& info) {
  SLdrStreamedAudio data;

  const int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    const uint propertyId = input.ReadInt32();
    const uint propertySize = input.ReadUint16();
    switch (propertyId) {
    case 0x255a4580:
      LoadTypedefSLdrEditorProperties(data.editorProperties, input);
      break;
    case 0xf6f3de1c: {
      const rstl::string value(input);
      data.fileName = value;
      break;
    }
    case 0x34b152c4:
      data.noStopOnDeactivate = input.ReadBool();
      break;
    case 0x90aa341f:
      data.fadeIn = input.ReadFloat();
      break;
    case 0x7c269ebc:
      data.fadeOut = input.ReadFloat();
      break;
    case 0x80c66c37:
      data.volume = input.ReadInt32();
      break;
    case 0x28f82261:
      data.loopMode = input.ReadInt32();
      break;
    case 0xd3356fe7:
      data.music = input.ReadBool();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }

  return new CScriptStreamedMusic(
      mgr.AllocateUniqueId(), LdrToEntityInfo(info, data.editorProperties),
      data.editorProperties.name, data.fileName, data.noStopOnDeactivate, data.fadeIn, data.fadeOut,
      data.volume, data.loopMode == 0, data.music);
}
