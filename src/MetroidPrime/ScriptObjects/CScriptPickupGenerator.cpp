#include "MetroidPrime/ScriptObjects/CScriptPickupGenerator.hpp"

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPickupGenerator.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPickup.hpp"
#include "MetroidPrime/TCastTo.hpp"

CPickupGeneratorRuleEvaluator::CPickupGeneratorRuleEvaluator(CAssetId rules)
: CRuleSetEvaluator(rules), mManager(nullptr), mLastDamageWeapon(CWeaponMode()), mLastDamageFlag(false) {
  for (int i = 0; i < 16; ++i) {
    mMinimumAmounts[i] = 0;
    mMaximumAmounts[i] = 0;
  }
}

void CPickupGeneratorRuleEvaluator::Refresh(CStateManager& mgr, const CHealthInfo* health) {
  for (int i = 0; i < 16; ++i) {
    mMinimumAmounts[i] = 0;
    mMaximumAmounts[i] = 0;
  }
  mManager = &mgr;
  mLastDamageWeapon = CWeaponMode();
  mLastDamageFlag = false;
  if (health != nullptr) {
    mLastDamageWeapon = health->GetCauseOfDeathWeapon();
    mLastDamageFlag = health->GetDamageFlag();
  }
  for (int i = 0; i < CPlayerState::kIT_ChargeCombo; ++i) {
    mPendingItemAmounts[i] = 0;
  }

  const CObjectList& actors = mgr.GetObjectListById(kOL_All);
  for (int index = actors.GetFirstObjectIndex(); index != -1;
       index = actors.GetNextObjectIndex(index)) {
    const CScriptPickup* pickup = TCastToConstPtr< CScriptPickup >(actors[index]);
    if (pickup != nullptr && pickup->GetActive() && pickup->GetCapacity() == 0) {
      mPendingItemAmounts[pickup->GetItem()] += pickup->GetAmount();
    }
  }

  EvaluateRules();
  mManager = nullptr;
}

CRuleValue CPickupGeneratorRuleEvaluator::GetConditionValue(FourCC condition) const {
  CPlayerState& state = *mManager->PlayerState(0);

  switch (condition) {
  case 'ALWS':
    return CRuleValue(true);
  case 'AHLT':
    return CRuleValue(int(GetEffectiveHealth(state)));
  case 'PHLT':
    return CRuleValue(100.f * GetEffectiveHealth(state) / state.CalculateHealth());
  case 'ADAM':
    return CRuleValue(GetEffectiveAmount(state, CPlayerState::kIT_DarkAmmo));
  case 'ALAM':
    return CRuleValue(GetEffectiveAmount(state, CPlayerState::kIT_LightAmmo));
  case 'AMSL':
    return CRuleValue(GetEffectiveAmount(state, CPlayerState::kIT_Missile));
  case 'APBM':
    return CRuleValue(GetEffectiveAmount(state, CPlayerState::kIT_Powerbomb));
  case '%DAM':
    {
      const float capacity = state.GetItemCapacity(CPlayerState::kIT_DarkAmmo);
      return CRuleValue(100.f * GetEffectiveAmount(state, CPlayerState::kIT_DarkAmmo) / capacity);
    }
  case '%LAM':
    {
      const float capacity = state.GetItemCapacity(CPlayerState::kIT_LightAmmo);
      return CRuleValue(100.f * GetEffectiveAmount(state, CPlayerState::kIT_LightAmmo) / capacity);
    }
  case '%MSL':
    {
      const float capacity = state.GetItemCapacity(CPlayerState::kIT_Missile);
      return CRuleValue(100.f * GetEffectiveAmount(state, CPlayerState::kIT_Missile) / capacity);
    }
  case '%PBM':
    {
      const float capacity = state.GetItemCapacity(CPlayerState::kIT_Powerbomb);
      return CRuleValue(100.f * GetEffectiveAmount(state, CPlayerState::kIT_Powerbomb) / capacity);
    }
  case 'CDAM':
    return CRuleValue(state.GetItemCapacity(CPlayerState::kIT_DarkAmmo));
  case 'CLAM':
    return CRuleValue(state.GetItemCapacity(CPlayerState::kIT_LightAmmo));
  case 'CMSL':
    return CRuleValue(state.GetItemCapacity(CPlayerState::kIT_Missile));
  case 'CPBM':
    return CRuleValue(state.GetItemCapacity(CPlayerState::kIT_Powerbomb));
  case '?HLT':
    return CRuleValue(state.CalculateHealth() > GetEffectiveHealth(state));
  case '?DBM':
    {
      bool result = false;
      if (state.GetItemCapacity(CPlayerState::kIT_DarkBeam) > 0) {
        const int capacity = state.GetItemCapacity(CPlayerState::kIT_DarkAmmo);
        if (GetEffectiveAmount(state, CPlayerState::kIT_DarkAmmo) < capacity) {
          result = true;
        }
      }
      return CRuleValue(result);
    }
  case '?LBM':
    {
      bool result = false;
      if (state.GetItemCapacity(CPlayerState::kIT_LightBeam) > 0) {
        const int capacity = state.GetItemCapacity(CPlayerState::kIT_LightAmmo);
        if (GetEffectiveAmount(state, CPlayerState::kIT_LightAmmo) < capacity) {
          result = true;
        }
      }
      return CRuleValue(result);
    }
  case '?MSL':
    {
      const int capacity = state.GetItemCapacity(CPlayerState::kIT_Missile);
      return CRuleValue(GetEffectiveAmount(state, CPlayerState::kIT_Missile) < capacity);
    }
  case '?PBM':
    {
      const int capacity = state.GetItemCapacity(CPlayerState::kIT_Powerbomb);
      return CRuleValue(GetEffectiveAmount(state, CPlayerState::kIT_Powerbomb) < capacity);
    }
  case 'HDBM':
    return CRuleValue(state.GetItemCapacity(CPlayerState::kIT_DarkBeam) > 0);
  case 'HLBM':
    return CRuleValue(state.GetItemCapacity(CPlayerState::kIT_LightBeam) > 0);
  case 'KWPB':
    return CRuleValue(mLastDamageWeapon.GetType() == kWT_Power);
  case 'KWDB':
    return CRuleValue(mLastDamageWeapon.GetType() == kWT_Dark);
  case 'KWLB':
    return CRuleValue(mLastDamageWeapon.GetType() == kWT_Light);
  case 'KWAB':
    return CRuleValue(mLastDamageWeapon.GetType() == kWT_Annihilator);
  case 'KWCB':
    return CRuleValue(mLastDamageWeapon.IsCharged());
  case 'KWKB':
    return CRuleValue(mLastDamageWeapon.IsComboed());
  case 'KWMS':
    return CRuleValue(mLastDamageWeapon.GetType() == kWT_Missile);
  case 'KLDB':
    return CRuleValue(mLastDamageWeapon.GetType() != kWT_Dark &&
                      mLastDamageWeapon.GetType() != kWT_Light &&
                      mLastDamageWeapon.GetType() != kWT_Annihilator);
  case 'KFCH':
    return CRuleValue(mLastDamageFlag);
  default:
    return CRuleValue(0);
  }
}

bool CPickupGeneratorRuleEvaluator::SetAmountRange(float chance, int ruleSlot, int minimum,
                                                   int maximum) {
  if (mManager->Random()->Range(0.f, 100.f) <= chance) {
    mMinimumAmounts[ruleSlot] = minimum;
    mMaximumAmounts[ruleSlot] = maximum;
    return false;
  }
  return true;
}

bool CPickupGeneratorRuleEvaluator::ExecuteAction(const CRuleAction& action) {
  const float chance = action.GetProperty(0).GetFloat();
  int minimum = 1;
  if (action.GetPropertyCount() > 1) {
    minimum = action.GetProperty(1).GetInt();
  }
  int maximum = 0;
  if (action.GetPropertyCount() > 2) {
    maximum = action.GetProperty(2).GetInt();
  }
  switch (action.GetId()) {
  case 'SLMS':
    return SetAmountRange(chance, 0, minimum, maximum);
  case 'BGMS':
    return SetAmountRange(chance, 1, minimum, maximum);
  case 'SLHL':
    return SetAmountRange(chance, 2, minimum, maximum);
  case 'MGHL':
    return SetAmountRange(chance, 4, minimum, maximum);
  case 'BGHL':
    return SetAmountRange(chance, 3, minimum, maximum);
  case 'GGHL':
    return SetAmountRange(chance, 5, minimum, maximum);
  case 'PBMB':
    return SetAmountRange(chance, 6, minimum, maximum);
  case 'LTAM':
    return SetAmountRange(chance, 7, minimum, maximum);
  case 'DKAM':
    return SetAmountRange(chance, 8, minimum, maximum);
  case 'LTAx':
    return SetAmountRange(chance, 9, minimum, maximum);
  case 'DKAx':
    return SetAmountRange(chance, 10, minimum, maximum);
  case 'LTAX':
    return SetAmountRange(chance, 11, minimum, maximum);
  case 'DKAX':
    return SetAmountRange(chance, 12, minimum, maximum);
  case 'LTBM':
    return SetAmountRange(chance, 13, minimum, maximum);
  case 'DKBM':
    return SetAmountRange(chance, 14, minimum, maximum);
  case 'ANBM':
    return SetAmountRange(chance, 15, minimum, maximum);
  default:
    return false;
  }
}

int CPickupGeneratorRuleEvaluator::GetRandomAmount(CStateManager& mgr, int ruleSlot) const {
  const int maximum = mMaximumAmounts[ruleSlot];
  const int minimum = mMinimumAmounts[ruleSlot];
  return maximum < minimum ? minimum : mgr.Random()->Range(minimum, maximum);
}

CScriptPickupGenerator::CScriptPickupGenerator(TUniqueId uid, const rstl::string& name,
                                               const CEntityInfo& info, const CVector3f& offset,
                                               CAssetId rules, bool offsetIsLocalSpace)
: CEntity(uid, info, name, 0)
, mOffset(offset)
, mRuleEvaluator(rules)
, mTemplatesCached(false)
, mOffsetIsLocalSpace(offsetIsLocalSpace) {}

CScriptPickupGenerator::~CScriptPickupGenerator() {}

void CScriptPickupGenerator::GetTargets(CStateManager& mgr, TUniqueId sender,
                                        rstl::vector< TUniqueId >& targets) const {
  targets.reserve(GetConnectionList().size() > 1 ? GetConnectionList().size() : 1);
  for (int i = 0; i < GetConnectionList().size(); ++i) {
    const SConnection& connection = GetConnectionList()[i];
    if (connection.state != kSS_Generate || connection.msg != kSM_Follow) {
      continue;
    }

    const TUniqueId id = mgr.GetIdForScript(connection.objId);
    const CEntity* entity = mgr.GetObjectById(id);
    if (id != kInvalidUniqueId && entity != nullptr && entity->GetActive()) {
      targets.push_back(id);
    }
  }

  if (targets.empty()) {
    targets.push_back(sender);
  }
}

CHealthInfo* CScriptPickupGenerator::GetTargetHealthInfo(CStateManager& mgr,
                                                         TUniqueId targetId) const {
  CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(targetId));
  return actor != nullptr ? actor->HealthInfo() : nullptr;
}

void CScriptPickupGenerator::GetSpawnablePickups(
    CStateManager& mgr, rstl::vector< rstl::pair< int, TEditorId > >& pickups, TUniqueId targetId) {
  mRuleEvaluator.Refresh(mgr, GetTargetHealthInfo(mgr, targetId));
  pickups.reserve(mPickupTemplates.size());

  for (int i = 0; i < mPickupTemplates.size(); ++i) {
    const SPickupTemplate& pickup = mPickupTemplates[i];
    int ruleSlot = -1;
    switch (pickup.mItem) {
    case CPlayerState::kIT_Missile:
      ruleSlot = pickup.mAmount < 6 ? 0 : 1;
      break;
    case CPlayerState::kIT_HealthRefill:
      ruleSlot = pickup.mAmount == 100  ? 5
                 : pickup.mAmount == 50 ? 4
                 : pickup.mAmount == 30 ? 3
                                        : 2;
      break;
    case CPlayerState::kIT_Powerbomb:
      ruleSlot = 6;
      break;
    case CPlayerState::kIT_LightAmmo:
      ruleSlot = pickup.mAmount < 10 ? 9 : pickup.mAmount == 10 ? 7 : 11;
      break;
    case CPlayerState::kIT_DarkAmmo:
      ruleSlot = pickup.mAmount < 10 ? 10 : pickup.mAmount == 10 ? 8 : 12;
      break;
    case CPlayerState::kIT_LightBeam:
      ruleSlot = 13;
      break;
    case CPlayerState::kIT_DarkBeam:
      ruleSlot = 14;
      break;
    case CPlayerState::kIT_AnnihilatorBeam:
      ruleSlot = 15;
      break;
    default:
      break;
    }

    if (ruleSlot >= 0) {
      const int amount = mRuleEvaluator.GetRandomAmount(mgr, ruleSlot);
      if (amount != 0) {
        pickups.push_back_unsafe(rstl::pair< int, TEditorId >(amount, pickup.mEditorId));
      }
    }
  }
}

void CScriptPickupGenerator::SpawnPickup(CStateManager& mgr, TEditorId templateId,
                                         TUniqueId targetId) const {
  if (mgr.GetObjectByIdFromListAll(targetId) == nullptr) {
    return;
  }

  // The template is generated through CStateManagerContainer using templateId.
  // TODO: Reconstruct CStateManagerContainer's generated-object API before creating the pickup.
}

void CScriptPickupGenerator::CachePickupTemplates(CStateManager& mgr) {
  if (mTemplatesCached) {
    return;
  }

  mTemplatesCached = true;
  int count = 0;
  for (int i = 0; i < GetConnectionList().size(); ++i) {
    const SConnection& connection = GetConnectionList()[i];
    // TODO: verify the FourCC for the state here
    if (connection.state == kSS_Generate && connection.msg == kSM_Activate) {
      ++count;
    }
  }
  mPickupTemplates.reserve(count);

  for (int i = 0; i < GetConnectionList().size(); ++i) {
    const SConnection& connection = GetConnectionList()[i];
    if (connection.state != kSS_Generate || connection.msg != kSM_Activate) {
      continue;
    }

    const TUniqueId id = mgr.GetIdForScript(connection.objId);
    CScriptPickup* pickup = TCastToPtr< CScriptPickup >(mgr.GetObjectByIdFromListAll(id));
    if (pickup != nullptr) {
      mPickupTemplates.push_back_unsafe(
          SPickupTemplate(pickup->GetItem(), pickup->GetAmount(), connection.objId));
    }
    // TODO: A non-pickup template can spawn a temporary actor with a Generate/Activate
    // connection to a pickup. The generator's object-creation API is not yet reconstructed.
  }

  mgr.fn_8003BE54();
}

void CScriptPickupGenerator::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  if (msg.GetMessage() == kSM_SetToZero && GetActive()) {
    TUniqueId sender = msg.GetOriginator();
    if (sender == kInvalidUniqueId) {
      sender = msg.GetUnk();
    }

    // TODO: Forward the zero message to linked generator objects before evaluating RULE.
    CachePickupTemplates(mgr);

    rstl::vector< TUniqueId > targets;
    GetTargets(mgr, sender, targets);
    rstl::vector< rstl::pair< int, TEditorId > > pickups;
    GetSpawnablePickups(mgr, pickups, sender);
    for (int i = 0; i < pickups.size(); ++i) {
      for (int count = 0; count < pickups[i].first; ++count) {
        const int index = int(mgr.Random()->Float() * targets.size() * 0.99f);
        SpawnPickup(mgr, pickups[i].second, targets[index]);
      }
    }
  }

  CEntity::AcceptScriptMsg(mgr, msg);
}

CEntity* LoadPickupGenerator(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrPickupGenerator sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrPickupGenerator.inc"

  if (sldrThis.rules == kInvalidAssetId) {
    return nullptr;
  }
  return rs_new CScriptPickupGenerator(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), sldrThis.offset, sldrThis.rules,
      sldrThis.offsetIsLocalSpace);
}
