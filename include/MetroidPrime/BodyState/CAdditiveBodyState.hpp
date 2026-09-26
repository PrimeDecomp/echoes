#ifndef _CADDITIVEBODYSTATE
#define _CADDITIVEBODYSTATE

#include "MetroidPrime/BodyState/CBodyState.hpp"

class CAdditiveBodyState : public CBodyState {
public:
  // CBodyState
  ~CAdditiveBodyState() override {}

  bool ApplyHeadTracking() const override { return true; }

  bool CanShoot() const override { return true; }

  bool UnkVtable2C() const override { return true; }
};

CHECK_SIZEOF(CAdditiveBodyState, 0x4)

#endif // _CADDITIVEBODYSTATE
