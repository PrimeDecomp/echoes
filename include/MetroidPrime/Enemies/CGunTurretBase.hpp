#ifndef _CGUNTURRETBASE
#define _CGUNTURRETBASE

#include "types.h"

#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"

#include "Collision/CCollidableAABox.hpp"
#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/TToken.hpp"

class CCollisionResponseData;
class CGenDescription;
class CGunTurretTop;

class CGunTurretBase : public CPatterned {
public:
  // Guessed names; the pole/base half of a pirate/GF gun turret.
  enum EState {
    kS_Sleep,
    kS_Patrol,
    kS_Attack,
    kS_AttackExit,
    kS_Spawn,
    kS_Withdraw,
    kS_OpenDoor,
    kS_CloseDoor,
    kS_IntoPan,
    kS_PanLeft,
    kS_PanRight,
  };

  CGunTurretBase(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                 const CTransform4f& xf, const CModelData& modelData,
                 const CPatternedInfo& patternedInfo, const CDamageInfo& attackDamage,
                 float hurtSleepDelay, float gunAimTurnSpeed, float gunLockOnTurnSpeed,
                 float minTimeBetweenAttacks, float maxTimeBetweenAttacks,
                 float minTimeBetweenShots, float maxTimeBetweenShots, float maxPitchAngleUp,
                 const CActorParameters& actorParameters, bool gunRespawns, uchar minShotsInABurst,
                 uchar maxShotsInABurst, bool isPirateTurret, CAssetId crscId,
                 CAssetId pirateProjectile, CAssetId pirateProjectileEffect, bool unknown5cf1,
                 bool unknown479d, float maxPitchAngleDown, float unknownFc03, float unknown8a35,
                 float unknownD49b, float attackDelay, float patrolDelay, float withdrawDelay,
                 float detectionHeightUp, float detectionHeightDown, float shotAngleVariance,
                 float attackLeashTime, ushort gfFireShotSfx, ushort pirateFireShotSfx,
                 ushort lockOnSfx, ushort gunPanSfx, ushort gfGunChargeSfx,
                 ushort pirateGunChargeSfx, ushort gunLowerLoopedSfx, ushort gunLowerOffSfx,
                 ushort gunRaiseLoopedSfx, ushort gunRaiseOffSfx,
                 ushort pirateGunDeathLowerLoopedSfx, ushort gfGunDeathLowerLoopedSfx,
                 ushort poleSparksSfx, float unknown80ce, float sfxFallOff, float sfxMaxDistance);

  // CEntity
  ~CGunTurretBase() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void Render(const CStateManager& mgr) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CPhysicsActor
  const CCollisionPrimitive* GetCollisionPrimitive() const override { return &mCollisionPrimitive; }

  // CPatterned
  void SetupStateMachine(CStateManager& mgr) override;
  bool IsScanVisorSelfRender() const override { return true; }
  CAABox GetScanVisorRenderBounds(const CStateManager& mgr) const override;
  void ScanVisorRender(const CStateManager& mgr, const CTransform4f& xf,
                       const CModelFlags& flags) const override;

  void Sleep(CStateManager& mgr, EStateMsg msg, float dt);
  void Spawn(CStateManager& mgr, EStateMsg msg, float dt);
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt);
  void Attack(CStateManager& mgr, EStateMsg msg, float dt);
  void Withdraw(CStateManager& mgr, EStateMsg msg, float dt);
  void Flinch(CStateManager& mgr, EStateMsg msg, float dt);
  void IntoPan(CStateManager& mgr, EStateMsg msg, float dt);
  void PanLeft(CStateManager& mgr, EStateMsg msg, float dt);
  void PanRight(CStateManager& mgr, EStateMsg msg, float dt);
  void AttackExit(CStateManager& mgr, EStateMsg msg, float dt);
  void OpenDoor(CStateManager& mgr, EStateMsg msg, float dt);
  void CloseDoor(CStateManager& mgr, EStateMsg msg, float dt);

  bool ShouldPatrol(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldAttack(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldPan(CStateManager& mgr, const CTriggerData& data) const;
  bool AnimOver(CStateManager& mgr, const CTriggerData& data) const;
  bool SpawnOver(CStateManager& mgr, const CTriggerData& data) const;
  bool WithdrawOver(CStateManager& mgr, const CTriggerData& data) const;
  bool Attacked(CStateManager& mgr, const CTriggerData& data) const;
  bool Delay(CStateManager& mgr, const CTriggerData& data) const;
  bool PatrolDelay(CStateManager& mgr, const CTriggerData& data) const;
  bool WithdrawDelay(CStateManager& mgr, const CTriggerData& data) const;
  bool GunDestroyed(CStateManager& mgr, const CTriggerData& data) const;
  bool AttackExitDone(CStateManager& mgr, const CTriggerData& data) const;

  void DestroyGun(CStateManager& mgr);
  void SetGunHit(bool hit) { mGunHit = hit; }
  float GetSfxFallOff() const { return mSfxFallOff; }
  float GetSfxMaxDistance() const { return mSfxMaxDistance; }
  CAABox GetModelBounds() const;
  int GetRenderAlpha(const CStateManager& mgr) const;

  static const char* const skConnectLocator;

private:
  void PlaySfx(const ushort& sfx, CStateManager& mgr) {
    ProcessSoundEvent(sfx, 1.f, 0, mSfxFallOff, mSfxMaxDistance, CSegId(0), 0, 0, 0.f, 20, 127,
                      GetClosestCameraDistanceSq(mgr), GetTranslation(),
                      mgr.GetNextAreaId().Value(), mgr, true);
  }
  void PlayLoopedSfx(ushort sfx, CStateManager& mgr) {
    ProcessSoundEvent(sfx | 0x80000000, 1.f, 0, mSfxFallOff, mSfxMaxDistance, CSegId(0), 0, 0, 0.f,
                      20, 127, GetClosestCameraDistanceSq(mgr), GetTranslation(),
                      mgr.GetNextAreaId().Value(), mgr, true);
  }
  float GetClosestCameraDistanceSq(CStateManager& mgr) const;
  CVector3f GetGunFirePosition(CStateManager& mgr) const;
  bool InRange(const CActor& actor, float range) const;
  void LaunchProjectile(CStateManager& mgr);
  void ResetAttack(CStateManager& mgr);
  void UpdateAttack(CStateManager& mgr, float dt);
  CActor* FindTarget(CStateManager& mgr);
  void UpdateGunOrientation(CStateManager& mgr, CSegId seg, bool aim, float dt);
  void UpdateGunPose(CStateManager& mgr, float dt);
  void LowerGun(float dt);
  void RaiseGun(float dt);
  bool InDetectionHeight(const CActor& actor, float up, float down) const;
  bool PlayerInRange(CStateManager& mgr, float range) const;

  float mDetectionRange;
  float mMaxAttackRange;
  float mMinAttackRange;
  CDamageInfo mAttackDamage;
  int mState;
  float mHurtSleepDelay;
  TUniqueId mTopId;
  mutable bool mGunDestroyed; // Cleared by the const Delay trigger once the gun respawns.
  bool mGunRespawns;
  CVector3f mTargetPos;
  CVector3f mOriginalFront;
  int mTeamIndex;
  float mGunAimTurnSpeed;
  float mGunLockOnTurnSpeed;
  CQuaternion mGunRotation;
  CQuaternion mTargetGunRotation;
  float mMinTimeBetweenAttacks;
  float mMaxTimeBetweenAttacks;
  float mTimeBetweenAttacks;
  float mMinTimeBetweenShots;
  float mMaxTimeBetweenShots;
  float mTimeBetweenShots;
  uchar mMinShotsInABurst;
  uchar mMaxShotsInABurst;
  uchar mShotsInBurst;
  float mAttackTimer;
  float mShotTimer;
  uint mShotCount;
  bool mIsPirateTurret;
  TCachedToken< CCollisionResponseData > mCrsc;
  CAssetId mPirateProjectile;
  rstl::optional_object< TLockedToken< CGenDescription > > mPirateProjectileEffect;
  CProjectileInfo mProjectileInfo;
  float x8ac_;
  float x8b0_;
  bool mFiring;
  uchar mEffectIndex;
  bool mInBurst;
  TUniqueId mHitTarget;
  bool mHitTargetValid;
  TUniqueId mLastHitTarget;
  bool x8be_;
  bool x8bf_;
  float mMaxPitchAngleUp;
  float mMaxPitchAngleDown;
  float xfc03_;
  float mRaiseSpeed;
  float mDestroyedLowerSpeed;
  float mAttackDelay;
  float mPatrolDelay;
  float mWithdrawDelay;
  float mShotAngleVariance;
  bool mGunHit;
  float mDelayTimer;
  float mLowerDuration;
  float mRaiseDuration;
  float mPanDuration;
  CDamageVulnerability mGunVulnerability;
  ushort mGFFireShotSfx;
  ushort mPirateFireShotSfx;
  ushort mLockOnSfx;
  ushort mGunPanSfx;
  ushort mGFGunChargeSfx;
  ushort mPirateGunChargeSfx;
  ushort mGunRaiseLoopedSfx;
  ushort mGunRaiseOffSfx;
  ushort mGunLowerLoopedSfx;
  ushort mGunLowerOffSfx;
  ushort mGFGunDeathLowerLoopedSfx;
  ushort mPirateGunDeathLowerLoopedSfx;
  ushort mPoleSparksSfx;
  float mDetectionHeightUp;
  float mDetectionHeightDown;
  bool mCanCharge;
  bool mCharging;
  float mChargeTime;
  bool mFirstShot;
  float mLeashTimer;
  float x95c_;
  float mAttackLeashTime;
  float mAttackLeashTimer;
  bool mTargetIsNonPlayer;
  CSfxHandle mChargeSfx;
  TUniqueId mShellWaypointId;
  CCollidableAABox mCollisionPrimitive;
  int mAdditiveAnim;
  float mMaxRaise;
  float mRaise;
  float mSfxFallOff;
  float mSfxMaxDistance;
  CAABox x9b4_;
  bool mAlert : 1;
  bool mOccluded : 1;
};
CHECK_SIZEOF(CGunTurretBase, 0x9d0)

#endif // _CGUNTURRETBASE
