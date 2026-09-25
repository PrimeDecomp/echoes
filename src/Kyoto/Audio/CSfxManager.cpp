#include "Kyoto/Audio/CSfxManager.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/CToken.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/math.hpp"

CSfxManager::CSfxChannel CSfxManager::mChannels[4];
CSfxManager::ESfxChannels CSfxManager::mCurrentChannel = kSC_Default;
bool CSfxManager::mDoUpdate = false;
bool CSfxManager::mMuted = false;
rstl::vector< short >* CSfxManager::mpTranslationTable = nullptr;
rstl::auto_ptr< CToken > CSfxManager::mpTranslationTableToken;
rstl::reserved_vector< CSfxManager::CSfxEmitterWrapper, 64 > CSfxManager::mEmitterWrapperPool;
rstl::reserved_vector< CSfxManager::CSfxWrapper, 64 > CSfxManager::mWrapperPool;
rstl::reserved_vector< CSfxManager::SLowPassFilter, 8 > CSfxManager::mAreaLowPassFilters;
rstl::reserved_vector< CSfxManager::SLowPassFilter, 8 > CSfxManager::mLowPassFilters;
int CSfxManager::mNextAreaFilterId = 0;
int CSfxManager::mNextFilterId = 0;
int CSfxManager::mAreaLowPassFrequency = 16000;
int CSfxManager::mLowPassFrequency = 16000;
rstl::reserved_vector< CSfxManager::SAreaVolume, 10 > CSfxManager::mAreaVolumes(10, SAreaVolume());
int CSfxManager::mCurrentArea = -1;
bool CSfxManager::mCurrentStudio = false;

const short CSfxManager::kMaxPriority = 255;
const short CSfxManager::kMedPriority = 127;
const ushort CSfxManager::kInternalInvalidSfxId = 0xffff;
const int CSfxManager::kAllAreas = -1;

CSfxManager::CSfxChannel::CSfxChannel() : mListeners(4, SListener()) {}

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
  return CBaseSfxWrapper::IsPlaying() && (IsLooped() || CAudioSys::S3dCheckEmitter(mEmitterHandle));
}

bool CSfxManager::CSfxEmitterWrapper::Ready() { return IsLooped() || mReady; }

short CSfxManager::CSfxEmitterWrapper::GetAudible(const CVector3f& position) {
  const float distanceSquared = (mEmitterData.mPos - position).MagSquared();
  const float maxDistanceSquared = mEmitterData.mMaxDist * mEmitterData.mMaxDist;
  if (distanceSquared < maxDistanceSquared * 0.25f) {
    return kSA_Aud3;
  }
  if (distanceSquared < maxDistanceSquared * 0.5f) {
    return kSA_Aud2;
  }
  return distanceSquared < maxDistanceSquared ? kSA_Aud1 : kSA_Aud0;
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

short CSfxManager::CSfxWrapper::GetAudible(const CVector3f&) { return kSA_Aud3; }

SND_VOICEID CSfxManager::CSfxWrapper::GetVoice() const { return mVoiceHandle; }

void CSfxManager::CSfxWrapper::SetVolume(short volume) { mVolume = volume; }

void CSfxManager::CSfxWrapper::UpdateEmitterSilent() { CAudioSys::SfxVolume(mVoiceHandle, 1); }

void CSfxManager::CSfxWrapper::UpdateEmitter() { CAudioSys::SfxVolume(mVoiceHandle, mVolume); }

void CSfxManager::Initialize() {
  mChannels[kSC_Game].mSounds.push_back(nullptr);
  // TODO: initialize the Echoes auxiliary-effect manager.
}

void CSfxManager::Shutdown() {
  delete mpTranslationTable;
  mpTranslationTable = nullptr;
  StopAndRemoveAllEmitters();
  // TODO: shut down auxiliary effects and release active effect records.
}

void CSfxManager::StopAndRemoveAllEmitters() {
  for (int i = 0; i < 4; ++i) {
    CSfxChannel& channel = mChannels[i];
    for (int j = 0; j < channel.mSounds.size(); ++j) {
      CBaseSfxWrapper* sound = channel.mSounds[j];
      if (sound != nullptr) {
        if (sound->IsPlaying()) {
          sound->Stop();
        }
        sound->Release();
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
  if (!IsQueued(handle)) {
    return;
  }
  CSfxEmitterWrapper* sound =
      static_cast< CSfxEmitterWrapper* >(mChannels[mCurrentChannel].mSounds[handle.GetIndex()]);
  if (!sound->IsPlaying()) {
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
  if (!IsQueued(handle)) {
    return;
  }
  CSfxWrapper* sound =
      static_cast< CSfxWrapper* >(mChannels[mCurrentChannel].mSounds[handle.GetIndex()]);
  const uchar areaVolume = GetAreaVolume(sound->GetArea());
  if (areaVolume != 127) {
    volume = areaVolume * rstl::min_val(int(volume), 127) / 127;
  }
  volume = rstl::max_val(1, rstl::min_val(int(volume), 127));
  sound->SetVolume(volume);
  if (!mMuted && sound->IsPlaying()) {
    CAudioSys::SfxVolume(sound->GetVoice(), volume);
  }
}

void CSfxManager::SfxPan(CSfxHandle handle, uchar pan) {
  if (!IsQueued(handle)) {
    return;
  }
  CBaseSfxWrapper* sound = mChannels[mCurrentChannel].mSounds[handle.GetIndex()];
  if (!sound->IsPlaying()) {
    Update(0.f);
  }
  if (sound->IsPlaying()) {
    CAudioSys::SfxPan(sound->GetVoice(), pan);
  }
}

void CSfxManager::SfxSpan(CSfxHandle handle, uchar span) {
  if (!IsQueued(handle)) {
    return;
  }
  CBaseSfxWrapper* sound = mChannels[mCurrentChannel].mSounds[handle.GetIndex()];
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
    if (sound != nullptr) {
      if (sound->IsPlaying()) {
        sound->Stop();
      }
      sound->Release();
    }
    sounds.mSounds[i] = nullptr;
  }
  // TODO: clear/reinitialize auxiliary effects when channel is kSC_Game.
}

void CSfxManager::StopSound(ESfxChannels channel, CSfxHandle handle) {
  CSfxChannel& sounds = mChannels[channel];
  if (handle.GetIndex() < sounds.mSounds.size()) {
    CBaseSfxWrapper* sound = sounds.mSounds[handle.GetIndex()];
    if (sound != nullptr && sound->GetSfxHandle() == handle) {
      mDoUpdate = true;
      if (sound->IsPlaying()) {
        sound->Stop();
      }
      sound->Release();
      sounds.mSounds[handle.GetIndex()] = nullptr;
      return;
    }
  }
  if (channel != kSC_Game) {
    StopSound(kSC_Game, handle);
  }
}

void CSfxManager::SetDuration(CSfxHandle handle, float duration) {
  if (IsQueued(handle)) {
    mChannels[mCurrentChannel].mSounds[handle.GetIndex()]->SetTimeRemaining(duration);
  }
}

void CSfxManager::SetChannel(ESfxChannels channel) {
  if (channel == mCurrentChannel) {
    return;
  }
  if (channel == kSC_Default) {
    mAreaLowPassFilters.clear();
    mLowPassFilters.clear();
  }
  if (mCurrentChannel != kSC_Invalid) {
    TurnOffChannel(mCurrentChannel);
  }
  TurnOnChannel(channel);
  mCurrentChannel = channel;
}

CSfxManager::ESfxChannels CSfxManager::GetChannel() { return mCurrentChannel; }

void CSfxManager::TurnOffChannel(ESfxChannels channel) {
  CSfxChannel& sounds = mChannels[channel];
  for (int i = 0; i < sounds.mSounds.size(); ++i) {
    CBaseSfxWrapper* sound = sounds.mSounds[i];
    if (sound != nullptr) {
      if (sound->IsLooped()) {
        sound->UpdateEmitterSilent();
      } else {
        sound->Stop();
      }
    }
  }
  for (int i = 0; i < sounds.mSounds.size(); ++i) {
    CBaseSfxWrapper* sound = sounds.mSounds[i];
    if (sound != nullptr && !sound->IsLooped()) {
      sound->Release();
      sounds.mSounds[i] = nullptr;
    }
  }
}

void CSfxManager::TurnOnChannel(ESfxChannels channel) {
  mDoUpdate = true;
  mCurrentChannel = channel;
  CSfxChannel& sounds = mChannels[channel];
  for (int i = 0; i < sounds.mListeners.size(); ++i) {
    if (!sounds.mListeners[i].mActive) {
      continue;
    }
    for (int j = 0; j < sounds.mSounds.size(); ++j) {
      if (sounds.mSounds[j] != nullptr) {
        sounds.mSounds[j]->UpdateEmitter();
      }
    }
    break;
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
  // TODO: rank/expire voices, finish translation-table loading, update pitch ramps,
  // transform emitters between the four listeners, and update filters/auxiliary effects.
}

void CSfxManager::PitchBend(CSfxHandle handle, int pitch) {
  if (IsQueued(handle)) {
    mChannels[mCurrentChannel].mSounds[handle.GetIndex()]->SetPitchBend(pitch);
    mDoUpdate = true;
  }
}

bool CSfxManager::IsPlaying(CSfxHandle handle) {
  return IsQueued(handle) && mChannels[mCurrentChannel].mSounds[handle.GetIndex()]->IsPlaying();
}

bool CSfxManager::IsQueued(CSfxHandle handle) {
  if (!handle || handle.GetIndex() >= mChannels[mCurrentChannel].mSounds.size()) {
    return false;
  }
  CBaseSfxWrapper* sound = mChannels[mCurrentChannel].mSounds[handle.GetIndex()];
  return sound != nullptr && sound->GetSfxHandle() == handle;
}

int CSfxManager::GetRank(CBaseSfxWrapper* sound) {
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
  const CSfxChannel& channel = mChannels[mCurrentChannel];
  for (int i = 0; i < channel.mListeners.size(); ++i) {
    if (channel.mListeners[i].mActive) {
      const short audible = sound->GetAudible(channel.mListeners[i].mListener.mPosition);
      rank = audible == kSA_Aud0 ? 0 : rank + audible * 2;
    }
  }
  return rank;
}

bool CSfxManager::LoadTranslationTable(CSimplePool* pool, const SObjectTag* tag) {
  if (tag == nullptr) {
    return false;
  }
  delete mpTranslationTable;
  mpTranslationTable = nullptr;
  mpTranslationTableToken = rs_new CToken(pool->GetObj(*tag));
  mpTranslationTableToken->Lock();
  return true;
}

ushort CSfxManager::TranslateSFXID(ushort id) {
  if (mpTranslationTable != nullptr && id < mpTranslationTable->size()) {
    const short translated = (*mpTranslationTable)[id];
    if (translated >= 0) {
      return translated;
    }
  }
  return kInternalInvalidSfxId;
}

void CSfxManager::SetActiveAreas(const rstl::reserved_vector< int, 10 >& areas, int currentArea) {
  // TODO: reconcile auxiliary effects and per-area volumes, swap studios, then update
  // each sound's in-area flag. The auxiliary-effect record is not yet reconstructed.
}

CSfxManager::CSfxEmitterWrapper*
CSfxManager::AllocateCSfxEmitterWrapper(const CSfxEmitterWrapper& sound) {
  for (int i = 0; i < mEmitterWrapperPool.size(); ++i) {
    if (mEmitterWrapperPool[i].Available()) {
      mEmitterWrapperPool[i] = sound;
      return &mEmitterWrapperPool[i];
    }
  }
  if (mEmitterWrapperPool.size() == mEmitterWrapperPool.capacity()) {
    return nullptr;
  }
  mEmitterWrapperPool.push_back(sound);
  return &mEmitterWrapperPool.back();
}

CSfxManager::CSfxWrapper* CSfxManager::AllocateCSfxWrapper(const CSfxWrapper& sound) {
  for (int i = 0; i < mWrapperPool.size(); ++i) {
    if (mWrapperPool[i].Available()) {
      mWrapperPool[i] = sound;
      return &mWrapperPool[i];
    }
  }
  if (mWrapperPool.size() == mWrapperPool.capacity()) {
    return nullptr;
  }
  mWrapperPool.push_back(sound);
  return &mWrapperPool.back();
}

void CSfxManager::SetMuted(bool muted) {
  mMuted = muted;
  mDoUpdate = true;
  if (muted) {
    TurnOffChannel(mCurrentChannel);
    return;
  }
  CSfxChannel& channel = mChannels[mCurrentChannel];
  for (int i = 0; i < channel.mSounds.size(); ++i) {
    if (channel.mSounds[i] != nullptr) {
      channel.mSounds[i]->UpdateEmitter();
    }
  }
}

short CSfxManager::GetReverbAmount() { return 127; }

uchar CSfxManager::GetStudio(int area) {
  const uchar studios[] = {1, 2};
  return studios[area != kAllAreas && area != mCurrentArea ? !mCurrentStudio : mCurrentStudio];
}

int CSfxManager::AddLowPassAreaFilter(int frequency, float duration) {
  if (mAreaLowPassFilters.size() == mAreaLowPassFilters.capacity()) {
    return 0;
  }
  if (++mNextAreaFilterId == 0) {
    ++mNextAreaFilterId;
  }
  mAreaLowPassFilters.push_back(SLowPassFilter(frequency, duration, mNextAreaFilterId));
  return mNextAreaFilterId;
}

void CSfxManager::UpdateLowPassAreaFilters(float dt) {
  mAreaLowPassFrequency = 44100;
  bool haveFrequency = false;
  for (int i = 0; i < mAreaLowPassFilters.size();) {
    SLowPassFilter& filter = mAreaLowPassFilters[i];
    filter.mTimeRemaining -= dt;
    if (filter.mTimed && !(filter.mTimeRemaining > 0.f)) {
      mAreaLowPassFilters.erase(mAreaLowPassFilters.begin() + i);
      continue;
    }
    if (!haveFrequency || filter.mFrequency < mAreaLowPassFrequency) {
      mAreaLowPassFrequency = filter.mFrequency;
      haveFrequency = true;
    }
    ++i;
  }
}

void CSfxManager::RemoveLowPassAreaFilter(int id) {
  if (id == 0) {
    return;
  }
  for (int i = 0; i < mAreaLowPassFilters.size(); ++i) {
    if (mAreaLowPassFilters[i].mId == id) {
      mAreaLowPassFilters.erase(mAreaLowPassFilters.begin() + i);
      return;
    }
  }
}

bool CSfxManager::IsLowPassAreaFilterEnabled() {
  return !mAreaLowPassFilters.empty() && mCurrentChannel == kSC_Game;
}

int CSfxManager::GetLowPassAreaFrequency() { return mAreaLowPassFrequency; }

int CSfxManager::AddLowPassFilter(int frequency, float duration) {
  if (mLowPassFilters.size() == mLowPassFilters.capacity()) {
    return 0;
  }
  if (++mNextFilterId == 0) {
    ++mNextFilterId;
  }
  mLowPassFilters.push_back(SLowPassFilter(frequency, duration, mNextFilterId));
  return mNextFilterId;
}

void CSfxManager::UpdateLowPassFilters(float dt) {
  mLowPassFrequency = 44100;
  bool haveFrequency = false;
  for (int i = 0; i < mLowPassFilters.size();) {
    SLowPassFilter& filter = mLowPassFilters[i];
    filter.mTimeRemaining -= dt;
    if (filter.mTimed && !(filter.mTimeRemaining > 0.f)) {
      mLowPassFilters.erase(mLowPassFilters.begin() + i);
      continue;
    }
    if (!haveFrequency || filter.mFrequency < mLowPassFrequency) {
      mLowPassFrequency = filter.mFrequency;
      haveFrequency = true;
    }
    ++i;
  }
}

void CSfxManager::RemoveLowPassFilter(int id) {
  if (id == 0) {
    return;
  }
  for (int i = 0; i < mLowPassFilters.size(); ++i) {
    if (mLowPassFilters[i].mId == id) {
      mLowPassFilters.erase(mLowPassFilters.begin() + i);
      return;
    }
  }
}

bool CSfxManager::IsLowPassEnabled() {
  return !mLowPassFilters.empty() && mCurrentChannel == kSC_Game;
}

int CSfxManager::GetLowPassFrequency() { return mLowPassFrequency; }

bool CSfxManager::ShouldApplyLowPass(CBaseSfxWrapper* sound) {
  return (sound->GetArea() != kAllAreas && IsLowPassAreaFilterEnabled() &&
          !sound->GetIgnoreAreaLowPass()) ||
         (sound->UseAcoustics() && IsLowPassEnabled());
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
    for (int i = 0; i < mAreaVolumes.size(); ++i) {
      if (mAreaVolumes[i].mArea == area) {
        return mAreaVolumes[i].mVolume;
      }
    }
  }
  return 127;
}

void CSfxManager::SetAreaVolume(int area, uchar volume) {
  for (int i = 0; i < mAreaVolumes.size(); ++i) {
    if (mAreaVolumes[i].mArea == area) {
      mAreaVolumes[i].mVolume = volume;
      return;
    }
  }
  for (int i = 0; i < mAreaVolumes.size(); ++i) {
    if (mAreaVolumes[i].mArea == kAllAreas) {
      mAreaVolumes[i].mArea = area;
      mAreaVolumes[i].mVolume = volume;
      return;
    }
  }
}

void CSfxManager::SetIgnoreAreaLowPass(CSfxHandle handle, bool ignore) {
  if (IsQueued(handle)) {
    mChannels[mCurrentChannel].mSounds[handle.GetIndex()]->SetIgnoreAreaLowPass(ignore);
  }
}

CSfxManager::CSfxWrapper::~CSfxWrapper() {}

bool CSfxManager::CSfxWrapper::IsEmitter() const { return false; }

const CFactoryFnReturn FAudioTranslationTableFactory(const SObjectTag&, CInputStream& in,
                                                     const CVParamTransfer&) {
  return rs_new rstl::vector< short >(in);
}
