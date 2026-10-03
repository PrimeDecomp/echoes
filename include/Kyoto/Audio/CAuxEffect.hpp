#ifndef _CAUXEFFECT
#define _CAUXEFFECT

#include "Kyoto/Audio/CAuxEffectParameters.hpp"
#include <musyx/musyx.h>

// Guessed class and interface names, based on auxiliary-effect registration and processing.
class CAuxEffect {
public:
  enum EType {
    kT_Invalid = -1,
    kT_ReverbHI,
    kT_Chorus,
    kT_ReverbSTD,
    kT_Delay,
    kT_Flanger,
    kT_Bitcrusher,
    kT_Phaser,
    kT_FilteredDelay,
  };

  CAuxEffect(const SND_AUX_REVERBHI& parameters, int area, uchar volume, int priority);
  CAuxEffect(const SND_AUX_CHORUS& parameters, int area, uchar volume, int priority);
  CAuxEffect(const SND_AUX_REVERBSTD& parameters, int area, uchar volume, int priority);
  CAuxEffect(const SND_AUX_DELAY& parameters, int area, uchar volume, int priority);
  CAuxEffect(const SFlangerAuxParameters& parameters, int area, uchar volume, int priority);
  CAuxEffect(const SBitcrusherAuxParameters& parameters, int area, uchar volume, int priority);
  CAuxEffect(const SPhaserAuxParameters& parameters, int area, uchar volume, int priority);
  CAuxEffect(const SFilteredDelayAuxParameters& parameters, int area, uchar volume, int priority);

  CAuxEffect& operator=(const CAuxEffect& other);
  void SetActive(bool active);
  void Deactivate();
  void SetBusIndex(uchar index);
  void Prepare();
  void Shutdown();
  void Process(uchar reason, SND_AUX_INFO* info);
  bool IsActive() const;
  int GetArea() const;
  void Register();
  void Unregister();
  bool IsRegistered() const;
  int GetId() const;
  void SetId(int id);
  int GetPriority() const;
  uchar GetVolume() const;
  int GetProcessingId() const;
  void SetProcessingId(int id);

private:
  // Guessed member names, established by constructors, registration and callback consumers.
  EType mType;
  int mArea;
  int mId;
  int mProcessingId;
  uchar mVolume;
  uchar mBusIndex;
  int mPriority : 4;
  bool mActive : 1;
  bool mRegistered : 1;
  bool mPrepared : 1;
  union {
    SAuxEffectProcessingState mCustomProcessing;
    SND_AUX_REVERBHI mReverbHI;
    SND_AUX_CHORUS mChorus;
    SND_AUX_REVERBSTD mReverbSTD;
    SND_AUX_DELAY mDelay;
    SFlangerAuxParameters mFlanger;
    SBitcrusherAuxParameters mBitcrusher;
    SPhaserAuxParameters mPhaser;
    SFilteredDelayAuxParameters mFilteredDelay;
  };
};
CHECK_SIZEOF(CAuxEffect, 0x1f4)

#endif // _CAUXEFFECT
