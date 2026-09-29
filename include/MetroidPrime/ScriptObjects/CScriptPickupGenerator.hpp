#ifndef _CSCRIPTPICKUPGENERATOR
#define _CSCRIPTPICKUPGENERATOR

#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CRuleSetEvaluator.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/Weapons/WeaponTypes.hpp"

class CHealthInfo;
class CScriptPickup;

// Guessed name. This Echoes-only RULE evaluator selects pickup counts.
class CPickupGeneratorRuleEvaluator : public CRuleSetEvaluator {
public:
  explicit CPickupGeneratorRuleEvaluator(CAssetId rules);

  // CRuleSetEvaluator
  CRuleValue GetConditionValue(FourCC condition) const override;
  bool ExecuteAction(const CRuleAction& action) override;

  void Refresh(CStateManager& mgr, const CHealthInfo* health);
  int GetRandomAmount(CStateManager& mgr, int ruleSlot) const;

private:
  int GetEffectiveAmount(const CPlayerState& state, CPlayerState::EItemType item) const {
    return state.GetItemAmount(item) + mPendingItemAmounts[item];
  }

  float GetEffectiveHealth(const CPlayerState& state) const {
    return state.GetHealthInfo().GetHP() + mPendingItemAmounts[CPlayerState::kIT_HealthRefill];
  }

  bool SetAmountRange(float chance, int ruleSlot, int minimum, int maximum);

  CStateManager* mManager;
  CWeaponMode mLastDamageWeapon;
  int mPendingItemAmounts[CPlayerState::kIT_Max];
  int mMinimumAmounts[16];
  int mMaximumAmounts[16];
  bool mLastDamageFlag : 1; // Meaning of the source health-info flag is unresolved.
};
CHECK_SIZEOF(CPickupGeneratorRuleEvaluator, 0x250)

class CScriptPickupGenerator : public CEntity {
public:
  CScriptPickupGenerator(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                         const CVector3f& offset, CAssetId rules, bool offsetIsLocalSpace);

  // CEntity
  ~CScriptPickupGenerator() override;
  CEntity* TypesMatch(int typeId) const override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

private:
  // Guessed name. Each connection supplies a pickup type, amount and template ID.
  struct SPickupTemplate {
    CPlayerState::EItemType mItem;
    int mAmount;
    TEditorId mEditorId;

    SPickupTemplate(CPlayerState::EItemType item, int amount, TEditorId editorId)
    : mItem(item), mAmount(amount), mEditorId(editorId) {}
  };

  void CachePickupTemplates(CStateManager& mgr);
  void SpawnPickup(CStateManager& mgr, TEditorId templateId, TUniqueId targetId) const;
  void GetSpawnablePickups(CStateManager& mgr,
                           rstl::vector< rstl::pair< int, TEditorId > >& pickups,
                           TUniqueId targetId);
  CHealthInfo* GetTargetHealthInfo(CStateManager& mgr, TUniqueId targetId) const;
  void GetTargets(CStateManager& mgr, TUniqueId sender, rstl::vector< TUniqueId >& targets) const;

  CVector3f mOffset;
  rstl::vector< SPickupTemplate > mPickupTemplates;
  CPickupGeneratorRuleEvaluator mRuleEvaluator;
  bool mTemplatesCached : 1;
  bool mOffsetIsLocalSpace : 1;
};
CHECK_SIZEOF(CScriptPickupGenerator, 0x294)

#endif // _CSCRIPTPICKUPGENERATOR
