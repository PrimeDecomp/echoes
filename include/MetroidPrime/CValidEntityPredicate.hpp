#ifndef _CVALIDENTITYPREDICATE
#define _CVALIDENTITYPREDICATE

#include "MetroidPrime/TGameTypes.hpp"

class CEntity;
class CStateManager;

class CValidEntityPredicate {
public:
  virtual ~CValidEntityPredicate();
  virtual bool IsValid(const CStateManager&, TUniqueId) const;
};

#endif // _CVALIDENTITYPREDICATE
