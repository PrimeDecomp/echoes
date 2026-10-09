#ifndef _CINGSPACEJUMPGUARDIAN
#define _CINGSPACEJUMPGUARDIAN

#include "types.h"

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/ScriptLoader/SLdrIngSpaceJumpGuardian.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class CCollisionActorManager;
class CDamageVulnerability;
class CGenDescription;

// Guessed class: the Ing Space Jump Guardian boss, a biped that jumps between jump points, throws
// mini portals and taunts. The class name is the original Wii export
// TypesMatch__21CIngSpaceJumpGuardian.
class CIngSpaceJumpGuardian : public CPatterned {
public:
  CIngSpaceJumpGuardian(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                        const CTransform4f& xf, const CModelData& modelData,
                        const CActorParameters& actorParams, const CPatternedInfo& patternedInfo,
                        const SLdrIngSpaceJumpGuardianData& data);

  // CEntity
  ~CIngSpaceJumpGuardian() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CAi
  bool IsListening() const override { return true; }
  bool Listen(CStateManager& mgr, const CVector3f& position, EListenNoiseType type) override;

  // CPatterned
  CPathFindSearch* GetSearchPath() override { return &mPathFindSearch; }
  float GetGravityConstant() const override { return 60.f; }
  void SetupStateMachine(CStateManager& mgr) override;

  // CIngSpaceJumpGuardian
  void Start(CStateManager& mgr, EStateMsg msg, float dt);
  void Dead(CStateManager& mgr, EStateMsg msg, float dt);
  void FollowIntroPattern(CStateManager& mgr, EStateMsg msg, float dt);
  void FaceTarget(CStateManager& mgr, EStateMsg msg, float dt);
  void Jump(CStateManager& mgr, EStateMsg msg, float dt);
  void Taunt(CStateManager& mgr, EStateMsg msg, float dt);
  void MiniPortalAttack(CStateManager& mgr, EStateMsg msg, float dt);
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt);
  void SteerToDest(CStateManager& mgr, EStateMsg msg, float dt);

  bool StateOver(CStateManager& mgr, const CTriggerData& data) const;
  bool HasIntroPattern(CStateManager& mgr, const CTriggerData& data) const;
  bool IntroPatternOver(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldJump(CStateManager& mgr, const CTriggerData& data) const;
  bool HasJumpTarget(CStateManager& mgr, const CTriggerData& data) const;
  bool IsFacingTarget(CStateManager& mgr, const CTriggerData& data) const;
  bool IsFacingWaypoint(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldTaunt(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldMiniPortalAttack(CStateManager& mgr, const CTriggerData& data) const;
  bool FoundMiniPortalAttack(CStateManager& mgr, const CTriggerData& data) const;
  bool SkipToNextJumpPoint(CStateManager& mgr, const CTriggerData& data) const;
  bool PathOver(CStateManager& mgr, const CTriggerData& data) const;
  bool PathShagged(CStateManager& mgr, const CTriggerData& data) const;
  bool PathExists(CStateManager& mgr, const CTriggerData& data) const;

  void EnableEnergyBar(CStateManager& mgr, float dt);
  void SelectTarget(CStateManager& mgr, float dt);
  void SelectAttackAction(CStateManager& mgr, float dt);
  void SelectJumpTarget(CStateManager& mgr, float dt);
  void FindBestMiniPortals(CStateManager& mgr, float dt);
  void SetIntroJumpTarget(CStateManager& mgr, float dt);
  void SetFaceJumpTarget(CStateManager& mgr, float dt);
  void SetFaceAttackTarget(CStateManager& mgr, float dt);
  void SetFaceWaypoint(CStateManager& mgr, float dt);

private:
  // Guessed names; the choice made by SelectAttackAction.
  enum EAttackAction { kAA_None = -1, kAA_Jump, kAA_MiniPortal, kAA_Taunt };
  // Guessed names; how the current jump target is reached.
  enum EJumpMode { kJM_None = -1, kJM_Location, kJM_Waypoint };

  void UpdateBlob(CStateManager& mgr, float dt);                                   // Guessed name
  void SpawnBlobEffect(CStateManager& mgr, const TToken< CGenDescription >& desc); // Guessed name
  rstl::vector< TUniqueId > FindJumpPoints(CStateManager& mgr);                    // Guessed name
  rstl::vector< TUniqueId >
  FilterJumpPoints(CStateManager& mgr,
                   const rstl::vector< TUniqueId >& points);             // Guessed name
  const SLdrIngSpaceJumpGuardianStruct* GetCurrentStruct() const;        // Guessed name
  void SpawnShockWave(CStateManager& mgr);                               // Guessed name
  void CameraShake(CStateManager& mgr, const rstl::string& locatorName); // Guessed name
  void UpdateLight(float dt, CStateManager& mgr);                        // Guessed name
  void CreateLight(CStateManager& mgr);                                  // Guessed name
  void SetCollisionVulnerability(CStateManager& mgr,
                                 const CDamageVulnerability& vulnerability); // Guessed name
  void CollisionDamage(CStateManager& mgr, TUniqueId senderId);              // Guessed name
  void TouchDamage(CStateManager& mgr, TUniqueId senderId);                  // Guessed name
  void UpdateCollisionVulnerabilities(CStateManager& mgr);                   // Guessed name
  void SetupCollision(CStateManager& mgr);                                   // Guessed name
  void ComputeJumpVelocity(CStateManager& mgr);                              // Guessed name

  SLdrIngSpaceJumpGuardianData mData;                                         // Guessed name
  CDamageInfo mMiniPortalDamage;                                              // Guessed name
  rstl::single_ptr< CCollisionActorManager > mCollisionManager;               // Guessed name
  CPathFindSearch mPathFindSearch;                                            // Guessed name
  float mDefaultSpeed;                                                        // Guessed name
  float mDeathTimer;                                                          // Guessed name
  TUniqueId mLightId;                                                         // Guessed name
  float mLightIntensity;                                                      // Guessed name
  CSegId mHeadSeg;                                                            // Guessed name
  CSegId mCollarSeg;                                                          // Guessed name
  int mAttackAction;                                                          // Guessed name
  TUniqueId mPlayerId;                                                        // Guessed name
  TUniqueId mFaceTarget;                                                      // Guessed name
  TUniqueId mIntroWaypoint;                                                   // Guessed name
  TUniqueId mLastJumpPoint;                                                   // Guessed name
  TUniqueId mBlobId;                                                          // Guessed name
  TUniqueId mJumpTarget;                                                      // Guessed name
  float mJumpApexHeight;                                                      // Guessed name
  int mJumpMode;                                                              // Guessed name
  float mJumpTimer;                                                           // Guessed name
  float mShieldTimer;                                                         // Guessed name
  float mJumpDuration;                                                        // Guessed name
  CSfxHandle mJumpSfx;                                                        // Guessed name
  rstl::optional_object< TLockedToken< CGenDescription > > mMiniPortalEffect; // Guessed name
  int mPortalCount;                                                           // Guessed name
  CVector3f mPortalPositions[3];                                              // Guessed name
  int mPortalIndex;                                                           // Guessed name
  bool mJumping : 1;                                                          // Guessed name
  bool mHasJumpPath : 1;                                                      // Guessed name
  bool mHeardNoise : 1;                                                       // Guessed name
  bool mLastJumpToWaypoint : 1;                                               // Guessed name
  bool mDamageable : 1;                                                       // Guessed name
  bool mBlobActive : 1;                                                       // Guessed name
  bool mRender : 1;                                                           // Guessed name
};
CHECK_SIZEOF(CIngSpaceJumpGuardian, 0xa38)

#endif // _CINGSPACEJUMPGUARDIAN
