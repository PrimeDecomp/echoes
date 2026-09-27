#ifndef _CECHOEMITTER
#define _CECHOEMITTER

#include "MetroidPrime/SEchoParameters.hpp"

#include "Kyoto/Math/CAABox.hpp"

class CStateManager;

class CEchoEmitter {
public:
  CEchoEmitter(const CAABox& bounds, const SEchoParameters& parameters);
  virtual ~CEchoEmitter();
  // Guessed names; retain the original virtual order.
  virtual void DestroyEmitter(CStateManager& mgr);
  virtual void Think(float dt, CStateManager& mgr);
  virtual void Render(const CStateManager& mgr) const;

  void CreateEmitter(CStateManager& mgr);
  void SetBounds(const CAABox& bounds) { mBounds = bounds; }
  void SetParameters(const SEchoParameters& parameters) { mParameters = parameters; }
  bool IsPendingDeletion() const { return mPendingDeletion; }

private:
  SEchoParameters mParameters;
  CAABox mBounds;
  int mPlayerEchoTokens[4];
  float mPlayerEchoVisibility[4];
  float mDamage;
  float mYellowDamage;
  float mDamageReductionRate;
  float mYellowDamageReductionRate;
  bool mActive : 1;
  bool mPendingDeletion : 1;
  CEchoEmitter* mNextEmitter;
};
CHECK_SIZEOF(CEchoEmitter, 0x64)

#endif // _CECHOEMITTER
