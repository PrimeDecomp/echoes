#ifndef _CLUMITE
#define _CLUMITE

#include "types.h"

#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CSurfaceAlignmentHelper.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/ScriptLoader/SLdrLumite.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"

class CElementGen;
class CGenDescription;
class CScriptSafeZone;
class CScriptTeamAiMgr;

// Guessed class: a creature that hops between surfaces and spits projectiles; it is only
// visible and targetable while it is inside sunlight.
class CLumite : public CPatterned {
public:
  CLumite(TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
          const CModelData& modelData, const CPatternedInfo& patternedInfo, CAssetId stateMachine2,
          float attackTimeMin, float attackTimeMax, float smallShotMinRange,
          float smallShotMaxRange, float bigShotMinRange, float bigShotMaxRange,
          float minHopDistance, float maxHopDistance, CAssetId smallShotProjectile,
          const CDamageInfo& smallShotDamage, CAssetId bigShotProjectile,
          const CDamageInfo& bigShotDamage, CAssetId trailEffect, CAssetId sunlightEffect,
          ushort phaseInSound, ushort phaseOutSound, const CActorParameters& actorParams);

  // CEntity
  ~CLumite() override {}
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  CVector3f GetOrbitPosition(const CStateManager& mgr) const override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;
  CScannableObjectInfo* GetScannableObjectInfo() const override;

  // CPhysicsActor
  void CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                    CStateManager& mgr) override;

  // CPatterned
  CProjectileInfo* ProjectileInfo() override;
  float GetGravityConstant() const override { return 0.f; }

  // CLumite
  virtual bool InSmallShotRange(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool InBigShotRange(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool InSunlight(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool FacingPlayer(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool CanAttack(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ReadyToMove(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool Landed(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool IsHeckler(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool CanTaunt(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool FacingHopPoint(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ClearLineOfFire(CStateManager& mgr, const CTriggerData& data) const;
  virtual void Null(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Pause(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Hop(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void SmallShot(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void BigShot(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Taunt(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Turn(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void TurnToHopPoint(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Dead(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void StickToNearestSurface(CStateManager& mgr, float dt);

private:
  // Guessed names
  struct SShotAttack {
    SShotAttack(CAssetId projectile, const CDamageInfo& damage, float minRange, float maxRange);

    CProjectileInfo mProjectile;
    float mMinRange;
    float mMaxRange;
  };

  struct SSurfaceTarget {
    SSurfaceTarget();

    CPlane mPlane;
    CVector3f mClosestPoint;
    int mKind; // 0 floor, 1 wall or ceiling, 2 safe zone
    TUniqueId mZoneId;
  };

  struct SHopPlan {
    SHopPlan(float minDistance, float maxDistance);
    void Reset();

    float mNextHopTime;
    CVector3f mTarget;
    CVector3f mLaunchDirection;
    TUniqueId mHintId;
    CVector3f x20_;
    CPlane mPlane;
    float mStartTime;
    float mSpeedScale;
    float mAngleDiff;
    float mDistance;
    float mMinDistance;
    float mMaxDistance;
    rstl::reserved_vector< TUniqueId, 16 > mCandidates;
    TUniqueId mChosenHintId;
    float mChosenScore;
    uint mIgnoreInUse : 1;
    uint mHopSelected : 1;
    uint mHopFailed : 1;
    uint mLaunched : 1;
    uint mFinished : 1;
    uint mLanded : 1;
    uint mDidSideTaunt : 1;
    uint mAborted : 1;
    uint mSunlightHop : 1;
  };

  struct SSunlightEffect {
    explicit SSunlightEffect(CAssetId id);
    ~SSunlightEffect() {}

    CAssetId mId;
    rstl::auto_ptr< CElementGen > mParticle;
    TLockedToken< CGenDescription > mToken;
    TUniqueId mExplosionId;
  };

  void SetupStateMachineHelper(CStateManager& mgr);
  void DisableGroundCollision(CStateManager& mgr);
  void StickToSurface(float dt);
  void UpdateSunlight(CStateManager& mgr, float dt);
  bool IsTouchingSunTrigger(CStateManager& mgr) const;
  void DamageSafeZoneOccupant(CStateManager& mgr, float dt);
  void SpawnZoneImpact(CStateManager& mgr, const CVector3f& position, float scale);
  void PlaySound(CStateManager& mgr, ushort sfx);
  int GetKneeIndex(const rstl::string& name) const;
  rstl::string GetKneeName(int index) const;

  void JoinTeam(CStateManager& mgr);
  void QuitTeam(CStateManager& mgr);
  void StartTeamAction(CStateManager& mgr);
  void EndTeamAction(CStateManager& mgr);
  CScriptTeamAiMgr* GetTeamAiMgr(CStateManager& mgr);
  const CScriptTeamAiMgr* GetTeamAiMgr(const CStateManager& mgr) const;
  int GetTeamRole(const CStateManager& mgr) const;

  void SetAttackState(int state, EStateMsg msg);
  void ResetAttackTimer(CStateManager& mgr);
  float DistanceToPlayer(const CStateManager& mgr) const;
  CVector3f RandomVector(CStateManager& mgr, float scale);
  CTransform4f BuildProjectileTransform(CStateManager& mgr, float spread);
  void FireSmallShot(CStateManager& mgr);
  void FireBigShot(CStateManager& mgr);

  bool FacingHint(const CVector3f& point) const;
  void TurnTowards(EStateMsg msg, const CVector3f& target);
  CScriptSafeZone* FindSafeZone(CStateManager& mgr, const CVector3f& position);
  bool TrySunlightHop(CStateManager& mgr, const CVector3f& hintPosition, CPlane& outPlane);
  void SelectHopTarget(CStateManager& mgr, const TUniqueId& hintId);
  bool ChooseHop(CStateManager& mgr);
  void BuildHopCandidates(CStateManager& mgr);

  template < typename T >
  void DeliverCommand(EStateMsg msg, pas::EAnimationState state, const T& cmd);

  float mElapsedTime;
  CVector3f mSpitPosition;
  CSurfaceAlignmentHelper mSurfaceAlign;
  rstl::optional_object< CToken > mStateMachine2;
  float mAttackTimeMin;
  float mAttackTimeMax;
  float mNextAttackTime;
  SShotAttack mSmallShot;
  SShotAttack mBigShot;
  int mAttackState;
  bool mTurnActive;
  bool mTurnLeft;
  SSurfaceTarget mSurface;
  uchar mKneeMask;
  SHopPlan mHop;
  bool mInSunlight : 1;
  bool mTransitioning : 1;
  CVector3f mLastSunlightPosition;
  float mAlphaFactor;
  float x968_;
  SSunlightEffect mSunlightEffect;
  pas::ETauntType mTauntType;
  pas::EStepDirection mTauntStepDir;
  float mLastTauntTime;
  TUniqueId mTeamAiMgrId;
  float mSpinRate;
  float mBounceSpeed;
  ushort mPhaseInSound;
  ushort mPhaseOutSound;
};
CHECK_SIZEOF(CLumite, 0x9a8)

#endif // _CLUMITE
