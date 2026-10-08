#include "MetroidPrime/Cameras/CCameraShakerManager.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CRumbleManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "rstl/math.hpp"

CCameraShakerManager::SShaker::SShaker(int id, int playerIndex, const CCameraShakerData& data,
                                       bool playSound, bool useThresholdTimes)
: mTime(0.f)
, mId(id)
, mPlayerIndex(playerIndex)
, mData(data)
, mPlaySound(playSound)
, mSoundStarted(false)
, mUseThresholdTimes(useThresholdTimes) {}

CVector3f CCameraShakerManager::SShaker::GetTranslation(const CStateManager& mgr) {
  CVector3f translation = mData.GetPoint(mTime);
  if ((mData.GetFlags() & CCameraShakerData::kF_DistanceAttenuation) &&
      !CMath::IsEpsilon(mData.GetAttenuationDistance(), 0.f, 0.00001f)) {
    translation *= GetDistanceAttenuation(mgr);
  }
  return translation;
}

float CCameraShakerManager::SShaker::GetDistanceAttenuation(const CStateManager& mgr) const {
  const CVector3f& delta = mData.GetPosition() - mgr.GetPlayer(mPlayerIndex)->GetTranslation();
  return 1.f - CMath::Clamp(0.f, delta.Magnitude() / mData.GetAttenuationDistance(), 1.f);
}

CCameraShakerManager::CCameraShakerManager(int playerIndex)
: mTranslation(CVector3f::Zero())
, mNextId(0)
, mPlayerIndex(playerIndex)
, x83e_(0)
, mRumbleCooldown(0.f) {
  // The native constructor leaves the two packed rumble flags untouched.
}

CCameraShakerManager::~CCameraShakerManager() {}

void CCameraShakerManager::StartSound(SShaker& shaker) {
  const CCameraShakerData& data = shaker.mData;
  if (!shaker.mPlaySound || !(data.GetDuration() > 0.f)) {
    return;
  }
  if (shaker.mUseThresholdTimes && !(data.GetCachedMaxAmplitude() > 0.2f)) {
    return;
  }

  shaker.mSoundStarted = true;
  ushort volume = 100;
  if (data.GetFlags() & CCameraShakerData::kF_AmplitudeScaledVolume) {
    volume = static_cast< ushort >(
        CMath::Clamp(64.f, 63.f * (data.GetCachedMaxAmplitude() * 0.5f) + 64.f, 127.f));
  }

  const ushort sfxId = static_cast< ushort >(data.GetAudioEffect());
  if (sfxId == CSfxManager::kInternalInvalidSfxId) {
    return;
  }

  CSfxHandle handle;
  if ((data.GetFlags() & CCameraShakerData::kF_NonPositionalSound) == 0) {
    handle = CSfxManager::AddEmitter(sfxId, data.GetPosition(), static_cast< uchar >(volume),
                                     CSfxManager::kAllAreas, false, false,
                                     CSfxManager::kMedPriority);
  } else {
    handle = CSfxManager::SfxStart(sfxId, static_cast< uchar >(volume), 64);
  }
  CSfxManager::SetDuration(handle, data.GetLastThresholdTime() - data.GetFirstThresholdTime());
}

int CCameraShakerManager::AddCameraShaker(const CCameraShakerData& data, CStateManager& mgr,
                                          bool playSound, bool useThresholdTimes) {
  if (mShakers.size() == mShakers.capacity()) {
    return -1;
  }
  if (mgr.CameraManager(mPlayerIndex)->IsInCinematicCamera() &&
      !(data.GetFlags() & CCameraShakerData::kF_AllowCinematic)) {
    return -1;
  }

  CCameraShakerData shakeData = data;
  if (useThresholdTimes) {
    shakeData.UpdateThresholdTimes();
  }
  const SShaker shaker(mNextId++, mPlayerIndex, shakeData, playSound, useThresholdTimes);
  if (mPendingRumble != true) {
    mPendingRumble = true;
    mRumbleCooldown = 0.5f;
  }
  mShakers.push_back(shaker);
  return shaker.mId;
}

void CCameraShakerManager::SShaker::SetData(const CCameraShakerData& data) { mData = data; }

void CCameraShakerManager::UpdateCameraShaker(int id, const CCameraShakerData& data) {
  for (rstl::reserved_vector< SShaker, 8 >::iterator it = mShakers.begin(); it != mShakers.end();
       ++it) {
    if (id == it->mId) {
      it->SetData(data);
      return;
    }
  }
}

void CCameraShakerManager::RemoveCameraShaker(int id) {
  for (rstl::reserved_vector< SShaker, 8 >::iterator it = mShakers.begin(); it != mShakers.end();) {
    rstl::reserved_vector< SShaker, 8 >::iterator cur = it++;
    if (id == cur->mId) {
      mShakers.erase(cur);
      return;
    }
  }
}

void CCameraShakerManager::Reset() {
  mShakers.clear();
  mNextId = 0;
  mRumbleCooldown = 0.f;
}

void CCameraShakerManager::Update(float dt, CStateManager& mgr) {
  mTranslation = CVector3f::Zero();
  float rumbleIntensity = 0.f;
  rstl::reserved_vector< SShaker, 8 >::iterator it = mShakers.begin();
  while (it != mShakers.end()) {
    it->mTime += dt;
    if (it->mPlaySound && !it->mSoundStarted && it->mTime >= it->mData.GetFirstThresholdTime()) {
      StartSound(*it);
    }
    if (it->mTime >= it->mData.GetDuration()) {
      mShakers.erase(it);
    } else {
      mTranslation += it->GetTranslation(mgr);
      if (it->mData.GetFlags() & CCameraShakerData::kF_RumbleDistanceAttenuation) {
        rumbleIntensity += it->GetDistanceAttenuation(mgr);
      } else {
        rumbleIntensity += 1.f;
      }
      ++it;
    }
  }

  if ((mgr.GetPlayer(mPlayerIndex)->GetCameraState() != CPlayer::kCS_FirstPerson &&
       !mgr.GetCameraManager(mPlayerIndex)->IsInCinematicCamera()) ||
      mgr.GetCameraManager(mPlayerIndex)->ShouldBypassInterpolationCamera() ||
      mgr.GetPlayer(mPlayerIndex)->GetTurretState() != CPlayer::kTS_None) {
    mTranslation = CVector3f::Zero();
  }

  if (!mShakers.empty() && !mRumbling && mPendingRumble) {
    mgr.RumbleManager(mPlayerIndex)
        ->Rumble(mgr, kRFX_CameraShake, rstl::min_val(1.f, rumbleIntensity), kRP_Two);
    mRumbling = true;
  }
  if (mRumbleCooldown > 0.f) {
    mRumbleCooldown -= dt;
  } else if (mRumbling) {
    mRumbling = mPendingRumble = false;
  }
}

CVector3f CCameraShakerManager::GetTranslation(const CStateManager& mgr) const {
  float scale = 1.f;
  if (mgr.GetNumPlayers() == 2) {
    scale = 0.75f;
  }
  float x = scale * mTranslation.GetX();
  float y = scale * mTranslation.GetY();
  float z = scale * mTranslation.GetZ();
  x = CMath::Clamp(-1.f, x, 1.f);
  y = CMath::Clamp(-1.f, y, 0.25f);
  z = CMath::Clamp(-1.f, z, 1.f);
  return CVector3f(x, y, z);
}
