#ifndef _CPOWERBOMBGUARDIANSTAGEDATA
#define _CPOWERBOMBGUARDIANSTAGEDATA

#include "types.h"

// Name corroborated by the Echoes Wii conversion export; member names are reconstructed.
struct CPowerBombGuardianStageData {
  CPowerBombGuardianStageData(float minTimeBetweenAttacks, float maxTimeBetweenAttacks,
                              float minTimeBetweenShots, float maxTimeBetweenShots,
                              uchar minShotsInABurst, uchar maxShotsInABurst,
                              float projectileGravityMultiplier, float unknown18,
                              float doubleShotChance, uchar unknown20, uchar unknown21)
  : mMinTimeBetweenAttacks(minTimeBetweenAttacks)
  , mMaxTimeBetweenAttacks(maxTimeBetweenAttacks)
  , mMinTimeBetweenShots(minTimeBetweenShots)
  , mMaxTimeBetweenShots(maxTimeBetweenShots)
  , mMinShotsInABurst(minShotsInABurst)
  , mMaxShotsInABurst(maxShotsInABurst)
  , mProjectileGravityMultiplier(projectileGravityMultiplier)
  , x18_(unknown18)
  , mDoubleShotChance(doubleShotChance)
  , x20_(unknown20)
  , x21_(unknown21) {}

  float mMinTimeBetweenAttacks;
  float mMaxTimeBetweenAttacks;
  float mMinTimeBetweenShots;
  float mMaxTimeBetweenShots;
  uchar mMinShotsInABurst;
  uchar mMaxShotsInABurst;
  float mProjectileGravityMultiplier;
  float x18_; // Serialized property 0xd356c997; meaning unresolved.
  float mDoubleShotChance;
  uchar x20_; // Serialized property 0x87cc8ba4; meaning unresolved.
  uchar x21_; // Serialized property 0x6491357e; meaning unresolved.
};
CHECK_SIZEOF(CPowerBombGuardianStageData, 0x24)

#endif // _CPOWERBOMBGUARDIANSTAGEDATA
