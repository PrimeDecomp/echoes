#ifndef _CPARASITE
#define _CPARASITE

#include "types.h"

#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CLineOfSightTracker.hpp"
#include "MetroidPrime/Enemies/CWallCrawler.hpp"

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class CCollisionActorManager;
class CHealthInfo;
class CSkinnedModel;

// Parasite REL. Prime's CParasite derives from CWallWalker; in Echoes the base is the WallCrawler REL
// class, and the same class drives the Parasite, Brizgee (ice zoomer path) and Crystallite loaders.
class CParasite : public CWallCrawler {
public:
  // Guessed names. Values of CWallCrawler::EType handled by this class; Prime's CWallWalker::EType
  // has the first four.
  enum EParasiteType {
    kPT_Parasite = 0,
    kPT_Oculus = 1,
    kPT_Geemer = 2,
    kPT_IceZoomer = 3,
    kPT_Crystallite = 10,
  };

  class CRepulsor {
  public:
    CRepulsor(CVector3f pos, float radius) : mPos(pos), mRadius(radius) {}

    const CVector3f& GetPos() const { return mPos; }
    float GetRadius() const { return mRadius; }

  private:
    CVector3f mPos;
    float mRadius;
  };

  CParasite(TUniqueId uid, const rstl::string& name, EFlavorType flavor, CEntityInfo& info,
            const CTransform4f& xf, const CModelData& mData, const CPatternedInfo& pInfo,
            EBodyType bodyType, float maxTelegraphReactDist, float advanceWpRadius, float f3,
            float alignAngVel, float f5, float stuckTimeThreshold, float collisionCloseMargin,
            float parasiteSearchRadius, float parasiteSeparationDist,
            float parasiteSeparationWeight, float parasiteAlignmentWeight,
            float parasiteCohesionWeight, float destinationSeekWeight, float forwardMoveWeight,
            float playerSeparationDist, float playerSeparationWeight,
            float playerObstructionMinDist, float haltDelay, bool disableMove, EParasiteType type,
            const CDamageVulnerability& dVuln, const CDamageInfo& dInfo, ushort haltSfx,
            ushort getUpSfx, ushort crouchSfx, CAssetId modelRes, CAssetId skinRes,
            float iceZoomerJointHP, float wallWalkerF6, const CDamageInfo& dInfo2,
            const CActorParameters& aParams);

  // CEntity
  ~CParasite() override;
  CEntity* TypesMatch(int typeId) const override;
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void Render(const CStateManager& mgr) const override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f& position,
                                                         const CVector3f& direction,
                                                         const CWeaponMode& mode,
                                                         int attributes) const override;

  // CPhysicsActor
  void CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                    CStateManager& mgr) override;

  // CAi
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;

  // CPatterned
  void ThinkAboutMove(float dt) override;
  void MassiveDeath(CStateManager& mgr) override;
  void MassiveFrozenDeath(CStateManager& mgr) override;
  CDamageInfo GetContactDamage() const override;
  void SetupStateMachine(CStateManager& mgr) override;
  bool IsOnGround() const override;

  // CParasite
  virtual CAdvancementDeltas UpdateWalkerAnimation(CStateManager& mgr, float dt);

  // States
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt);
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt);
  void TargetPlayer(CStateManager& mgr, EStateMsg msg, float dt);
  void TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt);
  void Halt(CStateManager& mgr, EStateMsg msg, float dt);
  void Run(CStateManager& mgr, EStateMsg msg, float dt);
  void Generate(CStateManager& mgr, EStateMsg msg, float dt);
  void Deactivate(CStateManager& mgr, EStateMsg msg, float dt);
  void Attack(CStateManager& mgr, EStateMsg msg, float dt);
  void Crouch(CStateManager& mgr, EStateMsg msg, float dt);
  void GetUp(CStateManager& mgr, EStateMsg msg, float dt);
  void TelegraphAttack(CStateManager& mgr, EStateMsg msg, float dt);
  void Jump(CStateManager& mgr, EStateMsg msg, float dt);
  void Retreat(CStateManager& mgr, EStateMsg msg, float dt);

  // Triggers
  bool AnimOver(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldAttack(CStateManager& mgr, const CTriggerData& data) const;
  bool HitSomething(CStateManager& mgr, const CTriggerData& data) const;
  bool Stuck(CStateManager& mgr, const CTriggerData& data) const;
  bool Landed(CStateManager& mgr, const CTriggerData& data) const;
  bool AttackOver(CStateManager& mgr, const CTriggerData& data) const;
  bool ShotAt(CStateManager& mgr, const CTriggerData& data) const;
  bool PatrolPathOver(CStateManager& mgr, const CTriggerData& data) const;

private:
  bool CloseToWall(CStateManager& mgr) const;
  void FaceTarget(CVector3f target);
  TUniqueId RecursiveFindClosestWayPoint(CStateManager& mgr, TUniqueId id, float& dist) const;
  TUniqueId GetClosestWaypointForState(EScriptObjectState state, CStateManager& mgr) const;
  void UpdatePFDestination(CStateManager& mgr);
  void DoFlockingBehavior(CStateManager& mgr);
  void SetupIceZoomerCollision(CStateManager& mgr);
  void SetupIceZoomerVulnerability(CStateManager& mgr, const CDamageVulnerability& dVuln,
                                   const CHealthInfo& hInfo);
  void AddDoorRepulsors(CStateManager& mgr);
  void UpdateCollisionActors(float dt, CStateManager& mgr);
  void DestroyActorManager(CStateManager& mgr);
  void UpdateJumpVelocity();
  void UpdateShell(CStateManager& mgr, int state);

  // WallCrawler REL static (fn_83_1EB8, Prime CWallWalker::ProjectVectorToPlane). Declared here only
  // until CWallCrawler.hpp declares it; then this line goes and the calls resolve to the base.
  static CVector3f ProjectVectorToPlane(const CVector3f& vec, const CVector3f& planeDir);

  static float skAttackTime;
  static float skAttackVelocity;
  static float skRetreatTime;
  static float skRetreatVelocity;

  rstl::vector< CRepulsor > mDoorRepulsors;
  int mStateProgress;
  CVector3f x87c_;
  CVector3f mTargetPos;
  float mActiveSpeed;
  float mTelegraphRemTime;
  float mStuckTime;
  float x8a0_;
  CVector3f mLastStuckPos;
  rstl::single_ptr< CCollisionActorManager > mCollisionActorManager;
  rstl::single_ptr< TLockedToken< CSkinnedModel > > mExtraModel;
  CVector3f mParasiteSeparationMove;
  CVector3f mParasiteCohesionMove;
  CVector3f mParasiteAlignmentMove;
  CDamageVulnerability mOculusHaltDVuln;
  CDamageInfo mOculusHaltDInfo;
  CDamageInfo x928_;
  float x944_;
  float mMaxTelegraphReactDist;
  float x94c_;
  float x950_;
  float x954_;
  float mStuckTimeThreshold;
  float mParasiteSearchRadius;
  float mParasiteSeparationDist;
  float mParasiteSeparationWeight;
  float mParasiteAlignmentWeight;
  float mParasiteCohesionWeight;
  float mDestinationSeekWeight;
  float mForwardMoveWeight;
  float mPlayerSeparationDist;
  float mPlayerSeparationWeight;
  float mUnmorphedRadius;
  float x984_;
  float mHaltDelay;
  float mIceZoomerJointHP;
  CVector3f x990_;
  CVector3f x99c_;
  CVector3f x9a8_;
  ushort mHaltSfx;
  ushort mGetUpSfx;
  ushort mCrouchSfx;
  TUniqueId x9ba_;
  TUniqueId x9bc_;
  int x9c0_;
  int x9c4_;
  bool mReceivedTelegraph : 1;
  bool mJumpVelDirty : 1;
  bool x9c8_26_ : 1;
  bool mLanded : 1;
  bool mOnGround : 1;
  bool x9c8_29_ : 1;
  bool mAttackOver : 1;
  bool x9c8_31_ : 1;
  bool mHalted : 1;
  bool mVulnerable : 1;
  bool mOculusShotAt : 1;
  bool mInJump : 1;
  CLineOfSightTracker mLineOfSight;
};
CHECK_SIZEOF(CParasite, 0xa10)

#endif // _CPARASITE
