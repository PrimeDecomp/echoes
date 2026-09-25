#ifndef _CPATTERNEDINFO
#define _CPATTERNEDINFO

#include "MetroidPrime/CAnimationParameters.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CEchoParameters.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrIngPossessionData.hpp"

class CPatternedInfo {
  friend class CPatterned;

public:
  const CAnimationParameters& GetAnimationParameters() const { return mAnimationParameters; }
  const CHealthInfo& GetHealthInfo() const { return mHealthInfo; }
  const CDamageVulnerability& GetDamageVulnerability() const { return mDamageVulnerability; }
  uint GetPathfindingIndex() const { return mPathfindingIndex; }

private:
  float mMass;
  float mSpeed;
  float mTurnSpeed;
  float mDetectionRange;
  float mDetectionHeightRange;
  float mDetectionAngle;
  float mMinAttackRange;
  float mMaxAttackRange;
  float mAverageAttackTime;
  float mAttackTimeVariation;
  float mLeashRadius;
  float mPlayerLeashRadius;
  float mPlayerLeashTime;
  CDamageInfo mContactDamageInfo;
  float mDamageWaitTime;
  CHealthInfo mHealthInfo;
  CDamageVulnerability mDamageVulnerability;
  float mHalfExtent;
  float mHeight;
  CVector3f mBodyOrigin;
  float mStepUpHeight;
  float mXDamageThreshold;
  float mFrozenXDamageThreshold;
  float mXDamageDelay;
  uint mDeathSfx;
  CAnimationParameters mAnimationParameters;
  CAssetId mStateMachineId;
  CAssetId mStateMachine2Id;
  float mIntoFreezeDuration;
  float mOutOfFreezeDuration;
  float mFreezeDuration;
  uint mPathfindingIndex;
  CVector3f mDeathExplosionOffset;
  CAssetId mDeathExplosionParticle;
  CAssetId mDeathExplosionElectric;
  CVector3f mIceDeathExplosionOffset;
  CAssetId mIceDeathExplosionParticle;
  uint mIceShatterSfx;
  uint mIceVocalSfx;
  uint mFrozenSfx;
  SLdrIngPossessionData mIngPossessionData;
  CAssetId mKnockBackRules;
  int mCreatureSize;
  CEchoParameters mEchoParameters;
};
CHECK_SIZEOF(CPatternedInfo, 0x2b4)

#endif
