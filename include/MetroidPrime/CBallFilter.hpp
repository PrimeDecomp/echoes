#ifndef _CBALLFILTER
#define _CBALLFILTER

#include "MetroidPrime/ICollisionFilter.hpp"

class CActor;

// Guessed class name; the actor reference is borrowed, not owned.
class CBallFilter : public ICollisionFilter {
public:
  explicit CBallFilter(const CActor& actor) : mActor(actor) {}

  // ICollisionFilter
  void Filter(const CCollisionInfoList& in, CCollisionInfoList& out) const override;

private:
  const CActor& mActor;
};
CHECK_SIZEOF(CBallFilter, 0x8)

#endif // _CBALLFILTER
