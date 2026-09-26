#ifndef _CBSSLIDE
#define _CBSSLIDE

#include "MetroidPrime/BodyState/CBodyState.hpp"

class CBSSlide : public CBodyState {
public:
  CBSSlide();

  // CBodyState
  ~CBSSlide() override;
  bool IsMoving() const override;
  bool ApplyHeadTracking() const override;
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController& bc) override;

private:
  float mRotateSpeed;
};
CHECK_SIZEOF(CBSSlide, 0x8)

#endif // _CBSSLIDE
