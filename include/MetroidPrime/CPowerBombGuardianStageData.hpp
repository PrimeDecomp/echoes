#ifndef _CPOWERBOMBGUARDIANSTAGEDATA
#define _CPOWERBOMBGUARDIANSTAGEDATA

#include "types.h"

// Name corroborated by the Echoes Wii conversion export; member names are reconstructed.
struct CPowerBombGuardianStageData {
  CPowerBombGuardianStageData(float minTimeBetweenAttacks, float maxTimeBetweenAttacks,
                              float minTimeBetweenShots, float maxTimeBetweenShots,
                              uchar minShotsInABurst, uchar maxShotsInABurst,
                              float projectileGravityMultiplier, float waypointTargetSpreadRadius,
                              float doubleShotChance, uchar minAttacksPerDoubleShot,
                              uchar maxAttacksPerDoubleShot)
  : mMinTimeBetweenAttacks(minTimeBetweenAttacks)
  , mMaxTimeBetweenAttacks(maxTimeBetweenAttacks)
  , mMinTimeBetweenShots(minTimeBetweenShots)
  , mMaxTimeBetweenShots(maxTimeBetweenShots)
  , mMinShotsInABurst(minShotsInABurst)
  , mMaxShotsInABurst(maxShotsInABurst)
  , mProjectileGravityMultiplier(projectileGravityMultiplier)
  , mWaypointTargetSpreadRadius(waypointTargetSpreadRadius)
  , mDoubleShotChance(doubleShotChance)
  , mMinAttacksPerDoubleShot(minAttacksPerDoubleShot)
  , mMaxAttacksPerDoubleShot(maxAttacksPerDoubleShot) {}

  float mMinTimeBetweenAttacks;
  float mMaxTimeBetweenAttacks;
  float mMinTimeBetweenShots;
  float mMaxTimeBetweenShots;
  uchar mMinShotsInABurst;
  uchar mMaxShotsInABurst;
  float mProjectileGravityMultiplier;
  float mWaypointTargetSpreadRadius; // Target-derived name; serialized property 0xd356c997.
  float mDoubleShotChance;
  uchar mMinAttacksPerDoubleShot; // Serialized property 0x87cc8ba4; name matches its property ID.
  uchar mMaxAttacksPerDoubleShot; // Serialized property 0x6491357e; name matches its property ID.
};
CHECK_SIZEOF(CPowerBombGuardianStageData, 0x24)

#endif // _CPOWERBOMBGUARDIANSTAGEDATA
