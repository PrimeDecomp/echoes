#ifndef _CINGSPOTDATA
#define _CINGSPOTDATA

#include "Kyoto/SObjectTag.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"

class CIngSpotData {
public:
  CIngSpotData(CAssetId blobEffect, CAssetId normalHitEffect, CAssetId heavyHitEffect,
               CAssetId deathEffect, float maxSpeed, float maxWallSpeed, float ballPursuitSpeed,
               float hurtSpeed, float turnSpeed, const CDamageVulnerability& vulnerability,
               ushort idleSound, ushort moveSound, ushort normalHitSound, ushort heavyHitSound,
               ushort deathSound);

  CAssetId GetBlobEffect() const { return mBlobEffect; }
  CAssetId GetNormalHitEffect() const { return mNormalHitEffect; }
  CAssetId GetHeavyHitEffect() const { return mHeavyHitEffect; }
  CAssetId GetDeathEffect() const { return mDeathEffect; }
  float GetMaxSpeed() const { return mMaxSpeed; }
  float GetMaxWallSpeed() const { return mMaxWallSpeed; }
  float GetBallPursuitSpeed() const { return mBallPursuitSpeed; }
  float GetHurtSpeed() const { return mHurtSpeed; } // Guessed name
  float GetTurnSpeed() const { return mTurnSpeed; }
  const CDamageVulnerability& GetVulnerability() const { return mVulnerability; }
  ushort GetIdleSound() const { return mIdleSound; }
  ushort GetMoveSound() const { return mMoveSound; }
  ushort GetNormalHitSound() const { return mNormalHitSound; }
  ushort GetHeavyHitSound() const { return mHeavyHitSound; }
  ushort GetDeathSound() const { return mDeathSound; }

private:
  CAssetId mBlobEffect;
  CAssetId mNormalHitEffect;
  CAssetId mHeavyHitEffect;
  CAssetId mDeathEffect;
  float mMaxSpeed;
  float mMaxWallSpeed;
  float mBallPursuitSpeed;
  float mHurtSpeed;
  float mTurnSpeed;
  CDamageVulnerability mVulnerability;
  ushort mIdleSound;
  ushort mMoveSound;
  ushort mNormalHitSound;
  ushort mHeavyHitSound;
  ushort mDeathSound;
};

CHECK_SIZEOF(CIngSpotData, 0x60)

#endif // _CINGSPOTDATA
