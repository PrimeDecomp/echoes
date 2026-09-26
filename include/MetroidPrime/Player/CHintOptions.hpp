#ifndef _CHINTOPTIONS
#define _CHINTOPTIONS

#include "types.h"

#include "rstl/vector.hpp"

enum EHintState { kHS_Zero, kHS_Waiting, kHS_Displaying, kHS_Delayed };

class CHintOptions {
public:
  struct SHintState {
    EHintState mState;
    float mTime;
    float mDismissalTimer; // Guessed name

    bool CanContinue() const;
  };

  CHintOptions();

  const SHintState* GetCurrentDisplayedHint() const;
  int GetNextHintIdx();
  const rstl::vector< SHintState >& GetHintStates() const { return mHintStates; }

private:
  rstl::vector< SHintState > mHintStates;
  int mNextHintIdx;
  bool mInRezbitState;     // Guessed name
  bool mScanDisplayActive; // Guessed name
};
NESTED_CHECK_SIZEOF(CHintOptions, SHintState, 0xc)
CHECK_SIZEOF(CHintOptions, 0x18)

#endif // _CHINTOPTIONS
