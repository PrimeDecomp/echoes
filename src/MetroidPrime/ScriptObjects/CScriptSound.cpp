#include "MetroidPrime/ScriptObjects/CScriptSound.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSound.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTriggerOrientated.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakGame.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Math/CQuad.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include <float.h>

bool CScriptSound::sFirstInFrame;

static int ScaleByMusicVolume(int volume) {
  const float musicVolume = float(int(gpGameState->GameOptions().GetMusicVolume()));
  CMayaSpline& volumeCurve = gpTweakGame->GetMusicVolumeSpline();
  const float musicScale = volumeCurve.EvaluateAt(musicVolume);
  return CCast::FtoS(float(volume * musicScale) / 127.f);
}

static CVector3f GetClosestSoundPosition(const CStateManager& mgr,
                                         const rstl::vector< TUniqueId >& sources) {
  CVector3f nearest = CVector3f::Zero();
  float nearestDistSq = FLT_MAX;
  for (rstl::vector< TUniqueId >::const_iterator it = sources.begin(); it != sources.end(); ++it) {
    const CEntity* entity = mgr.GetObjectById(*it);
    if (!entity) {
      continue;
    }

    rstl::reserved_vector< CPlane, 6 > planes;
    if (const CScriptTriggerOrientated* oriented =
            TCastToConstPtr< CScriptTriggerOrientated >(entity)) {
      for (int face = 0; face < 6; ++face) {
        const CQuad quad = oriented->GetOBBox().GetQuad(CAABox::EBoxFaceId(face));
        const CPlane plane = quad.GetTri(0).GetPlane();
        planes.push_back(plane);
      }
    } else if (const CActor* actor = TCastToConstPtr< CActor >(entity)) {
      const rstl::optional_object< CAABox > bounds = actor->GetTouchBounds();
      if (bounds) {
        for (int face = 0; face < 6; ++face) {
          const CQuad quad = bounds->GetQuad(CAABox::EBoxFaceId(face));
          const CPlane plane = quad.GetTri(0).GetPlane();
          planes.push_back(plane);
        }
      }
    }
    if (planes.empty()) {
      continue;
    }

    for (uint player = 0; player < uint(mgr.GetNumPlayers()); ++player) {
      const CVector3f listener =
          mgr.GetCameraManager(player)->GetCurrentCameraTransform(mgr, true).GetTranslation();
      CVector3f candidate = listener;
      for (rstl::reserved_vector< CPlane, 6 >::const_iterator plane = planes.begin();
           plane != planes.end(); ++plane) {
        const float dot = CVector3f::Dot(plane->GetNormal(), candidate);
        if (dot < plane->GetConstant()) {
          candidate -= (dot - plane->GetConstant()) * plane->GetNormal();
        }
      }
      const float distSq = (listener - candidate).MagSquared();
      if (distSq < nearestDistSq) {
        nearestDistSq = distSq;
        nearest = candidate;
      }
    }
  }
  return nearest;
}

CScriptSound::CScriptSound(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                           const CTransform4f& xf, ushort soundId, float maxDist, float distComp,
                           float startDelay, short minVolume, short volume, short unknown198,
                           short echoVisorVolume, short priority, short pan, short surroundPan,
                           short unknown1a2, bool looped, bool nonEmitter, bool playerRelativePan,
                           bool autoStart, bool occlusionTest, bool acoustics, bool worldSfx,
                           bool allowDuplicates, bool allAreas, bool scaleByMusicVolume, int pitch)
: CActor(uid, name, info, 0, xf, CModelData(), CMaterialList(kMT_Trigger), CActorParameters::None(),
         kInvalidUniqueId)
, mOcclusionUpdateTimer(0.f)
, mSfxHandle()
, mMaxVolume(0)
, mCurrentMaxVolume(0)
, mVolumeDelta(0)
, mEmitterPosition(xf.GetTranslation())
, mStartDelay(startDelay)
, mSoundId(soundId)
, mMaxDistance(maxDist)
, mDistanceCompensation(distComp)
, mMinVolume(minVolume)
, mVolume(volume)
, x198_(unknown198)
, mEchoVisorVolume(echoVisorVolume)
, mPriority(priority)
, mPan(pan)
, mSurroundPan(surroundPan)
, x1a2_(unknown1a2)
, mPitch(pitch + 8192)
, mPlayRequested(false)
, mLooped(looped)
, mNonEmitter(nonEmitter)
, mAutoStart(autoStart)
, mOcclusionTest(occlusionTest)
, mAcoustics(acoustics)
, mWorldSfx(worldSfx)
, mSelfFree(false)
, mAllowDuplicates(allowDuplicates)
, mProcessedThisFrame(false)
, mPlayerRelativePan(playerRelativePan)
, mSoundModifierAttached(false)
, mAllAreas(allAreas)
, mScaleByMusicVolume(scaleByMusicVolume) {
  if (mWorldSfx && !mNonEmitter) {
    mWorldSfx = false;
  }
}

void CScriptSound::PreThink(float dt, CStateManager& mgr) {
  CEntity::PreThink(dt, mgr);
  sFirstInFrame = true;
  mProcessedThisFrame = false;
}

CScriptSound::~CScriptSound() {}

void CScriptSound::Think(float dt, CStateManager& mgr) {
  if (mSelfFree && (!GetActive() || mLooped || !mAutoStart)) {
    mgr.DeleteObjectRequest(GetUniqueId());
    return;
  }
  if (!GetActive()) {
    return;
  }

  if (!mLooped && mAutoStart && !mPlayRequested && mSfxHandle &&
      !CSfxManager::IsPlaying(mSfxHandle) && !CSfxManager::IsQueued(mSfxHandle)) {
    mgr.DeleteObjectRequest(GetUniqueId());
  }

  if (mSfxHandle && !mNonEmitter) {
    if (!mPositionSources.empty()) {
      SetTranslation(GetClosestSoundPosition(mgr, mPositionSources));
    }
    const CVector3f& position = GetTranslation();
    if (!close_enough(mEmitterPosition, position)) {
      CSfxManager::UpdateEmitter(mSfxHandle, position, CVector3f::Zero(), uchar(mCurrentMaxVolume));
      mEmitterPosition = position;
    }
  }

  if (mSfxHandle && !mNonEmitter && mOcclusionTest) {
    if (mOcclusionUpdateTimer <= 0.f && sFirstInFrame) {
      sFirstInFrame = false;
      short newMax = CCast::FtoUS(float(mVolume) * GetOccludedVolumeAmount(GetTranslation(), mgr));
      if (newMax < mMinVolume) {
        newMax = mMinVolume;
      }
      if (mMaxVolume != newMax) {
        mMaxVolume = newMax;
        mVolumeDelta = short((mMaxVolume - mCurrentMaxVolume) / 30);
        if (mVolumeDelta == 0) {
          if (mCurrentMaxVolume < mMaxVolume) {
            mVolumeDelta = 1;
          } else {
            mVolumeDelta = -1;
          }
        }
      }
      mOcclusionUpdateTimer = 0.5f;
    } else {
      mOcclusionUpdateTimer -= dt;
    }

    if (mCurrentMaxVolume != mMaxVolume) {
      mCurrentMaxVolume += mVolumeDelta;
      if (mVolumeDelta > 0 && mCurrentMaxVolume > mMaxVolume) {
        mCurrentMaxVolume = mMaxVolume;
      }
      if (mVolumeDelta < 0 && mCurrentMaxVolume < mMaxVolume) {
        mCurrentMaxVolume = mMaxVolume;
      }
      CSfxManager::UpdateEmitter(mSfxHandle, GetTranslation(), CVector3f::Zero(),
                                 uchar(mCurrentMaxVolume));
    }
  }

  if (mPlayRequested) {
    mStartDelay -= dt;
    if (mStartDelay <= 0.f) {
      mPlayRequested = false;
      PlaySound(mgr, nullptr);
    }
  }

  if (!mSoundModifierAttached) {
    if (mPitch != 8192 && mSfxHandle) {
      CSfxManager::PitchBend(mSfxHandle, mPitch);
    }
    if (mSfxHandle && mSurroundPan != 0) {
      CSfxManager::SfxSpan(mSfxHandle, uchar(mSurroundPan));
    }
    if (mEchoVisorVolume && mSfxHandle && !mgr.IsMultiplayer()) {
      const short desired = mgr.GetPlayerState(0)->GetCurrentVisor() == CPlayerState::kPV_Echo
                                ? mEchoVisorVolume
                                : mVolume;
      if (desired != mCurrentMaxVolume) {
        const short low = mEchoVisorVolume < mVolume ? mEchoVisorVolume : mVolume;
        const short high = mEchoVisorVolume > mVolume ? mEchoVisorVolume : mVolume;
        int step = (high - low) / 30;
        if (step < 1) {
          step = 1;
        }
        if (desired < mCurrentMaxVolume) {
          step = -step;
        }
        short next = mCurrentMaxVolume + step;
        if (next < low) {
          next = low;
        } else if (next > high) {
          next = high;
        }
        mCurrentMaxVolume = next;
        if (!mNonEmitter) {
          CSfxManager::UpdateEmitter(mSfxHandle, GetTranslation(), CVector3f::Zero(),
                                     uchar(mCurrentMaxVolume));
          mMaxVolume = mCurrentMaxVolume;
        } else {
          CSfxManager::SfxVolume(mSfxHandle, uchar(mCurrentMaxVolume));
        }
      }
    }
  }
  if (mScaleByMusicVolume) {
    const uchar volume = ScaleByMusicVolume(mCurrentMaxVolume);
    CSfxManager::SfxVolume(mSfxHandle, volume);
  }
}

void CScriptSound::SetMaxVolume(short volume) {
  mVolume = volume;
  if (!mSfxHandle) {
    return;
  }

  if (mNonEmitter) {
    CSfxManager::SfxVolume(mSfxHandle,
                           uchar(mScaleByMusicVolume ? ScaleByMusicVolume(volume) : volume));
  } else {
    const CVector3f position = GetTranslation();
    mCurrentMaxVolume = mVolume;
    mMaxVolume = mCurrentMaxVolume;
    CSfxManager::UpdateEmitter(mSfxHandle, position, CVector3f::Zero(), uchar(volume));
  }
}

CSfxHandle CScriptSound::GetSfxHandle() const { return mSfxHandle; }

bool CScriptSound::IsNonEmitter() const { return mNonEmitter; }

void CScriptSound::SetSoundModifierAttached(bool attached) { mSoundModifierAttached = attached; }

void CScriptSound::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CActor::AcceptScriptMsg(mgr, msg);
  switch (message) {
  case kSM_Create:
    if (GetActive() && mAutoStart) {
      mPlayRequested = true;
    }
    mSelfFree = mgr.GetScriptObjectLoaderHelper().IsGeneratingObject();
    break;
  case kSM_AreaLoaded:
    for (rstl::vector< SConnection >::const_iterator it = GetConnectionList().begin();
         it != GetConnectionList().end(); ++it) {
      if (it->state == kSS_Connect) {
        if (mPositionSources.size() == mPositionSources.capacity()) {
          mPositionSources.reserve(mPositionSources.size() + 1);
        }
        mPositionSources.push_back_unsafe(mgr.GetIdForScript(it->objId));
      }
    }
    break;
  case kSM_Play:
    if (GetActive()) {
      PlaySound(mgr, &msg);
    }
    break;
  case kSM_Stop:
    if (GetActive()) {
      StopSound(mgr);
    }
    break;
  case kSM_Deactivate:
    StopSound(mgr);
    break;
  case kSM_Activate:
    if (mAutoStart) {
      mPlayRequested = true;
    }
    break;
  case kSM_Delete:
    if (!mWorldSfx) {
      StopSound(mgr);
    }
    break;
  default:
    break;
  }
}

void CScriptSound::PlaySound(CStateManager& mgr, const CScriptMsg* msg) {
  int areaId = mAllAreas ? CSfxManager::kAllAreas : GetCurrentAreaId().Value();
  if ((!mAllowDuplicates && mSfxHandle && CSfxManager::IsQueued(mSfxHandle)) ||
      mProcessedThisFrame) {
    return;
  }
  mProcessedThisFrame = true;

  short volume = mVolume;
  if (mEchoVisorVolume && !mgr.IsMultiplayer()) {
    volume = mgr.GetPlayerState(0)->GetCurrentVisor() == CPlayerState::kPV_Echo ? mEchoVisorVolume
                                                                                : mVolume;
  }

  if (mNonEmitter) {
    CWorld* world = mgr.World();
    if (!mWorldSfx || !world->HasGlobalSound(mSoundId)) {
      short pan = mPan;
      if (mPlayerRelativePan && !mWorldSfx && msg && mgr.GetNumPlayers() > 2) {
        const CEntity* originator = mgr.GetObjectById(msg->GetOriginator());
        const CPlayer* player = TCastToConstPtr< CPlayer >(originator);
        if (!player) {
          if (const CGameCamera* camera = TCastToConstPtr< CGameCamera >(originator)) {
            player = &camera->GetPlayer(mgr);
          }
        }
        if (player) {
          pan = CCast::FtoS(63.f * (float(mPan) / 127.f - 0.5f) +
                            float(player->GetSoundPan(CPlayer::kMSP_4)));
        }
      }
      mCurrentMaxVolume = volume;
      const short startVolume = mScaleByMusicVolume ? ScaleByMusicVolume(volume) : volume;
      if (mWorldSfx) {
        areaId = CSfxManager::kAllAreas;
      }
      mSfxHandle =
          CSfxManager::SfxStart(mSoundId, startVolume, pan, areaId, mAcoustics, mLooped, mPriority);
      if (mWorldSfx) {
        world->AddGlobalSound(mSoundId, mSfxHandle);
      }
    }
  } else {
    const float occlusion = mOcclusionTest ? GetOccludedVolumeAmount(GetTranslation(), mgr) : 1.f;
    mMaxVolume = CCast::FtoUS(float(volume) * occlusion);
    mCurrentMaxVolume = mMaxVolume;
    CAudioSys::C3DEmitterParmData data(mMaxDistance, mDistanceCompensation, 1, uchar(mMaxVolume),
                                       uchar(mMinVolume));
    data.mPos = GetTranslation();
    data.mSfxId = mSoundId;
    if (mLooped) {
      mSfxHandle = CSfxManager::AddEmitter(data, areaId, mAcoustics, true, mPriority);
    } else {
      mSfxHandle = CSfxManager::AddEmitter(data, areaId, mAcoustics, false, mPriority);
    }
  }
}

void CScriptSound::StopSound(CStateManager& mgr) {
  mPlayRequested = false;
  if (mWorldSfx && mNonEmitter) {
    mgr.World()->StopGlobalSound(mSoundId);
    mSfxHandle.Clear();
  } else if (mSfxHandle) {
    CSfxManager::RemoveEmitter(mSfxHandle);
    mSfxHandle.Clear();
  }
}

float CScriptSound::GetOccludedVolumeAmount(const CVector3f& pos, const CStateManager& mgr) {
  if (mgr.IsMultiplayer()) {
    return 1.f;
  }

  const CTransform4f cameraXf = mgr.GetCameraManager(0)->GetCurrentCameraTransform(mgr, true);
  const CVector3f soundToCamera = cameraXf.GetTranslation() - pos;
  const float distance = soundToCamera.Magnitude();
  const CVector3f direction = soundToCamera * (1.f / distance);
  const CVector3f side = CVector3f::Up() - direction * CVector3f::Dot(CVector3f::Up(), direction);
  const CVector3f cross = CVector3f::Cross(direction, side);
  static const float influence = 3.f / distance;
  static const float increment = influence;
  static const CMaterialFilter solidFilter = CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Solid), CMaterialList(kMT_ProjectilePassthrough));

  int total = 0;
  int unobstructed = 0;
  for (float i = -influence; i <= influence; i += increment) {
    for (float j = -influence; j <= influence; j += increment) {
      ++total;
      const CVector3f ray = (direction + i * side) + j * cross;
      if (!mgr.RayStaticIntersection(pos, ray.AsNormalized(), distance, solidFilter).IsValid()) {
        ++unobstructed;
      }
    }
  }
  return 0.42f * (float(unobstructed) / float(total)) + 0.58f;
}

CEntity* LoadSound(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrSound sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrSound.inc"
  if (sldrThis.sound < 0) {
    return nullptr;
  }

  return rs_new CScriptSound(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      static_cast< ushort >(sldrThis.sound), sldrThis.maxAudibleDistance, sldrThis.dropOff,
      sldrThis.delayTime, static_cast< short >(sldrThis.minVolume),
      static_cast< short >(sldrThis.maxVolume), 0,
      static_cast< short >(sldrThis.echoVisorMaxVolume), static_cast< short >(sldrThis.priority),
      static_cast< short >(63.f * (1.f + sldrThis.surroundPan.pan)),
      static_cast< short >(63.f * (1.f - sldrThis.surroundPan.surroundPan)), 0, sldrThis.loop,
      sldrThis.ambient, sldrThis.viewportDependent, sldrThis.autoStart, sldrThis.canOcclude,
      sldrThis.useRoomAcoustics, sldrThis.persistent, sldrThis.playAlways, sldrThis.allArea,
      sldrThis.soundIsMusic, sldrThis.pitch);
}
