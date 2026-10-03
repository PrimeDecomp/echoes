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
  explicit CRuleValue(bool value) : mType(0), mBool(value) {}
  explicit CRuleValue(float value) : mType(1), mFloat(value) {}
  explicit CRuleValue(int value) : mType(2), mValue(value) {}
  bool GetBool() const;
  int GetInt() const;
  float GetFloat() const;
  EType GetType() const { return static_cast< EType >(mType); }

private:
  int mType;
  union {
    int mValue;
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
  FourCC GetId() const { return mId; }
  EComparison GetComparison() const { return static_cast< EComparison >(mOperator); }
  const CRuleValue& GetValue() const { return mValue; }

private:
  FourCC mId;
  int mOperator;
  CRuleValue mValue;
};
CHECK_SIZEOF(CRuleCondition, 0x10)

class CRuleAction {
public:
  explicit CRuleAction(CInputStream&);
  FourCC GetId() const { return mId; }
  int GetPropertyCount() const { return mProperties.size(); }
  const CRuleValue& GetProperty(int index) const { return mProperties[index]; }

private:
  FourCC mId;
  rstl::vector< CRuleValue > mProperties;
};
CHECK_SIZEOF(CRuleAction, 0x14)

class CRuleSetRule {
public:
  explicit CRuleSetRule(CInputStream&);
  const rstl::vector< CRuleCondition >& GetConditions() const { return mConditions; }
  const rstl::vector< CRuleAction >& GetActions() const { return mActions; }

private:
  rstl::vector< CRuleCondition > mConditions;
  rstl::vector< CRuleAction > mActions;
};
CHECK_SIZEOF(CRuleSetRule, 0x20)

class CRuleSet {
public:
  explicit CRuleSet(CInputStream&);
  const rstl::vector< CRuleSetRule >& GetRules() const { return mRules; }
  const rstl::optional_object< TLockedToken< CRuleSet > >& GetParentRule() const {
    return mParentRule;
  }

private:
  rstl::vector< CRuleSetRule > mRules;
  rstl::optional_object< TLockedToken< CRuleSet > > mParentRule;
};
CHECK_SIZEOF(CRuleSet, 0x20)

CFactoryFnReturn FRuleSetFactory(const SObjectTag& tag, CInputStream& in,
                                 const CVParamTransfer& xfer);

#endif // _CRULESET
