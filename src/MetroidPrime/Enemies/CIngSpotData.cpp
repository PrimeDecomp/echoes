#include "MetroidPrime/Enemies/CIngSpotData.hpp"

CIngSpotData::CIngSpotData(CAssetId blobEffect, CAssetId normalHitEffect, CAssetId heavyHitEffect,
                           CAssetId deathEffect, float maxSpeed, float maxWallSpeed,
                           float ballPursuitSpeed, float unknown1c, float turnSpeed,
                           const CDamageVulnerability& vulnerability, ushort idleSound,
                           ushort moveSound, ushort normalHitSound, ushort heavyHitSound,
                           ushort deathSound)
: mBlobEffect(blobEffect)
, mNormalHitEffect(normalHitEffect)
, mHeavyHitEffect(heavyHitEffect)
, mDeathEffect(deathEffect)
, mMaxSpeed(maxSpeed)
, mMaxWallSpeed(maxWallSpeed)
, mBallPursuitSpeed(ballPursuitSpeed)
, x1c_(unknown1c)
, mTurnSpeed(turnSpeed)
, mVulnerability(vulnerability)
, mIdleSound(idleSound)
, mMoveSound(moveSound)
, mNormalHitSound(normalHitSound)
, mHeavyHitSound(heavyHitSound)
, mDeathSound(deathSound) {}
