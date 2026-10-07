#include "MetroidPrime/ScriptObjects/CScriptPickupGenerator.hpp"

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CScriptObjectLoaderHelper.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Enemies/CSwarmBasics.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPickupGenerator.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDebris.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPickup.hpp"
#include "MetroidPrime/TCastTo.hpp"

CPickupGeneratorRuleEvaluator::CPickupGeneratorRuleEvaluator(CAssetId rules)
: CRuleSetEvaluator(rules)
, mManager(nullptr)
, mLastDamageWeapon(CWeaponMode())
, mLastDamageFlag(false) {
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
  case '%DAM': {
    const float capacity = state.GetItemCapacity(CPlayerState::kIT_DarkAmmo);
    return CRuleValue(100.f * GetEffectiveAmount(state, CPlayerState::kIT_DarkAmmo) / capacity);
  }
  case '%LAM': {
    const float capacity = state.GetItemCapacity(CPlayerState::kIT_LightAmmo);
    return CRuleValue(100.f * GetEffectiveAmount(state, CPlayerState::kIT_LightAmmo) / capacity);
  }
  case '%MSL': {
    const float capacity = state.GetItemCapacity(CPlayerState::kIT_Missile);
    return CRuleValue(100.f * GetEffectiveAmount(state, CPlayerState::kIT_Missile) / capacity);
  }
  case '%PBM': {
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
  case '?DBM': {
    bool result = false;
    if (state.GetItemCapacity(CPlayerState::kIT_DarkBeam) > 0) {
      const int capacity = state.GetItemCapacity(CPlayerState::kIT_DarkAmmo);
      if (GetEffectiveAmount(state, CPlayerState::kIT_DarkAmmo) < capacity) {
        result = true;
      }
    }
    return CRuleValue(result);
  }
  case '?LBM': {
    bool result = false;
    if (state.GetItemCapacity(CPlayerState::kIT_LightBeam) > 0) {
      const int capacity = state.GetItemCapacity(CPlayerState::kIT_LightAmmo);
      if (GetEffectiveAmount(state, CPlayerState::kIT_LightAmmo) < capacity) {
        result = true;
      }
    }
    return CRuleValue(result);
  }
  case '?MSL': {
    const int capacity = state.GetItemCapacity(CPlayerState::kIT_Missile);
    return CRuleValue(GetEffectiveAmount(state, CPlayerState::kIT_Missile) < capacity);
  }
  case '?PBM': {
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
  for (rstl::vector< SConnection >::const_iterator it = GetConnectionList().begin();
       it != GetConnectionList().end(); ++it) {
    if (it->state != kSS_GeneratorConnection || it->msg != kSM_Follow) {
      continue;
    }

    const TUniqueId id = mgr.GetIdForScript(it->objId);
    if (id == kInvalidUniqueId) {
      continue;
    }
    const CEntity* entity = mgr.GetObjectById(id);
    if (entity != nullptr && entity->GetActive()) {
      targets.push_back_unsafe(id);
    }
  }

  if (targets.empty()) {
    targets.push_back_unsafe(sender);
  }
}

CHealthInfo* CScriptPickupGenerator::GetTargetHealthInfo(CStateManager& mgr,
                                                         TUniqueId targetId) const {
  CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(targetId));
  return actor != nullptr ? actor->HealthInfo() : nullptr;
}

static inline void AddSpawnablePickup(const CPickupGeneratorRuleEvaluator& evaluator,
                                      CStateManager& mgr, int ruleSlot, const TEditorId& editorId,
                                      rstl::vector< rstl::pair< int, TEditorId > >& pickups) {
  const int amount = evaluator.GetRandomAmount(mgr, ruleSlot);
  if (amount != 0) {
    pickups.push_back_unsafe(rstl::pair< int, TEditorId >(amount, editorId));
  }
}

void CScriptPickupGenerator::GetSpawnablePickups(
    CStateManager& mgr, rstl::vector< rstl::pair< int, TEditorId > >& pickups, TUniqueId targetId) {
  mRuleEvaluator.Refresh(mgr, GetTargetHealthInfo(mgr, targetId));
  pickups.reserve(mPickupTemplates.size());

  for (rstl::vector< SPickupTemplate >::const_iterator it = mPickupTemplates.begin();
       it != mPickupTemplates.end(); ++it) {
    const SPickupTemplate& pickup = *it;
    switch (pickup.mItem) {
    case CPlayerState::kIT_HealthRefill:
      if (pickup.mAmount == 100) {
        AddSpawnablePickup(mRuleEvaluator, mgr, 5, pickup.mEditorId, pickups);
      } else if (pickup.mAmount == 50) {
        AddSpawnablePickup(mRuleEvaluator, mgr, 4, pickup.mEditorId, pickups);
      } else if (pickup.mAmount == 30) {
        AddSpawnablePickup(mRuleEvaluator, mgr, 3, pickup.mEditorId, pickups);
      } else {
        AddSpawnablePickup(mRuleEvaluator, mgr, 2, pickup.mEditorId, pickups);
      }
      break;
    case CPlayerState::kIT_DarkAmmo:
      if (pickup.mAmount > 10) {
        AddSpawnablePickup(mRuleEvaluator, mgr, 12, pickup.mEditorId, pickups);
      } else if (pickup.mAmount < 10) {
        AddSpawnablePickup(mRuleEvaluator, mgr, 10, pickup.mEditorId, pickups);
      } else {
        AddSpawnablePickup(mRuleEvaluator, mgr, 8, pickup.mEditorId, pickups);
      }
      break;
    case CPlayerState::kIT_LightAmmo:
      if (pickup.mAmount > 10) {
        AddSpawnablePickup(mRuleEvaluator, mgr, 11, pickup.mEditorId, pickups);
      } else if (pickup.mAmount < 10) {
        AddSpawnablePickup(mRuleEvaluator, mgr, 9, pickup.mEditorId, pickups);
      } else {
        AddSpawnablePickup(mRuleEvaluator, mgr, 7, pickup.mEditorId, pickups);
      }
      break;
    case CPlayerState::kIT_LightBeam:
      AddSpawnablePickup(mRuleEvaluator, mgr, 13, pickup.mEditorId, pickups);
      break;
    case CPlayerState::kIT_DarkBeam:
      AddSpawnablePickup(mRuleEvaluator, mgr, 14, pickup.mEditorId, pickups);
      break;
    case CPlayerState::kIT_AnnihilatorBeam:
      AddSpawnablePickup(mRuleEvaluator, mgr, 15, pickup.mEditorId, pickups);
      break;
    case CPlayerState::kIT_Missile:
      if (pickup.mAmount < 6) {
        AddSpawnablePickup(mRuleEvaluator, mgr, 0, pickup.mEditorId, pickups);
      } else {
        AddSpawnablePickup(mRuleEvaluator, mgr, 1, pickup.mEditorId, pickups);
      }
      break;
    case CPlayerState::kIT_Powerbomb:
      AddSpawnablePickup(mRuleEvaluator, mgr, 6, pickup.mEditorId, pickups);
      break;
    default:
      break;
    }
  }
}

void CScriptPickupGenerator::SpawnPickup(CStateManager& mgr, TEditorId templateId,
                                         TUniqueId targetId) const {
  CEntity* target = mgr.ObjectById(targetId);
  if (target == nullptr) {
    return;
  }

  const CScriptObjectLoaderHelper::SGeneratedObject generated =
      mgr.ScriptObjectLoaderHelper().GenerateScriptObject(templateId, mgr);
  if (generated.mUniqueId == kInvalidUniqueId) {
    return;
  }

  CEntity* entity = generated.mEntity;
  CActor* actor = TCastToPtr< CActor >(entity);
  CScriptPickup* pickup = TCastToPtr< CScriptPickup >(entity);
  const CActor* targetActor = TCastToConstPtr< CActor >(target);
  const CSwarmBasics* targetSwarm = TCastToConstPtr< CSwarmBasics >(target);
  if (actor != nullptr && targetSwarm != nullptr) {
    actor->SetTranslation(targetSwarm->GetLastKilledOffset() + mOffset);
  } else if (actor != nullptr && targetActor != nullptr) {
    const CVector3f offset =
        mOffsetIsLocalSpace ? targetActor->GetTransform().Rotate(mOffset) : mOffset;
    actor->SetTranslation(targetActor->GetTranslation() + offset);
  }
  if (pickup != nullptr) {
    pickup->SetWasGenerated(mgr);
  }
  mgr.SendScriptMsg(entity, GetUniqueId(), kSM_Activate, kInvalidUniqueId);
}

void CScriptPickupGenerator::CachePickupTemplates(CStateManager& mgr) {
  if (mTemplatesCached) {
    return;
  }

  mTemplatesCached = true;
  int count = 0;
  for (rstl::vector< SConnection >::const_iterator it = GetConnectionList().begin();
       it != GetConnectionList().end(); ++it) {
    if (it->state == kSS_GeneratorConnection && it->msg == kSM_Activate) {
      ++count;
    }
  }
  mPickupTemplates.reserve(count);

  CScriptObjectLoaderHelper& loader = mgr.ScriptObjectLoaderHelper();
  for (rstl::vector< SConnection >::const_iterator it = GetConnectionList().begin();
       it != GetConnectionList().end(); ++it) {
    if (it->state != kSS_GeneratorConnection || it->msg != kSM_Activate) {
      continue;
    }

    const CScriptObjectLoaderHelper::SGeneratedObject generated =
        loader.GenerateScriptObject(it->objId, mgr);
    const CScriptPickup* pickup = TCastToConstPtr< CScriptPickup >(generated.mEntity);
    if (pickup != nullptr) {
      mPickupTemplates.push_back_unsafe(
          SPickupTemplate(pickup->GetItem(), pickup->GetAmount(), it->objId));
    } else if (const CScriptDebris* debris = TCastToConstPtr< CScriptDebris >(generated.mEntity)) {
      for (rstl::vector< SConnection >::const_iterator inner = debris->GetConnectionList().begin();
           inner != debris->GetConnectionList().end(); ++inner) {
        if (inner->state != kSS_GeneratorConnection || inner->msg != kSM_Activate) {
          continue;
        }

        const CScriptObjectLoaderHelper::SGeneratedObject innerGenerated =
            loader.GenerateScriptObject(inner->objId, mgr);
        const CScriptPickup* innerPickup = TCastToConstPtr< CScriptPickup >(innerGenerated.mEntity);
        if (innerPickup != nullptr) {
          mPickupTemplates.push_back_unsafe(
              SPickupTemplate(innerPickup->GetItem(), innerPickup->GetAmount(), it->objId));
        }
        mgr.DeleteObjectRequest(innerGenerated.mUniqueId);
      }
    }
    mgr.DeleteObjectRequest(generated.mUniqueId);
  }

  mgr.DispatchScriptMessages();
}

void CScriptPickupGenerator::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  if (msg.GetMessage() == kSM_SetToZero && GetActive()) {
    TUniqueId sender = msg.GetOriginator();
    if (sender == kInvalidUniqueId) {
      sender = msg.GetSenderId();
    }

    for (rstl::vector< SConnection >::const_iterator it = GetConnectionList().begin();
         it != GetConnectionList().end(); ++it) {
      if (it->state != kSS_GeneratorConnection || it->msg != kSM_SetToZero) {
        continue;
      }

      const CStateManager::TIdListResult ids = mgr.GetIdListForScript(it->objId);
      if (ids.first == ids.second) {
        continue;
      }
      const CScriptPickupGenerator* generator =
          TCastToConstPtr< CScriptPickupGenerator >(mgr.ObjectById(ids.first->second));
      if (generator != nullptr) {
        mgr.DeliverScriptMsg(CScriptMsg(GetUniqueId(), sender, generator->GetUniqueId(),
                                        kSM_SetToZero, kSS_GeneratorConnection));
      }
    }

    CachePickupTemplates(mgr);

    rstl::vector< rstl::pair< int, TEditorId > > pickups;
    GetSpawnablePickups(mgr, pickups, sender);
    if (pickups.size() == 0) {
      return;
    }

    rstl::vector< TUniqueId > targets;
    GetTargets(mgr, sender, targets);
    for (rstl::vector< rstl::pair< int, TEditorId > >::const_iterator it = pickups.begin();
         it != pickups.end(); ++it) {
      for (int i = 0; i < it->first; ++i) {
        const int index = int(mgr.Random()->Float() * targets.size() * 0.99f);
        SpawnPickup(mgr, it->second, targets[index]);
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
  return rs_new CScriptPickupGenerator(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                                       LdrToEntityInfo(info, sldrThis.editorProperties),
                                       sldrThis.offset, sldrThis.rules,
                                       sldrThis.offsetIsLocalSpace);
}
