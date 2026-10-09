#ifndef _CDARKCOMMANDO
#define _CDARKCOMMANDO

#include "types.h"

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CBoneTracking.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CRELFileToken.hpp"
#include "MetroidPrime/Enemies/CBouncyGrenade.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDarkCommando.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

class CElementGen;
class CPirateRagDoll;
class CScriptAIHint;

// Guessed class: the Dark Commando, a cloaking Ing trooper that melees, launches EMP grenades,
// fires a charge beam that leaves a mold on the player's visor and shadow-dashes between hints.
class CDarkCommando : public CPatterned {
public:
  CDarkCommando(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                const CTransform4f& xf, const CModelData& modelData,
                const CActorParameters& actorParams, const CPatternedInfo& patternedInfo,
                const SLdrDarkCommandoData& data);

  // CEntity
  ~CDarkCommando() override;
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  void PreRenderAllViewports(CStateManager& mgr) override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CAi
  void KnockBack(CStateManager& mgr, const CKnockBackInfo& info) override;

  // CPatterned
  CProjectileInfo* ProjectileInfo() override;
  float GetGravityConstant() const override { return 50.f; }
  void SetupStateMachine(CStateManager& mgr) override;

  // Triggers
  bool ShouldGetUp(CStateManager& mgr, const CTriggerData& data) const;          // Guessed name
  bool ShouldWarpOut(CStateManager& mgr, const CTriggerData& data) const;        // Guessed name
  bool ShouldMeleeAttack(CStateManager& mgr, const CTriggerData& data) const;    // Guessed name
  bool ShouldFireChargeBeam(CStateManager& mgr, const CTriggerData& data) const; // Guessed name
  bool ShouldFireEMP(CStateManager& mgr, const CTriggerData& data) const;        // Guessed name
  bool ShouldTaunt(CStateManager& mgr, const CTriggerData& data) const;          // Guessed name
  bool StateOver(CStateManager& mgr, const CTriggerData& data) const;            // Guessed name

  // States
  void Start(CStateManager& mgr, EStateMsg msg, float dt);              // Guessed name
  void Dead(CStateManager& mgr, EStateMsg msg, float dt);               // Guessed name
  void WarpIn(CStateManager& mgr, EStateMsg msg, float dt);             // Guessed name
  void WarpOut(CStateManager& mgr, EStateMsg msg, float dt);            // Guessed name
  void Lurk(CStateManager& mgr, EStateMsg msg, float dt);               // Guessed name
  void Taunt(CStateManager& mgr, EStateMsg msg, float dt);              // Guessed name
  void MeleeAttack(CStateManager& mgr, EStateMsg msg, float dt);        // Guessed name
  void FireEMP(CStateManager& mgr, EStateMsg msg, float dt);            // Guessed name
  void FireChargeBeam(CStateManager& mgr, EStateMsg msg, float dt);     // Guessed name
  void PostFireChargeBeam(CStateManager& mgr, EStateMsg msg, float dt); // Guessed name
  void ShadowDash(CStateManager& mgr, EStateMsg msg, float dt);         // Guessed name
  void FaceTarget(CStateManager& mgr, EStateMsg msg, float dt);         // Guessed name
  void GetUp(CStateManager& mgr, EStateMsg msg, float dt);              // Guessed name

  // Code functions
  void SelectAttackTarget(CStateManager& mgr, float dt); // Guessed name
  void SelectDashTarget(CStateManager& mgr, float dt);   // Guessed name
  void SelectAction(CStateManager& mgr, float dt);       // Guessed name
  void SetDashFaceVect(CStateManager& mgr, float dt);    // Guessed name
  void SetTargetFaceVect(CStateManager& mgr, float dt);  // Guessed name
  void ActivateCloak(CStateManager& mgr, float dt);      // Guessed name
  void DeactivateCloak(CStateManager& mgr, float dt);    // Guessed name

private:
  typedef rstl::pair< int, float > TActionChoice; // Guessed name: action id and its weight

  void LeaveTeam(CStateManager& mgr);                                // Guessed name
  void JoinTeam(CStateManager& mgr);                                 // Guessed name
  void UpdateSfx();                                                  // Guessed name
  void ThinkRagDoll(float dt, CStateManager& mgr, bool skipRagDoll); // Guessed name
  void ReserveHint(CStateManager& mgr, TUniqueId id);                // Guessed name
  void ReleaseHint(CStateManager& mgr);                              // Guessed name
  const CScriptAIHint* GetHint(const CStateManager& mgr) const;      // Guessed name
  void SolveGrenadeLaunch(const CVector3f& target, const CVector3f& origin, float& angle,
                          float& speed) const; // Guessed name
  CVector3f GetGrenadeTargetPosition(const CStateManager& mgr,
                                     const CActor& target) const; // Guessed name
  void LaunchGrenade(CStateManager& mgr);                         // Guessed name
  void FireChargeBeamShot(CStateManager& mgr, float dt);          // Guessed name
  void UpdateAdditiveAim(CStateManager& mgr);                     // Guessed name
  bool InChargeBeamRange(const CStateManager& mgr) const;         // Guessed name
  bool InEMPRange(const CStateManager& mgr) const;                // Guessed name
  void BuildActionChoices(CStateManager& mgr,
                          rstl::reserved_vector< TActionChoice, 4 >& choices); // Guessed name
  int SelectMeleeVariant(CStateManager& mgr) const;                            // Guessed name
  void ApplyMeleeDamage(CStateManager& mgr, CPhysicsActor& target);            // Guessed name
  void UpdateCloak(float dt, CStateManager& mgr);                              // Guessed name
  void SetCloakTarget(float target, float duration);                           // Guessed name
  template < typename T >
  void DeliverCommand(EStateMsg msg, pas::EAnimationState state, const T& cmd); // Guessed name

  SLdrDarkCommandoData mData;                                           // Guessed name
  int mCurrentAction;                                                   // Guessed name
  int mAttackState;                                                     // Guessed name
  rstl::single_ptr< CPirateRagDoll > mRagDoll;                          // Guessed name
  float mRagDollTimer;                                                  // Guessed name
  CBoneTracking mBoneTracking;                                          // Guessed name
  CProjectileInfo mChargeBeamInfo;                                      // Guessed name
  CDamageVulnerability mShadowDashVulnerability;                        // Guessed name
  CDamageInfo mBladeDamage;                                             // Guessed name
  CSegId mMeleeSegId;                                                   // Guessed name
  CSegId mWristSegId;                                                   // Guessed name
  CSegId mChargeBeamSegId;                                              // Guessed name
  CSegId mGrenadeSegId;                                                 // Guessed name
  float mCloakAlpha;                                                    // Guessed name
  float mCloakTargetAlpha;                                              // Guessed name
  float mCloakRate;                                                     // Guessed name
  TUniqueId mHintId;                                                    // Guessed name
  TUniqueId mPrevHintId;                                                // Guessed name
  TUniqueId mTargetId;                                                  // Guessed name
  TUniqueId mDecoyId;                                                   // Guessed name
  TUniqueId mTeamAiMgrId;                                               // Guessed name
  CVector3f mFaceVector;                                                // Guessed name
  CSfxHandle mSfxHandle;                                                // Guessed name
  rstl::optional_object< TLockedToken< CGenDescription > > mMoldEffect; // Guessed name
  bool mMeleeAttacking : 1;                                             // Guessed name
  bool mFiringEMP : 1;                                                  // Guessed name
  bool mFiringChargeBeam : 1;                                           // Guessed name
  bool mWarpingIn : 1;                                                  // Guessed name
  bool mWarpingOut : 1;                                                 // Guessed name
  bool mCloakFading : 1;                                                // Guessed name
  bool mShadowDashing : 1;                                              // Guessed name
  bool mMeleeDamageApplied : 1;                                         // Guessed name
  bool mDashMoving : 1;                                                 // Guessed name
  bool mWarpOutRequested : 1;                                           // Guessed name
  bool mKnockedBack : 1;                                                // Guessed name
  bool mHurled : 1;                                                     // Guessed name
};
CHECK_SIZEOF(CDarkCommando, 0xcb8)

// Guessed class: the charge beam shot, which leaves a mold on the player's visor.
class CDarkCommandoChargeBeam : public CEnergyProjectile {
public:
  CDarkCommandoChargeBeam(
      const TToken< CWeaponDescription >& description, const CTransform4f& xf,
      const CDamageInfo& damage, TUniqueId uid, TAreaId areaId, TUniqueId owner,
      const rstl::optional_object< TLockedToken< CGenDescription > >& moldEffect,
      const CDamageInfo& moldDamage);

  // CEntity
  ~CDarkCommandoChargeBeam() override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CGameProjectile
  void ResolveCollisionWithActor(const CRayCastResult& result, CActor& actor,
                                 CStateManager& mgr) override;

private:
  CDamageInfo mMoldDamage;                                              // Guessed name
  rstl::optional_object< TLockedToken< CGenDescription > > mMoldEffect; // Guessed name
  TUniqueId mBillboardId;                                               // Guessed name
  float mTimer;                                                         // Guessed name
  TUniqueId mPlayerId;                                                  // Guessed name
  CRELFileToken mRelToken;                                              // Guessed name
};
CHECK_SIZEOF(CDarkCommandoChargeBeam, 0x5a8)

// Guessed class: the EMP grenade, which jams the player's visor after it explodes.
class CDarkCommandoGrenade : public CBouncyGrenade {
public:
  CDarkCommandoGrenade(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                       const CTransform4f& xf, const CModelData& modelData,
                       const CActorParameters& actorParams, TUniqueId parentId,
                       const SLdrDarkCommandoEMPData& data, float velocity);

  // CEntity
  ~CDarkCommandoGrenade() override;
  void Think(float dt, CStateManager& mgr) override;

private:
  SLdrDarkCommandoEMPData mData; // Guessed name
  float mEMPTime;                // Guessed name
  CRELFileToken mRelToken;       // Guessed name
};
CHECK_SIZEOF(CDarkCommandoGrenade, 0x3e8)

// Guessed class: the shadow decoy left behind when the Dark Commando cloaks.
class CShadowDecoy : public CActor {
public:
  CShadowDecoy(TUniqueId uid, const CEntityInfo& info, TUniqueId owner, const CTransform4f& xf,
               const CAABox& bounds, CAssetId effect, const CDamageVulnerability& vulnerability,
               SLdrAudioPlaybackParms sound, const CVector3f& orbitPosition, const CVector3f& scale,
               float health);

  // CEntity
  ~CShadowDecoy() override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;
  CVector3f GetOrbitPosition(const CStateManager& mgr) const override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;

private:
  rstl::single_ptr< CElementGen > mEffect; // Guessed name
  CSfxHandle mSfxHandle;                   // Guessed name
  float mHealth;                           // Guessed name
  CAABox mBounds;                          // Guessed name
  CDamageVulnerability mVulnerability;     // Guessed name
  TUniqueId mOwnerId;                      // Guessed name
  CVector3f mOrbitPosition;                // Guessed name
  CRELFileToken mRelToken;                 // Guessed name
};
CHECK_SIZEOF(CShadowDecoy, 0x1c8)

#endif // _CDARKCOMMANDO
