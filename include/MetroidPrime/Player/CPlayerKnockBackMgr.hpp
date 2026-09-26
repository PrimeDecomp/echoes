#ifndef _CPLAYERKNOCKBACKMGR
#define _CPLAYERKNOCKBACKMGR

#include "MetroidPrime/Enemies/CKnockBackMgr.hpp"

// Wii SEL exports establish the class name; the remaining player-specific fields
// need semantic recovery. Its RULE_Player constructor is in a separate TU.
class CPlayerKnockBackMgr : public CKnockBackMgr {
public:
  CPlayerKnockBackMgr();
  ~CPlayerKnockBackMgr();

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

private:
  float x64_;
  float x68_;
  TUniqueId x6c_;
  float x70_;
  float x74_;
  float x78_;
  float x7c_;
  TUniqueId x80_;
  float x84_;
  float x88_;
  uchar x8c_;
  uchar x8d_;
};
CHECK_SIZEOF(CPlayerKnockBackMgr, 0x90)

#endif
