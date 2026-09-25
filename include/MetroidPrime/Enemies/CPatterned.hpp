#ifndef _CPATTERNED
#define _CPATTERNED

#include "MetroidPrime/CSteeringBehaviors.hpp"
#include "MetroidPrime/Enemies/CAi.hpp"
#include "MetroidPrime/Enemies/CAiKnockBackMgr.hpp"
#include "MetroidPrime/Enemies/CAnimationState.hpp"
#include "MetroidPrime/Enemies/CPathFindNavigation.hpp"
#include "MetroidPrime/Enemies/CPatternedInfo.hpp"
#include "MetroidPrime/Enemies/CWaypointNavigation.hpp"
#include "MetroidPrime/TStateMachineState.hpp"

#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Animation/CharacterCommon.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "rstl/single_ptr.hpp"

class CBodyController;
class CElectricDescription;
class CEnergyProjectile;
class CGenDescription;
class CImpactVisorEffect;
class CPathFindSearch;
class CPatterned;
class CProjectileInfo;
class CScriptCoverPoint;
class CSkinnedModel;
class CPASAnimParmData;
class CCharAnimTime;

enum EPatternedAI {
  kPAI_DarkSamus = 7,
};

template <>
struct TStateMachineFunctionTypes< CPatterned > {
  typedef void (CPatterned::*StateFunc)(CStateManager&, EStateMsg, float);
  typedef bool (CPatterned::*TriggerFunc)(CStateManager&, const CTriggerData&) const;
};

class CPatterned : public CAi {
public:
  enum EFlavorType { kFT_Zero, kFT_One };
  enum EMovementType { kMT_Ground, kMT_Flyer };
  enum EColliderType { kCT_Zero, kCT_One };

  typedef TStateMachineStateBase< CPatterned > StateMachine;

  static const float skDamageHitTime;
  static const float skActorApproachDistance;
  static const CColor skDamageColor;
  static const CColor skHitsWithoutDamageColor;

  CPatterned(EPatternedAI character, TUniqueId uid, const rstl::string& name, EFlavorType flavor,
             const CEntityInfo& info, const CTransform4f& xf, const CModelData& modelData,
             const CPatternedInfo& patternedInfo, EMovementType movement, EColliderType collider,
             EBodyType body, const CActorParameters& params);
  ~CPatterned() override;

  CEntity* TypesMatch(int typeId) const override;
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  bool CanRenderUnsorted(const CStateManager& mgr) const override;
  void PreRenderAllViewports(CStateManager& mgr) override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  const CDamageVulnerability* GetDamageVulnerability(const CVector3f& position,
                                                     const CVector3f& direction,
                                                     const CDamageInfo& damage) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override { return GetBoundingBox(); }
  void Touch(CActor& actor, CStateManager& mgr) override;
  CVector3f GetOrbitPosition(const CStateManager& mgr) const override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f& position,
                                                         const CVector3f& direction,
                                                         const CWeaponMode& mode,
                                                         int attributes) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;
  CScannableObjectInfo* GetScannableObjectInfo() const override;
  void CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                    CStateManager& mgr) override;
  bool IsOnStaticGround() const override;
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;
  void KnockBack(CStateManager& mgr, const CKnockBackInfo& info) override;
  void TakeDamage(const CVector3f&, float) override { mDamageCooldownTimer = skDamageHitTime; }

  virtual void RenderSystemsToBeDrawnFirst(const CStateManager& mgr, uint mask, uint target) const;
  virtual void RenderSystemsToBeDrawnLast(const CStateManager& mgr, uint mask, uint target) const;
  virtual void Freeze(CStateManager& mgr, const CVector3f& position, CUnitVector3f direction,
                      float duration, float intoFreezeDuration);
  virtual void ThinkAboutMove(float dt);
  virtual uchar GetModelAlphau8(const CStateManager&) const { return mColor.GetAlphau8(); }
  virtual void Burn(CStateManager& mgr, float duration, float damage);
  virtual void Shock(CStateManager& mgr, float duration, float damage);
  virtual void MassiveDeath(CStateManager& mgr);
  virtual void MassiveFrozenDeath(CStateManager& mgr);
  virtual CProjectileInfo* ProjectileInfo() { return nullptr; }
  virtual CPathFindSearch* GetSearchPath() { return nullptr; }
  virtual void* fn_80073c7c() { return nullptr; }
  virtual CDamageInfo GetContactDamage() const;
  virtual void UpdateHitDamageTime(float dt);
  virtual void SetupStateMachine(CStateManager& mgr);
  virtual bool fn_800358e0() const { return false; }
  virtual bool CanBeIngPossessed(CStateManager& mgr) const;
  virtual bool CanBeUnPossessed(CStateManager& mgr) const;
  virtual void SetIngPossessed(bool possessed, CStateManager& mgr);
  virtual void SetIngPossessed(bool possessed, float duration, CStateManager& mgr);
  virtual void SetAttackTarget(CStateManager& mgr, TUniqueId target);
  virtual TUniqueId GetAttackTarget() const { return kInvalidUniqueId; }
  virtual bool IsOnGround() const { return mOnGround; }
  virtual float GetGravityConstant() const { return CPhysicsActor::GravityConstant(); }
  virtual bool IsScanVisorSelfRender() const { return false; }
  virtual CAABox GetScanVisorRenderBounds(const CStateManager&) const;
  virtual void ScanVisorRender(const CStateManager&, const CTransform4f&,
                               const CModelFlags&) const {}
  virtual const rstl::optional_object< TCachedToken< CGenDescription > >&
  GetDeathExplosionParticle() const {
    return mDeathExplosionParticle;
  }
  virtual float GetDeathTimeScale() const;
  virtual bool TryToBeCaptured(CStateManager& mgr);
  virtual void IssueDeathBodyCommand(CStateManager& mgr, const CVector3f& direction);
  virtual float GetFadeOnDeathTime() const;
  virtual CVector3f GetIngSnatchingNormal(float t) const;
  virtual CVector3f GetIngSnatchingPoint(float t) const;
  virtual float GetIngSnatchingModelOverlapSize() const;
  virtual void RenderIngSnatchingTransition(const CStateManager& mgr) const;

  void BuildBodyController(EBodyType body);
  void SetDestPos(const CVector3f& position);
  CVector3f GetGunEyePos() const;
  bool ApplyBoneTracking() const;
  float GetAnimationDistance(const CPASAnimParmData& params) const;
  float GetAnimationDuration(const CPASAnimParmData& params) const;
  void SetupPlayerCollision(bool enabled);
  CScriptCoverPoint* GetCoverPoint(CStateManager& mgr, TUniqueId id) const;
  void ReleaseCoverPoint(CStateManager& mgr, TUniqueId& id, bool retainCooldown);
  void SetCoverPoint(CScriptCoverPoint* point, TUniqueId& id);
  void CreateXDamageParticles(CStateManager& mgr) const;
  void UpdateAlphaDelta(CStateManager& mgr, float dt);
  void InitializeStateMachine(CStateManager& mgr);
  void DeathDelete(CStateManager& mgr);
  CTransform4f GetLctrTransform(const rstl::string& name) const;
  CTransform4f GetLctrTransform(const CSegId& id) const;
  bool IsBeingSnatched() const;
  bool IsIngPossessed() const;
  void UpdateIngPossession(float dt);
  CEnergyProjectile* LaunchProjectile(const CTransform4f& xf, CStateManager& mgr,
                                      int maxProjectiles, uint attributes, bool homing,
                                      const CImpactVisorEffect& visorEffect,
                                      const CVector3f& scale);
  CCharAnimTime GetTimeOfUserEventForAnimation(const CPASAnimParmData& params,
                                               EUserEventType event) const;
  int GetNumUserEventsForAnimation(const CPASAnimParmData& params, EUserEventType event) const;
  float GetAverageAttackTime() const;
  void AddParticleEffect(CStateManager& mgr, const CTransform4f& xf, float duration,
                         CAssetId particle, uint flags, int index);
  void fn_800747a4(CAssetId model, CAssetId skinRules);
  void fn_80074e54(const CModelFlags& flags) const;
  void fn_80077aac(CStateManager& mgr, const CVector3f& direction, int followUp, float magnitude,
                   float duration, TUniqueId projectile);
  void fn_8007850c(CStateManager& mgr);
  bool fn_80073938(CStateManager& mgr, TUniqueId id) const;

  void Start(CStateManager& mgr, EStateMsg msg, float dt);
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt);
  void Dead(CStateManager& mgr, EStateMsg msg, float dt);
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt);
  bool OffLine(CStateManager& mgr, const CTriggerData& data) const;
  bool InRange(CStateManager& mgr, const CTriggerData& data) const;
  bool TooClose(CStateManager& mgr, const CTriggerData& data) const;
  bool InMaxRange(CStateManager& mgr, const CTriggerData& data) const;
  bool InDetectionRange(CStateManager& mgr, const CTriggerData& data) const;
  bool Leash(CStateManager& mgr, const CTriggerData& data) const;
  bool SpotPlayer(CStateManager& mgr, const CTriggerData& data) const;
  bool IsOnScreen(const CStateManager& mgr) const;
  bool PlayerSpot(CStateManager& mgr, const CTriggerData& data) const;
  bool Landed(CStateManager& mgr, const CTriggerData& data) const;
  bool PathOver(CStateManager& mgr, const CTriggerData& data) const;
  bool PathFound(CStateManager& mgr, const CTriggerData& data) const;
  bool PathShagged(CStateManager& mgr, const CTriggerData& data) const;
  bool NoPathNodes(CStateManager& mgr, const CTriggerData& data) const;
  bool Attacked(CStateManager& mgr, const CTriggerData& data) const;
  bool HasPatrolPath(CStateManager& mgr, const CTriggerData& data) const;
  bool InPosition(CStateManager& mgr, const CTriggerData& data) const;
  bool AnimOver(CStateManager& mgr, const CTriggerData& data) const;
  bool GetAnimOver(CStateManager&, const CTriggerData&) const { return mAnimationState.IsOver(); }
  bool Stuck(CStateManager& mgr, const CTriggerData& data) const;
  bool Delay(CStateManager& mgr, const CTriggerData& data) const;
  bool RandomDelay(CStateManager& mgr, const CTriggerData& data) const;
  bool FixedDelay(CStateManager& mgr, const CTriggerData& data) const;
  bool CodeTrigger(CStateManager& mgr, const CTriggerData& data) const;
  bool Random(CStateManager& mgr, const CTriggerData& data) const;
  bool FixedRandom(CStateManager& mgr, const CTriggerData& data) const;
  void ApproachDest(CStateManager& mgr);
  TUniqueId GetConnectedObject(CStateManager& mgr, EScriptObjectState state,
                               EScriptObjectMessage message) const;
  pas::EStepDirection FindBestStepDirection(const CVector3f& direction) const;
  void RotateToPoint(const CVector3f& position, float dt, float turnSpeed);
  void ApplyScreenShake(CStateManager& mgr, const CVector3f& position, TUniqueId shaker);
  void fn_801524fc(CStateManager& mgr);

  bool GetAlive() const { return mAlive; }
  CBodyController* BodyController() { return mBodyController.get(); }
  const CBodyController* GetBodyController() const { return mBodyController.get(); }

private:
  TUniqueId mDestObj;
  CVector3f mDestPos;
  CVector3f mReflectedDestPos;
  bool mInPosition : 1;
  bool mVerticalMovement : 1;
  bool mSolidCollision : 1;
  bool x34c_27_ : 1;
  bool mOnGround : 1;
  bool mOnStaticGround : 1;
  mutable bool mPrevOnGround : 1;
  bool mEnergyAttractor : 1;
  bool mLookAtDeathDir : 1;
  bool x34d_25_ : 1;
  bool x34d_26_ : 1;
  rstl::single_ptr< StateMachine > mStateMachine;
  EPatternedAI mCharacterType;
  int mCreatureSize;
  float mIngPossessionBlend;
  float mIngPossessionTarget;
  float mIngPossessionDelay;
  float mIngPossessionDuration;
  CDamageVulnerability mIngVulnerability;
  rstl::single_ptr< TLockedToken< CScannableObjectInfo > > mIngScanInfo;
  CVector3f mMoveVec;
  CVector3f mFaceVec;
  int mInitialAnimation;
  CVector3f mLatestLeashPosition;
  float mSpeed;
  float mTurnSpeed;
  float mDetectionRange;
  float mDetectionHeightRange;
  float mDetectionAngle; // Cosine of the configured detection angle.
  float mMinAttackRange;
  float mMaxAttackRange;
  float mAverageAttackTime;
  float mAttackTimeVariation;
  float mLeashRadius;
  float mPlayerLeashRadius;
  float mPlayerLeashTime;
  float mCurPlayerLeashTime;
  float mXDamageThreshold;
  float mFrozenXDamageThreshold;
  float mXDamageDelay;
  float mLastHP;
  float mAlphaDelta;
  float mPendingFireDamage;
  float mPendingShockDamage;
  float mBurnThinkRateTimer;
  EFlavorType mFlavor;
  uint mHitByPlayerProjectile : 1;
  uint mAlive : 1;
  uint x420_26_ : 1;
  uint mFadeToDeath : 1;
  uint mPendingMassiveDeath : 1;
  uint mPendingMassiveFrozenDeath : 1;
  uint mIsFlyer : 1;
  uint mPathOverCount : 2;
  uint mLaggedBurnDeath : 1;
  uint x421_26_ : 1;
  uint mPendingDeath : 1;
  uint mLostMassiveFrozenHP : 1;
  uint mDieIf80PercFrozen : 1;
  uint x421_30_ : 1;
  uint mDrawParticles : 1;
  uint mEnableStateMachine : 1;
  uint mStateControlledMassiveDeath : 1;
  uint x422_26_ : 2;
  uint x422_28_ : 1;
  uint x422_29_ : 1;
  uint x422_30_ : 1;
  uint mStopPhysics : 1;
  uint x423_24_ : 1;
  uint mSuppressKnockBack : 1;
  CDamageInfo mContactDamage;
  float mCurDamageRemTime;
  float mDamageWaitTime;
  float mDamageCooldownTimer;
  CColor mColor;
  CColor mDamageColor;
  CVector3f mPosDelta;
  CQuaternion mRotDelta;
  TLockedToken< CSkinnedModel > mNormalModel;
  rstl::optional_object< TLockedToken< CSkinnedModel > > mIngModel;
  rstl::single_ptr< CBodyController > mBodyController;
  uint mDeathSfx;
  uint mIceShatterSfx;
  uint mIceVocalSfx;
  uint mFrozenSfx;
  SLdrIngPossessionData mIngPossessionData;
  CSteeringBehaviors mSteeringBehaviors;
  CAiKnockBackMgr mKnockBackController;
  CAnimationState mAnimationState;
  CWaypointNavigation mWaypointNavigation;
  CPathFindNavigation mPathFindNavigation;
  CVector3f mLatestPredictedTranslation;
  float mPredictedLeashTime;
  float mIntoFreezeDuration;
  float mOutOfFreezeDuration;
  float mFreezeDuration;
  float mPreThinkDt;
  float mDamageDuration;
  EColliderType mColliderType;
  float mFadeOnDeathTime;
  CVector3f mDeathExplosionOffset;
  rstl::optional_object< TCachedToken< CGenDescription > > mDeathExplosionParticle;
  rstl::optional_object< TCachedToken< CElectricDescription > > mDeathExplosionElectric;
  CVector3f mIceDeathExplosionOffset;
  rstl::optional_object< TCachedToken< CGenDescription > > mIceDeathExplosionParticle;
  CVector3f mMoveScale;
  CPlane mIngSnatchingPlane;
  CVector3f mDisintegrationOrigin;
  CSegId mLockOnTarget;
};
CHECK_SIZEOF(CPatterned, 0x7c0)

#endif
