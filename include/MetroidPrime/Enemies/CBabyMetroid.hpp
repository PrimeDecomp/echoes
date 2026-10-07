#ifndef _CBABYMETROID
#define _CBABYMETROID

#include "Kyoto/TToken.hpp"
#include "MetroidPrime/Enemies/CMetroid.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/vector.hpp"

class CGenDescription;

// Original class name from the Wii SEL exports (TypesMatch__12CBabyMetroidCFi).
class CBabyMetroid : public CMetroid {
public:
  CBabyMetroid(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
               const CTransform4f& xf, const CModelData& mData, const CPatternedInfo& pInfo,
               const CActorParameters& aParms, const CMetroidData& metroidData);
  ~CBabyMetroid() override;

  // CEntity
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void Render(const CStateManager& mgr) const override;

  // CPatterned
  void SetupStateMachine(CStateManager& mgr) override;

  // CMetroid
  bool ShouldDodge(CStateManager& mgr, const CTriggerData& data) const override;
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Dodge(CStateManager& mgr, EStateMsg msg, float dt) override;

  // Triggers
  bool AnimOver(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldSeekEnergySource(CStateManager& mgr, const CTriggerData& data) const;
  bool AbsorbFinished(CStateManager& mgr, const CTriggerData& data) const;
  bool InEnergySourcePosition(CStateManager& mgr, const CTriggerData& data) const;

  // States
  void Generate(CStateManager& mgr, EStateMsg msg, float dt);
  void Attack(CStateManager& mgr, EStateMsg msg, float dt);
  void ExitHive(CStateManager& mgr, EStateMsg msg, float dt);
  void AbsorbEnergy(CStateManager& mgr, EStateMsg msg, float dt);
  void TransformIntoMetroid(CStateManager& mgr, EStateMsg msg, float dt);

  // Code functions
  void SetEnergySourceDest(CStateManager& mgr, float dt);

  void ApplyContactDamage(CStateManager& mgr, CActor& target,
                          const CDamageInfo& info);               // Guessed name.
  void TryJoinHive(CStateManager& mgr);                           // Guessed name.
  CVector3f FindAIHintPosition(CStateManager& mgr, int hintType); // Guessed name.

private:
  float xa48_;
  float xa4c_; // Absorb duration (guessed role).
  float xa50_; // Absorb timer (guessed role).
  rstl::vector< TUniqueId > xa54_;
  TUniqueId xa64_;
  TUniqueId xa66_;
  TUniqueId xa68_;
  float mInitialScale;
  float mBabyMetroidScale;
  float xa74_;
  float xa78_;
  float mChanceToDodge;
  float mDodgeCheckTimeInterval;
  float xa84_;
  rstl::optional_object< TLockedToken< CGenDescription > > mTransformationParticle;
  CDamageVulnerability mGrowthVulnerability;
  bool mShouldSeekEnergySource : 1;
  bool xac8_25_ : 1;
  bool mShouldDodge : 1;
};
CHECK_SIZEOF(CBabyMetroid, 0xAD0)

#endif // _CBABYMETROID
