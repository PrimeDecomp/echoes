#ifndef _CBODYSTATE
#define _CBODYSTATE

#include "Kyoto/Animation/CharacterCommon.hpp"
#include "types.h"

class CBodyController;
class CStateManager;

class CBodyState {
public:
  virtual ~CBodyState() = 0;

  virtual bool IsInAir(const CBodyController&) const { return false; }

  virtual bool IsDead() const { return false; }

  virtual bool IsDying() const { return false; }

  virtual bool IsMoving() const { return false; }

  virtual bool ApplyGravity() const { return true; }

  virtual bool ApplyHeadTracking() const { return true; }

  virtual bool ApplyAnimationDeltas() const { return true; }

  virtual bool CanShoot() const { return false; }

  virtual bool UnkVtable2C() const { return false; }

  virtual void Start(CBodyController& bc, CStateManager& mgr) = 0;
  virtual pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) = 0;
  virtual void Shutdown(CBodyController& bc) = 0;
};

inline CBodyState::~CBodyState() {}

CHECK_SIZEOF(CBodyState, 0x4)

#endif // _CBODYSTATE
