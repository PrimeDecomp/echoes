#ifndef _CFLYINGPIRATE
#define _CFLYINGPIRATE

#include "MetroidPrime/Enemies/CPatterned.hpp"

#include "MetroidPrime/CBoneTracking.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CLineOfSightTracker.hpp"
#include "MetroidPrime/Enemies/CBurstFire.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"

#include "Kyoto/Particles/CElementGen.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/reserved_vector.hpp"

class CGenDescription;
class CMaterialList;

// Echoes layout from the FlyingPirate REL. Names follow Prime's CFlyingPirate where the code
// corresponds; the generated SLdrFlyingPirate property name is noted otherwise.
class CFlyingPirate : public CPatterned {
public:
  class CFlyingPirateData {
  public:
    CFlyingPirateData(float maxCoverDistance, float hearingDistance, uint type, CAssetId projectile,
                      const CDamageInfo& projectileDamage, ushort gunSfx, CAssetId missile,
                      const CDamageInfo& missileDamage, CAssetId wpsc, float knockBackDelay,
                      float flyingHeight, CAssetId rocketPackExplosion,
                      const CDamageInfo& rocketPackExplosionDamage, float spiralChance,
                      float minimumMissileTime, float missileTimeVariation, float flightThrust,
                      ushort impactSfx, ushort spiralSfx, float coverCheckChance,
                      float intraBurstShotTime, float intraBurstShotVariation,
                      CAssetId landingCloudDirt, CAssetId landingCloudDust,
                      CAssetId landingCloudSnow, ushort hurledSfx, ushort deathSfx,
                      float aggressionChance, float jumpAggressionChance,
                      float projectileHomingDistance, float unknown_0xccf05648,
                      float unknown_0x2a90f9a9, float unknown_0x9ca8f357, float unknown_0x7ac85cb6);

    float mMaxCoverDistance;
    float mHearingDistance;
    uint mType;
    CAssetId mProjectile;
    CDamageInfo mProjectileDamage;
    ushort mGunSfx;
    CAssetId mMissile;
    CDamageInfo mMissileDamage;
    CAssetId mWpsc;
    CDamageInfo mWpscDamage;
    float mKnockBackDelay;
    float mFlyingHeight;
    CAssetId mRocketPackExplosion;
    CDamageInfo mDInfo;
    float mSpiralChance;
    float mMinimumMissileTime;
    float mMissileTimeVariation;
    float mFlightThrust;
    ushort mRagDollSfx1;
    ushort mRagDollSfx2;
    float mCoverCheckChance;
    float mIntraBurstShotTime;
    float mIntraBurstShotVariation;
    CAssetId mParticleGen1;
    CAssetId mParticleGen2;
    CAssetId mParticleGen3;
    ushort mKnockBackSfx;
    ushort mDeathSfx;
    float mAggressionChance;
    float mJumpAggressionChance;
    float mProjectileHomingDistance;
    float unknown_0xccf05648;
    float unknown_0x2a90f9a9;
    float unknown_0x9ca8f357;
    float unknown_0x7ac85cb6;
  };

  CFlyingPirate(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                const CTransform4f& xf, const CModelData& modelData,
                const CActorParameters& actParms, const CPatternedInfo& pInfo,
                const CFlyingPirateData& data);

  // CEntity
  ~CFlyingPirate() override {}
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  void PreRenderAllViewports(CStateManager& mgr) override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CPhysicsActor
  void CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                    CStateManager& mgr) override;

  // CAi
  void KnockBack(CStateManager& mgr, const CKnockBackInfo& info) override;
  bool Listen(CStateManager& mgr, const CVector3f& position, EListenNoiseType type) override;
  CVector3f GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                      const CVector3f& aimPos) const override;

  // CPatterned
  void MassiveDeath(CStateManager& mgr) override;
  CProjectileInfo* ProjectileInfo() override;
  void SetupStateMachine(CStateManager& mgr) override;

  CPathFindSearch* GetSearchPath() override { return &mPathFindSearch; }
  bool IsListening() const override { return true; }
  float GetGravityConstant() const override {
    return mIsAquaPirate ? skAquaGravityConstant : skGravityConstant;
  }

  bool IsAquaPirate() const { return mIsAquaPirate; }

  // States
  void Attack(CStateManager& mgr, EStateMsg msg, float dt);
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt);
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt);
  void TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt);
  void TurnAround(CStateManager& mgr, EStateMsg msg, float dt);
  void Dodge(CStateManager& mgr, EStateMsg msg, float dt);
  void Lurk(CStateManager& mgr, EStateMsg msg, float dt);
  void Taunt(CStateManager& mgr, EStateMsg msg, float dt);
  void Dead(CStateManager& mgr, EStateMsg msg, float dt);
  void GetUp(CStateManager& mgr, EStateMsg msg, float dt);
  void GetUpNow(CStateManager& mgr, EStateMsg msg, float dt);
  void Jump(CStateManager& mgr, EStateMsg msg, float dt);
  void ProjectileAttack(CStateManager& mgr, EStateMsg msg, float dt);
  void Land(CStateManager& mgr, EStateMsg msg, float dt);
  void Walk(CStateManager& mgr, EStateMsg msg, float dt);
  void Retreat(CStateManager& mgr, EStateMsg msg, float dt);
  void Explode(CStateManager& mgr, EStateMsg msg, float dt);
  void Enraged(CStateManager& mgr, EStateMsg msg, float dt);
  void Bounce(CStateManager& mgr, EStateMsg msg, float dt);
  void Deactivate(CStateManager& mgr, EStateMsg msg, float dt);
  void ChooseWaypoint(CStateManager& mgr, EStateMsg msg, float dt);

  // Triggers
  bool HearShot(CStateManager& mgr, const CTriggerData& data) const;
  bool HearPlayer(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldSpecialAttack(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldAttack(CStateManager& mgr, const CTriggerData& data) const;
  bool LineOfSight(CStateManager& mgr, const CTriggerData& data) const;
  bool PatternOver(CStateManager& mgr, const CTriggerData& data) const;
  bool PatternShagged(CStateManager& mgr, const CTriggerData& data) const;
  bool SpotPlayer(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldDodge(CStateManager& mgr, const CTriggerData& data) const;
  bool ShotAt(CStateManager& mgr, const CTriggerData& data) const;
  bool Attacked(CStateManager& mgr, const CTriggerData& data) const;
  bool CoverCheck(CStateManager& mgr, const CTriggerData& data) const;
  bool CoverFind(CStateManager& mgr, const CTriggerData& data) const;
  bool Landed(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldMove(CStateManager& mgr, const CTriggerData& data) const;
  bool Stuck(CStateManager& mgr, const CTriggerData& data) const;
  bool DeathOver(CStateManager& mgr, const CTriggerData& data) const;
  bool AnimOver(CStateManager& mgr, const CTriggerData& data) const;
  bool InRange(CStateManager& mgr, const CTriggerData& data) const;
  bool InPosition(CStateManager& mgr, const CTriggerData& data) const;
  bool AggressionCheck(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldRetreat(CStateManager& mgr, const CTriggerData& data) const;
  bool HasAttackPattern(CStateManager& mgr, const CTriggerData& data) const;

private:
  void CheckForProjectiles(CStateManager& mgr);
  void ResetFireMissilesCheck();
  void UpdateCanFireMissiles(CStateManager& mgr);
  void CheckFireMissiles(CStateManager& mgr);
  bool FireProjectile(CStateManager& mgr, float dt);
  CVector3f GetTargetPos(CStateManager& mgr);
  pas::EStepDirection GetDodgeDirection(CStateManager& mgr, float arg);
  CVector3f AvoidActors(CStateManager& mgr);
  bool LineOfSightTest(CStateManager& mgr, const CVector3f& start, const CVector3f& end,
                       const CMaterialList& exclude);
  void UpdateLandingSmoke(CStateManager& mgr, bool active);
  void UpdateParticleEffects(CStateManager& mgr, float intensity, bool active);
  void DeliverGetUp();
  void UpdatePatrolFacing(CStateManager& mgr);
  void StartSpinToDeath(CStateManager& mgr);
  void AddToTeam(CStateManager& mgr);
  void RemoveFromTeam(CStateManager& mgr);

  static const float skGravityConstant;
  static const float skAquaGravityConstant;

  CFlyingPirateData mData;
  CProjectileInfo mGunProjectileInfo;
  CProjectileInfo mAltProjectileInfo1;
  CProjectileInfo mAltProjectileInfo2;
  TLockedToken< CGenDescription > mParticleGenDesc;
  rstl::reserved_vector< TLockedToken< CGenDescription >, 3 > mParticleGenDescs;
  rstl::reserved_vector< rstl::auto_ptr< CElementGen >, 3 > mParticleGens;
  TUniqueId mCurrentCoverPoint;
  CPathFindSearch mPathFindSearch;
  float x78c_;
  int x790_;
  float mInitialHealth;
  CSegId mHeadSegId;
  CBoneTracking mBoneTracking;
  CSegId mGunSegId;
  float x7e4_;
  TUniqueId mTargetId;
  CBurstFire mBurstFire;
  pas::EStepDirection mDodgeDirection;
  float mHeight;
  float x854_;
  float x858_;
  TUniqueId mAttackObjectId;
  float x860_;
  rstl::reserved_vector< CSegId, 2 > mMissileSegments;
  float x86c_;
  CVector3f x870_;
  CVector3f x87c_;
  float x888_;
  float mRagDollTimer;
  TUniqueId mTeamAiMgr;
  float mPitchBend;
  float x898_;
  TUniqueId mPatrolTarget;
  float x8a4_;
  CLineOfSightTracker mLineOfSightTracker;
  int mFireMissilesCheck;
  int mFireMissilesCheckInterval;
  bool mCanFireMissiles : 1;
  bool mIsFlyingPirate : 1;
  bool mIsAquaPirate : 1;
  mutable bool mHearShot : 1;
  bool mCanPatrol : 1;
  bool x6a0_28_ : 1;
  bool mCheckForProjectiles : 1;
  bool x6a0_30_ : 1;
  bool mPrevInCineCam : 1;
  bool x6a1_25_ : 1;
  bool mIsAttackingObject : 1;
  bool x6a1_27_ : 1;
  bool x6a1_28_ : 1;
  bool mIsMoving : 1;
  bool mSpinToDeath : 1;
  bool mStopped : 1;
  bool mAggressive : 1;
  bool mAggressionChecked : 1;
  bool mJetpackActive : 1;
  bool mSparksActive : 1;
  bool x6a2_28_ : 1;
  bool xbba_29_ : 1;
  bool xbba_30_ : 1;
};
CHECK_SIZEOF(CFlyingPirate, 0xBC0)

#endif // _CFLYINGPIRATE
