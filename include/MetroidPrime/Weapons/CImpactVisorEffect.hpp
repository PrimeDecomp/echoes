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
    rstl::optional_object< TLockedToken< CGenDescription > > mParticle;
    TSfxId mSound;
    bool mSendCollideMessage : 1;
  };

  // Guessed name
  struct SBlurEffect {
    int mType; // Camera blur mode; enum declaration is not recovered yet.
    float mAmount;
    float mFadeOutTime;
  };

  CImpactVisorEffect();

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
