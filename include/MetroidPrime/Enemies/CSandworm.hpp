#ifndef _CSANDWORM
#define _CSANDWORM

#include "types.h"

#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/CToken.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CRELFileToken.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSandworm.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

class CCollisionActor;
class CScriptCoverPoint;
class CCollisionActorManager;
class CElementGen;
class CGenDescription;
class CBouncingBomb;
class CProjectedShadow;
class CSandwormEye;
class CJointCollisionDescription;
class CGenericFSM2;

struct SSpineSegment {
  const char* mName;
  float mRadius;
  float mScale;
};

// Original class name from the Wii SEL exports; the REL script object is the "BossBombGuardian".
class CSandworm : public CPatterned {
public:
  // Guessed names; the attack kinds stored in the attack history.
  enum EAttack {
    kA_Pursue = 0, // Guessed name
    kA_SpitAttack = 1,
    kA_Melee = 2,      // Guessed name
    kA_GrabAttack = 3, // Guessed name
    kA_Charge = 4,     // Guessed name
    kA_BombToss = 5,
    kA_BombSpread = 6,
    kA_BombFountain = 7,
  };

  // Guessed name; the charge attack parameters.
  struct SChargeData {
    SChargeData(float rangeMin, float rangeMax, float impulseH, float impulseV);

    CVector3f mStartPosition; // Guessed name
    float mRangeMin;
    float mRangeMax;
    float mImpulseHorizontal;
    float mImpulseVertical;
    CVector3f mTargetPosition; // Guessed name
    float mSavedTurnSpeed;     // Guessed name
    float mStartTime;          // Guessed name
    bool mReadyToCharge : 1;   // Guessed name
    bool mHitPlayer : 1;       // Guessed name
  };

  // Guessed name; the spit attack parameters.
  struct SSpitData {
    SSpitData(float minRange, float maxRange, float aimAngle, CAssetId projectile,
              const CDamageInfo& damage, CAssetId visorEffect);

    CProjectileInfo mProjectileInfo;
    CTransform4f mLocatorTransform; // Guessed name
    float mNextSpitTime;            // Guessed name
    float mMaxAimAngle;             // Guessed name
    float mMinRange;
    float mMaxRange;
    CVector3f mOrigin; // Guessed name
    rstl::optional_object< TLockedToken< CGenDescription > > mVisorEffect;
  };

  // Guessed name; the morph ball toss attack parameters.
  struct SMorphballTossData {
    SMorphballTossData(float impulseH, float impulseV, const CDamageInfo& damage);

    CTransform4f mLocatorTransform; // Guessed name
    CDamageInfo mDamage;
    float mImpulseHorizontal;
    float mImpulseVertical;
    float mLastTossTime;     // Guessed name
    bool mHoldingPlayer : 1; // Guessed name
    bool mTossFinished : 1;  // Guessed name
    bool mAttackFlagSet : 1; // Guessed name
  };

  // Guessed name; the pincer swipe attack parameters.
  struct SMeleeData {
    SMeleeData(const CDamageInfo& damage, float impulseH, float impulseV);

    CDamageInfo mDamage;
    float mDamageTimer; // Guessed name
    bool x20_;          // Initialized to true; no reader found
    float mImpulseHorizontal;
    float mImpulseVertical;
  };

  // Guessed name; the sounds played by the worm.
  struct SSoundData {
    SSoundData(ushort walk, ushort walkVocal, ushort melee, ushort eyeKilled, ushort bombBounce,
               ushort bombExplode);

    ushort mWalkSound;
    ushort mWalkVocalSound;
    ushort mMeleeAttackSound;
    ushort mEyeKilledSound;
    ushort mBombBounceSound;
    ushort mBombExplodeSound;
    float mWalkSoundTimer;    // Guessed name
    float mWalkSoundInterval; // Guessed name
    CSfxHandle mWalkHandle;
    CSfxHandle mWalkVocalHandle;
    CSfxHandle mMeleeHandle;
  };

  // Guessed name; the bomb attack effects.
  struct SBombData {
    SBombData();
    SBombData(CAssetId bombEffect, CAssetId explosionEffect, const CDamageInfo& damage);

    rstl::optional_object< TLockedToken< CGenDescription > > mBombEffect;
    rstl::optional_object< TLockedToken< CGenDescription > > mExplosionEffect;
    CDamageInfo mDamage;
  };

  // Guessed name; one particle effect slot of the eye glow.
  struct SEyeEffect {
    SEyeEffect(CAssetId effect);
    ~SEyeEffect();

    bool mActive;
    rstl::single_ptr< CElementGen > mGenerator;
  };

  // Guessed name; the glowing eye effects.
  struct SEyeEffects {
    SEyeEffects(CAssetId glow, CAssetId effect1, CAssetId effect2)
    : mGlow(glow), mEffect1(glow), mEffect2(effect1), mEffect3(effect2) {}

    SEyeEffect mGlow;
    SEyeEffect mEffect1;
    SEyeEffect mEffect2;
    SEyeEffect mEffect3;
  };

  // Guessed name; the attacks chosen so far.
  struct SAttackHistory {
    SAttackHistory();

    // Guessed name; true if the last count entries all equal attack.
    bool LastEntriesEqual(int attack, int count) const;
    void AddAttack(EAttack attack); // Guessed name
    void Clear();                   // Guessed name

    int mCurrentAttack;
    int mRunningAttack; // Guessed name
    rstl::reserved_vector< EAttack, 5 > mHistory;
  };

  // Guessed name; a point on the spine trail and the trail index it lies after.
  struct STrailPoint {
    STrailPoint(int index, int nextIndex, const CVector3f& position)
    : mIndex(index), mNextIndex(nextIndex), mPosition(position) {}

    int mIndex;
    int mNextIndex;
    CVector3f mPosition;
  };

  // Guessed name; the bomb toss attack state.
  struct SBombTossData {
    SBombTossData() : mOrigin(CVector3f::Zero()), mBombId(kInvalidUniqueId), mHoldingBomb(false) {}

    CVector3f mOrigin; // Guessed name
    TUniqueId mBombId;
    bool mHoldingBomb;
  };

  // Guessed name; the bomb spread attack state.
  struct SBombSpreadData {
    SBombSpreadData() : mOrigin(CVector3f::Zero()) {}

    CVector3f mOrigin; // Guessed name
  };

  // Guessed name.
  struct SBombFountainData {
    SBombFountainData();
    SBombFountainData(float damageThreshold, float unusedValue);

    CVector3f mOrigin;                 // Guessed name
    bool mHitDuringWindUp : 1;         // Guessed name
    bool mBombLaunched : 1;            // Guessed name
    bool mEyeKilledDuringFountain : 1; // Guessed name
    float x10_;                        // Set from the loader (unusedFountainValue); no reader found
    float mStartHealth;                // Guessed name
    float mDamageThreshold;            // Guessed name
    float mDamageTaken;                // Guessed name
    float mDamageFlashTimer;           // Guessed name
    float mThresholdHitTime;           // Guessed name
  };

  // Guessed name.
  struct SSpinePose {
    SSpinePose();

    rstl::reserved_vector< CQuaternion, 13 > mRotations; // Guessed name
    CVector3f mRootTranslation;                          // Guessed name
    CQuaternion mRootRotation;                           // Guessed name
    CVector3f mStoredTranslation;                        // Guessed name
    float mStraightenBlend;                              // Guessed name
  };

  CSandworm(TUniqueId uid, const rstl::string& name, CEntityInfo& info, const CTransform4f& xf,
            const CModelData& modelData, const CPatternedInfo& patternedInfo, CAssetId stateMachine,
            float pincerScale, float spitAttackMinRange, float spitAttackMaxRange,
            float spitAimAngle, float chargeRangeMin, float chargeRangeMax,
            float chargeImpulseHorizontal, float chargeImpulseVertical, bool startsUnderground,
            CAssetId pincerL, CAssetId pincerR, ushort walkSound, ushort walkVocalSound,
            ushort meleeAttackSound, ushort eyeKilledSound, ushort bombBounceSound,
            ushort bombExplodeSound, CAssetId spitAttackVisorEffect,
            float morphballTossImpulseHorizontal, float morphballTossImpulseVertical,
            float meleeImpulseHorizontal, float meleeImpulseVertical, float sulkHealthDrop,
            float lurkUndergroundTimeMin, float lurkUndergroundTimeMax,
            float pursuitFrustrationRadius, float pursuitFrustrationTimer, CAssetId projectile,
            const CDamageInfo& projectileDamage, const CDamageInfo& morphballTossDamage,
            const CDamageInfo& pincerSwipeDamage, CAssetId eyeGlow, CAssetId particle1,
            CAssetId particle2, CAssetId bombEffect, CAssetId bombExplosionEffect,
            const CDamageInfo& bombDamage, float bombDropRate, float fountainDamageThreshold,
            float unusedFountainValue, const SLdrSandwormStruct& struct0,
            const SLdrSandwormStruct& struct1, const SLdrSandwormStruct& struct2,
            const SLdrSandwormStruct& struct3, const SLdrSandwormStruct& struct4,
            const CActorParameters& actorParams);

  CSandworm(TUniqueId uid, const rstl::string& name, CEntityInfo& info, const CTransform4f& xf,
            const CModelData& modelData, const CPatternedInfo& patternedInfo, CAssetId stateMachine,
            float pincerScale, float spitAttackMinRange, float spitAttackMaxRange,
            float spitAimAngle, float chargeRangeMin, float chargeRangeMax,
            float chargeImpulseHorizontal, float chargeImpulseVertical, bool startsUnderground,
            CAssetId pincerL, CAssetId pincerR, ushort walkSound, ushort walkVocalSound,
            ushort meleeAttackSound, ushort eyeKilledSound, CAssetId spitAttackVisorEffect,
            float morphballTossImpulseHorizontal, float morphballTossImpulseVertical,
            float meleeImpulseHorizontal, float meleeImpulseVertical, float sulkHealthDrop,
            float lurkUndergroundTimeMin, float lurkUndergroundTimeMax,
            float pursuitFrustrationRadius, float pursuitFrustrationTimer, CAssetId projectile,
            const CDamageInfo& projectileDamage, const CDamageInfo& morphballTossDamage,
            const CDamageInfo& pincerSwipeDamage, CAssetId eyeGlow,
            const CActorParameters& actorParams);

  // CEntity
  ~CSandworm() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  void PreRenderAllViewports(CStateManager& mgr) override;
  CVector3f GetOrbitPosition(const CStateManager& mgr) const override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CPatterned
  CProjectileInfo* ProjectileInfo() override;
  CPathFindSearch* GetSearchPath() override { return &mPathFindSearch; }
  bool CanBeIngPossessed(CStateManager& mgr) const override;
  bool CanBeUnPossessed(CStateManager& mgr) const override;
  CVector3f GetIngSnatchingNormal(float t) const override;
  CVector3f GetIngSnatchingPoint(float t) const override;
  float GetIngSnatchingModelOverlapSize() const override;
  void RenderIngSnatchingTransition(const CStateManager& mgr) const override;

  // CSandworm; state machine triggers
  virtual bool Activated(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool BeginUnderground(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool CanAttack(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool CanCharge(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool CanDescend(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool CanGrabMorphball(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool CanMeleeAgain(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool CanSpitAgain(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ChargeOver(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool DoneLurking(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool DoneStraightening(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool DoneSulking(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool FacingPlayer(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool FacingPlayerForCharge(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ForceUnderground(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool GrabAttackFinished(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool HitDuringWindUp(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool InAttackRange(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool InChargeRange(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool InSpitRange(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool MeleeAttackRange(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool OneEyeKilled(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool PickedBombFountain(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool PickedBombSpread(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool PickedBombToss(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool PickedSpitAttack(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool PlayerHiding(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool PlayerIsOnPathMesh(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool PlayerIsMorphball(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool PlayerReachable(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool Primed(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ReadyToCharge(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool SequenceAttack(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ShouldSulk(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool SnatchStarted(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool SnatchEnded(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool SpitAngleOK(CStateManager& mgr, const CTriggerData& data) const;

  // CSandworm; state machine states
  virtual void BombFountainAttack(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void BombSpreadAttack(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void BombTossAttack(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void ChargePlayer(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void ChargeWindUp(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Deactivate(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Dead(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Descend(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void EyeKilledReaction(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void GrabAttack(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void LurkUnderground(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void MeleeAttack(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Null(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Patrol(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void PickMediumAttack(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Prime(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Pursue(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Rise(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Snatch(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void SpitAttack(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void StopGrabAttack(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Straighten(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Sulk(CStateManager& mgr, EStateMsg msg, float dt);

  // CSandworm; state machine code functions
  virtual void Interrupt(CStateManager& mgr, float dt);
  virtual bool x23c() const { return false; } // Guessed name

  // CActor
  rstl::optional_object< CAABox > GetTouchBounds() const override { return GetBoundingBox(); }

  CVector3f GetRadarPointPosition(int index) const;
  int GetRadarPointCount() const;

private:
  void CreateShadow();                   // Guessed name
  void ClearFloatArray();                // Guessed name
  void Initialize();                     // Guessed name
  CSegId GetSpineSegId(int index) const; // Guessed name

  // Guessed names
  bool IsIngControlled() const;
  int IsAnyEyeKilled(const CStateManager& mgr) const;
  bool ShouldReactToEyeKill(const CStateManager& mgr) const;
  bool IsRemainingEyeAlive(const CStateManager& mgr) const;
  float GetHealthPercent() const;
  bool IsNearHint(CStateManager& mgr) const;
  bool IsOnPathMesh(const CVector3f& position, float radius) const;
  bool HasUnknown11Hint(CStateManager& mgr) const;
  int GetNextAttack(CStateManager& mgr) const;
  const SLdrSandwormStruct& GetPhaseStruct() const;
  void UpdateBombDrop(CStateManager& mgr, float dt);
  bool CanDropBomb(CStateManager& mgr) const;
  void ResetBombTimer();
  CBouncingBomb* SpawnBomb(CStateManager& mgr, const CVector3f& position);
  ushort GetBombExplodeSound() const;
  ushort GetBombBounceSound() const;
  ushort GetBombPlacementSound() const;
  float GetAngleToTarget(const CVector3f& position, const CVector3f& direction) const;
  float GetSpitAngleDiff(const CVector3f& position) const;
  CVector3f GetMorphballTossPosition(float distance) const;
  bool IsPlayerInMeleeRange(CStateManager& mgr) const;
  bool IsPlayerInTossRange(CStateManager& mgr, float distance) const;
  bool UpdatePlayerReachable(CStateManager& mgr) const;
  void UpdateSounds(CStateManager& mgr);
  void UpdateSoundEmitter(CSfxHandle& handle);
  void PlayMeleeSound();
  void StopWalkVocalSound();
  void PlayWalkVocalSound();
  void UpdateWalkSound(CStateManager& mgr, float dt);
  void StopWalkSound();
  void UpdateBounds();
  CAABox CalculateBounds() const;
  float GetHitAngle(const CTransform4f& xf) const;          // Guessed name
  void SetPathArea(CStateManager& mgr);                     // Guessed name
  void UpdateCollisionActors(CStateManager& mgr, float dt); // Guessed name
  void SetupStateMachineHelper(CStateManager& mgr);         // Guessed name
  void BoostNearbyProjectileHoming(CStateManager& mgr);     // Guessed name
  void UpdateBossState(CStateManager& mgr);                 // Guessed name
  const CGenericFSM2* GetStateMachine() const;              // Guessed name
  void AddJointCollisions(
      const SSpineSegment* segments, int count,
      rstl::vector< CJointCollisionDescription >& descriptions) const; // Guessed name
  void SetCollisionRadiusMode(CStateManager& mgr, int mode);           // Guessed name
  void UpdateSideStep(CStateManager& mgr, float dt);                   // Guessed name
  void UpdateDamageFlash(float dt);                                    // Guessed name
  void InitializeSpine(CStateManager& mgr);                            // Guessed name
  void UpdatePose();                                                   // Guessed name
  void SetSpineRotation(int index, const CQuaternion& rotation);       // Guessed name
  void StoreTranslation();                                             // Guessed name
  void PoseSpine();                                                    // Guessed name
  void BuildTrail(CStateManager& mgr);                                 // Guessed name
  CVector3f GetSurfaceSamplePosition(CStateManager& mgr,
                                     const CVector3f& position) const;       // Guessed name
  void ResetSpineFacing();                                                   // Guessed name
  void UpdateTrail(CStateManager& mgr);                                      // Guessed name
  CVector3f GetLocalSpinePosition(int index) const;                          // Guessed name
  STrailPoint FindTrailPoint(const STrailPoint& from, float distance) const; // Guessed name
  void UpdateSegmentLengths();                                               // Guessed name
  CVector3f GetTrailPositionAtDistance(float distance) const;                // Guessed name
  void UpdateSpinePositions();                                               // Guessed name
  CSegId GetBodySegId(int index) const;                                      // Guessed name
  void ApplySpineRotations();                                                // Guessed name
  void RotateAroundPivot(CStateManager& mgr, CVector3f target, CVector3f& position, float speed,
                         float dt);                                            // Guessed name
  float GetSurfaceHeight(CStateManager& mgr, const CVector3f& position) const; // Guessed name
  CRelAngle GetShakeAngle(float time, float frequency, float amplitude) const; // Guessed name
  void RenderPincer(const CStateManager& mgr, const CModelData& model,
                    const CTransform4f& xf) const; // Guessed name
  void UpdateShadow(CStateManager& mgr);           // Guessed name
  void AccumulateModelBounds(CAABox& box, const CModelData& model,
                             const CTransform4f& xf) const;          // Guessed name
  void UpdateLocators();                                             // Guessed name
  void SetSegmentScale(const CSegId& segId, const CVector3f& scale); // Guessed name
  void UpdateDeathScale();                                           // Guessed name
  CVector3f GetPatrolDestination(CStateManager& mgr) const;          // Guessed name
  void EnsureCollisionActors(CStateManager& mgr);                    // Guessed name
  void UpdateSplineSegments(CStateManager& mgr);                     // Guessed name
  void CreateCollisionActors(CStateManager& mgr);                    // Guessed name
  void UpdateSpine(CStateManager& mgr);                              // Guessed name
  CTransform4f GetClawTransform(CStateManager& mgr, const char* name, bool isBack, bool isRight,
                                float angle) const; // Guessed name
  float GetClawGroundAngle(CStateManager& mgr, const char* name,
                           bool isBack) const; // Guessed name
  void FireSpit(CStateManager& mgr);           // Guessed name
  void TossBombRandom(CStateManager& mgr);     // Guessed name
  CBouncingBomb* CreateBomb(CStateManager& mgr, float fuseTime, float touchRadius,
                            float gravityScale, float bounceRestitution);           // Guessed name
  void TossBombAtPlayer(CStateManager& mgr);                                        // Guessed name
  void PositionHeldBomb(CStateManager& mgr);                                        // Guessed name
  void LaunchHeldBomb(CStateManager& mgr);                                          // Guessed name
  const CBouncingBomb* GetHeldBomb(const CStateManager& mgr) const;                 // Guessed name
  CBouncingBomb* GetHeldBomb(CStateManager& mgr) const;                             // Guessed name
  void DeleteHeldBomb(CStateManager& mgr);                                          // Guessed name
  void SpawnHeldBomb(CStateManager& mgr);                                           // Guessed name
  void LaunchSpitProjectile(CStateManager& mgr, const CVector3f& target);           // Guessed name
  CVector3f GetPursueTarget(CStateManager& mgr);                                    // Guessed name
  TUniqueId PickCoverPoint(CStateManager& mgr) const;                               // Guessed name
  bool IsCoverPointUsable(const CScriptCoverPoint* cover) const;                    // Guessed name
  void SetPathDestination(CStateManager& mgr, const CVector3f& position, float dt); // Guessed name
  void ChargeHitPlayer(CStateManager& mgr);                                         // Guessed name
  bool ShouldReverseDirection(CStateManager& mgr) const;                            // Guessed name
  void CheckPathObstruction(CStateManager& mgr);                                    // Guessed name
  CVector3f GetSpinePosition(int index) const;                                      // Guessed name
  CVector3f GetTrailPosition(int startIndex, int& outIndex, float distance) const;  // Guessed name
  int GetTrailIndexAtDistance(float distance) const;                                // Guessed name
  float GetTotalSegmentWeight() const;                                              // Guessed name
  float SumSegmentWeights(int count) const;                                         // Guessed name
  CVector3f GetLocatorPosition(int index) const;                                    // Guessed name
  CVector3f GetSpineLocatorPosition(int index) const;                               // Guessed name
  CVector3f GetSegmentPosition(const CSegId& segId) const;                          // Guessed name
  void UpdateSteeringSpeed();                                                       // Guessed name
  void UpdateSegmentWeights(float dt);                                              // Guessed name
  static float MoveToward(float current, float target, float step);                 // Guessed name
  void UpdateEyeBlend(float dt);                                                    // Guessed name
  float GetBlendWeight() const {
    return skBlendWeightMin + mSideStepScale * (skBlendWeightMax - skBlendWeightMin);
  }
  float GetSideStepThreshold() const {
    float threshold = skThresholdMin + mSideStepScale * (skThresholdMax - skThresholdMin);
    if (mIsCharging == true) {
      threshold *= 0.25f;
    }
    return threshold;
  }
  CVector3f GetChargeBlendedDirection(const CVector3f& direction) const; // Guessed name
  CVector3f GetPursueBlendedDirection(const CVector3f& direction) const; // Guessed name
  void UpdateMeleeDamage(CStateManager& mgr, float dt);                  // Guessed name
  void UpdateAttackHistory(EAttack attack, EStateMsg msg);               // Guessed name
  void UpdateTurnSpeed(float dt);                                        // Guessed name
  int CountGeneratedActors(CStateManager& mgr) const;                    // Guessed name
  uchar IsHeadCollisionActor(CStateManager& mgr,
                             const CCollisionActor* actor) const; // Guessed name
  uchar IsTailCollisionActor(CStateManager& mgr,
                             const CCollisionActor* actor) const;         // Guessed name
  void PushPlayer(CStateManager& mgr, const CVector3f& impulse);          // Guessed name
  void ResetAttackCooldown(CStateManager& mgr);                           // Guessed name
  void TeleportPlayerToToss(CStateManager& mgr);                          // Guessed name
  void TossPlayer(CStateManager& mgr);                                    // Guessed name
  void ReleasePlayer(CStateManager& mgr);                                 // Guessed name
  void GrabPlayerIfClose(CStateManager& mgr);                             // Guessed name
  TUniqueId GetFollowTarget(CStateManager& mgr) const;                    // Guessed name
  void ClearTossAttackFlag(CStateManager& mgr);                           // Guessed name
  void SetTossAttackFlag(CStateManager& mgr);                             // Guessed name
  CTransform4f GetFarthestSpawnTransform(CStateManager& mgr, int& index); // Guessed name
  void UpdateEyes(CStateManager& mgr, float dt);
  void UpdateEyeEffects(CStateManager& mgr, float dt);
  void SpawnEyes(CStateManager& mgr);
  void SetEyeVulnerable(CStateManager& mgr, int index, uchar vulnerable);
  void RemoveEyeAnimation(int index);
  void AddEyeAnimation(int index);
  void SpawnEyeKilledEffects(CStateManager& mgr, bool headKilled);
  bool IsActiveEyeAlive(const CStateManager& mgr) const;
  float GetAverageModelScale() const;
  CVector3f GetPincerOffset() const;
  void SetSegmentRange(int first, int last);
  void ClearSegmentRange();
  void UpdateAttackCooldown(float dt);
  void ReverseDirection(CStateManager& mgr);
  template < typename T >
  void TryCommand(EStateMsg msg, pas::EAnimationState state, const T& cmd);

  CSandwormEye* GetHeadEye(CStateManager& mgr) const;
  CSandwormEye* GetTailEye(CStateManager& mgr) const;
  const CSandwormEye* GetHeadEye(const CStateManager& mgr) const;
  const CSandwormEye* GetTailEye(const CStateManager& mgr) const;

  static const float skBlendWeightMin; // Guessed name
  static const float skBlendWeightMax; // Guessed name
  static const float skThresholdMin;   // Guessed name
  static const float skThresholdMax;   // Guessed name

  int mCollisionRadiusMode;                                     // Guessed name
  CPathFindSearch mPathFindSearch;                              // Guessed name
  float mTime;                                                  // Guessed name
  CVector3f mPathDestination;                                   // Guessed name
  bool mStartsUnderground : 1;                                  // Guessed name
  bool mPatrolling : 1;                                         // Guessed name
  bool mFirstThink : 1;                                         // Guessed name
  bool mIsBoss : 1;                                             // Guessed name
  bool mBossEyeEffectsSpawned : 1;                              // Guessed name
  bool mBossParamsSet : 1;                                      // Guessed name
  bool mLocatorsValid : 1;                                      // Guessed name
  bool mUnderground : 1;                                        // Guessed name
  bool mForceUnderground : 1;                                   // Guessed name
  bool mScanVisorActive : 1;                                    // Guessed name
  rstl::optional_object< SLdrSandwormStruct > mSandwormStruct0; // Guessed name
  rstl::optional_object< SLdrSandwormStruct > mSandwormStruct1; // Guessed name
  rstl::optional_object< SLdrSandwormStruct > mSandwormStruct2; // Guessed name
  rstl::optional_object< SLdrSandwormStruct > mSandwormStruct3; // Guessed name
  rstl::optional_object< SLdrSandwormStruct > mSandwormStruct4; // Guessed name
  int x964_;                                                    // Initialized to 0; no reader found
  uchar mSequenceStep;                                          // Guessed name
  float x96c_;                                        // Initialized to -1000; no reader found
  rstl::reserved_vector< CSegId, 13 > mSpineSegIds;   // Guessed name
  rstl::reserved_vector< CSegId, 16 > mLocatorSegIds; // Guessed name
  rstl::single_ptr< CCollisionActorManager > mCollisionActorManager; // Guessed name
  rstl::reserved_vector< CVector3f, 180 > mTrail;                    // Guessed name
  rstl::reserved_vector< CVector3f, 13 > mSpinePositions;            // Guessed name
  rstl::reserved_vector< int, 13 > x12b0_;                           // Never written or read
  float mSegmentBlendWeights[13];                                    // Guessed name
  float mSegmentLengths[12];                                         // Guessed name
  float mLastReverseTime;                                            // Guessed name
  float mAttackCooldown;                                             // Guessed name
  CTransform4f mRiseTransform;                                       // Guessed name
  float mPursueTime;                                                 // Guessed name
  CVector3f mSpineCenterPosition;                                    // Guessed name
  CVector3f mSpineHeadPosition;                                      // Guessed name
  CVector3f mSpineTailPosition;                                      // Guessed name
  float mPlayerOffPathTime;                                          // Guessed name
  CVector3f mPursueStartPosition;                                    // Guessed name
  rstl::single_ptr< CProjectedShadow > mShadow;                      // Guessed name
  rstl::reserved_vector< CTransform4f, 4 > mPincerTransforms;        // Guessed name
  rstl::optional_object< CToken > mStateMachineToken;                // Guessed name
  SAttackHistory mAttackHistory;                                     // Guessed name
  float mFrustrationTime;                                            // Guessed name
  float mPursuitFrustrationRadius;                                   // Guessed name
  float mPursuitFrustrationTimer;                                    // Guessed name
  float mDeathTime;                                                  // Guessed name
  bool mDying : 1;                                                   // Guessed name
  bool mDeathDeleted : 1;                                            // Guessed name
  uchar mBitfieldPadding0[3];                                        // Guessed name
  TUniqueId mHeadEyeId;                                              // Guessed name
  TUniqueId mTailEyeId;                                              // Guessed name
  int mEyeKilledEffectsSpawned;                                      // Guessed name
  uint mHeadEyeAnimation;                                            // Guessed name
  uint mTailEyeAnimation;                                            // Guessed name
  bool mEyeKillReactionDone;                                         // Guessed name
  SChargeData mChargeData;                                           // Guessed name
  float mSideStepTimer;                                              // Guessed name
  float mSideStepScale;                                              // Guessed name
  bool mIsCharging;                                                  // Guessed name
  int mSegmentRangeFirst;                                            // Guessed name
  int mSegmentRangeLast;                                             // Guessed name
  bool mSurfaced : 1;                                                // Guessed name
  bool mWasSurfaced : 1;                                             // Guessed name
  float mSegmentBlendTarget;                                         // Guessed name
  SSpitData mSpitData;                                               // Guessed name
  SMorphballTossData mMorphballTossData;                             // Guessed name
  CVector3f mFrontEyePosition;                                       // Guessed name
  CVector3f mBackEyePosition;                                        // Guessed name
  SMeleeData mMeleeData;                                             // Guessed name
  float mSulkHealthDrop;                                             // Guessed name
  float mLurkUndergroundTimeMin;                                     // Guessed name
  float mLurkUndergroundTimeMax;                                     // Guessed name
  float mSulkHealthPercent;                                          // Guessed name
  float mSulkEndTime;                                                // Guessed name
  float mLastRiseTime;                                               // Guessed name
  float mSavedTurnSpeed;                                             // Guessed name
  bool mTrackPlayerOnRise : 1;                                       // Guessed name
  int mSpawnIndex;                                                   // Guessed name
  SSpinePose mSpinePose;                                             // Guessed name
  CAssetId mPincerL;                                                 // Guessed name
  CAssetId mPincerR;                                                 // Guessed name
  float mPincerScale;                                                // Guessed name
  rstl::optional_object< CModelData > mPincerModelL;                 // Guessed name
  rstl::optional_object< CModelData > mPincerModelR;                 // Guessed name
  float mFrontClawAngle;                                             // Guessed name
  float mBackClawAngle;                                              // Guessed name
  float mLastDamageTime;                                             // Guessed name
  SSoundData mSoundData;                                             // Guessed name
  TUniqueId mCoverPointId;                                           // Guessed name
  TUniqueId mLastCoverPointId;                                       // Guessed name
  bool mGoToCover;                                                   // Guessed name
  uchar mBitfieldPadding1[3];                                        // Guessed name
  mutable bool mPlayerReachable;                                     // Guessed name
  mutable float mPlayerReachableTime;                                // Guessed name
  mutable CPathFindSearch mReachablePathSearch;                      // Guessed name
  SBombData mBombData;                                               // Guessed name
  float mBombDropRate;                                               // Guessed name
  float mBombDropTimer;                                              // Guessed name
  float mLastBombDropTime;                                           // Guessed name
  SBombTossData mBombTossData;                                       // Guessed name
  SBombSpreadData mBombSpreadData;                                   // Guessed name
  SBombFountainData mFountainData;                                   // Guessed name
  SEyeEffects mEyeEffects;                                           // Guessed name
  CVector3f mCachedPlayerPosition;                                   // Guessed name
  float mCachedPlayerPositionTime;                                   // Guessed name
  float mDefaultTurnSpeed;                                           // Guessed name
  float mTurnSpeedTimer;                                             // Guessed name
  float mBombAimTimer;                                               // Guessed name
  CAABox mCachedBounds;                                              // Guessed name
  float mCachedBoundsTime;                                           // Guessed name
};

CHECK_SIZEOF(CSandworm, 0x1a28)

// Original class name from the Wii SEL exports. Lives in the Sandworm REL.
class CSandwormEye : public CPhysicsActor {
public:
  CSandwormEye(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
               const CTransform4f& xf);

  // CEntity
  ~CSandwormEye() override;
  CEntity* TypesMatch(int typeId) const override;

  // CActor
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  CVector3f GetOrbitPosition(const CStateManager& mgr) const override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;

  // Guessed names
  void SetTouchBounds(const CAABox& bounds);
  CVector3f GetOrbitPosition() const;
  void SetOrbitPosition(const CVector3f& position);
  void SetFlag0();
  bool GetFlag0() const;
  void SetKilled();
  bool IsKilled() const;
  bool IsOrbitable() const;
  void SetOrbitable(CStateManager& mgr, bool orbitable);

private:
  CAABox mTouchBounds;           // Guessed name
  CVector3f mOrbitPosition;      // Guessed name
  bool mPositionInitialized : 1; // Guessed name
  bool mKilled : 1;              // Guessed name
  CRELFileToken mRelToken;       // Guessed name
};
CHECK_SIZEOF(CSandwormEye, 0x300)

// Guessed names. DOL queries forward through the Sandworm REL's registered callbacks;
// they do not require the concrete enemy layout in the DOL.
int GetSandwormRadarPointCount(const CSandworm* sandworm);
CVector3f GetSandwormRadarPointPosition(const CSandworm* sandworm, int index);

#endif // _CSANDWORM
