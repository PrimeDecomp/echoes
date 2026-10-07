#ifndef _CTRYCLOPS
#define _CTRYCLOPS

#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"

#include "Kyoto/Math/CTransform4f.hpp"
#include "rstl/single_ptr.hpp"

class CCollisionActorManager;
class CPlayer;

class CTryclops : public CPatterned {
public:
  CTryclops(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
            const CTransform4f& xf, const CModelData& mData, const CPatternedInfo& pInfo,
            const CActorParameters& actParms, float suckForceMultiplier, float suckAngle,
            float suckRange, float launchSpeed);

  // CEntity
  ~CTryclops() override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  const CDamageVulnerability* GetDamageVulnerability() const override;
  const CDamageVulnerability* GetDamageVulnerability(const CVector3f& position,
                                                     const CVector3f& direction,
                                                     const CDamageInfo& damage) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CPatterned
  CPathFindSearch* GetSearchPath() override { return &mPathFindSearch; }
  void SetupStateMachine(CStateManager& mgr) override;

  // CAi
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;
  bool IsListening() const override { return true; }

  // States
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt);
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt);
  void SelectTarget(CStateManager& mgr, EStateMsg msg, float dt);
  void TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt);
  void TargetPlayer(CStateManager& mgr, EStateMsg msg, float dt);
  void TargetCover(CStateManager& mgr, EStateMsg msg, float dt);
  void Attack(CStateManager& mgr, EStateMsg msg, float dt);
  void JumpBack(CStateManager& mgr, EStateMsg msg, float dt);
  void Shuffle(CStateManager& mgr, EStateMsg msg, float dt);
  void TurnAround(CStateManager& mgr, EStateMsg msg, float dt);
  void Crouch(CStateManager& mgr, EStateMsg msg, float dt);
  void GetUp(CStateManager& mgr, EStateMsg msg, float dt);
  void Suck(CStateManager& mgr, EStateMsg msg, float dt);
  void Cover(CStateManager& mgr, EStateMsg msg, float dt);
  void Approach(CStateManager& mgr, EStateMsg msg, float dt);
  void PathFindEx(CStateManager& mgr, EStateMsg msg, float dt);
  void Dizzy(CStateManager& mgr, EStateMsg msg, float dt);
  void FixedDelay(CStateManager& mgr, EStateMsg msg, float dt);

  // Triggers
  bool InAttackPosition(CStateManager& mgr, const CTriggerData& data) const;
  bool InRange(CStateManager& mgr, const CTriggerData& data) const;
  bool InMaxRange(CStateManager& mgr, const CTriggerData& data) const;
  bool InDetectionRange(CStateManager& mgr, const CTriggerData& data) const;
  bool SpotPlayer(CStateManager& mgr, const CTriggerData& data) const;
  bool InPosition(CStateManager& mgr, const CTriggerData& data) const;
  bool HearShot(CStateManager& mgr, const CTriggerData& data) const;
  bool CoverBlown(CStateManager& mgr, const CTriggerData& data) const;
  bool Inside(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldRetreat(CStateManager& mgr, const CTriggerData& data) const;
  bool IsDizzy(CStateManager& mgr, const CTriggerData& data) const;
  bool HasRetreatPattern(CStateManager& mgr, const CTriggerData& data) const;

private:
  static CVector3f kBombPosOffset;
  static const char* const kMouthLctr;

  void SetupCollisionManager(CStateManager& mgr);
  void GrabPlayer(CPlayer& player, CStateManager& mgr);    // Guessed name.
  void ReleasePlayer(CPlayer& player, CStateManager& mgr); // Guessed name.
  bool BallCloseToCollision(const CPlayer& player, const CStateManager& mgr) const;
  void ApplySeparationBehavior(CStateManager& mgr);
  void ShootPlayer(CPlayer& player, CStateManager& mgr, const CTransform4f& xf, float speed);
  void ShootBomb(CStateManager& mgr, const CTransform4f& xf);
  void SetPlayerPosition(CPlayer& player, CStateManager& mgr, const CVector3f& pos);
  void SetBombPosition(CStateManager& mgr);
  bool TargetCaught(const CVector3f& pos, float range) const;
  bool ObjectInVortexArea(const CVector3f& pos, const CVector3f& center, const CAABox& bounds,
                          const CStateManager& mgr) const;
  void AttractPlayer(CPlayer& player, CStateManager& mgr, float dt);
  void CenterPlayer(CPlayer& player, CStateManager& mgr, const CVector3f& pos, float dt);
  void AttractBomb(CStateManager& mgr, float dt);

  rstl::single_ptr< CCollisionActorManager > mCollisionActorManager;
  CPathFindSearch mPathFindSearch;
  mutable CTransform4f mPlayerRotation;
  float mSuckForceMultiplier;
  float mMinSuckAngleProj;
  float mSuckRange;
  float mLaunchSpeed;
  float mIgnoreMorphballTimer;
  uint x8f4_;
  mutable TUniqueId mBombId;
  TUniqueId x8fa_;
  mutable TUniqueId mTargetPlayerId; // Guessed name; player caught by InAttackPosition.
  CDamageVulnerability mPowerBombVulnerability;
  mutable bool mShotTarget : 1;
  mutable bool mTargetingBomb : 1;
  mutable bool mVulnerable : 1;
  mutable bool mDizzy : 1;
  float mDizzyTimer; // Guessed name; counts down while the player is held.
};
CHECK_SIZEOF(CTryclops, 0x938)

#endif // _CTRYCLOPS
