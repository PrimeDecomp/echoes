#ifndef _CKNOCKBACKMGR
#define _CKNOCKBACKMGR

#include "MetroidPrime/CRuleSetEvaluator.hpp"
#include "MetroidPrime/TGameTypes.hpp"

class CActor;
class CKnockBackInfo;
class CStateManager;

class CKnockBackMgr : public CRuleSetEvaluator {
public:
  enum EAnimReaction {
    kAR_Invalid = -1,
    kAR_None,
    kAR_Flinch,
    kAR_KnockBack,
    kAR_Hurled,
    kAR_Fall,
  };
  enum EPhysicsKnockBackType { kPKBT_None, kPKBT_Impulse };

  explicit CKnockBackMgr(CAssetId rules);
  ~CKnockBackMgr();
  CRuleValue GetConditionValue(FourCC condition) const override;
  bool ExecuteAction(const CRuleAction& action) override;
  virtual void Update(float dt, CStateManager& mgr, CActor& actor);
  virtual void KnockBack(CStateManager& mgr, CActor& actor, const CKnockBackInfo& info);
  // Remaining knockback-specific virtual hooks are not yet reconstructed.
  void EnableAnimReaction(EAnimReaction reaction, bool enabled);
  void EnableAllAnimReactions(bool enabled);
  bool IsAnimReactionEnabled(EAnimReaction reaction) const;
  void SetAnimReactionRange(EAnimReaction minimum, EAnimReaction maximum);

protected:
  // Guessed name. Echoes uses RULE resources instead of Prime's static parameter table.
  struct SReactionParameters {
    EAnimReaction mReaction;
    int mFollowUp;
    float mFollowUpDuration;
    float mIntoFreezeDuration;
    float x10_;
  };

  SReactionParameters mActiveParameters;
  SReactionParameters mDeferredParameters;
  float mFlinchRemainingTime;
  EAnimReaction mMinimumReaction;
  EAnimReaction mMaximumReaction;
  float mDeferredRemainingTime;
  uint x48_;
  uint x4c_;
  uint x50_;
  uint x54_;
  int x58_;
  int x5c_;
  uchar mAvailableReactions;
  bool x61_24_ : 1;
  bool mEnableFreeze : 1;
  bool mEnableShock : 1;
  bool mEnableBurn : 1;
  bool mEnableBurnDeath : 1;
  bool mEnableExplodeDeath : 1;
  bool mEnableLaggedBurnDeath : 1;
  bool x61_31_ : 1;
  bool x62_24_ : 1;
  bool mIsMultiplayer : 1;
};
CHECK_SIZEOF(CKnockBackMgr, 0x64)

#endif
