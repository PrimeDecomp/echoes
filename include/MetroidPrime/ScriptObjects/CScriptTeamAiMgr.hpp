#ifndef _CSCRIPTTEAMAIMGR
#define _CSCRIPTTEAMAIMGR

#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/Enemies/CTeamAiRole.hpp"

class CActor;
class CAi;

class CScriptTeamAiMgr : public CEntity {
public:
  enum EAttackType { kAT_Melee, kAT_Projectile };
  // Action values are supplied by callers; their individual meanings remain unresolved.
  enum ETeamAction { kTA_Unknown0 = 0 };

  // Guessed name
  struct CTeamAiData {
    CTeamAiData();

    uint mAiCount;
    uint mMeleeCount;
    uint mProjectileCount;
    uint mOtherRoleCount;
    uint mMaxMeleeAttackerCount;
    uint mMaxProjectileAttackerCount;
    uint mPositionMode;
    float mMeleeTimeInterval;
    float mProjectileTimeInterval;
  };

  CScriptTeamAiMgr(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                   const CTeamAiData& data);

  // CEntity
  ~CScriptTeamAiMgr() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;

  static TUniqueId GetAssociatedTeamId(const CAi& ai, CStateManager& mgr);
  static const CTeamAiRole* GetTeamAiRole(const CStateManager& mgr, TUniqueId teamId,
                                          TUniqueId memberId);
  static bool CanStartAttack(EAttackType type, CStateManager& mgr, TUniqueId teamId,
                             TUniqueId memberId);
  static bool StartAttack(EAttackType type, CStateManager& mgr, TUniqueId teamId,
                          TUniqueId memberId);
  static void EndAttack(EAttackType type, CStateManager& mgr, TUniqueId teamId, TUniqueId memberId,
                        bool clearRole);

  bool JoinTeam(const CAi& ai, CTeamAiRole::ETeamAiRole roleA, CTeamAiRole::ETeamAiRole roleB,
                CTeamAiRole::ETeamAiRole roleC);
  void QuitTeam(TUniqueId id);
  const CTeamAiRole* GetRole(TUniqueId id) const;
  void ClearTeamAiRole(TUniqueId id);
  bool HasTeamAiRole(TUniqueId id) const;
  bool IsPartOfTeam(TUniqueId id) const;
  bool IsMeleeAttacking(TUniqueId id) const;
  bool CanStartMeleeAttack(TUniqueId id) const;
  bool StartMeleeAttack(TUniqueId id);
  void EndMeleeAttack(TUniqueId id);
  bool CanStartProjectileAttack(TUniqueId id) const;
  bool StartProjectileAttack(TUniqueId id);
  void EndProjectileAttack(TUniqueId id);
  bool ShouldUpdateRoles(float dt);
  void UpdateRoles(CStateManager& mgr);
  void ResetRoles(CStateManager& mgr);
  void AssignRoles(CTeamAiRole::ETeamAiRole role, uint count);
  void SetPlayerForwardProjectionDistance(float distance);
  void PositionTeam(CStateManager& mgr);
  void SpacingSort(CStateManager& mgr, const CVector3f& position);
  void UpdateTeamCaptain();
  bool IsTeamMemberInRange(const CStateManager& mgr, const CActor& actor, float range) const;
  TUniqueId FindBestIndividualAttackTarget(CStateManager& mgr, const CAi& ai);
  // The GameCube ABI is static; the Wii export has a const member signature.
  static TUniqueId ChoosePlayer(const CStateManager& mgr, const CActor& actor);
  void SetMemberTargetId(TUniqueId memberId, TUniqueId targetId);
  CTeamAiRole::ETeamAiRole GetTeamRole(TUniqueId memberId) const;
  void NotifyWasHit();
  bool GetWasHit() const;
  void StartTeamAction(TUniqueId id, ETeamAction action);
  void EndTeamAction(TUniqueId id, ETeamAction action);
  int GetTeamActionCount(ETeamAction action) const;
  // Guessed names
  bool IsPerformingTeamAction(TUniqueId id, ETeamAction action) const;
  void RemoveInvalidTeamActions(CStateManager& mgr);

  bool AnyMembersInCircle(CStateManager& mgr, const CVector3f& position, float radius,
                          TUniqueId excludeId) const;
  CVector3f GetCenter(CStateManager& mgr) const;
  TUniqueId TouchingAnyTeammates(CStateManager& mgr, TUniqueId id, float margin) const;

private:
  // Guessed name
  struct STeamAction {
    STeamAction(TUniqueId ownerId, ETeamAction action) : mOwnerId(ownerId), mAction(action) {}

    TUniqueId mOwnerId;
    ETeamAction mAction;
  };

  CTeamAiData mData;
  rstl::vector< CTeamAiRole > mRoles;
  rstl::vector< TUniqueId > mMeleeAttackers;
  rstl::vector< TUniqueId > mProjectileAttackers;
  float mTimeDirty;
  TUniqueId mTeamCaptainId;
  float mTimeSinceMelee;
  float mTimeSinceProjectile;
  bool mWasHit;
  float mPlayerForwardProjectionDistance;
  rstl::vector< STeamAction > mTeamActions;
};
CHECK_SIZEOF(CScriptTeamAiMgr, 0xa0)

#endif
