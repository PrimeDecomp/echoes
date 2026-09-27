#ifndef _CPLAYERTARGETING
#define _CPLAYERTARGETING

#include "MetroidPrime/TGameTypes.hpp"

class CStateManager;

// Partial interface; class and method names are guessed from targeting/scan consumers.
class CPlayerTargeting {
public:
  int GetScanTargetIndex(const CStateManager& mgr, TUniqueId id) const;
};

#endif // _CPLAYERTARGETING
