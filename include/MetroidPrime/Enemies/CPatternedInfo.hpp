#ifndef _CPATTERNEDINFO
#define _CPATTERNEDINFO

#include "MetroidPrime/CAnimationParameters.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/SEchoParameters.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrIngPossessionData.hpp"

struct SLdrPatternedAITypedef;

class CPatternedInfo {
  friend class CPatterned;
  friend CPatternedInfo LdrToPatternedInfo(const SLdrPatternedAITypedef& data,
                                           const SLdrIngPossessionData* possession);

public:
  CPatternedInfo(const CHealthInfo& health, const CDamageVulnerability& vulnerability,
                 CAssetId stateMachine, CAssetId stateMachine2);
  ~CPatternedInfo();

  const CAnimationParameters& GetAnimationParameters() const { return mAnimationParameters; }
  const CHealthInfo& GetHealthInfo() const { return mHealthInfo; }
  const CDamageVulnerability& GetDamageVulnerability() const { return mDamageVulnerability; }
  const CDamageInfo& GetContactDamage() const { return mContactDamageInfo; }
  const float& GetHalfExtent() const { return mHalfExtent; }
  const CVector3f& GetBodyOrigin() const { return mBodyOrigin; }
  uint GetPathfindingIndex() const { return mPathfindingIndex; }
  bool IsAnEncounter() const { return mIngPossessionData.isAnEncounter; } // Guessed name
  const SLdrIngPossessionData& GetIngPossessionData() const { return mIngPossessionData; }
  float GetHeight() const { return mHeight; }
  float GetDetectionRange() const { return mDetectionRange; }
  float GetDetectionHeightRange() const { return mDetectionHeightRange; }
  float GetSpeed() const { return mSpeed; }
  float GetTurnSpeed() const { return mTurnSpeed; }
  float GetMinAttackRange() const { return mMinAttackRange; }
  float GetMaxAttackRange() const { return mMaxAttackRange; }

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
  SEchoParameters mEchoParameters;
};
CHECK_SIZEOF(CPatternedInfo, 0x2b4)

#endif
