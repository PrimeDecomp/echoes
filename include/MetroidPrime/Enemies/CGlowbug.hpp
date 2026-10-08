#ifndef _CGLOWBUG
#define _CGLOWBUG

#include "types.h"

#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CParticleElectric.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"

// Guessed class: a glowing bug that patrols, telegraphs and then attacks with an electric beam.
class CGlowbug : public CPatterned {
public:
  CGlowbug(TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
           const CModelData& modelData, const CPatternedInfo& patternedInfo,
           const CActorParameters& actorParams, CAssetId deathFlashEffect,
           CAssetId deathBreakApartEffect, CAssetId attackEffect, CAssetId attackEchoEffect,
           const CVector3f& attackAimOffset, ushort attackTelegraphSound, ushort attackSound,
           CAssetId attackTelegraphEffect, CAssetId scanModel, bool isInLightWorld,
           float attackDuration, float attackTelegraphDuration);

  // CEntity
  ~CGlowbug() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;

  // CAi
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;

  // CPatterned
  void SetupStateMachine(CStateManager& mgr) override;
  bool IsScanVisorSelfRender() const override;
  CAABox GetScanVisorRenderBounds(const CStateManager& mgr) const override;
  void ScanVisorRender(const CStateManager& mgr, const CTransform4f& xf,
                       const CModelFlags& flags) const override;

  // CGlowbug
  void Dead(CStateManager& mgr, EStateMsg msg, float dt);
  void Attack(CStateManager& mgr, EStateMsg msg, float dt);
  void AttackTelegraph(CStateManager& mgr, EStateMsg msg, float dt);
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt);
  bool AttackTelegraphDelay(CStateManager& mgr, const CTriggerData& data) const;
  bool AttackFinished(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldAttack(CStateManager& mgr, const CTriggerData& data) const;
  bool AnimOver(CStateManager& mgr, const CTriggerData& data) const;

private:
  void FindAttackTarget(CStateManager& mgr);              // Guessed name
  bool IsInRange(const CActor& other, float range) const; // Guessed name

  CAssetId mDeathFlashEffect;                          // Guessed name
  CAssetId mDeathBreakApartEffect;                     // Guessed name
  rstl::auto_ptr< CParticleElectric > mAttackElectric; // Guessed name
  rstl::auto_ptr< CElementGen > mAttackEchoGen;        // Guessed name
  uchar mEffectIndex;                                  // Guessed name
  bool x7d9_;
  float x7dc_;
  CDamageInfo mAttackDamage; // Guessed name
  float mAttackDuration;     // Guessed name
  int x800_;
  TUniqueId x804_;
  float mMinAttackRange; // Guessed name
  float mMaxAttackRange; // Guessed name
  bool x810_;
  CVector3f mAttackAimOffset;      // Guessed name
  float mAttackTelegraphDuration;  // Guessed name
  ushort mAttackSound;             // Guessed name
  ushort mAttackTelegraphSound;    // Guessed name
  CAssetId mAttackTelegraphEffect; // Guessed name
  TUniqueId x82c_;
  bool mIsInDarkWorld; // Guessed name
  TUniqueId x830_;
  CModelData mScanModel; // Guessed name
  bool x880_;
};
CHECK_SIZEOF(CGlowbug, 0x888)

#endif // _CGLOWBUG
