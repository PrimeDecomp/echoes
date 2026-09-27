#ifndef _CHINTOPTIONS
#define _CHINTOPTIONS

#include "types.h"

#include "rstl/string.hpp"
#include "rstl/vector.hpp"

enum EHintState { kHS_Zero, kHS_Waiting, kHS_Displaying, kHS_Delayed };

class CStateManager;
class CBitStreamReader;
class CBitStreamWriter;

class CHintOptions {
public:
  struct SHintState {
    SHintState();
    SHintState(EHintState state, float time);

    EHintState mState;
    float mTime;
    float mDismissalTimer; // Guessed name

    bool CanContinue() const;
    bool IsDismissed() const { return mDismissalTimer > 0.f; } // Guessed name
  };

  CHintOptions();
  explicit CHintOptions(CBitStreamReader& in);
  void PutTo(CBitStreamWriter& out) const;
  void InitializeMemoryState();

  void EnsureHintNextTime(); // Prime PAL name; Echoes only has this max() variant
  void Update(float dt, CStateManager& mgr);

  void DelayHint(const rstl::string& name);
  void ActivateImmediateHintTimer(const rstl::string& name);
  void ActivateContinueDelayHintTimer(const rstl::string& name);
  void DismissDisplayedHint();

  const SHintState* GetCurrentDisplayedHint() const;
  int GetNextHintIdx();
  const rstl::vector< SHintState >& GetHintStates() const { return mHintStates; }

private:
  static uint GetBitCount(uint value);

  rstl::vector< SHintState > mHintStates;
  int mNextHintIdx;
  bool mInRezbitState;     // Guessed name
  bool mScanDisplayActive; // Guessed name
};
NESTED_CHECK_SIZEOF(CHintOptions, SHintState, 0xc)
CHECK_SIZEOF(CHintOptions, 0x18)

#endif // _CHINTOPTIONS
