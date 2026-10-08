#ifndef _CINGSPIDERBALLGUARDIAN
#define _CINGSPIDERBALLGUARDIAN

#include "types.h"

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/ScriptLoader/SLdrIngSpiderballGuardian.hpp"

// Guessed class: the Spiderball Guardian boss, a rolling ball that follows waypoint paths.
class CIngSpiderballGuardian : public CPatterned {
public:
  // Guessed names; the rolling behaviour state.
  enum EGuardianState {
    kGS_Patrol,
    kGS_Damaged,
    kGS_Stunned,
    kGS_Charging,
  };

  CIngSpiderballGuardian(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                         const CTransform4f& xf, const CModelData& modelData,
                         const CActorParameters& actorParams, const CPatternedInfo& patternedInfo,
                         const SLdrIngSpiderballGuardianData& data);

  // CEntity
  ~CIngSpiderballGuardian() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CAi
  bool IsListening() const override { return true; }
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;

  // CPatterned
  void SetupStateMachine(CStateManager& mgr) override;

  // CIngSpiderballGuardian
  void BeginNextPuzzleSection(CStateManager& mgr, float dt);
  void EnableEnergyBar(CStateManager& mgr, float dt);
  void Dead(CStateManager& mgr, EStateMsg msg, float dt);
  void StunnedReaction(CStateManager& mgr, EStateMsg msg, float dt);
  void DamageReaction(CStateManager& mgr, EStateMsg msg, float dt);
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt);
  void Start(CStateManager& mgr, EStateMsg msg, float dt);
  bool Stunned(CStateManager& mgr, const CTriggerData& data) const;
  bool Damaged(CStateManager& mgr, const CTriggerData& data) const;
  bool StateOver(CStateManager& mgr, const CTriggerData& data) const;

private:
  const SLdrIngSpiderballGuardianStruct& GetPhaseProperties() const; // Guessed name
  void ApplyStunDamage(CStateManager& mgr, float damage);            // Guessed name
  void SetGuardianState(CStateManager& mgr, EGuardianState state);   // Guessed name
  void UpdateRollSound();                                            // Guessed name
  void UpdateStateTimers(CStateManager& mgr, float dt);              // Guessed name
  void UpdateEffects(CStateManager& mgr, bool active);               // Guessed name
  void ApplyProximityDamage(CStateManager& mgr);                     // Guessed name
  void MoveAlongWaypoints(CStateManager& mgr, float dt);             // Guessed name
  void SelectNextWaypoint(CStateManager& mgr);                       // Guessed name

  SLdrIngSpiderballGuardianData mData; // Guessed name
  CDamageInfo mProximityDamage;        // Guessed name
  TUniqueId x944_;                     // Never read by this REL.
  CSfxHandle mRollSfx;                 // Guessed name
  EGuardianState mState;               // Guessed name
  EGuardianState mSoundState;          // Guessed name
  int mHitCount;                       // Guessed name
  int mPhase;                          // Guessed name
  TUniqueId mPrevWaypointId;           // Guessed name
  TUniqueId mTargetWaypointId;         // Guessed name
  CVector3f mPrevWaypointPos;          // Guessed name
  float mAnimSpeed;                    // Guessed name
  float mRadius;                       // Guessed name
  float mDefaultSpeed;                 // Guessed name
  float mWaypointSpeed;                // Guessed name
  float mCurrentSpeed;                 // Guessed name
  float mTargetSpeed;                  // Guessed name
  float mStunHealth;                   // Guessed name
  float mStunTimer;                    // Guessed name
  float mRecoveryDelay;                // Guessed name
  float mChargeTimer;                  // Guessed name
  bool mEffectsActive : 1;             // Guessed name
  bool mProximityDamageEnabled : 1;    // Guessed name
  bool mRolling : 1;                   // Guessed name
  bool mStunned : 1;                   // Guessed name
  bool mCanBeDamaged : 1;              // Guessed name
  bool mPlayStunSound : 1;             // Guessed name
  bool mReturnToPatrolRequested : 1;   // Guessed name
};
CHECK_SIZEOF(CIngSpiderballGuardian, 0x998)

#endif // _CINGSPIDERBALLGUARDIAN
