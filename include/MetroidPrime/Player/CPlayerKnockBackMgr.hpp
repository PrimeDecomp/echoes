#ifndef _CPLAYERKNOCKBACKMGR
#define _CPLAYERKNOCKBACKMGR

#include "Collision/CMaterialList.hpp"
#include "Kyoto/Animation/CharacterCommon.hpp"
#include "MetroidPrime/Enemies/CKnockBackMgr.hpp"

class CPlayer;

// Wii SEL exports establish the class name and ApplyPlayerKnockBackForce.
// Other player-specific method and member names are guessed from native consumers.
class CPlayerKnockBackMgr : public CKnockBackMgr {
public:
  CPlayerKnockBackMgr();
  ~CPlayerKnockBackMgr() {}

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

  void ResetEffects(CStateManager& mgr, CPlayer& player);
  float GetBurnDeathAlpha() const;
  float GetBurnRemainingTime() const { return mBurnRemainingTime; }
  bool IsDeathAnimationStarted() const { return mDeathAnimationStarted; }
  bool IsRagDollPending() const { return mRagDollPending; }
  void Burn(float duration, float damagePerSecond, TUniqueId owner);
  void DouseFlames();
  void StopBurnDeath(CStateManager& mgr, CPlayer& player);
  void Freeze(float duration, CPlayer& player);
  void Shock(float duration, float damagePerSecond, CPlayer& player, TUniqueId owner);
  void DouseElectrocution();

  static void ApplyPlayerKnockBackForce(CPlayer& player, const CVector3f& direction, float power,
                                        float unused);

private:
  enum EExplosionDeathType { kEDT_Normal, kEDT_Ice };
  enum EBurnDeathType { kBDT_Normal, kBDT_Lagged };

  void UpdateBurning(float dt, CStateManager& mgr, CPlayer& player);
  void StartBurnDeath(CStateManager& mgr, CPlayer& player, EBurnDeathType type);
  void ExplodeDeath(CStateManager& mgr, CPlayer& player, EExplosionDeathType type,
                    TUniqueId source);
  void UpdateElectrocution(float dt, CStateManager& mgr, CPlayer& player);
  bool CanApplyKnockBackForce(CStateManager& mgr, CPlayer& player,
                              const CKnockBackInfo& info) const;
  void StartBlackHoleDeath(CStateManager& mgr, TUniqueId source, CPlayer& player);
  void UpdateImplosion(CStateManager& mgr, CPlayer& player);

  float mBurnRemainingTime;
  float mBurnDamagePerSecond;
  TUniqueId mBurnOwner;
  float mBallExtinguishRemainingTime;
  float mBurnDeathRemainingTime;
  float mElectrocutionRemainingTime;
  float mElectrocutionDamagePerSecond;
  TUniqueId mElectrocutionOwner;
  float mRagDollDelay;
  float mFreezeDuration;
  bool mWasBall : 1;
  bool mWasFrozen : 1;
  bool mWasOnGround : 1;
  bool mLaggedBurnDeath : 1;
  bool mImploding : 1;
  bool mBurnDeath : 1;
  bool mRagDollPending : 1;
  bool mDeathAnimationStarted : 1;
  bool mFreezePending : 1;
  bool mExplosionDeathStarted : 1;

  static const int skAnimationStates[5];
  static EMaterialTypes sDamageMaterial; // Guessed name; burn/electrocution damage filter.
};
CHECK_SIZEOF(CPlayerKnockBackMgr, 0x90)

#endif
