#ifndef _CIMPACTVISOREFFECT
#define _CIMPACTVISOREFFECT

#include "Kyoto/TToken.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"

class CGenDescription;

class CImpactVisorEffect {
public:
  // Guessed name
  struct SParticleEffect {
    SParticleEffect(const rstl::optional_object< TLockedToken< CGenDescription > >& particle,
                    TSfxId sound, bool sendCollideMessage)
    : mParticle(particle), mSound(sound), mSendCollideMessage(sendCollideMessage) {}

    rstl::optional_object< TLockedToken< CGenDescription > > mParticle;
    TSfxId mSound;
    bool mSendCollideMessage : 1;
  };

  // Guessed name
  struct SBlurEffect {
    SBlurEffect(int type, float amount, float fadeOutTime)
    : mType(type), mAmount(amount), mFadeOutTime(fadeOutTime) {}

    int mType; // Camera blur mode; enum declaration is not recovered yet.
    float mAmount;
    float mFadeOutTime;
  };

  CImpactVisorEffect(const rstl::optional_object< SParticleEffect >& particleEffect,
                     const rstl::optional_object< SBlurEffect >& blurEffect,
                     const rstl::optional_object< rstl::pair< int, float > >& lowPassFilter)
  : mParticleEffect(particleEffect)
  , mBlurEffect(blurEffect)
  , mLowPassFilter(lowPassFilter)
  , mForcedVisor(CPlayerState::kPV_Invalid)
  , mForcedVisorDuration(0.f) {}

  // Guessed name.
  static CImpactVisorEffect
  ParticleEffect(const rstl::optional_object< TLockedToken< CGenDescription > >& particle,
                 TSfxId sound, bool sendCollideMessage) {
    return CImpactVisorEffect(rstl::optional_object< SParticleEffect >(
                                  SParticleEffect(particle, sound, sendCollideMessage)),
                              rstl::optional_object< SBlurEffect >(),
                              rstl::optional_object< rstl::pair< int, float > >());
  }

  // Guessed name
  static CImpactVisorEffect None() {
    return CImpactVisorEffect(rstl::optional_object_null(), rstl::optional_object_null(),
                              rstl::optional_object_null());
  }

  const rstl::optional_object< SParticleEffect >& GetParticleEffect() const {
    return mParticleEffect;
  }
  const rstl::optional_object< SBlurEffect >& GetBlurEffect() const { return mBlurEffect; }
  CPlayerState::EPlayerVisor GetForcedVisor() const { return mForcedVisor; }
  float GetForcedVisorDuration() const { return mForcedVisorDuration; }

  // Guessed name
  rstl::optional_object< rstl::pair< int, float > > GetLowPassFilter() const {
    return mLowPassFilter;
  }

private:
  rstl::optional_object< SParticleEffect > mParticleEffect;
  rstl::optional_object< SBlurEffect > mBlurEffect;
  rstl::optional_object< rstl::pair< int, float > > mLowPassFilter; // Frequency and duration.
  CPlayerState::EPlayerVisor mForcedVisor;
  float mForcedVisorDuration;
};
CHECK_SIZEOF(CImpactVisorEffect, 0x3c)

#endif // _CIMPACTVISOREFFECT
