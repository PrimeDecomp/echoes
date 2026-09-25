#include "MetroidPrime/ScriptObjects/CScriptStreamedMusic.hpp"

#include "Kyoto/Audio/CStreamAudioManager.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/CDvdFile.hpp"
#include "MetroidPrime/CInGameTweakManager.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrStreamedAudio.hpp"
#include "rstl/StringExtras.hpp"

#include <string.h>

extern "C" void fn_8015DCF0(CScriptStreamedMusic*);

CScriptStreamedMusic::CScriptStreamedMusic(TUniqueId id, const CEntityInfo& info,
                                           const rstl::string& name, const rstl::string& fileName,
                                           bool noStopOnDeactivate, float fadeIn, float fadeOut,
                                           uint volume, bool loop, bool music)
: CEntity(id, info, name, 0)
, mFileName(fileName)
, mNoStopOnDeactivate(noStopOnDeactivate)
, mFileIsDsp(IsDSPFile(fileName))
, mLoop(loop)
, mMusic(music)
, mPreloadPending(false)
, mFadeIn(fadeIn)
, mFadeOut(fadeOut)
, mVolume(volume)
, mPreload() {
  fn_8015DCF0(this);
}

extern "C" void fn_8015DCF0(CScriptStreamedMusic*) {}

CScriptStreamedMusic::~CScriptStreamedMusic() {}

bool CScriptStreamedMusic::IsDSPFile(const rstl::string& fileName) {
  return CStringExtras::CompareCaseInsensitive(fileName, rstl::string_l("sw")) == 0 ||
         CStringExtras::IndexOfSubstring(fileName, rstl::string_l(".dsp")) != -1;
}

int CScriptStreamedMusic::IsOneShot(bool loop) { return loop ? 0 : 1; }

void CScriptStreamedMusic::Think(float dt, CStateManager& mgr) {
  if (mPreloadPending && mPreload->IsReady()) {
    SendScriptMsgs(static_cast< EScriptObjectState >(0x41525256), mgr, kInvalidUniqueId, kSM_None);
    mPreloadPending = false;
  }
}

void CScriptStreamedMusic::StartStream() {
  if (!mPreload || mPreload->IsReady()) {
    CStreamAudioManager::Start(IsOneShot(mLoop), mFileName, static_cast< uchar >(mVolume),
                               mMusic, mFadeIn, mFadeOut);
  }
}

void CScriptStreamedMusic::StopStream() {
  CStreamAudioManager::Stop(IsOneShot(mLoop), mFileName);
}

namespace rstl {
// Keep the iterator-range constructor's distance calculation inline in this TU.
template <>
inline long distance< string::const_iterator >(string::const_iterator first,
                                               string::const_iterator last) {
  return last - first;
}
} // namespace rstl

template <>
template <>
rstl::basic_string< char >::basic_string(rstl::basic_string< char >::const_iterator first,
                                         rstl::basic_string< char >::const_iterator last,
                                         const rstl::rmemory_allocator&) {
  const int len = rstl::distance(first, last);
  internal_allocate(len + 1);
  int i = 0;
  for (const_iterator it = first; it != last; it = it + 1, ++i) {
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
  if (mPreload) {
    mPreloadPending = true;
    return;
  }
  const char* fileName = mFileName.data();
  if (strncmp(fileName, "mem:", strlen("mem:")) == 0) {
    const rstl::string path = mFileName.substr(4);
    if (CDvdFile::FileExists(path.data())) {
      mPreload = CFilePreload(path);
      mPreloadPending = true;
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

    mFileName = fileName;
    mFileIsDsp = IsDSPFile(mFileName);
    mFadeIn = fadeIn;
    mVolume = volume;
    mFadeOut = fadeOut;
    fn_8015DCF0(this);
    SetStereoPair();
  }
}

template <>
template <>
int rstl::basic_string< char >::internal_search< rstl::string::const_iterator, const char* >(
    rstl::string::const_iterator first, rstl::string::const_iterator last,
    const char* searchFirst, const char* searchLast) {
  if (searchFirst == searchLast) {
    return 0;
  }
  int index = 0;
  for (const_iterator it = first; it != last; ++it, ++index) {
    if (char_traits< char >::eq(*it, *searchFirst)) {
      const_iterator next = it;
      ++next;
      const char* search = searchFirst + 1;
      const char* nextData = &*next;
      while (search != searchLast && next != last) {
        if (!char_traits< char >::eq(*nextData, *search)) {
          break;
        }
        ++next;
        ++nextData;
        ++search;
      }
      if (search == searchLast) {
        return index;
      }
    }
  }
  return -1;
}

void CScriptStreamedMusic::SetStereoPair() {
  if (mFileIsDsp && mFileName.find('|', 0) == -1 && mFileName.size() >= 5) {
    if (CStringExtras::CompareCaseInsensitive(
            rstl::string_l(mFileName.data() + mFileName.size() - 5),
            rstl::string_l("L.dsp")) == 0) {
      rstl::string right = rstl::string(mFileName.begin(), mFileName.end() - 5) + "R.dsp";
      if (CDvdFile::FileExists(right.data())) {
        mFileName = mFileName + '|' + right;
      }
    }
  }
}

void CScriptStreamedMusic::StopNonDsp() {
  CStreamAudioManager::sub_8036590c(mFadeOut);
}

void CScriptStreamedMusic::PlayNonDsp() {
  const char volume = mVolume;
  if (mNoStopOnDeactivate) {
    CStreamAudioManager::SetDefaultAudio(mFileName, mFadeOut, mFadeIn, volume);
  } else {
    CStreamAudioManager::SetCurrentAudio(mFileName, mFadeOut, mFadeIn, volume);
  }
}

void CScriptStreamedMusic::Stop() {
  if (mFileIsDsp) {
    StopStream();
  } else {
    StopNonDsp();
  }
}

void CScriptStreamedMusic::PlayAudio() {
  if (mFileIsDsp) {
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
  if (mFileIsDsp) {
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
  case kSM_Load:
    if (GetActive() && mFileIsDsp) {
      PreloadMemoryAudio();
    }
    break;
  case kSM_Stop:
    if (GetActive()) {
      Stop();
    }
    break;
  case kSM_Deactivate:
    if ((!mFileIsDsp && !mNoStopOnDeactivate) || mFileIsDsp) {
      Stop();
    }
    break;
  case kSM_Increment:
    if (mFileIsDsp) {
      CStreamAudioManager::FadeBackIn(IsOneShot(mLoop), mFadeIn);
    } else {
      CStreamAudioManager::sub_803653f8(mFadeIn);
    }
    break;
  case kSM_Decrement:
    if (mFileIsDsp) {
      CStreamAudioManager::TemporaryFadeOut(IsOneShot(mLoop), mFadeOut);
    } else {
      CStreamAudioManager::sub_80365424(mFadeOut);
    }
    break;
  default:
    break;
  }
}

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
      data.songFile = value;
      break;
    }
    case 0x34b152c4:
      data.defaultAudio = input.ReadBool();
      break;
    case 0x90aa341f:
      data.fadeInTime = input.ReadFloat();
      break;
    case 0x7c269ebc:
      data.fadeOutTime = input.ReadFloat();
      break;
    case 0x80c66c37:
      data.volume = input.ReadInt32();
      break;
    case 0x28f82261:
      data.softwareChannel = input.ReadInt32();
      break;
    case 0xd3356fe7:
      data.softwareIsMusic = input.ReadBool();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }

  return new CScriptStreamedMusic(
      mgr.AllocateUniqueId(), LdrToEntityInfo(info, data.editorProperties),
      data.editorProperties.name, data.songFile, data.defaultAudio, data.fadeInTime, data.fadeOutTime,
      data.volume, data.softwareChannel == 0, data.softwareIsMusic);
}
