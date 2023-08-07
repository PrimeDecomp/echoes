#include "MetroidPrime/HUD/CHUDMemoParms.hpp"

CHUDMemoParms::CHUDMemoParms(float dispTime, bool clearMemoWindow, bool fadeOutOnly, bool hintMemo,
                             int playerMask, bool fadeInText)
: mDispTime(dispTime)
, mClearMemoWindow(clearMemoWindow)
, mFadeOutOnly(fadeOutOnly)
, mHintMemo(hintMemo)
, mFadeInText(fadeInText)
, playerMask(playerMask) {}

bool CHUDMemoParms::EnabledForPlayer(int playerIndex) const {
  return playerMask & 1 << playerIndex;
}
