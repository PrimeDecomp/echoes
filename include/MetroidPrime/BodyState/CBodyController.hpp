#ifndef _CBODYCONTROLLER
#define _CBODYCONTROLLER

// Pointer-only interface; storage and construction remain to be reconstructed.
class CBodyController {
public:
  ~CBodyController();
  float GetPercentageFrozen() const;
  void SetOnFire(float duration);
  void SetElectrocuting(float duration);
  void SetPlaybackRate(float rate);
  void Freeze(float intoFreezeDuration, float frozenDuration, float breakoutDuration);

private:
  CBodyController();
  CBodyController(const CBodyController&);
  CBodyController& operator=(const CBodyController&);
};

#endif
