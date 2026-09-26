#ifndef _CRULESETEVALUATOR
#define _CRULESETEVALUATOR

#include "MetroidPrime/CRuleSet.hpp"

// Guessed name. The RULE evaluator supplies conditions and consumes matching actions.
class CRuleSetEvaluator {
public:
  explicit CRuleSetEvaluator(CAssetId rules);
  ~CRuleSetEvaluator();
  void EvaluateRules(); // Guessed name; evaluates the locked RULE and its parents.

  // Guessed names; these two slots precede the knockback-specific virtual interface.
  virtual CRuleValue GetConditionValue(FourCC condition) const = 0;
  virtual bool ExecuteAction(const CRuleAction& action) = 0;

protected:
  TLockedToken< CRuleSet > mRules;
};
CHECK_SIZEOF(CRuleSetEvaluator, 0x10)

#endif
