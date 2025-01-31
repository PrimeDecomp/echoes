#include "MetroidPrime/CRuleSet.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

CRuleSet::CRuleSet(CInputStream& input) {
  input.ReadInt32(); // magic
  input.ReadInt8();  // version
  CAssetId parentRule = static_cast< CAssetId >(input.ReadInt32());
  if (parentRule != kInvalidAssetId) {
    m_parentRule = gpSimplePool->GetObj(SObjectTag('RULE', parentRule));
  }
  int ruleCount = input.ReadInt8();
  m_rules.reserve(ruleCount);
  for (int i = 0; i < ruleCount; ++i) {
    m_rules.push_back_unsafe(CRuleSetRule(input));
  }
}

CRuleSetRule::CRuleSetRule(CInputStream& input) {
  int conditionCount = input.ReadInt8();
  m_conditions.reserve(conditionCount);
  for (int i = 0; i < conditionCount; ++i) {
    m_conditions.push_back(CRuleCondition(input));
  }

  int actionCount = input.ReadInt8();
  m_actions.reserve(actionCount);
  for (int i = 0; i < actionCount; ++i) {
    m_actions.push_back_unsafe(CRuleAction(input));
  }
}

CRuleCondition::CRuleCondition(CInputStream& input)
: m_id(input.ReadInt32()), m_operator(input.ReadInt8()), m_value(input.ReadInt8(), input) {}

CRuleAction::CRuleAction(CInputStream& input) : m_id(input.ReadInt32()) {
  int propCount = input.ReadInt8();
  m_properties.reserve(propCount);
  for (int i = 0; i < propCount; ++i) {
    m_properties.push_back_unsafe(CRuleValue(3, input));
  }
}

CRuleValue::CRuleValue(int type, CInputStream& input) : m_type(type), m_value(input.ReadInt32()) {}

bool CRuleValue::GetBool() const { return *reinterpret_cast< const bool* >(&m_value); }

int CRuleValue::GetInt() const { return m_value; }

float CRuleValue::GetFloat() const { return *reinterpret_cast< const float* >(&m_value); }
