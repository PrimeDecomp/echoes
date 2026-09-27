#ifndef _CBSTURN
#define _CBSTURN

#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/BodyState/CBodyState.hpp"

class CBSTurn : public CBodyState {
public:
  CBSTurn();

  // CBodyState
  ~CBSTurn() override {}
  bool CanShoot() const override { return true; }
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController& bc) override;

protected:
  virtual pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
  bool FacingDest(CBodyController& bc) const;

  float mRotateSpeed;
  CVector2f mDest;
  pas::ETurnDirection mTurnDir;
};
CHECK_SIZEOF(CBSTurn, 0x14)

class CBSFlyerTurn : public CBSTurn {
public:
  CBSFlyerTurn();

  // CBodyState
  ~CBSFlyerTurn() override;
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
};
CHECK_SIZEOF(CBSFlyerTurn, 0x14)

// Guessed name: the pitchable body type uses a three-dimensional facing direction.
class CBSPitchableFlyerTurn : public CBSTurn {
public:
  CBSPitchableFlyerTurn();

  // CBodyState
  ~CBSPitchableFlyerTurn() override;
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;

private:
  CVector3f mFaceDirection;
};
CHECK_SIZEOF(CBSPitchableFlyerTurn, 0x20)

#endif // _CBSTURN
