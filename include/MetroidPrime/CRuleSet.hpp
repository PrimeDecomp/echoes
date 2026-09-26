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
  CRuleValue(int type, CInputStream&);
  explicit CRuleValue(bool value) : m_type(0), mBool(value) {}
  bool GetBool() const;
  int GetInt() const;
  float GetFloat() const;

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
  explicit CRuleCondition(CInputStream&);

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
  const CRuleValue& GetProperty(int index) const { return m_properties[index]; }

private:
  FourCC m_id;
  rstl::vector< CRuleValue > m_properties;
};
CHECK_SIZEOF(CRuleAction, 0x14)

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
  rstl::optional_object< TLockedToken< CRuleSet > > m_parentRule;
};
CHECK_SIZEOF(CRuleSet, 0x20)

CFactoryFnReturn FRuleSetFactory(const SObjectTag& tag, CInputStream& in,
                                 const CVParamTransfer& xfer);

#endif // _CRULESET
