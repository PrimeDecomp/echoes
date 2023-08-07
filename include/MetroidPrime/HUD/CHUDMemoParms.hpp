#ifndef _CHUDMEMOPARMS
#define _CHUDMEMOPARMS

#include "types.h"

class CInputStream;

class CHUDMemoParms {
public:
  CHUDMemoParms(float dispTime, bool clearMemoWindow, bool fadeOutOnly, bool hintMemo, int playerMask, bool fadeInText);

  float GetDisplayTime() const { return mDispTime; }
  bool IsClearMemoWindow() const { return mClearMemoWindow; }
  bool IsFadeOutOnly() const { return mFadeOutOnly; }
  bool IsHintMemo() const { return mHintMemo; }
  bool EnabledForPlayer(int playerIndex) const;

private:
  float mDispTime;
  bool mClearMemoWindow;
  bool mFadeOutOnly;
  bool mHintMemo;
  bool mFadeInText;
  int playerMask;
};

#endif // _CHUDMEMOPARMS
