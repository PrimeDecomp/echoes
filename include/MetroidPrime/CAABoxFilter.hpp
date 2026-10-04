#ifndef _CAABOXFILTER
#define _CAABOXFILTER

#include "MetroidPrime/ICollisionFilter.hpp"

class CActor;

// Guessed class and helper names; the actor reference is borrowed, not owned.
class CAABoxFilter : public ICollisionFilter {
public:
  explicit CAABoxFilter(const CActor& actor) : mActor(actor) {}
  static void FilterBoxFloorCollisions(const CCollisionInfoList& in, CCollisionInfoList& out);

  // ICollisionFilter
  void Filter(const CCollisionInfoList& in, CCollisionInfoList& out) const override;

private:
  const CActor& mActor;
};
CHECK_SIZEOF(CAABoxFilter, 0x8)

#endif // _CAABOXFILTER
