#include "MetroidPrime/ScriptObjects/CScriptSound.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSound.hpp"

bool CScriptSound::sFirstInFrame;

static int fn_8009dce8(int volume) {
  // TODO: scale volume with the music-volume tweak spline.
  return volume;
}

static CVector3f fn_8009dd94(const CStateManager& mgr, const rstl::vector< TUniqueId >& sources) {
  // TODO: find the closest point in the connected sound volumes to any player's listener.
  return CVector3f::Zero();
}

CScriptSound::CScriptSound(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                           const CTransform4f& xf, ushort soundId, float maxDist, float distComp,
                           float startDelay, short minVolume, short volume, short unknown198,
                           short darkVisorVolume, short priority, short pan, short surroundPan,
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
, mDarkVisorVolume(darkVisorVolume)
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
  // TODO: lifetime, moving emitters, throttled occlusion, pitch and visor-volume interpolation.
}

void CScriptSound::SetMaxVolume(short volume) {
  mVolume = volume;
  // TODO: update the live emitter/non-emitter handle.
}

CSfxHandle CScriptSound::GetSfxHandle() const { return mSfxHandle; }

bool CScriptSound::IsNonEmitter() const { return mNonEmitter; }

void CScriptSound::SetSoundModifierAttached(bool attached) { mSoundModifierAttached = attached; }

void CScriptSound::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CActor::AcceptScriptMsg(mgr, msg);
  switch (msg.GetMessage()) {
  case kSM_Activate:
    if (mAutoStart) {
      mPlayRequested = true;
    }
    break;
  case kSM_Deactivate:
    StopSound(mgr);
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
  case kSM_XCRT:
    if (GetActive() && mAutoStart) {
      mPlayRequested = true;
    }
    // TODO: manager generation flag controls self-free behavior.
    break;
  case kSM_XDelete:
    if (!mWorldSfx) {
      StopSound(mgr);
    }
    break;
  case kSM_XALD:
    // TODO: resolve connected sound-position sources.
    break;
  default:
    break;
  }
}

void CScriptSound::PlaySound(CStateManager& mgr, const CScriptMsg* msg) {
  // TODO: emitter/non-emitter setup, multiplayer panning and duplicate suppression.
}

void CScriptSound::StopSound(CStateManager& mgr) {
  mPlayRequested = false;
  // TODO: distinguish world-loop removal from CSfxManager handle shutdown.
}

float CScriptSound::GetOccludedVolumeAmount(const CVector3f& pos, const CStateManager& mgr) {
  // TODO: listener-relative ray grid and occlusion attenuation.
  return 1.f;
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
