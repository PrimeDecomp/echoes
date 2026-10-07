#include "MetroidPrime/ScriptObjects/CScriptRoomAcoustics.hpp"

#include "Kyoto/Audio/CAuxEffectParameters.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrRoomAcoustics.hpp"

CScriptRoomAcoustics::CScriptRoomAcoustics(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info, uint volumeScale,
    int priority, bool reverbHi, bool reverbHiDisable, float reverbHiTime, float reverbHiPreDelay,
    float reverbHiDamping, float reverbHiColoration, float reverbHiCrosstalk, float reverbHiMix,
    bool chorus, float chorusBaseDelay, float chorusVariation, float chorusPeriod, bool reverbStd,
    bool reverbStdDisable, float reverbStdTime, float reverbStdPreDelay, float reverbStdDamping,
    float reverbStdColoration, float reverbStdMix, bool delay, int delayL, int delayR, int delayS,
    int feedbackL, int feedbackR, int feedbackS, int outputL, int outputR, int outputS,
    bool flanger, float flangerDelay, float flangerDelayPhase, float flangerDry,
    float flangerFeedback, float flangerLFODepth, float flangerLFOFrequency, float flangerLFOWave,
    float flangerOut, bool bitcrusher, float bitcrusherDistortionType, float bitcrusherGain,
    float bitcrusherBitDepth, float bitcrusherSampleRateReduction, bool phaser,
    float phaserFrequency, float phaserFeedback, float phaserInvert, float phaserMix,
    float phaserSweep)
: CEntity(uid, info, name, 0)
, mVolumeScale(volumeScale)
, mAuxEffectId(0)
, mPriority(priority)
, mReverbHi(reverbHi)
, mReverbHiDisable(reverbHiDisable)
, mReverbHiTime(reverbHiTime)
, mReverbHiPreDelay(reverbHiPreDelay)
, mReverbHiDamping(reverbHiDamping)
, mReverbHiColoration(reverbHiColoration)
, mReverbHiCrosstalk(reverbHiCrosstalk)
, mReverbHiMix(reverbHiMix)
, mChorus(chorus)
, mChorusBaseDelay(chorusBaseDelay)
, mChorusVariation(chorusVariation)
, mChorusPeriod(chorusPeriod)
, mReverbStd(reverbStd)
, mReverbStdDisable(reverbStdDisable)
, mReverbStdTime(reverbStdTime)
, mReverbStdPreDelay(reverbStdPreDelay)
, mReverbStdDamping(reverbStdDamping)
, mReverbStdColoration(reverbStdColoration)
, mReverbStdMix(reverbStdMix)
, mDelay(delay)
, mDelayL(delayL)
, mDelayR(delayR)
, mDelayS(delayS)
, mFeedbackL(feedbackL)
, mFeedbackR(feedbackR)
, mFeedbackS(feedbackS)
, mOutputL(outputL)
, mOutputR(outputR)
, mOutputS(outputS)
, mFlanger(flanger)
, mFlangerDelay(flangerDelay)
, mFlangerDelayPhase(flangerDelayPhase)
, mFlangerDry(flangerDry)
, mFlangerFeedback(flangerFeedback)
, mFlangerLFODepth(flangerLFODepth)
, mFlangerLFOFrequency(flangerLFOFrequency)
, mFlangerLFOWave(flangerLFOWave)
, mFlangerOut(flangerOut)
, mBitcrusher(bitcrusher)
, mBitcrusherDistortionType(bitcrusherDistortionType)
, mBitcrusherGain(bitcrusherGain)
, mBitcrusherBitDepth(bitcrusherBitDepth)
, mBitcrusherSampleRateReduction(bitcrusherSampleRateReduction)
, mPhaser(phaser)
, mPhaserFrequency(phaserFrequency)
, mPhaserFeedback(phaserFeedback)
, mPhaserInvert(phaserInvert)
, mPhaserMix(phaserMix)
, mPhaserSweep(phaserSweep) {}

void CScriptRoomAcoustics::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CEntity::AcceptScriptMsg(mgr, msg);

  switch (message) {
  case kSM_Activate:
  case kSM_Create:
    if (GetActive()) {
      EnableAuxCallbacks();
    }
    break;
  case kSM_Deactivate:
  case kSM_Delete:
    DisableAuxCallbacks();
    break;
  default:
    break;
  }
}

void CScriptRoomAcoustics::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
}

void CScriptRoomAcoustics::EnableAuxCallbacks() {
  if (mAuxEffectId == 0 && GetActive()) {
    int applied = 0;
    if (mReverbHi && applied <= 0) {
      SND_AUX_REVERBHI reverb;
      reverb.tempDisableFX = mReverbHiDisable;
      reverb.time = mReverbHiTime;
      reverb.preDelay = mReverbHiPreDelay;
      reverb.damping = mReverbHiDamping;
      reverb.coloration = mReverbHiColoration;
      reverb.crosstalk = mReverbHiCrosstalk;
      reverb.mix = mReverbHiMix;
      ++applied;
      mAuxEffectId =
          CSfxManager::AddAuxEffect(GetCurrentAreaId().Value(), reverb, mVolumeScale, mPriority);
    }
    if (mChorus && applied < 1) {
      SND_AUX_CHORUS chorus;
      chorus.baseDelay = mChorusBaseDelay;
      chorus.variation = mChorusVariation;
      chorus.period = mChorusPeriod;
      ++applied;
      mAuxEffectId =
          CSfxManager::AddAuxEffect(GetCurrentAreaId().Value(), chorus, mVolumeScale, mPriority);
    }
    if (mReverbStd && applied < 1) {
      SND_AUX_REVERBSTD reverb;
      reverb.tempDisableFX = mReverbStdDisable;
      reverb.time = mReverbStdTime;
      reverb.preDelay = mReverbStdPreDelay;
      reverb.damping = mReverbStdDamping;
      reverb.coloration = mReverbStdColoration;
      reverb.mix = mReverbStdMix;
      ++applied;
      mAuxEffectId =
          CSfxManager::AddAuxEffect(GetCurrentAreaId().Value(), reverb, mVolumeScale, mPriority);
    }
    if (mDelay && applied < 1) {
      SND_AUX_DELAY delay;
      delay.delay[0] = mDelayL;
      delay.delay[1] = mDelayR;
      delay.delay[2] = mDelayS;
      delay.feedback[0] = mFeedbackL;
      delay.feedback[1] = mFeedbackR;
      delay.feedback[2] = mFeedbackS;
      delay.output[0] = mOutputL;
      delay.output[1] = mOutputR;
      delay.output[2] = mOutputS;
      ++applied;
      mAuxEffectId =
          CSfxManager::AddAuxEffect(GetCurrentAreaId().Value(), delay, mVolumeScale, mPriority);
    }
    if (mFlanger && applied < 1) {
      SFlangerAuxParameters flanger;
      flanger.mProcessing.mProcessReplacing = true;
      flanger.mDelay = mFlangerDelay;
      flanger.mFeedback = mFlangerFeedback;
      flanger.mOut = mFlangerOut;
      flanger.mLFOFrequency = mFlangerLFOFrequency;
      flanger.mLFODepth = mFlangerLFODepth;
      flanger.mLFOWave = mFlangerLFOWave;
      flanger.mDry = mFlangerDry;
      flanger.mDelayPhase = mFlangerDelayPhase;
      ++applied;
      mAuxEffectId =
          CSfxManager::AddAuxEffect(GetCurrentAreaId().Value(), flanger, mVolumeScale, mPriority);
    }
    if (mBitcrusher && applied < 1) {
      SBitcrusherAuxParameters bitcrusher;
      bitcrusher.mProcessing.mProcessReplacing = true;
      bitcrusher.mDistortionType = mBitcrusherDistortionType;
      bitcrusher.mGain = mBitcrusherGain;
      bitcrusher.mBitDepth = mBitcrusherBitDepth;
      bitcrusher.mSampleRateReduction = mBitcrusherSampleRateReduction;
      mAuxEffectId = CSfxManager::AddAuxEffect(GetCurrentAreaId().Value(), bitcrusher, mVolumeScale,
                                               mPriority);
      // Native code does not set applied here; a phaser may also be registered.
    }
    if (mPhaser && applied < 1) {
      SPhaserAuxParameters phaser;
      phaser.mProcessing.mProcessReplacing = false;
      phaser.mFrequency = mPhaserFrequency;
      phaser.mFeedback = mPhaserFeedback;
      phaser.mInvert = mPhaserInvert;
      phaser.mWet = mPhaserMix;
      phaser.mDry = 1.f - mPhaserMix;
      phaser.mSweep = mPhaserSweep;
      mAuxEffectId =
          CSfxManager::AddAuxEffect(GetCurrentAreaId().Value(), phaser, mVolumeScale, mPriority);
    }
  }
}

void CScriptRoomAcoustics::DisableAuxCallbacks() {
  if (mAuxEffectId != 0) {
    CSfxManager::RemoveAuxEffect(mAuxEffectId);
    mAuxEffectId = 0;
  }
}

CEntity* LoadRoomAcoustics(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrRoomAcoustics sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrRoomAcoustics.inc"

  return rs_new CScriptRoomAcoustics(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), sldrThis.roomVolume, sldrThis.priority,
      sldrThis.reverbHiEnabled, sldrThis.unknown_0x3263c26e, sldrThis.reverbHiTime,
      sldrThis.reverbHiPreDelay, sldrThis.reverbHiDamping, sldrThis.reverbHiColoration,
      sldrThis.reverbHiCrossTalk, sldrThis.reverbHiMix, sldrThis.chorusEnabled,
      sldrThis.chorusBaseDelay, sldrThis.chorusVariation, sldrThis.chorusPeriod,
      sldrThis.reverbStdEnabled, sldrThis.unknown_0x4a5bbf90, sldrThis.reverbStdTime,
      sldrThis.reverbStdPreDelay, sldrThis.reverbStdDamping, sldrThis.reverbStdColoration,
      sldrThis.reverbStdMix, sldrThis.delayEnabled, sldrThis.delay0, sldrThis.delay1,
      sldrThis.delay2, sldrThis.delayFeedback0, sldrThis.delayFeedback1, sldrThis.delayFeedback2,
      sldrThis.delayOutput0, sldrThis.delayOutput1, sldrThis.delayOutput2, sldrThis.flangerEnabled,
      sldrThis.flangerDelay, sldrThis.flangerDelayPhase, sldrThis.flangerDry,
      sldrThis.flangerFeedback, sldrThis.flangerLFODepth, sldrThis.flangerLFOFrequency,
      sldrThis.flangerLFOWave, sldrThis.flangerOut, sldrThis.bitcrusherEnabled,
      CCast::LtoF(sldrThis.unknown_0xf51a1d6a), sldrThis.bitcrusherGain / 24.5f,
      CCast::LtoF(sldrThis.bitcrusherBitDepth) / 24.f, sldrThis.bitcrusherSampleRateReduction / 40.f,
      sldrThis.phaserEnabled, sldrThis.phaserFrequency / 4.f, sldrThis.phaserFeedback / 0.99f,
      (1.f + sldrThis.phaserInvert) / 2.f, sldrThis.phaserMix,
      (sldrThis.phaserSweep - 200.f) / 7800.f);
}
