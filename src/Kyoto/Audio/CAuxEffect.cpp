#include "Kyoto/Audio/CAuxEffect.hpp"

CAuxEffect::CAuxEffect(const SND_AUX_REVERBHI& parameters, int area, uchar volume, int priority)
: mType(kT_ReverbHI)
, mArea(area)
, mId(0)
, mProcessingId(0)
, mVolume(volume)
, mPriority(priority)
, mActive(false)
, mRegistered(false)
, mPrepared(false) {
  mReverbHI = parameters;
}

CAuxEffect::CAuxEffect(const SND_AUX_CHORUS& parameters, int area, uchar volume, int priority)
: mType(kT_Chorus)
, mArea(area)
, mId(0)
, mProcessingId(0)
, mVolume(volume)
, mPriority(priority)
, mActive(false)
, mRegistered(false)
, mPrepared(false) {
  mChorus = parameters;
}

CAuxEffect::CAuxEffect(const SND_AUX_REVERBSTD& parameters, int area, uchar volume, int priority)
: mType(kT_ReverbSTD)
, mArea(area)
, mId(0)
, mProcessingId(0)
, mVolume(volume)
, mPriority(priority)
, mActive(false)
, mRegistered(false)
, mPrepared(false) {
  mReverbSTD = parameters;
}

CAuxEffect::CAuxEffect(const SND_AUX_DELAY& parameters, int area, uchar volume, int priority)
: mType(kT_Delay)
, mArea(area)
, mId(0)
, mProcessingId(0)
, mVolume(volume)
, mPriority(priority)
, mActive(false)
, mRegistered(false)
, mPrepared(false) {
  mDelay = parameters;
}

CAuxEffect::CAuxEffect(const SFlangerAuxParameters& parameters, int area, uchar volume,
                       int priority)
: mType(kT_Flanger)
, mArea(area)
, mId(0)
, mProcessingId(0)
, mVolume(volume)
, mPriority(priority)
, mActive(false)
, mRegistered(false)
, mPrepared(false) {
  mFlanger = parameters;
}

CAuxEffect::CAuxEffect(const SBitcrusherAuxParameters& parameters, int area, uchar volume,
                       int priority)
: mType(kT_Bitcrusher)
, mArea(area)
, mId(0)
, mProcessingId(0)
, mVolume(volume)
, mPriority(priority)
, mActive(false)
, mRegistered(false)
, mPrepared(false) {
  mBitcrusher = parameters;
}

CAuxEffect::CAuxEffect(const SPhaserAuxParameters& parameters, int area, uchar volume, int priority)
: mType(kT_Phaser)
, mArea(area)
, mId(0)
, mProcessingId(0)
, mVolume(volume)
, mPriority(priority)
, mActive(false)
, mRegistered(false)
, mPrepared(false) {
  mPhaser = parameters;
}

CAuxEffect::CAuxEffect(const SFilteredDelayAuxParameters& parameters, int area, uchar volume,
                       int priority)
: mType(kT_FilteredDelay)
, mArea(area)
, mId(0)
, mProcessingId(0)
, mVolume(volume)
, mPriority(priority)
, mActive(false)
, mRegistered(false)
, mPrepared(false) {
  mFilteredDelay = parameters;
}

void CAuxEffect::SetActive(bool active) { mActive = active; }

void CAuxEffect::Deactivate() { mActive = false; }

void CAuxEffect::SetBusIndex(uchar index) { mBusIndex = index; }

void CAuxEffect::Prepare() {
  switch (mType) {
  case kT_ReverbHI:
    sndAuxCallbackPrepareReverbHI(&mReverbHI);
    break;
  case kT_Chorus:
    sndAuxCallbackPrepareChorus(&mChorus);
    break;
  case kT_ReverbSTD:
    sndAuxCallbackPrepareReverbSTD(&mReverbSTD);
    break;
  case kT_Delay:
    sndAuxCallbackPrepareDelay(&mDelay);
    break;
  case kT_Flanger:
    PrepareFlangerAux(&mFlanger);
    break;
  case kT_Bitcrusher:
    PrepareBitcrusherAux(&mBitcrusher);
    break;
  case kT_Phaser:
    PreparePhaserAux(&mPhaser);
    break;
  case kT_FilteredDelay:
    PrepareFilteredDelayAux(&mFilteredDelay);
    break;
  }
  mPrepared = true;
}

void CAuxEffect::Shutdown() {
  if (mPrepared) {
    switch (mType) {
    case kT_ReverbHI:
      sndAuxCallbackShutdownReverbHI(&mReverbHI);
      break;
    case kT_Chorus:
      sndAuxCallbackShutdownChorus(&mChorus);
      break;
    case kT_ReverbSTD:
      sndAuxCallbackShutdownReverbSTD(&mReverbSTD);
      break;
    case kT_Delay:
      sndAuxCallbackShutdownDelay(&mDelay);
      break;
    case kT_Flanger:
      ShutdownFlangerAux(&mFlanger);
      break;
    case kT_Bitcrusher:
      ShutdownBitcrusherAux(&mBitcrusher);
      break;
    case kT_Phaser:
      ShutdownPhaserAux(&mPhaser);
      break;
    case kT_FilteredDelay:
      ShutdownFilteredDelayAux(&mFilteredDelay);
      break;
    }
    mPrepared = false;
  }
}

CAuxEffect& CAuxEffect::operator=(const CAuxEffect& other) {
  if (mPrepared) {
    Shutdown();
  }
  switch (other.mType) {
  case kT_ReverbHI:
    mReverbHI = other.mReverbHI;
    break;
  case kT_Chorus:
    mChorus = other.mChorus;
    break;
  case kT_ReverbSTD:
    mReverbSTD = other.mReverbSTD;
    break;
  case kT_Delay:
    mDelay = other.mDelay;
    break;
  case kT_Flanger:
    mFlanger = other.mFlanger;
    break;
  case kT_Bitcrusher:
    mBitcrusher = other.mBitcrusher;
    break;
  case kT_Phaser:
    mPhaser = other.mPhaser;
    break;
  case kT_FilteredDelay:
    mFilteredDelay = other.mFilteredDelay;
    break;
  }
  mType = other.mType;
  mArea = other.mArea;
  mId = other.mId;
  mPriority = other.mPriority;
  mVolume = other.mVolume;
  mBusIndex = other.mBusIndex;
  if (other.mPrepared) {
    Prepare();
  }
  return *this;
}

void CAuxEffect::Process(uchar reason, SND_AUX_INFO* info) {
  switch (mType) {
  case kT_ReverbHI:
    sndAuxCallbackReverbHI(reason, info, &mReverbHI);
    break;
  case kT_Chorus:
    sndAuxCallbackChorus(reason, info, &mChorus);
    break;
  case kT_ReverbSTD:
    sndAuxCallbackReverbSTD(reason, info, &mReverbSTD);
    break;
  case kT_Delay:
    sndAuxCallbackDelay(reason, info, &mDelay);
    break;
  case kT_Flanger:
  case kT_Bitcrusher:
  case kT_Phaser:
    ProcessCustomAux(reason, info, &mCustomProcessing);
    break;
  case kT_FilteredDelay:
    ProcessFilteredDelayAux(reason, info, &mFilteredDelay);
    break;
  }
}

bool CAuxEffect::IsActive() const { return mActive; }

int CAuxEffect::GetArea() const { return mArea; }

void CAuxEffect::Register() { mRegistered = true; }

void CAuxEffect::Unregister() {
  Deactivate();
  mRegistered = false;
}

bool CAuxEffect::IsRegistered() const { return mRegistered; }

int CAuxEffect::GetId() const { return mId; }

void CAuxEffect::SetId(int id) { mId = id; }

int CAuxEffect::GetPriority() const { return mPriority; }

uchar CAuxEffect::GetVolume() const { return mVolume; }

int CAuxEffect::GetProcessingId() const { return mProcessingId; }

void CAuxEffect::SetProcessingId(int id) { mProcessingId = id; }
