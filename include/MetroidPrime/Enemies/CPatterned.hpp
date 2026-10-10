#ifndef _CPATTERNED
#define _CPATTERNED

#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CSteeringBehaviors.hpp"
#include "MetroidPrime/Enemies/CAi.hpp"
#include "MetroidPrime/Enemies/CAiKnockBackMgr.hpp"
#include "MetroidPrime/Enemies/CAnimationState.hpp"
#include "MetroidPrime/Enemies/CPathFindNavigation.hpp"
#include "MetroidPrime/Enemies/CPatternedInfo.hpp"
#include "MetroidPrime/Enemies/CWaypointNavigation.hpp"
#include "MetroidPrime/TStateMachineState.hpp"

#include "Kyoto/Animation/CAdvancementDeltas.hpp"
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
class CPathFindPointSearch;
class CPathFindSearch;
class CPatterned;
class CProjectileInfo;
class CRagDoll;
class CScriptCoverPoint;
class CSkinnedModel;
class CPASAnimParmData;
class CCharAnimTime;

enum EPatternedAI {
  kPAI_AtomicAlpha = 0, // Guessed name; AtomicAlpha REL constructor.
  kPAI_AtomicBeta = 1,  // Guessed name; AtomicBeta REL constructor.
  kPAI_ChozoGhost = 4,  // Guessed name; ChozoGhost REL constructor.
  kPAI_DarkCommando = 6, // Guessed name; DarkCommando REL constructor.
  kPAI_DarkSamus = 7,
  kPAI_DarkTrooper = 8, // Guessed name; DarkTrooper REL constructor.
  kPAI_EmperorIngStage2Tentacle = 0xe, // Guessed name; EmperorIngStage2Tentacle REL constructor.
  kPAI_EyeBall = 0x10,                 // Guessed name; EyeBall REL constructor.
  kPAI_FlyingPirate = 0x15,            // Guessed name; FlyingPirate REL constructor.
  kPAI_Grenchler = 0x16,               // Guessed name; Grenchler REL constructor.
  kPAI_Ing = 0x18,                     // Guessed name; Ing REL constructor.
  kPAI_IngSpaceJumpGuardian = 0x1a,    // Guessed name; IngSpaceJumpGuardian REL constructor.
  kPAI_IngSpiderballGuardian = 0x1b,   // Guessed name; IngSpiderballGuardian REL constructor.
  kPAI_Lumite = 0x1d, // Guessed name; Lumite REL constructor.
  kPAI_MediumIng = 0x1f, // Guessed name; MediumIng REL constructor.
  kPAI_Metaree = 0x20,                 // Guessed name; Metaree REL constructor.
  kPAI_Metroid = 0x21,                 // Guessed name; Metroid REL constructor.
  kPAI_MysteryFlyer = 0x25,            // Guessed name; MysteryFlyer REL constructor.
  kPAI_Parasite = 0x27,                // Guessed name; Parasite REL constructor.
  kPAI_PillBug = 0x27,                 // Guessed name; PillBug REL constructor, same value.
  kPAI_PuddleSpore = 0x2b,             // Guessed name; PuddleSpore REL constructor.
  kPAI_Puffer = 0x2d,                  // Guessed name; Puffer REL constructor.
  kPAI_Rezbit = 0x2e,                  // Guessed name; Rezbit REL constructor.
  kPAI_Ripper = 0x30,                  // Guessed name; Ripper REL constructor.
  kPAI_Sandworm = 0x32,                // Guessed name; Sandworm REL constructor.
  kPAI_SpacePirate = 0x35,             // Guessed name; SpacePirate REL constructor.
  kPAI_SpankWeed = 0x36,               // Guessed name; SpankWeed REL constructor.
  kPAI_Splinter = 0x37,                // Guessed name; Splinter REL constructor.
  kPAI_SplitterMainChassis = 0x38, // Guessed name; SplitterMainChassis REL constructor.
  kPAI_SplitterCommandModule = 0x39, // Guessed name; SplitterCommandModule REL constructor.
  kPAI_StoneToad = 0x3a,               // Guessed name; StoneToad REL constructor.
  kPAI_SwampBossStage2 = 0x3c,         // Guessed name; SwampBossStage2 REL constructor.
  kPAI_Tryclops = 0x3f,                // Guessed name; Tryclops REL constructor.
  kPAI_WispTentacle = 0x42,            // Guessed name; WispTentacle REL constructor.
  kPAI_GunTurretBase = 0x43,           // Guessed name; GunTurretBase REL constructor.
  kPAI_GunTurretTop = 0x44,            // Guessed name; GunTurretTop REL constructor.
  kPAI_SporbBase = 0x47,               // Guessed name; SporbBase REL constructor.
  kPAI_SporbTop = 0x48,                // Guessed name; SporbTop REL constructor.
  kPAI_SporbProjectile = 0x49,         // Guessed name; SporbProjectile REL constructor.
  kPAI_Shrieker = 0x4a,                // Guessed name; Shrieker REL constructor.
  kPAI_MinorIng = 0x4b,                // Guessed name; MinorIng REL constructor.
  kPAI_WallWalker = 0x4d,              // Guessed name; WallWalker REL constructor.
  kPAI_Shredder = 0x4e,                // Guessed name; Shredder REL constructor.
  kPAI_Blogg = 0x4c,                   // Guessed name; Blogg REL constructor.
  kPAI_Krocuss = 0x4f,                 // Guessed name; Krocuss REL constructor.
  kPAI_OctapedeSegment = 0x50, // Guessed name; OctapedeSegment REL constructor.
};

template <>
struct TStateMachineFunctionTypes< CPatterned > {
  typedef EStateMsg StateMsg;
  typedef CTriggerData TriggerArg;
  typedef void (CPatterned::*StateFunc)(CStateManager&, EStateMsg, float);
  typedef bool (CPatterned::*TriggerFunc)(CStateManager&, const CTriggerData&) const;
};

namespace rstl {
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(TStateMachineFunctionTypes< CPatterned >::StateFunc)
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(TStateMachineFunctionTypes< CPatterned >::TriggerFunc)
} // namespace rstl

class CPatterned : public CAi {
public:
  enum EFlavorType { kFT_Zero, kFT_One };
  enum EMovementType { kMT_Ground, kMT_Flyer };
  enum EColliderType { kCT_Zero, kCT_One };
  // Guessed names; flags in the established two-bit animation-delta field.
  enum EAnimationDeltaFlags { kADF_Translation = 1, kADF_Rotation = 2 };

  typedef TStateMachineStateBase< CPatterned > StateMachine;

  static const float skDamageHitTime;
  static const float skActorApproachDistance;
  static const CColor skDamageColor;
  static const CColor skHitsWithoutDamageColor;
  static const CColor skFrozenColor;         // Guessed name; native frozen-interpolation endpoint.
  static const CColor skDisintegrateColor;   // Guessed Prime name; native ash tint.
  static const CColor skBlackDeathColor;     // Guessed name; native purple death tint.
  static const CColor skDisintegrationColor; // Guessed name; native white death tint.

  CPatterned(EPatternedAI character, TUniqueId uid, const rstl::string& name, EFlavorType flavor,
             const CEntityInfo& info, const CTransform4f& xf, const CModelData& modelData,
             const CPatternedInfo& patternedInfo, EMovementType movement, EColliderType collider,
             EBodyType body, const CActorParameters& params);
  // CEntity
  ~CPatterned() override {}
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  CEntity* TypesMatch(int typeId) const override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  bool CanRenderUnsorted(const CStateManager& mgr) const override;
  void PreRenderAllViewports(CStateManager& mgr) override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  const CDamageVulnerability* GetDamageVulnerability(const CVector3f& position,
                                                     const CVector3f& direction,
                                                     const CDamageInfo& damage) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
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

  // CPhysicsActor
  void CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                    CStateManager& mgr) override;
  bool IsOnStaticGround() const override;

  // CAi
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;
  void KnockBack(CStateManager& mgr, const CKnockBackInfo& info) override;
  void TakeDamage(const CVector3f&, float) override { mDamageCooldownTimer = skDamageHitTime; }

  // CPatterned
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
  CPathFindSearch* GetSearchPath() const {
    return const_cast< CPatterned* >(this)->GetSearchPath();
  }
  // Guessed name; the point-based path used by CIngSpotPathFindNavigation.
  virtual CPathFindPointSearch* GetPointSearchPath() { return nullptr; }
  CPathFindPointSearch* GetPointSearchPath() const {
    return const_cast< CPatterned* >(this)->GetPointSearchPath();
  }
  virtual CDamageInfo GetContactDamage() const;
  virtual void UpdateHitDamageTime(float dt);
  virtual void SetupStateMachine(CStateManager& mgr);
  virtual CRagDoll* GetRagDoll() const { return nullptr; } // Guessed name
  virtual bool CanBeIngPossessed(CStateManager& mgr) const;
  virtual bool CanBeUnPossessed(CStateManager& mgr) const;
  virtual void SetIngPossessed(bool possessed, CStateManager& mgr);
  virtual void SetIngPossessed(bool possessed, float duration, CStateManager& mgr);
  virtual void SetAttackTarget(CStateManager& mgr, TUniqueId target);
  virtual TUniqueId GetAttackTarget() const { return kInvalidUniqueId; }
  virtual bool IsOnGround() const { return mOnGround; }
  virtual float GetGravityConstant() const { return kDefaultGravityAccel; }
  virtual bool IsScanVisorSelfRender() const;
  virtual CAABox GetScanVisorRenderBounds(const CStateManager&) const;
  virtual void ScanVisorRender(const CStateManager&, const CTransform4f&, const CModelFlags&) const;
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
  bool GetFadeToDeath() const { return mFadeToDeath; }    // Guessed Prime name.
  void SetFadeToDeath(bool fade) { mFadeToDeath = fade; } // Guessed Prime name.

  int GetIngPossessionAnimation() const { return mIngPossessionData.unknown_0x2befc1bf; }

  float GetIngPossessedDamageMultiplier() const {
    return mIngPossessionData.ingPossessedDamageMultiplier;
  }

  void UpdateIngPossession(float dt);
  CEnergyProjectile* LaunchProjectile(const CTransform4f& xf, CStateManager& mgr,
                                      int maxProjectiles, uint attributes, bool homing,
                                      const CImpactVisorEffect& visorEffect,
                                      const CVector3f& scale);
  CCharAnimTime GetTimeOfUserEventForAnimation(const CPASAnimParmData& params,
                                               EUserEventType event) const;
  int GetNumUserEventsForAnimation(const CPASAnimParmData& params, EUserEventType event) const;
  float GetAverageAttackTime() const;
  void AddParticleEffect(CStateManager& mgr, const CTransform4f& xf, float particleScale,
                         CAssetId particle, uint name, int flags);
  void BuildIngModel(CAssetId model, CAssetId skinRules);
  void RenderIceModelWithFlags(const CModelFlags& flags) const;
  // Guessed name; dispatches the selected knockback follow-up effect.
  void ApplyKnockBackFollowUp(CStateManager& mgr, const CVector3f& direction,
                              CKnockBackMgr::EFollowUp followUp, float duration,
                              float secondaryDuration, TUniqueId source, TUniqueId owner);
  void GenerateIceDeathExplosion(CStateManager& mgr); // Guessed Prime-correlated name.

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
  bool GetAnimOver(CStateManager&, const CTriggerData&) const;
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
  bool GetHitByPlayerProjectile() const { return mHitByPlayerProjectile; }
  void SetHitByPlayerProjectile(bool hit) { mHitByPlayerProjectile = hit; }
  void SetPendingDeath(bool pending) { mPendingDeath = pending; }
  TUniqueId GetDestObj() const { return mDestObj; }
  EFlavorType GetFlavorType() const { return mFlavor; }
  bool IsMakingBigStrike() const { return mIsMakingBigStrike; }
  float GetDamageDuration() const { return mDamageDuration; }
  int GetCreatureSize() const { return mCreatureSize; }
  const CPlane& GetIngSnatchingPlane() const { return mIngSnatchingPlane; }

  bool GetVerticalMovement() const { return mVerticalMovement; }

  bool IsInCollision() const { return mSolidCollision; }

  float GetSpeed() const { return mSpeed; }

  // Guessed name
  bool HasBlockingCollision() const { return mBlockingCollision; }

  CBodyController* BodyController() { return mBodyController.get(); }

  TStateMachineState< CPatterned >& StateMachineState() {
    return static_cast< TStateMachineState< CPatterned >& >(*mStateMachine);
  }

  const CBodyController* GetBodyController() const { return mBodyController.get(); }

  CAiKnockBackMgr& KnockBackController() { return mKnockBackController; }
  const CAiKnockBackMgr& GetKnockBackController() const { return mKnockBackController; }

protected:
  mutable TUniqueId mDestObj;
  CVector3f mDestPos;
  mutable CVector3f mReflectedDestPos;
  mutable bool mInPosition : 1;
  bool mVerticalMovement : 1;
  bool mSolidCollision : 1;
  bool mBlockingCollision : 1; // Guessed name
  bool mOnGround : 1;
  bool mOnStaticGround : 1;
  bool mPrevOnGround : 1;
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
  mutable uint mHitByPlayerProjectile : 1;
  uint mAlive : 1;
  uint x420_26_ : 1;
  uint mFadeToDeath : 1;
  uint mPendingMassiveDeath : 1;
  uint mPendingMassiveFrozenDeath : 1;
  uint mIsFlyer : 1;
  uint mPathOverCount : 2;
  uint mBurning : 1;         // Guessed Prime name; native burn-death rendering gate.
  uint mLaggedBurnDeath : 1; // Guessed Prime name; delays fire-pop and ash effects.
  uint mPendingDeath : 1;
  uint mLostMassiveFrozenHP : 1;
  uint mDieIf80PercFrozen : 1;
  uint mIsMakingBigStrike : 1; // Guessed Prime name; native gun strike reaction.
  uint mDrawParticles : 1;
  uint mEnableStateMachine : 1;
  uint mStateControlledMassiveDeath : 1;
  uint mDisabledAnimationDeltas : 2; // Guessed name; translation and rotation gates.
  uint mUseDisintegrationPlane : 1;  // Guessed name; enables plane clipping.
  uint mBlackDeath : 1;              // Guessed name; purple implosion/death path.
  uint mDisintegrating : 1;          // Guessed name; white disintegration path.
  uint mStopPhysics : 1;
  uint x423_24_ : 1;
  uint mSuppressKnockBack : 1;
  CDamageInfo mContactDamage;
  float mCurDamageRemTime;
  float mDamageWaitTime;
  float mDamageCooldownTimer;
  CColor mColor;
  CColor mDamageColor;
  CAdvancementDeltas mAnimationDeltas; // Guessed name; animation translation and rotation.
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

// Defined after the class: a REL that never builds an optional_object< CAABox > itself calls
// the constructor out of line here (weak copy at the end of the module), like the originals.
inline rstl::optional_object< CAABox > CPatterned::GetTouchBounds() const {
  return GetBoundingBox();
}

#endif
