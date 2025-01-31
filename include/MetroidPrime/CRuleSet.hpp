#ifndef _CRULESET
#define _CRULESET

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/TToken.hpp"
#include "rstl/vector.hpp"
#include "types.h"

class CInputStream;

class CRuleValue {
public:
  CRuleValue(int type, CInputStream&);
  bool GetBool() const;
  int GetInt() const;
  float GetFloat() const;

private:
  int m_type;
  int m_value;
};

class CRuleCondition {
public:
  explicit CRuleCondition(CInputStream&);

private:
  FourCC m_id;
  int m_operator;
  CRuleValue m_value;
};

class CRuleAction {
public:
  explicit CRuleAction(CInputStream&);

private:
  FourCC m_id;
  rstl::vector< CRuleValue > m_properties;
};

class CRuleSetRule {
public:
  explicit CRuleSetRule(CInputStream&);

private:
  rstl::vector< CRuleCondition > m_conditions;
  rstl::vector< CRuleAction > m_actions;
};
CHECK_SIZEOF(CRuleSetRule, 0x20)

class CRuleSet {
public:
  explicit CRuleSet(CInputStream&);

private:
  rstl::vector< CRuleSetRule > m_rules;
  TCachedToken< CRuleSet > m_parentRule;
  char m_pad[0x4];
};
CHECK_SIZEOF(CRuleSet, 0x20)

#endif // _CRULESET
