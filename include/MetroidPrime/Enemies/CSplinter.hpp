#ifndef _CSPLINTER
#define _CSPLINTER

#include "types.h"

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CSurfaceAlignmentHelper.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"

class CGenDescription;
class CScriptAIHint;
class CScriptAiJumpPoint;
class CScriptWaypoint;
class CScriptTeamAiMgr;
class CSplinterAcidSac;

// Guessed class: a spider-like Ing creature that climbs cables, grabs the morph ball with its
// acid sac and pounces; the mega variant spits projectiles.
class CSplinter : public CPatterned {
public:
  // Guessed name; aligns the splinter to the walls and ceilings it crawls over.
  struct SSurfaceAlignment : public CSurfaceAlignmentHelper {
    SSurfaceAlignment();

    CVector3f mLastSurfacePosition; // Guessed name
  };

  // Guessed name; the optional generic state machine selected by the script.
  struct SStateMachine2 {
    SStateMachine2(CAssetId stateMachine);

    rstl::optional_object< CToken > mToken;
  };

  // Guessed name; a particle effect and the damage it applies.
  struct SParticleData {
    SParticleData(CAssetId particle, const CDamageInfo& damage);

    rstl::optional_object< TLockedToken< CGenDescription > > mEffect;
    CDamageInfo mDamage;
  };

  // Guessed names; the jump parameters chosen by the jump point search.
  struct SJumpData {
    SJumpData();
    void Reset();

    // Bit order follows the original bitfield declaration.
    bool mLanded : 1;
    bool mIsHighJump : 1;
    bool mUnknown2 : 1;
    bool mHasTarget : 1;
    bool mJumping : 1;
    bool mHasValidTarget : 1;
    bool mVelocitySet : 1;
    bool mHasLineOfSight : 1;
    bool mUnknown8 : 1;
    CVector3f mStart;
    CVector3f mTarget;
    float mHeight;
    int mPhase;
  };

  // Guessed names; the sidestep directions tested for free space.
  enum EEvadeDirection { kED_Left, kED_Right, kED_Back, kED_Forward };

  // Guessed name; a line tested for free space when choosing a sidestep.
  struct SEvadeCheck {
    SEvadeCheck() : mStart(CVector3f::Zero()), mEnd(CVector3f::Zero()), mClear(false) {}

    CVector3f mStart;
    CVector3f mEnd;
    bool mClear;
  };

  // Guessed names; the acid sac carried below the cable and the morph ball it grabbed.
  struct SSacData {
    SSacData(int a, int b, int c, int d, bool e, bool f);
    ~SSacData();

    TUniqueId mGrabbedId;
    float mDescendStartZ;
    CVector3f mMorphBallPos;
    float mMorphBallIdleTime;
    CVector3f mUnused;
    bool mSacOnCable : 1;
    bool mHoldingSac : 1;
    bool mCarriesSac : 1;
    int mSacAncs;
    int mSacCharacter;
    int mSacAnim;
    int mUnknown34;
    TUniqueId mSacId;
    CVector3f mSacOffset;
    float mDropHeight;
  };

  // Guessed names; the mega splinter's spit attack.
  struct SSpitData {
    SSpitData(float f1, CAssetId projectile, float f2, const CDamageInfo& damage,
              CAssetId visorEffect);

    CVector3f mPosition;
    CProjectileInfo mProjectileInfo;
    rstl::optional_object< TLockedToken< CGenDescription > > mVisorEffect;
    CVector3f mOrbitPosition;
    float mUnknown50;
    float mUnknown54;
    float mUnknown58;
    float mUnknown5c;
    float mUnknown60;
    bool mSpitting : 1;
    bool mOrbitValid : 1;
  };

  CSplinter(TUniqueId uid, const rstl::string& name, CEntityInfo& info, const CTransform4f& xf,
            const CModelData& modelData, const CPatternedInfo& patternedInfo,
            CAssetId stateMachine2, float f1, float f2, float f3, float f4, float f5, float f6,
            float f7, const CDamageInfo& attackDamage, bool isWorker, bool skipAlert, int i1,
            int i2, bool inCocoon, bool b2, bool b3, bool b5, int sacAncs, int sacCharacter,
            int sacAnim, int sacAnim2, CAssetId particle, const CDamageInfo& damageInfo,
            bool isMegaSplinter, CAssetId spitProjectile, const CDamageInfo& spitDamage,
            CAssetId spitVisorEffect, const CActorParameters& actorParams);

  // CEntity
  ~CSplinter() override {}
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void Render(const CStateManager& mgr) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  CVector3f GetOrbitPosition(const CStateManager& mgr) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CPhysicsActor
  void CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                    CStateManager& mgr) override;

  // CAi
  void KnockBack(CStateManager& mgr, const CKnockBackInfo& info) override;
  bool IsListening() const override;
  bool Listen(CStateManager& mgr, const CVector3f& position, EListenNoiseType type) override;

  // CPatterned
  void MassiveDeath(CStateManager& mgr) override;
  CProjectileInfo* ProjectileInfo() override;
  CPathFindSearch* GetSearchPath() override { return &mPathFindSearch; }
  CDamageInfo GetContactDamage() const override;
  void SetupStateMachine(CStateManager& mgr) override;
  float GetGravityConstant() const override;

  // CSplinter triggers
  virtual bool AlertMessageReceived(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool AttackPointIsLegal(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool CableShot(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool CanPounce(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool CanSeePlayer(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool DropCompleted(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool FacingPlayer(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool FacingPlayerExactly(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool FleesFromPlayer(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool GrabbedIt(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool HasAnyLandPoints(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool HasValidJumpTarget(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool HoldingSac(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool InAttackRange(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool InCocoon(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool IngSwarmIncoming(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool IsDarkSplinter(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool IsHeckler(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool InHeckleRange(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool IsMegaDark(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool IsMegaLight(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool IsMelee(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool IsOnCable(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool IsOnPad(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool IsWorkerSplinter(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool Landed(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool MBallMoved(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool MorphballWaiting(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool NeedsToSlideOff(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool PlayerIsInSafeZone(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool PounceTimeOut(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ReachedMBall(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ReachedSac(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ReachedTop(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool SacMissing(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool SacPresent(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ShotFromOtherArea(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ShouldDrop(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ShouldEmergeFromCocoon(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ShouldTaunt(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ShouldUnHide(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool SkipAlert(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool Stuck(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool StuckOnTeammate(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool TeamAttacked(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool TooMuchTurning(CStateManager& mgr, const CTriggerData& data) const;

  // CSplinter states
  virtual void Alert(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Dead(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Descend(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Drop(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void DropAndExplode(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void FallFromCocoon(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void FastTurn(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void GoToPad(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void GrabMBall(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void GrabSac(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Jump(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void JumpToPoint(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void MegaSplinterSpit(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Null(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void PathFind(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Patrol(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Pounce(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Retreat(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Rise(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void RunAndHide(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Scream(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Sidestep(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void SlideOff(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Taunt(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void TurnToPlayer(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void WaitInCocoon(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void WaitOnCable(CStateManager& mgr, EStateMsg msg, float dt);

private:
  void SetupStateMachineHelper(CStateManager& mgr);                     // Guessed name
  void JoinTeam(CStateManager& mgr);                                    // Guessed name
  void QuitTeam(CStateManager& mgr);                                    // Guessed name
  bool IsMega() const;                                                  // Guessed name
  const CActor* GetGrabbed(const CStateManager& mgr) const;             // Guessed name
  CActor* FindGrabbed(CStateManager& mgr) const;                        // Guessed name
  CSplinterAcidSac* GetSac(CStateManager& mgr) const;                   // Guessed name
  bool HasUnpoppedSac(CStateManager& mgr) const;                        // Guessed name
  void PopSac(CStateManager& mgr);                                      // Guessed name
  bool IsSacMissing(const CStateManager& mgr) const;                    // Guessed name
  bool IsGrabbingPlayer(CStateManager& mgr) const;                      // Guessed name
  void PlayStepAnim(CStateManager& mgr, int direction, bool loop);      // Guessed name
  void AttachMorphBall(CStateManager& mgr);                             // Guessed name
  void DetachMorphBall(CStateManager& mgr);                             // Guessed name
  TUniqueId FindConnectedId(const CStateManager& mgr) const;            // Guessed name
  void CreateSac(CStateManager& mgr);                                   // Guessed name
  void SetCableMovement(CStateManager& mgr, bool onCable);              // Guessed name
  const CScriptTeamAiMgr* GetTeamAiMgr(const CStateManager& mgr) const; // Guessed name
  CScriptTeamAiMgr* FindTeamAiMgr(CStateManager& mgr) const;            // Guessed name
  bool HasRetreatPath(CStateManager& mgr) const;                        // Guessed name
  float GetHeckleRange() const;                                         // Guessed name
  CVector3f GetPlayerTargetPosition(CStateManager& mgr) const;          // Guessed name
  bool IsFacing(const CVector3f& point, float angle) const;             // Guessed name
  void RotateToPlayer(CStateManager& mgr, float dt, float turnSpeed,
                      float aimOffset);                                      // Guessed name
  void FaceTargetAtPlayer(CStateManager& mgr);                               // Guessed name
  bool IsNearPath(const CVector3f& position, float tolerance) const;         // Guessed name
  CScriptAIHint* FindPadHint(CStateManager& mgr) const;                      // Guessed name
  void StartPathTo(CStateManager& mgr, const CVector3f& position, float dt); // Guessed name
  void GoToTarget(CStateManager& mgr, EStateMsg msg, const CActor* target,
                  float dt);                                                    // Guessed name
  void RecoverCollision(CStateManager& mgr);                                    // Guessed name
  void SetUnhideTime(CStateManager& mgr);                                       // Guessed name
  void AddEvadeCheck(const CVector3f& start, const CVector3f& end, bool clear); // Guessed name
  bool CanEvade(CStateManager& mgr, EEvadeDirection direction);                 // Guessed name
  int ChooseEvasion(CStateManager& mgr, bool preferStep);                       // Guessed name
  void DoEvasion(CStateManager& mgr, EStateMsg msg);                            // Guessed name
  template < typename T >
  void DeliverCommand(EStateMsg msg, pas::EAnimationState state, const T& cmd); // Guessed name
  void KillSelf(CStateManager& mgr);                                            // Guessed name
  void ExplodeSac(CStateManager& mgr);                                          // Guessed name
  bool IsAttackBlocked(CStateManager& mgr) const;                               // Guessed name
  void CheckFooting(CStateManager& mgr);                                        // Guessed name
  void UpdateAlignmentMode(EStateMsg msg);                                      // Guessed name
  bool IsNearWall(CStateManager& mgr) const;                                    // Guessed name
  float ScoreJump(const CVector3f& jumpPoint, const CVector3f& waypoint,
                  const CVector3f& playerPosition, bool nearWall,
                  float weight) const; // Guessed name
  rstl::pair< CScriptAiJumpPoint*, CScriptWaypoint* >
  FindJumpPoints(CStateManager& mgr) const;                              // Guessed name
  bool FindJump(CStateManager& mgr);                                     // Guessed name
  void ApplyJumpVelocity();                                              // Guessed name
  void ApplyLaunchVelocity();                                            // Guessed name
  void ApplyAmbushVelocity(bool playerMorphed);                          // Guessed name
  void UpdatePitchBend();                                                // Guessed name
  void InitPathArea(CStateManager& mgr);                                 // Guessed name
  CVector3f GetAttackPosition(CStateManager& mgr) const;                 // Guessed name
  void UpdateAlignment(CStateManager& mgr, float dt);                    // Guessed name
  void UpdateSwarmGrab(CStateManager& mgr);                              // Guessed name
  void UpdateEyes(CStateManager& mgr);                                   // Guessed name
  void UpdateHealthMessage(CStateManager& mgr, float dt);                // Guessed name
  float GetPatrolDelay(CStateManager& mgr) const;                        // Guessed name
  CScriptAIHint* FindHideHint(CStateManager& mgr) const;                 // Guessed name
  bool HasClearPathTo(CStateManager& mgr, const CVector3f& point) const; // Guessed name

  TUniqueId mTeamAiMgrId;                               // Guessed name
  CPathFindSearch mPathFindSearch;                      // Guessed name
  float mTime;                                          // Guessed name
  float mUnknown8b4;                                    // Guessed name
  float mUnhideTime;                                    // Guessed name
  TUniqueId mUnknownId;                                 // Guessed name
  float mStuckDeadline;                                 // Guessed name
  CVector3f mSeparationForce;                           // Guessed name
  float mUnknown8d0;                                    // Guessed name
  float mUnknown8d4;                                    // Guessed name
  float mUnknown8d8;                                    // Guessed name
  mutable float mUnknown8dc;                            // Guessed name
  mutable uchar mUnknown8e0;                            // Guessed name
  CSegId mWebAttachLocator;                             // Guessed name
  CVector3f mUnknown8e4;                                // Guessed name
  CVector3f mEyeDirection;                              // Guessed name
  uchar mUnknown8fc[0x30];                              // Guessed name
  uchar mUnknown92c;                                    // Guessed name
  float mUnknown930;                                    // Guessed name
  bool mUnknown934_0 : 1;                               // Guessed name
  bool mInCocoon : 1;                                   // Guessed name
  bool mUnknown934_2 : 1;                               // Guessed name
  bool mUnknown934_3 : 1;                               // Guessed name
  bool mAlertMessageReceived : 1;                       // Guessed name
  bool mIsWorker : 1;                                   // Guessed name
  bool mSkipAlert : 1;                                  // Guessed name
  bool mUnknown934_7 : 1;                               // Guessed name
  bool mIngEffectActive : 1;                            // Guessed name
  CVector3f mPathDestination;                           // Guessed name
  float mUnknown944;                                    // Guessed name
  SSurfaceAlignment mAlignment;                         // Guessed name
  mutable int mUnknown9b0;                              // Guessed name
  int mEvasion;                                         // Guessed name
  int mPrevEvasion;                                     // Guessed name
  SJumpData mJump;                                      // Guessed name
  float mUnknown9e0;                                    // Guessed name
  float mUnknown9e4;                                    // Guessed name
  float mUnknown9e8;                                    // Guessed name
  float mUnknown9ec;                                    // Guessed name
  int mUnknown9f0;                                      // Guessed name
  int mUnknown9f4;                                      // Guessed name
  CDamageInfo mAttackDamage;                            // Guessed name
  rstl::reserved_vector< SEvadeCheck, 4 > mEvadeChecks; // Guessed name
  TUniqueId mUnknowna88;                                // Guessed name
  bool mUnknowna8a : 1;                                 // Guessed name
  SSacData mSac;                                        // Guessed name
  float mUnknownad8;                                    // Guessed name
  uchar mUnknownadc;                                    // Guessed name
  float mUnknownae0;                                    // Guessed name
  SStateMachine2 mStateMachine2;                        // Guessed name
  SParticleData mParticle;                              // Guessed name
  float mUnknownb1c;                                    // Guessed name
  uchar mUnknownb20;                                    // Guessed name
  float mUnknownb24;                                    // Guessed name
  rstl::optional_object< SSpitData > mSpit; // Guessed name; engaged for the mega splinter
  float mUnknownb94;                        // Guessed name
  mutable TUniqueId mUnknownb98;            // Guessed name
};

CHECK_SIZEOF(CSplinter, 0xba0)

#endif // _CSPLINTER
