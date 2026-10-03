#ifndef _CRULESET
#define _CRULESET

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/TToken.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/vector.hpp"
#include "types.h"

class CFactoryFnReturn;
class CInputStream;
class CVParamTransfer;

class CRuleValue {
public:
  // Guessed names for the native value tags.
  enum EType { kT_Bool, kT_Float, kT_Int };

  CRuleValue(int type, CInputStream&);
  explicit CRuleValue(bool value) : m_type(0), mBool(value) {}
  explicit CRuleValue(float value) : m_type(1), mFloat(value) {}
  explicit CRuleValue(int value) : m_type(2), m_value(value) {}
  bool GetBool() const;
  int GetInt() const;
  float GetFloat() const;
  EType GetType() const { return static_cast< EType >(m_type); }

private:
  int m_type;
  union {
    int m_value;
    bool mBool;
    float mFloat;
  };
};
CHECK_SIZEOF(CRuleValue, 0x8)

class CRuleCondition {
public:
  // Guessed names for the native comparison operators.
  enum EComparison { kC_Less, kC_LessEqual, kC_Equal, kC_GreaterEqual, kC_Greater };

  explicit CRuleCondition(CInputStream&);
  FourCC GetId() const { return m_id; }
  EComparison GetComparison() const { return static_cast< EComparison >(m_operator); }
  const CRuleValue& GetValue() const { return m_value; }

private:
  FourCC m_id;
  int m_operator;
  CRuleValue m_value;
};
CHECK_SIZEOF(CRuleCondition, 0x10)

class CRuleAction {
public:
  explicit CRuleAction(CInputStream&);
  FourCC GetId() const { return m_id; }
  int GetPropertyCount() const { return m_properties.size(); }
  const CRuleValue& GetProperty(int index) const { return m_properties[index]; }

private:
  FourCC m_id;
  rstl::vector< CRuleValue > m_properties;
};
CHECK_SIZEOF(CRuleAction, 0x14)

class CRuleSetRule {
public:
  explicit CRuleSetRule(CInputStream&);
  const rstl::vector< CRuleCondition >& GetConditions() const { return m_conditions; }
  const rstl::vector< CRuleAction >& GetActions() const { return m_actions; }

private:
  rstl::vector< CRuleCondition > m_conditions;
  rstl::vector< CRuleAction > m_actions;
};
CHECK_SIZEOF(CRuleSetRule, 0x20)

class CRuleSet {
public:
  explicit CRuleSet(CInputStream&);
  const rstl::vector< CRuleSetRule >& GetRules() const { return m_rules; }
  const rstl::optional_object< TLockedToken< CRuleSet > >& GetParentRule() const {
    return m_parentRule;
  }

private:
  rstl::vector< CRuleSetRule > m_rules;
  rstl::optional_object< TLockedToken< CRuleSet > > m_parentRule;
};
CHECK_SIZEOF(CRuleSet, 0x20)

CFactoryFnReturn FRuleSetFactory(const SObjectTag& tag, CInputStream& in,
                                 const CVParamTransfer& xfer);

#endif // _CRULESET
