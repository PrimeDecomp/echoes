#ifndef _ICOLLISIONFILTER
#define _ICOLLISIONFILTER

#include "types.h"

class CCollisionInfoList;

// Guessed name and qualifiers, correlated with Prime and native filter dispatch.
class ICollisionFilter {
public:
  virtual void Filter(const CCollisionInfoList& in, CCollisionInfoList& out) const = 0;
};
CHECK_SIZEOF(ICollisionFilter, 0x4)

#endif // _ICOLLISIONFILTER
