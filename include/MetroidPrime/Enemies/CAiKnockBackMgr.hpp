#ifndef _CAIKNOCKBACKMGR
#define _CAIKNOCKBACKMGR

#include "Kyoto/Animation/CharacterCommon.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/Enemies/CKnockBackMgr.hpp"

class CPhysicsActor;

class CAiKnockBackMgr : public CKnockBackMgr {
public:
  explicit CAiKnockBackMgr(CAssetId rules);
  ~CAiKnockBackMgr() {}

  // CKnockBackMgr
  void Update(float dt, CStateManager& mgr, CActor& actor) override;
  void KnockBack(CStateManager& mgr, CActor& actor, const CKnockBackInfo& info) override;
  bool IsAlive(const CActor& actor) const override;
  bool IsBall() const override;
  bool WasFrozen() const override;
  bool WasOnGround() const override;
  ECharacterState GetCharacterState(const CActor& actor) const override;
  bool HasAnimReaction(const CActor& actor, EAnimReaction reaction) const override;
  void DoKnockBackAnimation(const CVector3f& direction, CStateManager& mgr, CActor& actor,
                            float magnitude) override;
  void ApplyFollowUp(CActor& actor, CStateManager& mgr, TUniqueId source, TUniqueId owner) override;
  void ApplyKnockBackEffects(CActor& actor, CStateManager& mgr,
                             const CKnockBackInfo& info) override;

  void SetPhysicsKnockBackType(EPhysicsKnockBackType type);
  void EnableKnockBackPhysics(bool enabled);
  void SetAdditiveFlinchWeight(float weight);
  float GetAdditiveFlinchWeight() const;

  float GetFlinchRemainingTime() const { return mFlinchRemainingTime; } // Guessed name.

  // Guessed names, correlated with Prime's impulse implementation.
  void ApplyImpulse(float dt, CPhysicsActor& actor);
  void ResetKnockBackImpulse(CActor& actor, const CVector3f& direction, float magnitude);

private:
  // Guessed member names, recovered from initialization and native consumers.
  pas::ESeverity mSeverity;
  int mFlinchType; // Guessed name; enum parameter for the directional flinch animation.
  float mFlinchRemainingTime;
  EPhysicsKnockBackType mPhysicsKnockBackType;
  CVector3f mImpulseDirection;
  float mImpulseMagnitude;
  float mImpulseRemainingTime;
  float mAdditiveFlinchWeight;
  float mPhysicsImpulseMagnitude;
  bool mKnockBackPhysicsEnabled : 1;
  bool mHurlVelocityEnabled : 1; // Guessed name; selects computed hurl velocity versus zero.
  bool mWasFrozen : 1;
  bool mWasOnGround : 1;

  static const float skImpulseDurations[2];
  static const pas::EAnimationState skReactionStates[5];
};
CHECK_SIZEOF(CAiKnockBackMgr, 0x94)

#endif
