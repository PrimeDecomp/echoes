#ifndef _CBSLOOPREACTION
#define _CBSLOOPREACTION

#include "MetroidPrime/BodyState/CBodyState.hpp"

class CBSLoopReaction : public CBodyState {
public:
  CBSLoopReaction();

  // CBodyState
  ~CBSLoopReaction() override;
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController& bc) override;

private:
  pas::ELoopState mState;
  pas::EReactionType mReactionType;
  bool mLoopHit : 1;
};
CHECK_SIZEOF(CBSLoopReaction, 0x10)

#endif // _CBSLOOPREACTION
