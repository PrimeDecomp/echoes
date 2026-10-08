#ifndef _CBASICSWARMDATA
#define _CBASICSWARMDATA

#include "Kyoto/SObjectTag.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CHealthInfo.hpp"

struct SLdrBasicSwarmProperties;

// Name corroborated by the Echoes Wii conversion export; member names are reconstructed.
class CBasicSwarmData {
  friend CBasicSwarmData LdrToBasicSwarmData(const SLdrBasicSwarmProperties& data);
  friend class CSwarmBasics;
  friend class CMetareeSwarm;
  friend class CIngBlobSwarm;

public:
  CBasicSwarmData(const CDamageInfo& damage, const CHealthInfo& health,
                  const CDamageVulnerability& vulnerability, CAssetId deathParticleEffect);

private:
  CDamageInfo mContactDamage;
  CHealthInfo mHealth;
  CDamageVulnerability mDamageVulnerability;
  float mDamageWaitTime;
  float mCollisionRadius;
  float mTouchRadius;
  float mDamageRadius;
  float mSpeed;
  int mCount;
  int mMaxCount;
  float mInfluenceRadius;
  float mCohesionPriority;
  float mAlignmentPriority;
  float mSeparationPriority;
  float mPathFollowingPriority;
  float mPlayerAttractPriority;
  float mPlayerAttractDistance;
  float mSpawnSpeed;
  CAssetId mDeathParticleEffect;
  int mNumDeathParticles;
  int mAttackerCount;
  float mAttackProximity;
  float mAttackTimer;
  float mSafeZoneAvoidancePriority;
  float mTurnRate;
  ushort mLocomotionLoopedSound;
  ushort mAttackLoopedSound;
  float mSoundFallOff;
  float mMaxAudibleDistance;
  uchar mMinVolume;
  uchar mMaxVolume;
  float mFreezeDuration;
  float mLifeTime;
  bool mIsVulnerableToSafeZone : 1;
  bool xdc_1 : 1; // Serialized property 0x7eb5d9e8; meaning unresolved.
  bool mIsOrbitable : 1;
  bool mIndividuallyTargetable : 1;
};
CHECK_SIZEOF(CBasicSwarmData, 0xe0)

#endif // _CBASICSWARMDATA
