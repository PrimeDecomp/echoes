#ifndef _CKNOCKBACKMGR
#define _CKNOCKBACKMGR

#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/CRuleSetEvaluator.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Weapons/WeaponTypes.hpp"

class CActor;
class CDamageInfo;
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
  // Guessed enumerator names; both profiles apply an impulse.
  enum EPhysicsKnockBackType { kPKBT_Constant, kPKBT_Decaying };

  // Guessed names, reconstructed from RULE actions and their consumers.
  enum EFollowUp {
    kFU_None,
    kFU_Slow,
    kFU_Freeze,
    kFU_Shock,
    kFU_Burn,
    kFU_BurnPhase,
    kFU_Death,
    kFU_ExplodeDeath,
    kFU_IceDeath,
    kFU_BurnDeath,
    kFU_LaggedBurnDeath,
    kFU_BlackDeath,
    kFU_ImmediateExplosion,
    kFU_ImmediateDisintegration,
    kFU_FreezeBurn,
    kFU_FreezeDisintegration
  };
  enum ECharacterState { kCS_Invalid = -1, kCS_Alive, kCS_Dead };
  enum EKnockBackWeaponType {
    kKBWT_Invalid = -1,
    kKBWT_Power,
    kKBWT_PowerCharged,
    kKBWT_PowerCombo,
    kKBWT_Dark,
    kKBWT_DarkChargedDirect,
    kKBWT_DarkChargedIndirect,
    kKBWT_DarkCombo,
    kKBWT_Light,
    kKBWT_LightCharged,
    kKBWT_LightCombo,
    kKBWT_LightMissile,
    kKBWT_Annihilator,
    kKBWT_AnnihilatorNoDamage,
    kKBWT_AnnihilatorCharged,
    kKBWT_AnnihilatorChargedEffect,
    kKBWT_AnnihilatorCombo,
    kKBWT_Bomb,
    kKBWT_PowerBomb,
    kKBWT_Missile,
    kKBWT_BoostBall,
    kKBWT_CannonBall,
    kKBWT_ScrewAttack,
    kKBWT_Phazon,
    kKBWT_AI,
    kKBWT_PoisonWater1,
    kKBWT_PoisonWater2,
    kKBWT_Lava,
    kKBWT_Heat,
    kKBWT_Unused,
    kKBWT_AreaLight,
    kKBWT_AreaDark,
    kKBWT_UnknownSource,
    kKBWT_SafeZone
  };

  explicit CKnockBackMgr(CAssetId rules);
  ~CKnockBackMgr();

  // CRuleSetEvaluator
  CRuleValue GetConditionValue(FourCC condition) const override;
  bool ExecuteAction(const CRuleAction& action) override;

  // CKnockBackMgr
  virtual void Update(float dt, CStateManager& mgr, CActor& actor);
  virtual void KnockBack(CStateManager& mgr, CActor& actor, const CKnockBackInfo& info);
  // Guessed hook names; order is shared by the AI and player implementations.
  virtual bool IsAlive(const CActor& actor) const = 0;
  virtual bool IsBall() const = 0;
  virtual bool WasFrozen() const = 0;
  virtual bool WasOnGround() const = 0;
  virtual ECharacterState GetCharacterState(const CActor& actor) const = 0;
  virtual bool HasAnimReaction(const CActor& actor, EAnimReaction reaction) const = 0;
  virtual void DoKnockBackAnimation(const CVector3f& direction, CStateManager& mgr, CActor& actor,
                                    float magnitude) = 0;
  virtual void ApplyFollowUp(CActor& actor, CStateManager& mgr, TUniqueId source,
                             TUniqueId owner) = 0;
  virtual void ApplyKnockBackEffects(CActor& actor, CStateManager& mgr,
                                     const CKnockBackInfo& info) = 0;

  void EnableAnimReaction(EAnimReaction reaction, bool enabled);
  void EnableAllAnimReactions(bool enabled);
  bool IsAnimReactionEnabled(EAnimReaction reaction) const;
  void SetAnimReactionRange(EAnimReaction minimum, EAnimReaction maximum);
  CVector3f GetKnockBackDirection(const CVector3f& direction, const CActor& actor) const;
  EKnockBackWeaponType GetKnockBackWeaponType(const CDamageInfo& info, EWeaponType weapon,
                                              bool direct) const;
  void SelectDamageState(const CActor& actor, const CKnockBackInfo& info);
  void ValidateState(const CActor& actor);
  void DeferFollowUp(float delay, EFollowUp followUp, float duration);
  float CalculateExtraHurlVelocity(CStateManager& mgr, float magnitude, float resistance) const;

protected:
  // Guessed name. Echoes uses RULE resources instead of Prime's static parameter table.
  struct SReactionParameters {
    EAnimReaction mReaction;
    EFollowUp mFollowUp;
    float mFollowUpDuration;
    float mSecondaryDuration;
    uint mFlags;
  };

  SReactionParameters mActiveParameters;
  SReactionParameters mDeferredParameters;
  float mDeferredRemainingTime;
  EAnimReaction mMinimumReaction;
  EAnimReaction mMaximumReaction;
  float x44_;
  uint x48_;
  uint x4c_;
  uint x50_;
  uint x54_;
  ECharacterState mCharacterState;
  EKnockBackWeaponType mWeaponType;
  uchar mAvailableReactions;
  bool mEnableSlow : 1;
  bool mEnableFreeze : 1;
  bool mEnableShock : 1;
  bool mEnableBurn : 1;
  bool mEnableBurnDeath : 1;
  bool mEnableExplodeDeath : 1;
  bool mEnableLaggedBurnDeath : 1;
  bool x61_31_ : 1;
  bool mLocomotionDuringElectrocution : 1;
  bool mIsMultiplayer : 1;

  static const SReactionParameters skDefaultParameters;
  static const EKnockBackWeaponType skWeaponTypes[kWT_Max];
};
CHECK_SIZEOF(CKnockBackMgr, 0x64)

#endif
