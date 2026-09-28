#ifndef _CCAMERABLURPASS
#define _CCAMERABLURPASS

#include "types.h"
#include <dolphin/gx.h>

class CCameraBlurPass {
public:
  enum EBlurType { kBT_NoBlur, kBT_LoBlur, kBT_HiBlur };

  CCameraBlurPass();
  void Update(float dt);
  void SetBlur(EBlurType type, float amount, float duration, bool usePersistentFb);
  void DisableBlur(float duration);
  void Draw() const;
  static void GetFbCopy(GXTexFmt format);
  void AllocatePersistentFbTexture();
  void FreePersistentFbTexture();

private:
  EBlurType mCurrentType;
  EBlurType mNextType;
  float mPreviousValue;
  float mCurrentValue;
  float mNextValue;
  float mDuration;
  float mRemainingTime;
  bool mUsePersistent;
  mutable bool mNoPersistentCopy;
};
CHECK_SIZEOF(CCameraBlurPass, 0x20)

#endif // _CCAMERABLURPASS
