#ifndef _CSPORBBASE
#define _CSPORBBASE

#include "types.h"

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CPowerBombGuardianStageData.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"
#include "rstl/ownership_transfer.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class CSporbPowerBomb;
class CTendrilGen;
class CWeaponDescription;

// Guessed class: the body of a Sporb, which grabs the morph ball with a tendril and can also act
// as a power bomb guardian.
class CSporbBase : public CPatterned {
public:
  // Guessed names; the behavior the body is currently running.
  enum EState {
    kS_Sleeping = 0,
    kS_Patrolling = 1,
    kS_Attacking = 2,
    kS_Firing = 3,
    kS_ContinueFire = 4,
    kS_AttackExit = 5,
    kS_FakeDeath = 6,
    kS_FakeDead = 7,
    kS_Flailing = 8,
    kS_Spitting = 9,
    kS_SpitExit = 10,
    kS_Flinching = 11,
  };

  // Guessed names; how the attack is carried out.
  enum EAttackType {
    kAT_Invalid = -1,
    kAT_Shoot = 0,
    kAT_Grab = 1,
  };

  // Guessed names; the phase of the tendril grab.
  enum EGrabberState {
    kGS_Launch = 0,
    kGS_Extend = 1,
    kGS_Hold = 2,
    kGS_Retract = 3,
    kGS_Idle = 4,
    kGS_Attach = 5,
    kGS_Spit = 6,
  };

  CSporbBase(const TUniqueId& uid, const rstl::string& name, const CEntityInfo& info,
             const CTransform4f& xf, const CModelData& modelData,
             const CPatternedInfo& patternedInfo, const CActorParameters& actorParams,
             float minTimeBetweenAttacks, float maxTimeBetweenAttacks, float minTimeBetweenShots,
             float maxTimeBetweenShots, float shotAngleVariance, float grabberOutAcceleration,
             float grabberInAcceleration, float initialGrabberOutSpeed, uchar minShots,
             uchar maxShots, const CVector3f& attackAimOffset, float initialGrabberInSpeed,
             float grabberAttachTime, float minGrabberGrabTime, float maxGrabberGrabTime,
             float spitForce, CAssetId tendrilParticleEffect, ushort fireSound, ushort flightSound,
             ushort hitPlayerSound, ushort hitWorldSound, ushort retractSound,
             ushort retractMissedPlayerSound, ushort morphballSpitSound, ushort explosionSound,
             ushort ballEscapeSound, ushort needleTelegraphSound, ushort grabberTelegraphSound,
             float spitDamage, float grabDamage, float flailDamage, float maxGrabberGrabRange,
             float minGrabberGrabRange, bool isPowerBombGuardian,
             CAssetId powerBombProjectileParticleEffect,
             const CDamageInfo& powerBombProjectileDamage, float maxPowerBombProjectileHeight,
             float powerBombProjectileFuseTime, ushort powerBombProjectileSound,
             float powerBombEmitterMaxDistance, float powerBombEmitterDistanceComp,
             float startDamageTime, float endDamageTime,
             const rstl::vector< rstl::ownership_transfer< CPowerBombGuardianStageData > >& stages,
             float maxTimerScale, float aimPredictionTimeScale, float aimSpreadRadius,
             float damageWaitTime);

  // CEntity
  ~CSporbBase() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  void PreRenderAllViewports(CStateManager& mgr) override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  CVector3f GetScanObjectIndicatorPosition(const CStateManager& mgr) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CPatterned
  CProjectileInfo* ProjectileInfo() override;
  CDamageInfo GetContactDamage() const override;
  void SetupStateMachine(CStateManager& mgr) override;
  bool IsScanVisorSelfRender() const override { return true; }
  CAABox GetScanVisorRenderBounds(const CStateManager& mgr) const override;
  void ScanVisorRender(const CStateManager& mgr, const CTransform4f& xf,
                       const CModelFlags& flags) const override;

  // CSporbBase
  void Sleep(CStateManager& mgr, EStateMsg msg, float dt);
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt);
  void Attack(CStateManager& mgr, EStateMsg msg, float dt);
  void WakeUp(CStateManager& mgr, EStateMsg msg, float dt);
  void GoToSleep(CStateManager& mgr, EStateMsg msg, float dt);
  void Fire(CStateManager& mgr, EStateMsg msg, float dt);
  void ContinueFire(CStateManager& mgr, EStateMsg msg, float dt);
  void AttackExit(CStateManager& mgr, EStateMsg msg, float dt);
  void FakeDeath(CStateManager& mgr, EStateMsg msg, float dt);
  void FakeDead(CStateManager& mgr, EStateMsg msg, float dt);
  void Flail(CStateManager& mgr, EStateMsg msg, float dt);
  void ContinueFlail(CStateManager& mgr, EStateMsg msg, float dt);
  void Spit(CStateManager& mgr, EStateMsg msg, float dt);
  void ContinueSpit(CStateManager& mgr, EStateMsg msg, float dt);
  void SpitExit(CStateManager& mgr, EStateMsg msg, float dt);
  void Flinch(CStateManager& mgr, EStateMsg msg, float dt);

  bool ShouldPatrol(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldAttack(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldFire(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldSpit(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldFlail(CStateManager& mgr, const CTriggerData& data) const;
  bool AnimOver(CStateManager& mgr, const CTriggerData& data) const;
  bool AttackOver(CStateManager& mgr, const CTriggerData& data) const;
  bool SpitOver(CStateManager& mgr, const CTriggerData& data) const;
  bool AttackExitOver(CStateManager& mgr, const CTriggerData& data) const;

  CAABox GetModelBounds() const;

  static const char* const skConnectLocator;

private:
  // Guessed name; a looping power bomb sound that follows its projectile.
  struct SPowerBombEmitter {
    SPowerBombEmitter() : mHandle(), mProjectileId(kInvalidUniqueId) {}

    CSfxHandle mHandle;
    TUniqueId mProjectileId;
  };

  void SetStage(uchar stage);
  float GetGravity() const;
  float PredictFlightTime(float height, float gravity, const CVector3f& from,
                          const CVector3f& to) const;
  CVector3f PredictAimOffset(float height, CStateManager& mgr, const CVector3f& from) const;
  void ShootPowerBomb(float height, const CVector3f& from, CStateManager& mgr, int count,
                      const CVector3f& to);
  CSporbPowerBomb* CreatePowerBomb(CStateManager& mgr, const TToken< CWeaponDescription >& token,
                                   const CTransform4f& xf, const CDamageInfo& damage);
  CVector3f GetTopAttachPosition(const CStateManager& mgr) const;
  static CVector3f GetLeanDirection(float forward, float back, float left, float right);
  void ClearTendrilParticles();
  void StopTendril();
  CVector3f PickTargetPosition(CStateManager& mgr, bool trackPlayer);
  void SpitPlayer(CStateManager& mgr, float force);
  CVector3f PickWaypointPosition(CStateManager& mgr);
  void ResetAfterGrab(CStateManager& mgr, const CVector3f& position);
  CTransform4f BuildLookAtTransform(CStateManager& mgr, const CVector3f& from);
  void UpdateAim(CStateManager& mgr);
  void DecayAim();
  CVector3f GetTopHeadPosition(CStateManager& mgr);
  void ResetShotTimers(CStateManager& mgr);
  void UpdateShooting(float dt, CStateManager& mgr);
  CVector3f GetLocatorPosition() const;
  void AttachPlayer(CStateManager& mgr);
  void UpdateGrabber(float dt, CStateManager& mgr);
  void UpdateAttack(float dt, CStateManager& mgr);
  float GetAimAngle(const CStateManager& mgr) const;
  bool IsWithinRange(float range, const CActor& other) const;
  void FindTargetPlayer(CStateManager& mgr);

  EState mState;                   // Guessed name
  TUniqueId mTopId;                // Guessed name
  TUniqueId mProjectileId;         // Guessed name
  int mForwardLeanAnim;            // Guessed name
  int mLeftLeanAnim;               // Guessed name
  int mRightLeanAnim;              // Guessed name
  int mBackLeanAnim;               // Guessed name
  int mFlinchAnim;                 // Guessed name
  float mDetectionDistance;        // Guessed name
  float mMaxAttackDistance;        // Guessed name
  float mMinAttackDistance;        // Guessed name
  float x7e8_;                     // Unresolved; only initialized
  float mForwardLean;              // Guessed name
  float mBackLean;                 // Guessed name
  float mLeftLean;                 // Guessed name
  float mRightLean;                // Guessed name
  float x7fc_;                     // Unresolved; only initialized
  TUniqueId mTargetPlayerId;       // Guessed name
  CDamageInfo mSpitDamage;         // Guessed name
  CDamageInfo mGrabDamage;         // Guessed name
  CDamageInfo mFlailDamage;        // Guessed name
  float mShotTimer;                // Guessed name
  float x85c_;                     // Unresolved; only reset
  float x860_;                     // Unresolved; only reset
  float mTimeBetweenAttacks;       // Guessed name
  float mTimeBetweenShots;         // Guessed name
  uchar mShotsInBurst;             // Guessed name
  float mMinTimeBetweenAttacks;    // Guessed name
  float mMaxTimeBetweenAttacks;    // Guessed name
  float mMinTimeBetweenShots;      // Guessed name
  float mMaxTimeBetweenShots;      // Guessed name
  float x880_;                     // Unresolved; never accessed
  uchar mMinShots;                 // Guessed name
  uchar mMaxShots;                 // Guessed name
  uchar mShotsFired;               // Guessed name
  bool mShooting;                  // Guessed name
  bool mReadyToFire;               // Guessed name
  bool x889_;                      // Unresolved; only initialized
  bool x88a_;                      // Unresolved; only reset
  bool x88b_;                      // Unresolved; only reset
  bool x88c_;                      // Unresolved; only reset
  bool mGrabFinished;              // Guessed name
  bool mBallEscaped;               // Guessed name
  ushort x890_;                    // Unresolved; only initialized
  ushort mNeedleSound;             // Guessed name
  float mTimeSinceDetection;       // Guessed name
  TEditorId mNeedleSpawnerId;      // Guessed name
  float mShotAngleVariance;        // Guessed name
  CVector3f mAttackAimOffset;      // Guessed name
  int mTendrilParticleCount;       // Guessed name
  float mGrabberOutSpeed;          // Guessed name
  float mGrabberInSpeed;           // Guessed name
  float mGrabberOutAcceleration;   // Guessed name
  float mGrabberInAcceleration;    // Guessed name
  CVector3f mRetractPosition;      // Guessed name
  float mHoldTimer;                // Guessed name
  float mHoldDuration;             // Guessed name
  float mAttachTimer;              // Guessed name
  float mAttachDuration;           // Guessed name
  float mInitialGrabberOutSpeed;   // Guessed name
  float mInitialGrabberInSpeed;    // Guessed name
  float mMinGrabTime;              // Guessed name
  float mMaxGrabTime;              // Guessed name
  float mGrabTime;                 // Guessed name
  float mSpitTimer;                // Guessed name
  float mSpitForce;                // Guessed name
  bool x8f8_;                      // Unresolved; set while the ball is held
  float mReleaseTimer;             // Guessed name
  float mMaxGrabRange;             // Guessed name
  float mMinGrabRange;             // Guessed name
  bool mCanSpit;                   // Guessed name
  bool mAlerted;                   // Guessed name
  EAttackType mAttackType;         // Guessed name
  EGrabberState mGrabberState;     // Guessed name
  CVector3f mAimTarget;            // Guessed name
  CQuaternion mProjectileRotation; // Guessed name
  rstl::single_ptr< TCachedToken< CGenDescription > > mTendrilDesc;                // Guessed name
  rstl::single_ptr< CTendrilGen > mTendrilGen;                                     // Guessed name
  ushort mFireSound;                                                               // Guessed name
  ushort mFlightSound;                                                             // Guessed name
  ushort mHitPlayerSound;                                                          // Guessed name
  ushort mHitWorldSound;                                                           // Guessed name
  ushort mRetractSound;                                                            // Guessed name
  ushort mRetractMissedPlayerSound;                                                // Guessed name
  ushort mMorphballSpitSound;                                                      // Guessed name
  ushort mExplosionSound;                                                          // Guessed name
  ushort mBallEscapeSound;                                                         // Guessed name
  ushort mNeedleTelegraphSound;                                                    // Guessed name
  ushort mGrabberTelegraphSound;                                                   // Guessed name
  bool mFiring;                                                                    // Guessed name
  bool mTopHit;                                                                    // Guessed name
  CVector3f mTopAttachPosition;                                                    // Guessed name
  float mTopBlend;                                                                 // Guessed name
  CVector3f mProjectilePosition;                                                   // Guessed name
  float mSavedForwardLean;                                                         // Guessed name
  float mSavedBackLean;                                                            // Guessed name
  float mSavedLeftLean;                                                            // Guessed name
  float mSavedRightLean;                                                           // Guessed name
  CDamageVulnerability mSavedVulnerability;                                        // Guessed name
  bool mIsPowerBombGuardian;                                                       // Guessed name
  CProjectileInfo mProjectileInfo;                                                 // Guessed name
  float mMaxPowerBombHeight;                                                       // Guessed name
  float mPowerBombFuseTime;                                                        // Guessed name
  rstl::vector< TUniqueId > mWaypointIds;                                          // Guessed name
  ushort mPowerBombSound;                                                          // Guessed name
  float mPowerBombEmitterMaxDistance;                                              // Guessed name
  float mPowerBombEmitterDistanceComp;                                             // Guessed name
  float mStartDamageTime;                                                          // Guessed name
  float mEndDamageTime;                                                            // Guessed name
  uchar mStageIndex;                                                               // Guessed name
  float mMaxTimerScale;                                                            // Guessed name
  float mTimerScale;                                                               // Guessed name
  uchar mAttacksSinceDoubleShot;                                                   // Guessed name
  uchar mAttacksPerDoubleShot;                                                     // Guessed name
  float mDamageWaitTime;                                                           // Guessed name
  uchar mPowerBombsFired;                                                          // Guessed name
  rstl::vector< rstl::ownership_transfer< CPowerBombGuardianStageData > > mStages; // Guessed name
  rstl::reserved_vector< SPowerBombEmitter, 4 > mPowerBombEmitters;                // Guessed name
  CVector3f mPowerBombAimPosition;                                                 // Guessed name
  float mAimPredictionTimeScale;                                                   // Guessed name
  float mAimSpreadRadius;                                                          // Guessed name
  pas::EGenerateType mFireGenerateType;                                            // Guessed name
  CVector3f mGrabberPosition;                                                      // Guessed name
  bool mActive : 1;                                                                // Guessed name
  bool mAimLocked : 1;                                                             // Guessed name
  bool mTimerScaleMaxed : 1;                                                       // Guessed name
};
CHECK_SIZEOF(CSporbBase, 0xa78)

CEntity* LoadSporbBase(CStateManager& mgr, CInputStream& input, CEntityInfo& info);

#endif // _CSPORBBASE
