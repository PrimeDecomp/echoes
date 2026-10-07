#ifndef _CRULESETEVALUATOR
#define _CRULESETEVALUATOR

#include "MetroidPrime/CRuleSet.hpp"

// Guessed class name
class CRuleSetEvaluator {
public:
  explicit CRuleSetEvaluator(CAssetId rules);
  ~CRuleSetEvaluator() {}
  void EvaluateRules(); // Guessed name

  // Guessed names
  virtual CRuleValue GetConditionValue(FourCC condition) const = 0;
  virtual int ExecuteAction(const CRuleAction& action) = 0;

protected:
  TLockedToken< CRuleSet > mRules; // Guessed name

private:
  bool CheckCondition(const CRuleCondition& condition) const; // Guessed name
  void EvaluateRuleSet(const CRuleSet& rules);                // Guessed name
};
CHECK_SIZEOF(CRuleSetEvaluator, 0x10)

#endif
