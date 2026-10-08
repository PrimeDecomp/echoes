#ifndef _CDARKTROOPER
#define _CDARKTROOPER

#include "types.h"

#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CBoneTracking.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CLineOfSightTracker.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

class CPirateRagDoll;
class CScannableObjectInfo;
class CPFArea;

// Guessed class: a Dark Trooper, a bipedal Ing-possessed trooper that melees, fires small shots
// or missiles, and can be a sleeper that rises when alerted.
class CDarkTrooper : public CPatterned {
public:
  CDarkTrooper(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
               const CTransform4f& xf, const CModelData& modelData,
               const CPatternedInfo& patternedInfo, CAssetId stateMachine2, float meleeMinRange,
               float meleeMaxRange, float attackCooldown, float rangedMinRange,
               float rangedMaxRange, bool flotsam, bool avoidDownFrames, int initialAnim,
               const CDamageInfo& meleeDamage, CAssetId rangedProjectile,
               const CDamageInfo& rangedDamage, bool firesMissiles, CAssetId missileProjectile,
               const CDamageInfo& missileDamage, ushort ragdollImpactSound,
               CAssetId scannableInfoWhenAttacking, const CActorParameters& actorParams);

  // CEntity
  ~CDarkTrooper() override;
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void Render(const CStateManager& mgr) const override;
  void PreRenderAllViewports(CStateManager& mgr) override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;
  CScannableObjectInfo* GetScannableObjectInfo() const override;

  // CAi
  void KnockBack(CStateManager& mgr, const CKnockBackInfo& info) override;

  // CPatterned
  CProjectileInfo* ProjectileInfo() override;
  CPathFindSearch* GetSearchPath() override { return &mPathFindSearch; }
  CDamageInfo GetContactDamage() const override;
  CRagDoll* GetRagDoll() const override;
  bool CanBeUnPossessed(CStateManager& mgr) const override;
  CVector3f GetIngSnatchingNormal(float t) const override;
  CVector3f GetIngSnatchingPoint(float t) const override;

  // CDarkTrooper
  virtual bool Alerted(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool BreakMissileAttack(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool BreakSmallShotAttack(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool CanMeleeAttack(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool CanMissileAttack(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool CanSmallShotAttack(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ClearLineOfFire(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool DonePausing(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool FacingPlayer(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool InMeleeRange(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool InMissileRange(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool InSmallShotRange(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool IsSleeper(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ReadyToRumble(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ShouldPause(CStateManager& mgr, const CTriggerData& data) const;
  virtual void Dead(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void MeleeAttack(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void MissileAttack(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Null(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Pause(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Pursue(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Rise(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Sleep(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void SmallShotAttack(CStateManager& mgr, EStateMsg msg, float dt);

private:
  // Guessed name: the projectile fired by a ranged attack.
  class CAttackProjectile {
  public:
    CAttackProjectile(CAssetId projectile, const CDamageInfo& damage) {
      if (projectile != kInvalidAssetId) {
        mProjectile = CProjectileInfo(projectile, damage);
        mProjectile->Token().Lock();
      }
    }

    rstl::optional_object< CProjectileInfo > mProjectile; // Guessed name
  };

  // Guessed name: the bookkeeping shared by the small shot and missile attacks.
  class CRangedAttack : public CAttackProjectile {
  public:
    CRangedAttack(float cooldown, CAssetId projectile, const CDamageInfo& damage)
    : CAttackProjectile(projectile, damage)
    , mShotsFired(0)
    , mShotLimit(0)
    , mNextAttackTime(-1000.f)
    , mCooldown(cooldown)
    , mFinished(false)
    , mBreakAttack(false) {}

    void Reset() {
      mShotsFired = 0;
      mFinished = mBreakAttack = false;
      mShotLimit = 0;
    }

    int mShotsFired;       // Guessed name
    int mShotLimit;        // Guessed name
    float mNextAttackTime; // Guessed name
    float mCooldown;       // Guessed name
    bool mFinished : 1;    // Guessed name
    bool mBreakAttack : 1; // Guessed name
  };

  class CSmallShotAttack : public CRangedAttack {
  public:
    CSmallShotAttack(float cooldown, CAssetId projectile, const CDamageInfo& damage)
    : CRangedAttack(cooldown, projectile, damage) {}
  };

  class CMissileAttack : public CRangedAttack {
  public:
    CMissileAttack(float cooldown, CAssetId projectile, const CDamageInfo& damage)
    : CRangedAttack(cooldown, projectile, damage) {}
  };

  // Guessed name
  class CMeleeAttack {
  public:
    CMeleeAttack(float minRange, float maxRange, const CDamageInfo& damage)
    : mMinRange(minRange), mMaxRange(maxRange), mDamage(damage), mNextAttackTime(-1000.f) {}

    float mMinRange;       // Guessed name
    float mMaxRange;       // Guessed name
    CDamageInfo mDamage;   // Guessed name
    float mNextAttackTime; // Guessed name
  };

  // Guessed name: owns the ragdoll created when the trooper dies.
  class CRagDollHolder {
  public:
    CRagDollHolder(bool flotsam, ushort impactSound)
    : mRagDoll(nullptr), mFlotsam(flotsam), mLastDamage(0.f), mImpactSound(impactSound) {}

    rstl::single_ptr< CPirateRagDoll > mRagDoll; // Guessed name
    bool mFlotsam : 1;                           // Guessed name
    float mLastDamage;                           // Guessed name
    ushort mImpactSound;                         // Guessed name
  };

  void SetupStateMachineHelper(CStateManager& mgr);                             // Guessed name
  void ThinkRagDoll(float dt, CStateManager& mgr);                              // Guessed name
  void CreateRagDoll(CStateManager& mgr);                                       // Guessed name
  void ThinkBoneTracking(float dt, CStateManager& mgr);                         // Guessed name
  void PreRenderBoneTracking(CStateManager& mgr);                               // Guessed name
  bool ShouldTrack() const;                                                     // Guessed name
  bool InRange(CStateManager& mgr, float minRange, float maxRange) const;       // Guessed name
  CVector3f GetTargetPosition(CStateManager& mgr) const;                        // Guessed name
  CPFArea* GetPathArea(CStateManager& mgr) const;                               // Guessed name
  void SetPathArea(CStateManager& mgr);                                         // Guessed name
  void MarkPathRegion(CStateManager& mgr);                                      // Guessed name
  void UnmarkPathRegion(CStateManager& mgr);                                    // Guessed name
  void MoveToTarget(CStateManager& mgr, float dt, const CVector3f& target);     // Guessed name
  void PushPlayer(CStateManager& mgr, float impulseScale, float verticalSpeed); // Guessed name
  void SetAttackState(int state, EStateMsg msg);                                // Guessed name
  void ApplyMeleeDamage(CStateManager& mgr);                                    // Guessed name
  void FireMissile(CStateManager& mgr);                                         // Guessed name
  void FireSmallShot(CStateManager& mgr);                                       // Guessed name
  void EndMissileAttack();                                                      // Guessed name
  void EndSmallShotAttack();                                                    // Guessed name
  template < typename T >
  void DeliverCommand(EStateMsg msg, pas::EAnimationState state, const T& cmd); // Guessed name

  CPathFindSearch mPathFindSearch;
  float mElapsedTime;                                                       // Guessed name
  CVector3f mPathDestination;                                               // Guessed name
  CTransform4f mGunTransform;                                               // Guessed name
  CAABox mSavedBounds;                                                      // Guessed name
  rstl::single_ptr< TLockedToken< CScannableObjectInfo > > mAttackScanInfo; // Guessed name
  CVector3f mHeadDirection;                                                 // Guessed name
  CVector3f mRootPosition;                                                  // Guessed name
  float mDefaultTurnSpeed;                                                  // Guessed name
  int mMarkedRegionIndex;                                                   // Guessed name
  bool mAlerted : 1;                                                        // Guessed name
  bool mFiresMissiles : 1;                                                  // Guessed name
  bool mUpdatingAnimation : 1;                                              // Guessed name
  bool mAvoidDownFrames : 1;                                                // Guessed name
  CSegId mGunSegId;                                                         // Guessed name
  CSegId mHeadSegId;                                                        // Guessed name
  CSegId mRootSegId;                                                        // Guessed name
  CBoneTracking mBoneTracking;                                              // Guessed name
  CLineOfSightTracker mLineOfSightTracker;                                  // Guessed name
  rstl::reserved_vector< int, 4 > mAttackHistory;                           // Guessed name
  int mPreviousAttackState;                                                 // Guessed name
  int mAttackState;                                                         // Guessed name
  rstl::optional_object< CToken > mStateMachine2;                           // Guessed name
  CRagDollHolder mRagDollHolder;                                            // Guessed name
  CMeleeAttack mMeleeAttack;                                                // Guessed name
  float mRangedAttackMinRange;                                              // Guessed name
  float mRangedAttackMaxRange;                                              // Guessed name
  CSmallShotAttack mSmallShotAttack;                                        // Guessed name
  CMissileAttack mMissileAttack;                                            // Guessed name
  float mPauseEndTime;                                                      // Guessed name
  bool mShouldPause : 1;                                                    // Guessed name
  pas::ELocomotionType mLocomotionType;                                     // Guessed name
  bool mIsSleeper : 1;                                                      // Guessed name
  bool mTransitioning : 1;                                                  // Guessed name
};
CHECK_SIZEOF(CDarkTrooper, 0xaa0)

#endif // _CDARKTROOPER
