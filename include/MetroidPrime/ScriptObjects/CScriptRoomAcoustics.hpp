#ifndef _CSCRIPTROOMACOUSTICS
#define _CSCRIPTROOMACOUSTICS

#include "MetroidPrime/CEntity.hpp"

class CScriptRoomAcoustics : public CEntity {
public:
  CScriptRoomAcoustics(
      TUniqueId uid, const rstl::string& name, const CEntityInfo& info, uint volumeScale,
      int priority, bool reverbHi, bool reverbHiDisable, float reverbHiTime, float reverbHiPreDelay,
      float reverbHiDamping, float reverbHiColoration, float reverbHiCrosstalk, float reverbHiMix,
      bool chorus, float chorusBaseDelay, float chorusVariation, float chorusPeriod, bool reverbStd,
      bool reverbStdDisable, float reverbStdTime, float reverbStdPreDelay, float reverbStdDamping,
      float reverbStdColoration, float reverbStdMix, bool delay, int delayL, int delayR, int delayS,
      int feedbackL, int feedbackR, int feedbackS, int outputL, int outputR, int outputS,
      bool flanger, float flangerDelay, float flangerDelayPhase, float flangerDry,
      float flangerFeedback, float flangerLFODepth, float flangerLFOFrequency, float flangerLFOWave,
      float flangerOut, bool bitcrusher, float bitcrusherValue, float bitcrusherGain,
      float bitcrusherBitDepth, float bitcrusherSampleRateReduction, bool phaser,
      float phaserFrequency, float phaserFeedback, float phaserInvert, float phaserMix,
      float phaserSweep);

  // CEntity
  ~CScriptRoomAcoustics() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  void EnableAuxCallbacks();
  void DisableAuxCallbacks();

private:
  uint mVolumeScale;
  int mAuxEffectId;
  int mPriority;
  bool mReverbHi;
  bool mReverbHiDisable;
  float mReverbHiTime;
  float mReverbHiPreDelay;
  float mReverbHiDamping;
  float mReverbHiColoration;
  float mReverbHiCrosstalk;
  float mReverbHiMix;
  bool mChorus;
  float mChorusBaseDelay;
  float mChorusVariation;
  float mChorusPeriod;
  bool mReverbStd;
  bool mReverbStdDisable;
  float mReverbStdTime;
  float mReverbStdPreDelay;
  float mReverbStdDamping;
  float mReverbStdColoration;
  float mReverbStdMix;
  bool mDelay;
  int mDelayL;
  int mDelayR;
  int mDelayS;
  int mFeedbackL;
  int mFeedbackR;
  int mFeedbackS;
  int mOutputL;
  int mOutputR;
  int mOutputS;
  bool mFlanger;
  float mFlangerDelay;
  float mFlangerDelayPhase;
  float mFlangerDry;
  float mFlangerFeedback;
  float mFlangerLFODepth;
  float mFlangerLFOFrequency;
  float mFlangerLFOWave;
  float mFlangerOut;
  bool mBitcrusher;
  float xC4_; // Verified property 0xf51a1d6a; runtime meaning unresolved.
  float mBitcrusherGain;
  float mBitcrusherBitDepth;
  float mBitcrusherSampleRateReduction;
  bool mPhaser;
  float mPhaserFrequency;
  float mPhaserFeedback;
  float mPhaserInvert;
  float mPhaserMix;
  float mPhaserSweep;
};

CHECK_SIZEOF(CScriptRoomAcoustics, 0xec)

#endif // _CSCRIPTROOMACOUSTICS
