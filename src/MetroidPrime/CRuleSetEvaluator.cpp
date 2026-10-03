#include "MetroidPrime/CRuleSetEvaluator.hpp"

#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/SObjectTag.hpp"

CRuleSetEvaluator::CRuleSetEvaluator(CAssetId rules)
: mRules(gpSimplePool->GetObj(SObjectTag('RULE', rules))) {}

void CRuleSetEvaluator::EvaluateRules() { EvaluateRuleSet(**mRules); }

void CRuleSetEvaluator::EvaluateRuleSet(const CRuleSet& rules) {
  const rstl::vector< CRuleSetRule >& ruleList = rules.GetRules();
  for (int i = 0; i < ruleList.size(); ++i) {
    const CRuleSetRule& rule = ruleList[i];
    const rstl::vector< CRuleCondition >& conditions = rule.GetConditions();
    int condition = 0;
    for (; condition < conditions.size(); ++condition) {
      if (!CheckCondition(conditions[condition])) {
        break;
      }
    }
    if (condition == conditions.size()) {
      bool continueEvaluation = true;
      const rstl::vector< CRuleAction >& actions = rule.GetActions();
      for (int action = 0; action < actions.size(); ++action) {
        if (!ExecuteAction(actions[action])) {
          continueEvaluation = false;
        }
      }
      if (!continueEvaluation) {
        return;
      }
    }
  }

  const rstl::optional_object< TLockedToken< CRuleSet > >& parent = rules.GetParentRule();
  if (parent.valid()) {
    EvaluateRuleSet(**parent.data());
  }
}

bool CRuleSetEvaluator::CheckCondition(const CRuleCondition& condition) const {
  const CRuleValue value = GetConditionValue(condition.GetId());
  if (value.GetType() == CRuleValue::kT_Bool) {
    return value.GetBool();
  }

  switch (value.GetType()) {
  case CRuleValue::kT_Float:
    switch (condition.GetComparison()) {
    case CRuleCondition::kC_Less:
      return value.GetFloat() < condition.GetValue().GetFloat();
    case CRuleCondition::kC_LessEqual:
      return value.GetFloat() <= condition.GetValue().GetFloat();
    case CRuleCondition::kC_Equal:
      return value.GetFloat() == condition.GetValue().GetFloat();
    case CRuleCondition::kC_GreaterEqual:
      return value.GetFloat() >= condition.GetValue().GetFloat();
    case CRuleCondition::kC_Greater:
      return value.GetFloat() > condition.GetValue().GetFloat();
    }
    break;
  case CRuleValue::kT_Int:
    switch (condition.GetComparison()) {
    case CRuleCondition::kC_Less:
      return value.GetInt() < condition.GetValue().GetInt();
    case CRuleCondition::kC_LessEqual:
      return value.GetInt() <= condition.GetValue().GetInt();
    case CRuleCondition::kC_Equal:
      return value.GetInt() == condition.GetValue().GetInt();
    case CRuleCondition::kC_GreaterEqual:
      return value.GetInt() >= condition.GetValue().GetInt();
    case CRuleCondition::kC_Greater:
      return value.GetInt() > condition.GetValue().GetInt();
    }
    break;
  }
  return false;
}
