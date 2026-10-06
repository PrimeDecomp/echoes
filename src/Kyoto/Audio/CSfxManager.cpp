#include "Kyoto/Audio/CSfxManager.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/CToken.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/TToken.hpp"
#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"
#include <math.h>

CSfxManager::CSfxChannel CSfxManager::mChannels[4];
rstl::reserved_vector< CSfxManager::SLowPassFilter, 8 > CSfxManager::mAreaLowPassFilters;
rstl::reserved_vector< CSfxManager::SLowPassFilter, 8 > CSfxManager::mLowPassFilters;
rstl::auto_ptr< CToken > CSfxManager::mpTranslationTableToken;
rstl::reserved_vector< CSfxManager::CSfxEmitterWrapper, 64 > CSfxManager::mEmitterWrapperPool;
rstl::reserved_vector< CSfxManager::CSfxWrapper, 64 > CSfxManager::mWrapperPool;
rstl::reserved_vector< CSfxPitchBend, 8 > CSfxManager::mPitchBends;
rstl::reserved_vector< CAuxEffect, 10 > CSfxManager::mAuxEffects;
rstl::reserved_vector< CSfxManager::SAreaVolume, 10 > CSfxManager::mAreaVolumes(10, SAreaVolume());
CAuxEffectManager CSfxManager::mAuxEffectManager;
rstl::pair< int, bool > CSfxManager::mStudioState(-1, false);

CSfxManager::ESfxChannels CSfxManager::mCurrentChannel = kSC_Default;
bool CSfxManager::mDoUpdate = false;
bool CSfxManager::mMuted = false;
rstl::vector< short >* CSfxManager::mpTranslationTable = nullptr;
int CSfxManager::mNextAreaFilterId = 0;
int CSfxManager::mNextFilterId = 0;
int CSfxManager::mAreaLowPassFrequency = 16000;
int CSfxManager::mLowPassFrequency = 16000;
int CSfxManager::mCurrentArea = -1;
int CSfxManager::mNextAuxEffectId = 0;

const short CSfxManager::kMaxPriority = 255;
const short CSfxManager::kMedPriority = 127;
const ushort CSfxManager::kInternalInvalidSfxId = 0xffff;
const int CSfxManager::kAllAreas = -1;

CSfxManager::CSfxChannel::CSfxChannel() : mListeners(4, SListener()) {}

CSfxManager::SListener::SListener() : mActive(false) {}

bool CSfxManager::CSfxEmitterWrapper::IsEmitter() const { return true; }

CSfxManager::CBaseSfxWrapper::CBaseSfxWrapper(bool looped, short priority, CSfxHandle handle,
                                              bool useAcoustics, int area)
: mTimeRemaining(15.f)
, mRank(0)
, mPriority(priority)
, mPitchBend(0x2000)
, mHandle(handle)
, mArea(area)
, mActive(true)
, mPlaying(false)
, mLooped(looped)
, mInArea(true)
, mReleased(false)
, mUseAcoustics(useAcoustics)
, mIgnoreAreaLowPass(false) {}

bool CSfxManager::CBaseSfxWrapper::Available() const { return mReleased; }

void CSfxManager::CBaseSfxWrapper::Release() {
  mReleased = true;
  mTimeRemaining = 15.f;
}

float CSfxManager::CBaseSfxWrapper::GetTimeRemaining() { return mTimeRemaining; }

void CSfxManager::CBaseSfxWrapper::SetTimeRemaining(float time) { mTimeRemaining = time; }

void CSfxManager::CBaseSfxWrapper::SetActive(bool active) { mActive = active; }

void CSfxManager::CBaseSfxWrapper::SetPlaying(bool playing) { mPlaying = playing; }

void CSfxManager::CBaseSfxWrapper::SetInArea(bool inArea) { mInArea = inArea; }

void CSfxManager::CBaseSfxWrapper::SetRank(short rank) { mRank = rank; }

bool CSfxManager::CBaseSfxWrapper::IsLooped() const { return mLooped; }

bool CSfxManager::CBaseSfxWrapper::IsInArea() const { return mInArea; }

bool CSfxManager::CBaseSfxWrapper::IsPlaying() const { return mPlaying; }

bool CSfxManager::CBaseSfxWrapper::IsActive() const { return mActive; }

bool CSfxManager::CBaseSfxWrapper::UseAcoustics() const { return mUseAcoustics; }

int CSfxManager::CBaseSfxWrapper::GetRank() const { return mRank; }

int CSfxManager::CBaseSfxWrapper::GetPriority() const { return mPriority; }

CSfxHandle CSfxManager::CBaseSfxWrapper::GetSfxHandle() const { return mHandle; }

int CSfxManager::CBaseSfxWrapper::GetArea() const { return mArea; }

void CSfxManager::CBaseSfxWrapper::SetPitchBend(ushort pitch) { mPitchBend = pitch; }

ushort CSfxManager::CBaseSfxWrapper::GetPitchBend() const { return mPitchBend; }

bool CSfxManager::CBaseSfxWrapper::GetIgnoreAreaLowPass() const { return mIgnoreAreaLowPass; }

void CSfxManager::CBaseSfxWrapper::SetIgnoreAreaLowPass(bool ignore) {
  mIgnoreAreaLowPass = ignore;
}

CSfxManager::CSfxEmitterWrapper::CSfxEmitterWrapper(bool looped, short priority,
                                                    CAudioSys::C3DEmitterParmData& emitter,
                                                    CSfxHandle handle, bool useAcoustics, int area)
: CBaseSfxWrapper(looped, priority, handle, useAcoustics, area)
, mEmitterData(emitter)
, mEmitterHandle(SND_ID_ERROR)
, mReady(true)
, mUpdatePending(false) {}

void CSfxManager::CSfxEmitterWrapper::SetReverb(char reverb) {
  if (UseAcoustics()) {
    mParameters[mParameterInfo.numPara - 1].paraData.value7 = reverb;
  }
}

void CSfxManager::CSfxEmitterWrapper::Play() {
  mParameterInfo.numPara = 0;
  mParameterInfo.paraArray = mParameters;
  mEmitterData.mStudio = UseAcoustics() ? GetStudio(GetArea()) : 0;
  mParameters[mParameterInfo.numPara].ctrl = SND_MIDICTRL_REVERB;
  mParameters[mParameterInfo.numPara].paraData.value7 = UseAcoustics() ? GetReverbAmount() : 0;
  ++mParameterInfo.numPara;

  mEmitterHandle = CAudioSys::S3dAddEmitterParaEx(mEmitterData, GetSfxHandle().GetIndex() & 0xff,
                                                  &mParameterInfo);
  if (mEmitterHandle != SND_ID_ERROR) {
    SetPlaying(true);
  }
  mReady = false;
}

ushort CSfxManager::CSfxEmitterWrapper::GetSfxId() { return mEmitterData.mSfxId; }

bool CSfxManager::CSfxEmitterWrapper::IsSilent() const { return GetEmitter().mMaxVol == 1; }

void CSfxManager::CSfxEmitterWrapper::Stop() {
  if (mEmitterHandle != SND_ID_ERROR) {
    CAudioSys::S3dRemoveEmitter(mEmitterHandle);
    SetPlaying(false);
    mEmitterHandle = SND_ID_ERROR;
  }
}

CAudioSys::C3DEmitterParmData& CSfxManager::CSfxEmitterWrapper::GetEmitter() {
  return mEmitterData;
}

const CAudioSys::C3DEmitterParmData& CSfxManager::CSfxEmitterWrapper::GetEmitter() const {
  return mEmitterData;
}

uint CSfxManager::CSfxEmitterWrapper::GetHandle() const { return mEmitterHandle; }

bool CSfxManager::CSfxEmitterWrapper::IsPlaying() const {
  if (IsLooped()) {
    return CBaseSfxWrapper::IsPlaying();
  }
  return CBaseSfxWrapper::IsPlaying() && CAudioSys::S3dCheckEmitter(mEmitterHandle);
}

bool CSfxManager::CSfxEmitterWrapper::Ready() { return IsLooped() || mReady; }

short CSfxManager::CSfxEmitterWrapper::GetAudible(const CVector3f& position) {
  const float distanceSquared = (mEmitterData.mPos - position).MagSquared();
  const float maxDistanceSquared = mEmitterData.mMaxDist * mEmitterData.mMaxDist;
  if (distanceSquared < maxDistanceSquared * 0.25f) {
    return kSA_High;
  }
  if (distanceSquared < maxDistanceSquared * 0.5f) {
    return kSA_Medium;
  }
  return static_cast< ESfxAudibility >(distanceSquared < maxDistanceSquared);
}

SND_VOICEID CSfxManager::CSfxEmitterWrapper::GetVoice() const {
  return IsPlaying() ? CAudioSys::S3dEmitterVoiceID(mEmitterHandle) : SND_ID_ERROR;
}

void CSfxManager::CSfxEmitterWrapper::UpdateEmitterSilent() {
  if (mEmitterData.mMaxVol != 1) {
    mCachedMaxVolume = mEmitterData.mMaxVol;
    mEmitterData.mMaxVol = 1;
    CAudioSys::S3dUpdateEmitter(mEmitterHandle, mEmitterData.mPos, mEmitterData.mDir, 1);
  }
}

void CSfxManager::CSfxEmitterWrapper::UpdateEmitter() { mUpdatePending = true; }

CSfxManager::CSfxWrapper::CSfxWrapper(bool looped, short priority, ushort sfxId, short volume,
                                      short pan, CSfxHandle handle, bool useAcoustics, int area)
: CBaseSfxWrapper(looped, priority, handle, useAcoustics, area)
, mSfxId(sfxId)
, mVoiceHandle(SND_ID_ERROR)
, mVolume(volume)
, mPan(pan)
, mReady(true) {}

void CSfxManager::CSfxWrapper::SetReverb(char reverb) {
  if (UseAcoustics()) {
    CAudioSys::SfxCtrl(mVoiceHandle, SND_MIDICTRL_REVERB, reverb);
  }
}

void CSfxManager::CSfxWrapper::Play() {
  const uchar studio = UseAcoustics() ? GetStudio(GetArea()) : 0;
  mVoiceHandle = CAudioSys::SfxStart(mSfxId, 127, mPan, studio);
  CAudioSys::SfxVolume(mVoiceHandle, mVolume);
  if (mVoiceHandle != SND_ID_ERROR) {
    if (UseAcoustics()) {
      CAudioSys::SfxCtrl(mVoiceHandle, SND_MIDICTRL_REVERB, GetReverbAmount());
    }
    SetPlaying(true);
    if (IsLowPassAreaFilterEnabled() && GetArea() != kAllAreas) {
      CAudioSys::SfxSetFilter(mVoiceHandle, 1, GetLowPassAreaFrequency());
    }
  }
  mReady = false;
}

ushort CSfxManager::CSfxWrapper::GetSfxId() { return mSfxId; }

void CSfxManager::CSfxWrapper::Stop() {
  if (mVoiceHandle != SND_ID_ERROR) {
    CAudioSys::SfxStop(mVoiceHandle);
    SetPlaying(false);
    mVoiceHandle = SND_ID_ERROR;
  }
}

bool CSfxManager::CSfxWrapper::IsPlaying() const {
  return CBaseSfxWrapper::IsPlaying() && CAudioSys::SfxCheck(mVoiceHandle) != SND_ID_ERROR;
}

bool CSfxManager::CSfxWrapper::Ready() { return IsLooped() || mReady; }

short CSfxManager::CSfxWrapper::GetAudible(const CVector3f&) { return kSA_High; }

SND_VOICEID CSfxManager::CSfxWrapper::GetVoice() const { return mVoiceHandle; }

void CSfxManager::CSfxWrapper::SetVolume(short volume) { mVolume = volume; }

void CSfxManager::CSfxWrapper::UpdateEmitterSilent() { CAudioSys::SfxVolume(mVoiceHandle, 1); }

void CSfxManager::CSfxWrapper::UpdateEmitter() { CAudioSys::SfxVolume(mVoiceHandle, mVolume); }

void CSfxManager::Initialize() {
  mChannels[kSC_Game].mSounds.push_back(nullptr);
  mAuxEffectManager.Initialize();
}

void CSfxManager::Shutdown() {
  delete mpTranslationTable;
  mpTranslationTable = nullptr;
  StopAndRemoveAllEmitters();
  mAuxEffectManager.Shutdown();
  for (rstl::reserved_vector< CAuxEffect, 10 >::iterator it = mAuxEffects.begin();
       it != mAuxEffects.end(); ++it) {
    if (it->IsRegistered() && it->IsActive()) {
      it->Deactivate();
    }
  }
}

void CSfxManager::StopAndRemoveAllEmitters() {
  for (int i = 0; i < 4; ++i) {
    CSfxChannel& channel = mChannels[i];
    for (int j = 0; j < channel.mSounds.size(); ++j) {
      if (channel.mSounds[j] != nullptr) {
        if (channel.mSounds[j]->IsPlaying()) {
          channel.mSounds[j]->Stop();
        }
        channel.mSounds[j]->Release();
        channel.mSounds[j] = nullptr;
      }
    }
  }
}

CSfxManager::CSfxListener::CSfxListener(CVector3f position, CVector3f direction, CVector3f heading,
                                        CVector3f up, float frontSur, float backSur,
                                        float soundSpeed, uint flags, uchar maxVolume)
: mPosition(position)
, mDirection(direction)
, mHeading(heading)
, mUp(up)
, mFrontSur(frontSur)
, mBackSur(backSur)
, mSoundSpeed(soundSpeed)
, mFlags(flags)
, mMaxVolume(maxVolume) {}

void CSfxManager::AddListener(ESfxChannels channel, const CVector3f& position,
                              const CVector3f& direction, const CVector3f& heading,
                              const CVector3f& up, float frontSur, float backSur, float soundSpeed,
                              uint flags, uchar maxVolume, int listener) {
  SListener& entry = mChannels[channel].mListeners[listener];
  entry.mListener = CSfxListener(position, direction, heading, up, frontSur, backSur, soundSpeed,
                                 flags, maxVolume);
  entry.mActive = true;
  CAudioSys::S3dAddListener(position, direction, heading, up, frontSur, backSur, soundSpeed, flags,
                            maxVolume);
}

void CSfxManager::UpdateListener(const CVector3f& position, const CVector3f& direction,
                                 const CVector3f& heading, const CVector3f& up, uchar maxVolume,
                                 int listener) {
  SListener& entry = mChannels[mCurrentChannel].mListeners[listener];
  entry.mListener.mPosition = position;
  entry.mListener.mDirection = direction;
  entry.mListener.mHeading = heading;
  entry.mListener.mUp = up;
  entry.mListener.mMaxVolume = maxVolume;
  entry.mActive = true;
}

CSfxHandle CSfxManager::AddEmitter(ushort id, const CVector3f& position, int area,
                                   bool useAcoustics, bool looped, short priority) {
  CAudioSys::C3DEmitterParmData params(150.f, 0.1f, 1, 127, 20);
  params.mPos = position;
  params.mDir = CVector3f::Zero();
  params.mSfxId = id;
  return AddEmitter(params, area, useAcoustics, looped, priority);
}

CSfxHandle CSfxManager::AddEmitter(ushort id, const CVector3f& position, uchar volume, int area,
                                   bool useAcoustics, bool looped, short priority) {
  CAudioSys::C3DEmitterParmData params(150.f, 0.1f, 1, rstl::max_val(int(volume), 21), 20);
  params.mPos = position;
  params.mDir = CVector3f::Zero();
  params.mSfxId = id;
  return AddEmitter(params, area, useAcoustics, looped, priority);
}

CSfxManager::CSfxEmitterWrapper::~CSfxEmitterWrapper() {}

CSfxHandle CSfxManager::AddEmitter(CAudioSys::C3DEmitterParmData& params, int area,
                                   bool useAcoustics, bool looped, short priority) {
  if ((mMuted && !looped) || params.mSfxId == kInternalInvalidSfxId) {
    return CSfxHandle::NullHandle();
  }
  CAudioSys::C3DEmitterParmData emitter(params);
  if (looped) {
    emitter.mFlags |= 6;
  }
  emitter.mSfxId = TranslateSFXID(params.mSfxId);
  const uchar areaVolume = GetAreaVolume(area);
  if (areaVolume != 127) {
    emitter.mMaxVol = areaVolume * rstl::min_val(int(emitter.mMaxVol), 127) / 127;
  }
  if (emitter.mSfxId == kInternalInvalidSfxId) {
    return CSfxHandle::NullHandle();
  }

  mDoUpdate = true;
  const CSfxHandle handle = LocateHandle();
  if (handle) {
    CSfxEmitterWrapper* sound = AllocateCSfxEmitterWrapper(
        CSfxEmitterWrapper(looped, priority, emitter, handle, useAcoustics, area));
    if (mMuted) {
      sound->UpdateEmitterSilent();
    }
    mChannels[mCurrentChannel].mSounds[handle.GetIndex()] = sound;
  }
  return handle;
}

void CSfxManager::UpdateEmitter(CSfxHandle handle, const CVector3f& position,
                                const CVector3f& direction, uchar maxVolume) {
  CSfxChannel& channel = mChannels[mCurrentChannel];
  const int index = handle.GetIndex();
  if (index < 0 || index >= channel.mSounds.size()) {
    return;
  }
  CSfxEmitterWrapper* sound = static_cast< CSfxEmitterWrapper* >(channel.mSounds[index]);
  if (sound == nullptr || handle != sound->GetSfxHandle() || !sound->IsPlaying()) {
    return;
  }
  mDoUpdate = true;
  CAudioSys::C3DEmitterParmData& emitter = sound->GetEmitter();
  emitter.mPos = position;
  emitter.mDir = direction;
  if (!sound->IsSilent()) {
    const uchar areaVolume = GetAreaVolume(sound->GetArea());
    if (areaVolume != 127) {
      maxVolume = areaVolume * rstl::min_val(int(maxVolume), 127) / 127;
    }
    emitter.mMaxVol = rstl::max_val(int(maxVolume), 2);
  }
}

void CSfxManager::RemoveEmitter(CSfxHandle handle) { StopSound(mCurrentChannel, handle); }

CSfxHandle CSfxManager::SfxStart(ushort id, short volume, short pan, int area, bool useAcoustics,
                                 bool looped, short priority) {
  if ((mMuted && !looped) || id == kInternalInvalidSfxId) {
    return CSfxHandle::NullHandle();
  }
  mDoUpdate = true;
  const CSfxHandle handle = LocateHandle();
  if (handle) {
    const ushort translatedId = TranslateSFXID(id);
    if (translatedId == kInternalInvalidSfxId) {
      return CSfxHandle::NullHandle();
    }
    const uchar areaVolume = GetAreaVolume(area);
    if (areaVolume != 127) {
      volume = areaVolume * rstl::min_val(int(uchar(volume)), 127) / 127;
    }
    mChannels[mCurrentChannel].mSounds[handle.GetIndex()] = AllocateCSfxWrapper(CSfxWrapper(
        looped, priority, translatedId, uchar(volume), pan, handle, useAcoustics, area));
  }
  return handle;
}

void CSfxManager::SfxStop(CSfxHandle handle) { StopSound(mCurrentChannel, handle); }

void CSfxManager::SfxStop(ESfxChannels channel, CSfxHandle handle) { StopSound(channel, handle); }

void CSfxManager::SfxVolume(CSfxHandle handle, uchar volume) {
  CSfxChannel& channel = mChannels[mCurrentChannel];
  const int index = handle.GetIndex();
  if (index < 0 || index >= channel.mSounds.size()) {
    return;
  }
  CBaseSfxWrapper* base = channel.mSounds[index];
  if (base == nullptr || handle != base->GetSfxHandle()) {
    return;
  }
  CSfxWrapper* sound = static_cast< CSfxWrapper* >(base);
  const uchar areaVolume = GetAreaVolume(sound->GetArea());
  const uchar scaled = areaVolume == 127
                           ? volume
                           : uchar(areaVolume * rstl::min_val(volume, uchar(127)) / 127);
  const uchar clamped = scaled < 1 ? 1 : (scaled > 127 ? 127 : scaled);
  sound->SetVolume(clamped);
  if (!mMuted && sound->IsPlaying()) {
    CAudioSys::SfxVolume(sound->GetVoice(), clamped);
  }
}

void CSfxManager::SfxPan(CSfxHandle handle, uchar pan) {
  if (!handle) {
    return;
  }
  CSfxChannel& channel = mChannels[mCurrentChannel];
  const int index = handle.GetIndex();
  if (index < 0 || index >= channel.mSounds.size()) {
    return;
  }
  CBaseSfxWrapper* sound = channel.mSounds[index];
  if (sound == nullptr || handle != sound->GetSfxHandle()) {
    return;
  }
  if (!sound->IsPlaying()) {
    Update(0.f);
  }
  if (sound->IsPlaying()) {
    CAudioSys::SfxPan(sound->GetVoice(), pan);
  }
}

void CSfxManager::SfxSpan(CSfxHandle handle, uchar span) {
  if (!handle) {
    return;
  }
  CSfxChannel& channel = mChannels[mCurrentChannel];
  const int index = handle.GetIndex();
  if (index < 0 || index >= channel.mSounds.size()) {
    return;
  }
  CBaseSfxWrapper* sound = channel.mSounds[index];
  if (sound == nullptr || handle != sound->GetSfxHandle()) {
    return;
  }
  if (!sound->IsPlaying()) {
    Update(0.f);
  }
  if (sound->IsPlaying()) {
    CAudioSys::SfxSpan(sound->GetVoice(), span);
  }
}

void CSfxManager::KillAll(ESfxChannels channel) {
  CSfxChannel& sounds = mChannels[channel];
  for (int i = 0; i < sounds.mSounds.size(); ++i) {
    CBaseSfxWrapper* sound = sounds.mSounds[i];
    if (sound != nullptr && sound->IsPlaying()) {
      sound->Stop();
    }
    if (sound != nullptr) {
      sound->Release();
    }
    sounds.mSounds[i] = nullptr;
  }
  if (channel == kSC_Game) {
    mAuxEffectManager.Shutdown();
    mAuxEffectManager.Initialize();
  }
}

void CSfxManager::StopSound(ESfxChannels channel, CSfxHandle handle) {
  CSfxChannel& sounds = mChannels[channel];
  const int index = handle.GetIndex();
  if (index < 0 || index >= sounds.mSounds.size()) {
    if (channel != kSC_Game) {
      StopSound(kSC_Game, handle);
    }
    return;
  }
  CBaseSfxWrapper* sound = sounds.mSounds[index];
  if (sound == nullptr || handle != sound->GetSfxHandle()) {
    if (channel != kSC_Game) {
      StopSound(kSC_Game, handle);
    }
    return;
  }
  mDoUpdate = true;
  if (sound->IsPlaying()) {
    sound->Stop();
  }
  sound->Release();
  sounds.mSounds[handle.GetIndex()] = nullptr;
}

void CSfxManager::SetDuration(CSfxHandle handle, float duration) {
  CSfxChannel& channel = mChannels[mCurrentChannel];
  const int index = handle.GetIndex();
  if (index < 0 || index >= channel.mSounds.size()) {
    return;
  }
  CBaseSfxWrapper* sound = channel.mSounds[index];
  if (sound == nullptr || handle != sound->GetSfxHandle()) {
    return;
  }
  sound->SetTimeRemaining(duration);
}

void CSfxManager::SetChannel(ESfxChannels channel) {
  ESfxChannels current = mCurrentChannel;
  if (channel == current) {
    return;
  }
  if (channel == kSC_Default) {
    mAreaLowPassFilters.clear();
    mLowPassFilters.clear();
  }
  if (current != kSC_Invalid) {
    TurnOffChannel(current);
  }
  TurnOnChannel(channel);
  mCurrentChannel = channel;
}

CSfxManager::ESfxChannels CSfxManager::GetChannel() { return mCurrentChannel; }

void CSfxManager::TurnOffChannel(ESfxChannels channel) {
  CSfxChannel& sounds = mChannels[channel];
  for (int i = 0; i < sounds.mSounds.size(); ++i) {
    if (sounds.mSounds[i] != nullptr) {
      if (sounds.mSounds[i]->IsLooped()) {
        sounds.mSounds[i]->UpdateEmitterSilent();
      } else {
        sounds.mSounds[i]->Stop();
      }
    }
  }
  for (int i = 0; i < sounds.mSounds.size(); ++i) {
    if (sounds.mSounds[i] != nullptr && !sounds.mSounds[i]->IsLooped()) {
      sounds.mSounds[i]->Release();
      sounds.mSounds[i] = nullptr;
    }
  }
}

void CSfxManager::TurnOnChannel(ESfxChannels channel) {
  mCurrentChannel = channel;
  CSfxChannel& sounds = mChannels[channel];
  mDoUpdate = true;
  bool hasListener = false;
  for (int i = 0; i < 4; ++i) {
    if (sounds.mListeners[i].mActive) {
      hasListener = true;
      break;
    }
  }
  if (hasListener) {
    for (int j = 0; j < sounds.mSounds.size(); ++j) {
      if (sounds.mSounds[j] != nullptr) {
        sounds.mSounds[j]->UpdateEmitter();
      }
    }
  }
}

CSfxHandle CSfxManager::LocateHandle() {
  CSfxChannel& channel = mChannels[mCurrentChannel];
  for (int i = 0; i < channel.mSounds.size(); ++i) {
    if (channel.mSounds[i] == nullptr) {
      return CSfxHandle(i);
    }
  }
  if (channel.mSounds.size() == channel.mSounds.capacity()) {
    return CSfxHandle::NullHandle();
  }
  channel.mSounds.push_back(nullptr);
  return CSfxHandle(channel.mSounds.size() - 1);
}

void CSfxManager::Update(float dt) {
  short i;
  ushort count = 0;
  CSfxChannel& channel = mChannels[mCurrentChannel];
  for (i = 0; i < channel.mSounds.size(); ++i) {
    if (channel.mSounds[i] == nullptr || channel.mSounds[i]->IsLooped()) {
      continue;
    }
    const float remaining = channel.mSounds[i]->GetTimeRemaining();
    channel.mSounds[i]->SetTimeRemaining(remaining - dt);
    if (remaining < 0.f) {
      channel.mSounds[i]->Stop();
      mDoUpdate = true;
    }
  }
  ushort ranked[72];
  if (mDoUpdate) {
    for (i = 0; i < channel.mSounds.size(); ++i) {
      if (channel.mSounds[i] != nullptr) {
        ranked[count++] = i;
        channel.mSounds[i]->SetRank(GetRank(channel.mSounds[i]));
      }
    }
    for (i = 0; i < count; ++i) {
      bool done = true;
      for (int j = 0; j < count - 1; ++j) {
        if (channel.mSounds[ranked[j]]->GetRank() < channel.mSounds[ranked[j + 1]]->GetRank()) {
          done = false;
          rstl::swap(ranked[j], ranked[j + 1]);
        }
      }
      if (done) {
        break;
      }
    }
    for (i = 48; i < count; ++i) {
      if (channel.mSounds[ranked[i]] != nullptr && channel.mSounds[ranked[i]]->IsPlaying()) {
        channel.mSounds[ranked[i]]->Stop();
      }
    }
    for (i = 0; i < count; ++i) {
      if (channel.mSounds[ranked[i]] != nullptr && channel.mSounds[ranked[i]]->IsPlaying() &&
          !channel.mSounds[ranked[i]]->IsInArea()) {
        channel.mSounds[ranked[i]]->Stop();
      }
    }
  }
  CAudioSys::S3dFlushUnusedEmitters();
  if (mDoUpdate && !mMuted) {
    for (int available = 48, j = 0; j < count && available != 0; ++j) {
      if (channel.mSounds[ranked[j]] == nullptr) {
        continue;
      }
      if (channel.mSounds[ranked[j]]->IsPlaying()) {
        --available;
      } else if (channel.mSounds[ranked[j]]->Ready() && channel.mSounds[ranked[j]]->IsInArea()) {
        channel.mSounds[ranked[j]]->Play();
        --available;
      }
    }
    mDoUpdate = false;
  }
  for (int j = 0; j < channel.mSounds.size(); ++j) {
    if (channel.mSounds[j] != nullptr && !channel.mSounds[j]->IsPlaying() &&
        !channel.mSounds[j]->IsLooped()) {
      channel.mSounds[j]->Release();
      channel.mSounds[j] = nullptr;
      mDoUpdate = true;
    }
  }
  if (mpTranslationTableToken.get() && mpTranslationTableToken->HasLock() &&
      mpTranslationTableToken->IsLoaded()) {
    if (mpTranslationTable == nullptr) {
      TToken< rstl::vector< short > > token(*mpTranslationTableToken);
      mpTranslationTable = rs_new rstl::vector< short >(*token.GetT());
    }
    mpTranslationTableToken = nullptr;
  }
  if (!(fabsf(dt - 0.f) < 0.00001f)) {
    UpdatePitchBends(dt);
  }
  int primary = -1;
  for (int j = 0; j < 4; ++j) {
    if (channel.mListeners[j].mActive) {
      const CSfxListener& listener = channel.mListeners[j].mListener;
      CAudioSys::S3dUpdateListener(listener.mPosition, listener.mDirection, listener.mHeading,
                                   listener.mUp, listener.mMaxVolume);
      primary = j;
      break;
    }
  }
  if (primary != -1) {
    rstl::reserved_vector< CVector3f, 4 > rightVectors;
    for (int j = primary; j < 4; ++j) {
      if (channel.mListeners[j].mActive) {
        const CSfxListener& listener = channel.mListeners[j].mListener;
        rightVectors.push_back(CVector3f::Cross(listener.mHeading, listener.mUp).AsNormalized());
      } else {
        rightVectors.push_back(CVector3f::Right());
      }
    }
    const CSfxListener& listener = channel.mListeners[primary].mListener;
    for (int j = 0; j < channel.mSounds.size(); ++j) {
      CBaseSfxWrapper* sound = channel.mSounds[j];
      if (sound == nullptr || !sound->IsEmitter() || !sound->IsPlaying()) {
        continue;
      }
      CSfxEmitterWrapper* emitter = static_cast< CSfxEmitterWrapper* >(sound);
      if (emitter->IsSilent() && !emitter->mUpdatePending) {
        continue;
      }
      const CVector3f& position = emitter->GetEmitter().mPos;
      const CVector3f& direction = emitter->GetEmitter().mDir;
      uchar volume = emitter->GetEmitter().mMaxVol;
      if (emitter->mUpdatePending) {
        volume = emitter->mCachedMaxVolume;
        emitter->mUpdatePending = false;
        emitter->GetEmitter().mMaxVol = volume;
      }
      int closest = -1;
      float distance = FLT_MAX;
      for (int k = primary; k < 4; ++k) {
        if (channel.mListeners[k].mActive) {
          const float candidate =
              (channel.mListeners[k].mListener.mPosition - position).MagSquared();
          if (candidate < distance) {
            closest = k;
            distance = candidate;
          }
        }
      }
      if (closest == primary || closest == -1) {
        CAudioSys::S3dUpdateEmitter(emitter->GetHandle(), position, direction, volume);
      } else {
        const CSfxListener& other = channel.mListeners[closest].mListener;
        const CVector3f relative = position - other.mPosition;
        const float forward = CVector3f::Dot(other.mHeading, relative);
        const float right = CVector3f::Dot(rightVectors[closest], relative);
        const float up = CVector3f::Dot(other.mUp, relative);
        const CVector3f transformedPosition = listener.mPosition + forward * listener.mHeading +
                                              right * rightVectors[primary] + up * listener.mUp;
        const CVector3f transformedDirection =
            direction.IsNonZero()
                ? CVector3f::Dot(other.mHeading, direction) * listener.mHeading +
                      CVector3f::Dot(rightVectors[closest], direction) * rightVectors[primary] +
                      CVector3f::Dot(other.mUp, direction) * listener.mUp
                : CVector3f::Zero();
        CAudioSys::S3dUpdateEmitter(emitter->GetHandle(), transformedPosition, transformedDirection,
                                    volume);
      }
    }
  }
  UpdateLowPassAreaFilters(dt);
  UpdateLowPassFilters(dt);
  if (mCurrentChannel == kSC_Game) {
    CSfxChannel& game = mChannels[kSC_Game];
    for (int j = 0; j < game.mSounds.size(); ++j) {
      CBaseSfxWrapper* sound = game.mSounds[j];
      if (sound != nullptr && sound->IsPlaying()) {
        const int area = sound->GetArea();
        const bool acoustics = sound->UseAcoustics();
        if (area != kAllAreas || acoustics) {
          const bool lowPass = ShouldApplyLowPass(sound);
          const int frequency = GetLowPassFrequency(sound);
          CAudioSys::SfxSetFilter(sound->GetVoice(), lowPass, frequency);
        }
        CAudioSys::SfxPitchBend(sound->GetVoice(), sound->GetPitchBend());
      }
    }
  }
  mAuxEffectManager.Cleanup();
}

void CSfxManager::PitchBend(CSfxHandle handle, int pitch) {
  CSfxChannel& channel = mChannels[mCurrentChannel];
  if (!handle) {
    return;
  }
  CBaseSfxWrapper* sound = channel.mSounds[handle.GetIndex()];
  if (sound == nullptr || handle != sound->GetSfxHandle()) {
    return;
  }
  sound->SetPitchBend(pitch);
  mDoUpdate = true;
}

bool CSfxManager::IsPlaying(CSfxHandle handle) {
  if (!handle) {
    return false;
  }
  const int index = handle.GetIndex();
  CSfxChannel& channel = mChannels[mCurrentChannel];
  if (index < 0 || index >= channel.mSounds.size()) {
    return false;
  }
  CBaseSfxWrapper* sound = channel.mSounds[index];
  if (sound == nullptr || handle != sound->GetSfxHandle() || !sound->IsPlaying()) {
    return false;
  }
  return sound->IsPlaying();
}

bool CSfxManager::IsQueued(CSfxHandle handle) {
  if (!handle) {
    return false;
  }
  const int index = handle.GetIndex();
  CSfxChannel& channel = mChannels[mCurrentChannel];
  if (index < 0 || index >= channel.mSounds.size()) {
    return false;
  }
  CBaseSfxWrapper* sound = channel.mSounds[index];
  if (sound == nullptr || handle != sound->GetSfxHandle()) {
    return false;
  }
  return true;
}

int CSfxManager::GetRank(CBaseSfxWrapper* sound) {
  const CSfxChannel& channel = mChannels[mCurrentChannel];
  if (!sound->IsInArea()) {
    return 0;
  }
  int rank = sound->GetPriority() >> 2;
  if (sound->IsPlaying()) {
    ++rank;
  }
  if (sound->IsLooped()) {
    rank -= 2;
  }
  if (sound->Ready() && !sound->IsPlaying()) {
    rank += 3;
  }
  for (int i = 0; i < 4; ++i) {
    if (channel.mListeners[i].mActive) {
      const int audible = sound->GetAudible(channel.mListeners[i].mListener.mPosition);
      if (audible == kSA_Inaudible) {
        rank = 0;
      } else {
        rank += audible * 2;
      }
    }
  }
  return rank;
}

bool CSfxManager::LoadTranslationTable(CSimplePool* pool, const SObjectTag* tag) {
  if (tag == nullptr) {
    return false;
  }
  if (mpTranslationTable != nullptr) {
    delete mpTranslationTable;
  }
  mpTranslationTable = nullptr;
  mpTranslationTableToken = rs_new CToken(pool->GetObj(*tag));
  mpTranslationTableToken->Lock();
  return true;
}

ushort CSfxManager::TranslateSFXID(ushort id) {
  if (mpTranslationTable == nullptr || id >= mpTranslationTable->size()) {
    return kInternalInvalidSfxId;
  }
  short translated = (*mpTranslationTable)[id];
  if (translated < 0) {
    return kInternalInvalidSfxId;
  }
  return translated;
}

void CSfxManager::SetActiveAreas(const rstl::reserved_vector< int, 10 >& areas, int currentArea) {
  mCurrentArea = currentArea;
  CSfxChannel& channel = mChannels[mCurrentChannel];
  for (CAuxEffect* effect = mAuxEffects.begin(); effect != mAuxEffects.end(); ++effect) {
    if (!effect->IsRegistered() || !effect->IsActive() || effect->GetArea() == kAllAreas) {
      continue;
    }
    const int* area = areas.begin();
    for (; area != areas.end(); ++area) {
      if (*area == effect->GetArea()) {
        break;
      }
    }
    if (area == areas.end()) {
      effect->Deactivate();
      mAuxEffectManager.RemoveEffect(effect->GetProcessingId());
      SetAreaVolume(effect->GetArea(), 127);
    }
  }
  for (SAreaVolume* volume = mAreaVolumes.begin(); volume != mAreaVolumes.end(); ++volume) {
    if (volume->mArea == kAllAreas) {
      continue;
    }
    const int* area = areas.begin();
    for (; area != areas.end(); ++area) {
      if (volume->mArea == *area) {
        break;
      }
    }
    if (area == areas.end()) {
      volume->mArea = kAllAreas;
    }
  }
  if (currentArea != mStudioState.first) {
    mStudioState.first = currentArea;
    mStudioState.second = (uchar)!mStudioState.second;
  }
  for (const int* area = areas.begin(); area != areas.end(); ++area) {
    int priority = -1;
    CAuxEffect* best = mAuxEffects.end();
    for (CAuxEffect* effect = mAuxEffects.begin(); effect != mAuxEffects.end(); ++effect) {
      if (effect->IsRegistered() && effect->GetArea() == *area &&
          effect->GetPriority() > priority) {
        best = effect;
        priority = effect->GetPriority();
      }
    }
    if (best == mAuxEffects.end() || best->IsActive()) {
      continue;
    }
    for (CAuxEffect* effect = mAuxEffects.begin(); effect != mAuxEffects.end(); ++effect) {
      if (effect->IsRegistered() && effect->GetArea() == *area) {
        mAuxEffectManager.RemoveEffect(effect->GetProcessingId());
        effect->SetActive(false);
      }
    }
    int bus = mStudioState.second;
    if (best->GetArea() != mStudioState.first) {
      bus = mStudioState.second ? 0 : 1;
    }
    best->SetProcessingId(
        mAuxEffectManager.AddEffect(bus, *best, CAuxEffectManager::kEC_Parallel, true));
    best->SetActive(true);
    best->SetBusIndex(bus);
    SetAreaVolume(best->GetArea(), best->GetVolume());
    break;
  }
  for (int i = 0; i < channel.mSounds.size(); ++i) {
    if (channel.mSounds[i] == nullptr) {
      continue;
    }
    const int soundArea = channel.mSounds[i]->GetArea();
    if (soundArea == kAllAreas) {
      channel.mSounds[i]->SetInArea(true);
    } else {
      bool inArea = false;
      for (const int* area = areas.begin(); area != areas.end(); ++area) {
        if (*area == soundArea) {
          inArea = true;
        }
      }
      mDoUpdate = true;
      channel.mSounds[i]->SetInArea(inArea);
    }
  }
}

CSfxManager::CSfxEmitterWrapper*
CSfxManager::AllocateCSfxEmitterWrapper(const CSfxEmitterWrapper& sound) {
  CSfxEmitterWrapper* result = nullptr;
  for (int i = 0; i < mEmitterWrapperPool.size(); ++i) {
    if (mEmitterWrapperPool[i].Available()) {
      mEmitterWrapperPool[i] = sound;
      result = &mEmitterWrapperPool[i];
      break;
    }
  }
  if (result == nullptr && mEmitterWrapperPool.size() != mEmitterWrapperPool.capacity()) {
    mEmitterWrapperPool.push_back(sound);
    result = &mEmitterWrapperPool.back();
  }
  return result;
}

CSfxManager::CSfxWrapper* CSfxManager::AllocateCSfxWrapper(const CSfxWrapper& sound) {
  CSfxWrapper* result = nullptr;
  for (int i = 0; i < mWrapperPool.size(); ++i) {
    if (mWrapperPool[i].Available()) {
      mWrapperPool[i] = sound;
      result = &mWrapperPool[i];
      break;
    }
  }
  if (result == nullptr && mWrapperPool.size() != mWrapperPool.capacity()) {
    mWrapperPool.push_back(sound);
    result = &mWrapperPool.back();
  }
  return result;
}

void CSfxManager::SetMuted(bool muted) {
  CSfxChannel& channel = mChannels[mCurrentChannel];
  mDoUpdate = true;
  mMuted = muted;
  if (muted) {
    for (int i = 0; i < channel.mSounds.size(); ++i) {
      if (channel.mSounds[i] != nullptr) {
        if (channel.mSounds[i]->IsLooped()) {
          channel.mSounds[i]->UpdateEmitterSilent();
        } else {
          channel.mSounds[i]->Stop();
        }
      }
    }
    for (int i = 0; i < channel.mSounds.size(); ++i) {
      if (channel.mSounds[i] != nullptr && !channel.mSounds[i]->IsLooped()) {
        channel.mSounds[i]->Release();
        channel.mSounds[i] = nullptr;
      }
    }
  } else {
    for (int i = 0; i < channel.mSounds.size(); ++i) {
      if (channel.mSounds[i] != nullptr) {
        channel.mSounds[i]->UpdateEmitter();
      }
    }
  }
}

short CSfxManager::GetReverbAmount() { return 127; }

uchar CSfxManager::GetStudio(int area) {
  static const uchar studios[] = {1, 2};
  if (area == kAllAreas || area == mStudioState.first) {
    return studios[mStudioState.second];
  }
  return studios[mStudioState.second ? 0 : 1];
}

void CSfxManager::AddPitchBend(const CSfxPitchBend& pitchBend) {
  if (mPitchBends.size() < mPitchBends.capacity()) {
    mPitchBends.push_back(pitchBend);
  }
}

void CSfxManager::UpdatePitchBends(float dt) {
  if (mCurrentChannel != kSC_Game) {
    return;
  }
  for (CSfxPitchBend* bend = mPitchBends.begin(); bend != mPitchBends.end();) {
    bend->Update(dt);
    PitchBend(bend->GetHandle(), bend->GetPitch());
    const bool queued = IsQueued(bend->GetHandle());
    if (bend->IsFinished() || !queued) {
      bend = mPitchBends.erase(bend);
    } else {
      ++bend;
    }
  }
}

void CSfxManager::RemoveAuxEffect(int id) {
  for (CAuxEffect* effect = mAuxEffects.begin(); effect != mAuxEffects.end(); ++effect) {
    if (effect->IsRegistered() && effect->GetId() == id) {
      if (effect->IsActive()) {
        mAuxEffectManager.RemoveEffect(effect->GetProcessingId());
        SetAreaVolume(effect->GetArea(), 127);
      }
      effect->Unregister();
    }
  }
}

int CSfxManager::RegisterAuxEffect(const CAuxEffect& effect) {
  if (++mNextAuxEffectId == 0) {
    ++mNextAuxEffectId;
  }
  CAuxEffect* slot = nullptr;
  for (CAuxEffect* candidate = mAuxEffects.begin(); candidate != mAuxEffects.end(); ++candidate) {
    if (!candidate->IsRegistered()) {
      slot = candidate;
      break;
    }
  }
  if (slot == nullptr) {
    if (mAuxEffects.size() >= mAuxEffects.capacity()) {
      return 0;
    }
    mAuxEffects.push_back(effect);
    slot = &mAuxEffects.back();
  }
  *slot = effect;
  slot->Register();
  slot->SetActive(false);
  slot->SetId(mNextAuxEffectId);
  if (slot->GetArea() == kAllAreas) {
    slot->SetProcessingId(
        mAuxEffectManager.AddEffect(2, *slot, CAuxEffectManager::kEC_Serial, true));
    slot->SetActive(true);
  }
  return mNextAuxEffectId;
}

int CSfxManager::AddAuxEffect(int area, const SND_AUX_REVERBHI& params, uchar volume,
                              int priority) {
  return RegisterAuxEffect(CAuxEffect(params, area, volume, priority));
}

int CSfxManager::AddAuxEffect(int area, const SND_AUX_CHORUS& params, uchar volume, int priority) {
  return RegisterAuxEffect(CAuxEffect(params, area, volume, priority));
}

int CSfxManager::AddAuxEffect(int area, const SND_AUX_REVERBSTD& params, uchar volume,
                              int priority) {
  return RegisterAuxEffect(CAuxEffect(params, area, volume, priority));
}

int CSfxManager::AddAuxEffect(int area, const SND_AUX_DELAY& params, uchar volume, int priority) {
  return RegisterAuxEffect(CAuxEffect(params, area, volume, priority));
}

int CSfxManager::AddAuxEffect(int area, const SFlangerAuxParameters& params, uchar volume,
                              int priority) {
  return RegisterAuxEffect(CAuxEffect(params, area, volume, priority));
}

int CSfxManager::AddAuxEffect(int area, const SBitcrusherAuxParameters& params, uchar volume,
                              int priority) {
  return RegisterAuxEffect(CAuxEffect(params, area, volume, priority));
}

int CSfxManager::AddAuxEffect(int area, const SPhaserAuxParameters& params, uchar volume,
                              int priority) {
  return RegisterAuxEffect(CAuxEffect(params, area, volume, priority));
}

int CSfxManager::AddAuxEffect(int area, const SFilteredDelayAuxParameters& params, uchar volume,
                              int priority) {
  return RegisterAuxEffect(CAuxEffect(params, area, volume, priority));
}

int CSfxManager::AddLowPassAreaFilter(int frequency, float duration) {
  if (mAreaLowPassFilters.size() < mAreaLowPassFilters.capacity()) {
    if (++mNextAreaFilterId == 0) {
      mNextAreaFilterId = 1;
    }
    mAreaLowPassFilters.push_back(SLowPassFilter(frequency, duration, mNextAreaFilterId));
    return mNextAreaFilterId;
  }
  return 0;
}

void CSfxManager::UpdateLowPassAreaFilters(float dt) {
  mAreaLowPassFrequency = 44100;
  bool haveFrequency = false;
  for (SLowPassFilter* it = mAreaLowPassFilters.begin(); it != mAreaLowPassFilters.end();) {
    it->mTimeRemaining -= dt;
    const bool active = !it->mTimed || it->mTimeRemaining > 0.f;
    if (!active) {
      it = mAreaLowPassFilters.erase(it);
      continue;
    }
    if (!haveFrequency || it->mFrequency < mAreaLowPassFrequency) {
      mAreaLowPassFrequency = it->mFrequency;
      haveFrequency = true;
    }
    ++it;
  }
}

void CSfxManager::RemoveLowPassAreaFilter(int id) {
  if (id == 0) {
    return;
  }
  for (SLowPassFilter* it = mAreaLowPassFilters.begin(); it != mAreaLowPassFilters.end(); ++it) {
    if (id == it->mId) {
      mAreaLowPassFilters.erase(it);
      return;
    }
  }
}

bool CSfxManager::IsLowPassAreaFilterEnabled() {
  return mAreaLowPassFilters.size() > 0 && mCurrentChannel == kSC_Game;
}

int CSfxManager::GetLowPassAreaFrequency() { return mAreaLowPassFrequency; }

int CSfxManager::AddLowPassFilter(int frequency, float duration) {
  if (mLowPassFilters.size() < mLowPassFilters.capacity()) {
    if (++mNextFilterId == 0) {
      mNextFilterId = 1;
    }
    mLowPassFilters.push_back(SLowPassFilter(frequency, duration, mNextFilterId));
    return mNextFilterId;
  }
  return 0;
}

void CSfxManager::UpdateLowPassFilters(float dt) {
  mLowPassFrequency = 44100;
  bool haveFrequency = false;
  for (SLowPassFilter* it = mLowPassFilters.begin(); it != mLowPassFilters.end();) {
    it->mTimeRemaining -= dt;
    const bool active = !it->mTimed || it->mTimeRemaining > 0.f;
    if (!active) {
      it = mLowPassFilters.erase(it);
      continue;
    }
    if (!haveFrequency || it->mFrequency < mLowPassFrequency) {
      mLowPassFrequency = it->mFrequency;
      haveFrequency = true;
    }
    ++it;
  }
}

void CSfxManager::RemoveLowPassFilter(int id) {
  if (id == 0) {
    return;
  }
  for (SLowPassFilter* it = mLowPassFilters.begin(); it != mLowPassFilters.end(); ++it) {
    if (id == it->mId) {
      mLowPassFilters.erase(it);
      return;
    }
  }
}

bool CSfxManager::IsLowPassEnabled() {
  return mLowPassFilters.size() > 0 && mCurrentChannel == kSC_Game;
}

int CSfxManager::GetLowPassFrequency() { return mLowPassFrequency; }

bool CSfxManager::ShouldApplyLowPass(CBaseSfxWrapper* sound) {
  if (sound->GetArea() != kAllAreas && IsLowPassAreaFilterEnabled() &&
      !sound->GetIgnoreAreaLowPass()) {
    return true;
  }
  return sound->UseAcoustics() && IsLowPassEnabled();
}

int CSfxManager::GetLowPassFrequency(CBaseSfxWrapper* sound) {
  int frequency = mAreaLowPassFrequency;
  if (sound->GetArea() != kAllAreas && IsLowPassAreaFilterEnabled() &&
      !sound->GetIgnoreAreaLowPass()) {
    frequency = GetLowPassAreaFrequency();
  }
  if (sound->UseAcoustics() && IsLowPassEnabled()) {
    frequency = rstl::min_val(frequency, GetLowPassFrequency());
  }
  return frequency;
}

uchar CSfxManager::GetAreaVolume(int area) {
  if (area != kAllAreas) {
    for (SAreaVolume* it = mAreaVolumes.begin(); it != mAreaVolumes.end(); ++it) {
      if (area == it->mArea) {
        return it->mVolume;
      }
    }
  }
  return 127;
}

void CSfxManager::SetAreaVolume(int area, uchar volume) {
  SAreaVolume* it;
  for (it = mAreaVolumes.begin(); it != mAreaVolumes.end(); ++it) {
    if (area == it->mArea) {
      it->mVolume = volume;
      break;
    }
  }
  if (it != mAreaVolumes.end()) {
    return;
  }
  for (it = mAreaVolumes.begin(); it != mAreaVolumes.end(); ++it) {
    if (it->mArea == kAllAreas) {
      it->mArea = area;
      it->mVolume = volume;
      return;
    }
  }
}

void CSfxManager::SetIgnoreAreaLowPass(CSfxHandle handle, bool ignore) {
  CSfxChannel& channel = mChannels[mCurrentChannel];
  const int index = handle.GetIndex();
  if (index < 0 || index >= channel.mSounds.size()) {
    return;
  }
  CBaseSfxWrapper* sound = channel.mSounds[index];
  if (sound == nullptr || handle != sound->GetSfxHandle()) {
    return;
  }
  sound->SetIgnoreAreaLowPass(ignore);
}

CSfxManager::CSfxWrapper::~CSfxWrapper() {}

bool CSfxManager::CSfxWrapper::IsEmitter() const { return false; }

CFactoryFnReturn FAudioTranslationTableFactory(const SObjectTag&, CInputStream& in,
                                               const CVParamTransfer&) {
  return rs_new rstl::vector< short >(in);
}
